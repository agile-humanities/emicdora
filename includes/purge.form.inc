<?php

/**
 * @file
 * This file handles the deletion/purging of all objects.
 */

/**
 * Find all sources / versions of this edition and remove the association.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_islandora_delete_object_form_submit_delete_edition(array $form, array &$form_state) {
  $object = $form_state['object'];
  emicdora_remove_members_batch_set($object);
  emicdora_delete_mvds_batch_set($object);
  emicdora_delete_objects_batch_set(array($object->id));
}

/**
 * Creates and starts a batch for removing all the members of the given object.
 *
 * @param AbstractObject $parent
 *   The parent object from which all members will be removed.
 */
function emicdora_remove_members_batch_set(AbstractObject $parent) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $members = array_keys(emicdora_get_members($parent->id));
  $batch = array(
    'operations' => array(
      array('emicdora_remove_members_batch_operation', array(
          $parent->id,
          $members,
        ),
      ),
    ),
    'title' => t('Deleting objects(s) ...'),
    'init_message' => t('Preparing to remove object(s)'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/purge.form.inc',
  );
  batch_set($batch);
}

/**
 * Batch operation for removing an objects members.
 *
 * @param string $parent
 *   The parent object the members belong to.
 * @param array $members
 *   All the objects to remove.
 * @param array $context
 *   The context of this batch operation.
 */
function emicdora_remove_members_batch_operation($parent, array $members, array &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox']['current'] = 0;
    $context['sandbox']['max'] = count($members);
  }
  $current = &$context['sandbox']['current'];
  $chunk = array_slice($members, $current, 5);
  foreach ($chunk as $pid) {
    $current++;
    try {
      $member_object = islandora_object_load($pid);
      $member_object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $parent);
    }
    catch (Exception $exception) {
      watchdog_exception('emicdora', $exception);
    }
  }
  if ($current != $context['sandbox']['max']) {
    $context['finished'] = $current / $context['sandbox']['max'];
  }
}

/**
 * Creates and starts a batch for deleting the given objects.
 *
 * @param AbstractObject $edition
 *   The edition to delete the MVD's from.
 */
