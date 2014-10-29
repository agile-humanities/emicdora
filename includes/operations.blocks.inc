<?php
/**
 * @file
 * Contains all operations block functionality for EMiC.
 */

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
