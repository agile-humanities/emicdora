<?php
/**
 * @file
 * Contains all operations block functionality for EMiC.
 */

/**
 * Adds the links for operations for source objects.
 *
 * @param string $source_pid
 *   The PID of the source object.
 * @param string $critical_edition_pid
 *   The PID of the critical edition the source object is linked to.
 *
 * @return array
 *   An array of links to be rendered.
 */
function emicdora_source_object_operations_block_links($source_pid, $critical_edition_pid) {
  $links = array();
  emicdora_append_edit_and_delete_links($links, $source_pid, $critical_edition_pid);
  return $links;
}

/**
 * Creates the links for versionable objects operations block.
 *
 * @param string $version_pid
 *   The PID of the versionable object.
 * @param string $critical_edition_pid
 *   The PID of the critical edition object.
 * @param string $source_pid
 *   The PID of the source object.
 *
 * @return array
 *   An array of links to be used in the operations block.
 */
function emicdora_versionable_object_operations_block_links($version_pid, $critical_edition_pid, $source_pid) {
  $links = array();
  $links[] = array(
    'title' => 'View Source Object',
    'href' => "islandora/object/$critical_edition_pid/source/$source_pid",
  );
  $destination_params = array(
    'path' => 'source',
    'pid' => $source_pid,
  );
  emicdora_append_edit_and_delete_links($links, $version_pid, $critical_edition_pid, $destination_params);
  return $links;
}

/**
 * Creates the links for transcription objects operations block.
 *
 * @param string $transcription_pid
 *   The PID of the transcription object.
 * @param string $critical_edition_pid
 *   The PID of the critical edition object.
 * @param string $version_pid
 *   The PID of the versionable object.
 *
 * @return array
 *   An array of links to be used in the operations block.
 */
function emicdora_transcription_object_operations_block_links($transcription_pid, $critical_edition_pid, $version_pid) {
  $links = array();
  $object = islandora_object_load($transcription_pid);
  if (islandora_object_access(ISLANDORA_METADATA_EDIT, $object)) {
    $destination = drupal_get_destination();
    $links[] = array(
      'title' => 'Edit',
      'href' => "islandora/object/$critical_edition_pid/transcription/$transcription_pid/edit",
      'query' => array(
        'destination' => $destination['destination'],
      ),
    );
  }
  if (islandora_object_access(ISLANDORA_PURGE, $object)) {
    $links[] = array(
      'title' => 'Delete',
      'href' => "islandora/object/$transcription_pid/delete",
      'query' => array(
        'destination' => "islandora/object/$critical_edition_pid/version/$version_pid",
      ),
    );
  }
  return $links;
}

/**
 * Creates the links for collation objects operations block.
 *
 * @param string $critical_edition_pid
 *   The PID of the critical edition object.
 * @param string $mvd
 *   The label of the MVD that we are performing operations on.
 * @param string $current_view
 *   The current view that the opeartions is being rendered.
 *
 * @return array
 *   An array of links to be used in the operations block.
 */
function emicdora_collation_object_operations_block_links($critical_edition_pid, $mvd, $current_view) {
  $links[] = array(
    'title' => 'Refresh Transcriptions',
    'href' => "islandora/object/$critical_edition_pid/refresh/$mvd/$current_view",
  );
  $links[] = array(
    'title' => 'Delete',
    'href' => "islandora/object/$critical_edition_pid/delete/$mvd/$current_view",
  );
  return $links;
}

/**
 * Helper function to add the Edit and Delete links.
 *
 * @param array $links
 *   A reference to the links array to append to.
 * @param string $pid
 *   The PID of the object to link to.
 * @param string $critical_edition_pid
 *   The PID of the Critical Edition the object is related to.
 * @param bool|array $destination_params
 *   FALSE if no nesting is taking place, otherwise an array containing:
 *   -path: A string containing the path to go under the critical edition.
 *   -pid: A string of the PID of the object to link to.
 */
function emicdora_append_edit_and_delete_links(&$links, $pid, $critical_edition_pid, $destination_params = FALSE) {
  $object = islandora_object_load($pid);
  $type = 'source';
  if (in_array('islandora:versionCModel', $object->models)) {
    $type = 'version';
  }
  if (islandora_object_access(ISLANDORA_METADATA_EDIT, $object)) {
    $destination = drupal_get_destination();
    $links[] = array(
      'title' => 'Edit Metadata',
      'href' => "islandora/edit_form/$pid/MODS",
      'query' => array(
        'destination' => $destination['destination'],
      ),
    );
  }
  if (islandora_object_access(ISLANDORA_PURGE, $object)) {
    $destination = "islandora/object/$critical_edition_pid";
    if ($destination_params) {
      $destination = "$destination/{$destination_params['path']}/{$destination_params['pid']}";
    }
    $links[] = array(
      'title' => 'Remove',
      'href' => "emicdora/remove/$type/$critical_edition_pid/$pid",
      'query' => array(
        'destination' => $destination,
      ),
    );
  }
}