function emicdora_delete_mvds_batch_set(AbstractObject $edition) {
  module_load_include('inc', 'collation', 'includes/calliope');
  // The keys are the MVD's labels and they are used to uniquely identify them.
  try {
    $mvds = array_keys(emicdora_calliope_find_mvds_no_exit($edition));
  }
  catch (Exception $exception) {
    watchdog_exception('emicdora', $exception);
    return;
  }
  $batch = array(
    'operations' => array(
      array('emicdora_delete_mvds_batch_operation', array($edition->id, $mvds)),
    ),
    'title' => t('Deleting MVD(s) ...'),
    'init_message' => t('Preparing to delete MVD(s)'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/purge.form.inc',
  );
  batch_set($batch);
}

/**
 * Batch operation for deleting a MVD's and their collations.
 *
 * @param string $edition
 *   The PID of the edition in which we are removing the MVD's from.
 * @param array $mvds
 *   Labels of all MVD's to be deleted.
 * @param array $context
 *   The context of this batch operation.
 */
function emicdora_delete_mvds_batch_operation($edition, array $mvds, array &$context) {
  module_load_include('inc', 'collation', 'includes/mvd');
  module_load_include('inc', 'collation', 'includes/calliope');
  if (empty($context['sandbox'])) {
    $context['sandbox']['current'] = 0;
    $context['sandbox']['max'] = count($mvds);
  }
  $current = &$context['sandbox']['current'];
  $chunk = array_slice($mvds, $current, 5);
  foreach ($chunk as $mvd) {
    try {
      $current++;
      $query = array(
        'metadata.critical_edition' => $edition,
        'metadata.name' => (string) $mvd,
      );
      $results = emicdora_calliope_find_mvd_no_exit($query);
      if (!empty($results)) {
        emicdora_calliope_delete_mvd_no_exit($results[0]['_id']);
        emicdora_calliope_delete_collation_no_exit((string) $mvd);
      }
    }
    catch (Exception $exception) {
      watchdog_exception('emicdora', $exception);
    }
  }
  if ($current != $context['sandbox']['max']) {
    $context['finished'] = $current / $context['sandbox']['max'];
  }
}

/**
 * Find all related versions of this source object and delete them too.
 *
 * A version can have only one source object so we won't be removing anything in
 * which should be shared with another source.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_islandora_delete_object_form_submit_delete_source(array $form, array &$form_state) {
  $source = $form_state['object'];
  $versions = emicdora_get_source_versions($source);
  foreach ($versions as $version) {
    emicdora_delete_version_batch_set($version);
  }
  // We don't delete the source as that's handle by a different submit handler.
  // Little hack to get around the form redirect set by the normal source submit
  // handler.
  $_GET['destination'] = 'co-op';
  drupal_static_reset('drupal_get_destination');
  drupal_get_destination();
}

/**
 * Find all the Versions derived from the given source.
 *
 * @param AbstractObject $source
 *   The source object in which to fetch the versions from.
 *
 * @return array
 *   An array of PIDs identifying all the versionable objects derived from
 *   the given source object.
 */
function emicdora_get_source_versions(AbstractObject $source) {
  // Find all the versions and add batches to delete each.
  $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?pid
FROM <#ri>
WHERE {
  ?pid <fedora-rels-ext:isDerivationOf> <info:fedora/{$source->id}> ;
       <fedora-model:hasModel> <info:fedora/islandora:versionCModel> ;
}
EOQ;
  $results = $source->repository->ri->sparqlQuery($query);
  $get_pid = function($o) {
    return $o['pid']['value'];
  };
  return array_values(array_map($get_pid, $results));
}

/**
 * Find all objects related to the given version and delete them too.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_islandora_delete_object_form_submit_delete_version(array $form, array &$form_state) {
  $object = $form_state['object'];
  emicdora_delete_version_batch_set($object);
  // If no destination is set, send the user back to the workbench.
  $form_state['redirect'] = 'workbench';
}

/**
 * Starts a batch to delete the given version, and all its children.
 *
 * A version can be created from an existing versions TEI-RDF objects, but
 * this is done though a copy so each version has it's own pages /
 * transcriptions and they can't be shared, so when deleting a version we
 * delete everything related to it.
 *
 * @param string $pid
 *   The version to delete
 */
function emicdora_delete_version_batch_set($pid) {
  $tuque = islandora_get_tuque_connection();
  $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?pid ?model
FROM <#ri>
WHERE {
  ?pid <fedora-rels-ext:isMemberOf> <info:fedora/$pid> ;
       <fedora-model:hasModel> ?model .
}
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($query);
  $get_pid = function($o) {
    return $o['pid']['value'];
  };
  $objects = array_values(array_map($get_pid, $results));
  $objects[] = $pid;
  emicdora_delete_objects_batch_set($objects);
  // Delete MongoDB info associated with the transcriptions.
  $is_transcription = function($o) {
    return $o['model']['value'] === 'islandora:transcriptionCModel';
  };
  $transcriptions = array_filter($results, $is_transcription);
  $transcriptions = array_values(array_map($get_pid, $transcriptions));
  emicdora_delete_calliope_files_batch_set($transcriptions);
}

/**
 * Creates and starts a batch for deleting the given objects.
 *
 * @param array $objects
 *   All the objects to delete.
 */
function emicdora_delete_objects_batch_set(array $objects) {
  $batch = array(
    'operations' => array(
      array('emicdora_delete_objects_batch_operation', array($objects)),
    ),
    'title' => t('Deleting objects(s) ...'),
    'init_message' => t('Preparing to delete object(s)'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/purge.form.inc',
  );
  batch_set($batch);
}

/**
 * Batch operation for deleting an object.
 *
 * @param array $objects
 *   All the objects to delete.
 * @param array $context
 *   The context of this batch operation.
 */
function emicdora_delete_objects_batch_operation(array $objects, array &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox']['current'] = 0;
    $context['sandbox']['max'] = count($objects);
  }
  $current = &$context['sandbox']['current'];
  $chunk = array_slice($objects, $current, 5);
  foreach ($chunk as $pid) {
    $current++;
    try {
      $object = islandora_object_load($pid);
      if ($object) {
        islandora_delete_object($object);
      }
    }
    catch (Exception $exception) {
      watchdog_exception('emicdora', $exception);
    }
  }
  if ($current != $context['sandbox']['max']) {
    $context['finished'] = $current / $context['sandbox']['max'];
  }
}

/**
 * Creates and starts a batch for deleting the given objects from calliope.
 *
 * @param array $objects
 *   All the objects to delete.
 */
function emicdora_delete_calliope_files_batch_set(array $objects) {
  $batch = array(
    'operations' => array(
      array('emicdora_delete_calliope_file_batch_operation', array($objects)),
    ),
    'title' => t('Deleting files(s) ...'),
    'init_message' => t('Preparing to delete files(s)'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/purge.form.inc',
  );
  batch_set($batch);
}


/**
 * Batch operation for deleting a objects from Mongo.
 *
 * @param array $objects
 *   All the objects to delete from Mongo.
 * @param array $context
 *   The context of this batch operation.
 */
function emicdora_delete_calliope_file_batch_operation(array $objects, array &$context) {
  module_load_include('inc', 'collation', 'includes/calliope');
  if (empty($context['sandbox'])) {
    $context['sandbox']['current'] = 0;
    $context['sandbox']['max'] = count($objects);
  }
  $current = &$context['sandbox']['current'];
  $chunk = array_slice($objects, $current, 5);
  foreach ($chunk as $pid) {
    $current++;
    try {
      emicdora_calliope_delete_file_no_exit($pid);
    }
    catch (Exception $exception) {
      watchdog_exception('emicdora', $exception);
    }
  }
  if ($current != $context['sandbox']['max']) {
    $context['finished'] = $current / $context['sandbox']['max'];
  }
}

/**
 * Delete the transcription and it's MongoDB data.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_islandora_delete_object_form_submit_delete_transcription(array &$form, array &$form_state) {
  module_load_include('inc', 'collation', 'includes/calliope');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $object = $form_state['object'];
  try {
    $parents = islandora_get_parents_from_rels_ext($object);
    $parent = array_pop($parents);
    emicdora_calliope_delete_file_no_exit($object->id);
    islandora_delete_object($object);
  }
  catch (Exception $exception) {
    watchdog_exception('emicdora', $exception);
  }
  drupal_set_message(t("Transcription has been deleted."));
  $form_state['redirect'] = isset($parent) ? "islandora/object/{$parent->id}" : 'workbench';
}
