<?php

/**
 * @file
 * DB utilities for emicdora.
 */

/**
 * Get terms associated with critical edition
 *
 * @param string $critical_edition
 *   PID of critical edition
 * @param string $type
 *   Term type - either source or version
 *
 * @return type
 */
function emicdora_get_terms($critical_edition, $type) {
  $list = db_select('emicdora_facets', 'c')
      ->fields('c')
      ->condition('critical_edition', $critical_edition)
      ->condition('type', $type)
      ->execute();
  while ($data = $list->fetchObject()) {
    $to_return[] = unserialize($data->terms);
  }
  return $to_return;
}

/**
 * Inserts or updated terms associated with critical edition.
 *
 * @param type $critical_edition
 * @param type $type
 * @param type $terms
 */
function emicdora_insert_terms($critical_edition, $type, $terms = array()) {

  db_insert('emicdora_facets')
      ->fields(array(
        'critical_edition' => $critical_edition,
        'type' => $type,
        'terms' => serialize($terms),
      ))
      ->execute();
}

function emicdora_delete_terms($critical_edition) {
  db_delete('emicdora_facets')
      ->condition('critical_edition', $critical_edition)
      ->execute();
}
