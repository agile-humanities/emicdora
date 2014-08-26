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
 *
 * @return array
 *   An array of links to be rendered.
 */
function emicdora_source_object_operations_block_links($source_pid, $critical_edition_pid) {
  $links = array();
  emicdora_append_edit_and_delete_links($source_pid, $critical_edition_pid, $links);
  return $links;
}

function emicdora_versionable_object_operations_block_links($pid) {
  $block['subject'] = t('Operations');
  $block['content'] = array(
    array(
      '#theme' => 'links',
      '#links' => $links,
    ),
  );
}

function emicdora_transcription_object_operations_block_links() {

}

function emicdora_collation_object_operations_block() {

}

/**
 * Helper function to add the Edit and Delete links.
 *
 * @param string $pid
 *   The PID of the object to link to.
 * @param string $critical_edition_pid
 *   The PID of the Critical Edition the object is related to.
 * @param array $links
 *   A reference to the links array to append to.
 */
function emicdora_append_edit_and_delete_links($pid, $critical_edition_pid, &$links) {
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
    $links[] = array(
      'title' => 'Delete',
      'href' => "islandora/object/$pid/delete",
      'query' => array(
        'destination' => "islandora/object/$critical_edition_pid",
      ),
    );
  }
}