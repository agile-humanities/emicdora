<?php

/**
 * @file
 * Handles the management of MVD.
 */

/**
 * The form for creating a MVD object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_mvd_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'collation', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  $filter_options = array(
    'Empty' => 'Empty',
    'CCE' => 'CCE',
    'Novel' => 'Novel',
    'Play' => 'Play',
    'Poem' => 'Poem',
  );

  $form = array();

  $form['mvd'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of new Collation'),
    '#size' => 40,
    '#required' => TRUE,
  );
  $form['mvd_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description of new Collation'),
  );

  $members = emicdora_get_members($object->id);
  $versionable_objects = array_keys($members, 'islandora:versionableObjectCModel');
  $transcriptions = array();
  foreach ($versionable_objects as $versionable_object) {
    $vo_members = emicdora_get_members($versionable_object);
    $transcriptions = array_merge($transcriptions, array_keys($vo_members, 'islandora:transcriptionCModel'));
  }
  $rows = array();

  foreach ($transcriptions as $transcription) {
    $transcription_object = islandora_object_load($transcription);
    $rows[$transcription] = array(
      'pid' => l($transcription, "islandora/object/$transcription"),
      'title' => l($transcription_object->label, "islandora/object/$transcription"),
    );
  }
  $header = array(
    'title' => array('data' => t('Transcription Title')),
    'pid' => array('data' => t('PID')),
  );
  $table_element = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t("There are no transcriptions associated with this Critical Edition."),
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['filter'] = array(
    '#type' => 'select',
    '#title' => t('Filter'),
    '#options' => $filter_options,
  );

  $form['intro'] = array(
    '#type' => 'markup',
    '#markup' => t("Choose transcriptions of @label to compare.", array('@label' => $object->label)),
  );
  $form['transcriptions'] = $table_element;
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Create Collation'),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  return $form;
}

/**
 * Drupal Form Validation.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal Form State
 */
function emicdora_mvd_form_validate($form, $form_state) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $critical_edition_object = islandora_object_load($form_state['values']['pid']);
  $mvds = emicdora_calliope_find_mvds($critical_edition_object);
  $match = FALSE;
  foreach ($mvds as $mvd) {
    if ($mvd['name'] == $form_state['values']['mvd']) {
      form_set_error('mvd', t('A collation with this name is already associated with this critical edition.'));
    }
  }
}

/**
 * Sumit handler for MVD form.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal Form State
 */
function emicdora_mvd_form_submit($form, $form_state) {
  module_load_include('inc', 'collation', 'includes/calliope');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $transcriptions = array_filter($form_state['values']['transcriptions']);
  $critical_edition_pid = $form_state['values']['pid'];
  $mvd_title = $form_state['values']['mvd'];
  $mvd_description = $form_state['values']['mvd_description'];
  $filter = $form_state['values']['filter'];
  $success = emicdora_calliope_create_mvd($critical_edition_pid, $mvd_title, $mvd_description, $transcriptions, $filter);
  drupal_set_message(check_plain($success));
}

/**
 * Handles all crud operations for MVDs.
 *
 * @param string $critical_edition_pid
 *   PID of Critical Edition
 * @param string $mvd_label
 *   Identifier for MVD
 * @param string $operation
 *   action to perform
 */
function emicdora_mvd_crud($critical_edition_pid, $mvd_label, $operation) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $query = array(
    'metadata.critical_edition' => $critical_edition_pid,
    'metadata.name' => $mvd_label,
  );
  $mvds = emicdora_calliope_find_mvd($query);
  $mvd = $mvds[0];
  $mvd_description = isset($mvd['metadata']['description']) ? $mvd['metadata']['description'] : t("No description");

  switch ($operation) {
    case 'refresh':
      foreach ($mvd['metadata']['resources'] as $transcription) {
        $resource_ids[] = $transcription['id'];
      }
      $filter = $mvd['metadata']['filter'];
      emicdora($critical_edition_pid, $mvd_label, $mvd_description, $resource_ids, $filter);
      drupal_goto("islandora/object/$critical_edition_pid/edit");

      break;

    case 'delete':
      emicdora($mvd['_id']);
      drupal_set_message(t("Collation successfully removed"));
      drupal_goto("islandora/object/$critical_edition_pid");
      break;

    case 'compare':
      drupal_goto("collationtools/compare", array('fragment' => "$mvd_label/$critical_edition_pid"));
      break;

    case 'apparatus':
      drupal_goto("collationtools/apparatus", array('fragment' => "$mvd_label/$critical_edition_pid"));
      break;

    case 'list':
      $cursor = emicdora_calliope_compare_callback($critical_edition_pid);
      $results = array();
      foreach ($cursor as $obj) {
        $results['results'][] = $obj['metadata'];
      }
      echo json_encode($results);
      break;

    default:
      break;
  }
}
