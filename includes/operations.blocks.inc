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

function emicdora_transcription_object_operations_block_links($transcription_pid, $critical_edition_pid, $version_pid) {
  $links = array();
  $destination_params = array(
    'path' => 'version',
    'pid' => $version_pid,
  );
  emicdora_append_edit_and_delete_links($links, $transcription_pid, $critical_edition_pid, $destination_params);
  return $links;
}

function emicdora_collation_object_operations_block() {

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
  if (islandora_object_access(ISLANDORA_METADATA_EDIT, $object)) {
    $destination = drupal_get_destination();
    $links[] = array(
      'title' => 'Edit Metatdata',
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
      'title' => 'Delete',
      'href' => "islandora/object/$pid/delete",
      'query' => array(
        'destination' => $destination,
      ),
    );
  }
}