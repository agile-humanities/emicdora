<?php

/**
 * @file
 * DB utilities for emicdora.
 */

/**
 * Get terms associated with critical edition.
 *
 * @param string $critical_edition
 *   PID of critical edition
 * @param string $type
 *   Term type - either source or version
 *
 * @return array
 *   Terms associated with this Critical Edition
 */
function emicdora_get_terms($critical_edition, $type) {
  $list = db_select('emicdora_facets', 'c')
      ->fields('c')
      ->condition('critical_edition', $critical_edition)
      ->condition('type', $type)
      ->execute();
  $to_return = array();
  while ($data = $list->fetchObject()) {
    $to_return[] = unserialize($data->terms);
  }
  return $to_return;
}

/**
 * Inserts or updated terms associated with critical edition.
 *
 * @param string $critical_edition
 *   PID if Critical Edition
 * @param string $type
 *   Term type - either source or version
 * @param array $terms
 *   Terms to be associated with this critical edition
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

/**
 * Deletes all terms associated with a Critical Edition.
 *
 * @param string $critical_edition
 *   PID if Critical Edition
 */
function emicdora_delete_terms($critical_edition) {
  db_delete('emicdora_facets')
      ->condition('critical_edition', $critical_edition)
      ->execute();
}

/**
 * Get terms associated with critical edition.
 *
 * @param string $critical_edition
 *   PID of critical edition
 * @param string $type
 *   Term type - either source or version
 *
 * @return array
 *   Terms associated with this Critical Edition
 */
function emicdora_get_configuration($critical_edition, $type) {
  $list = db_select('emicdora_facets', 'c')
      ->fields('c')
      ->condition('critical_edition', $critical_edition)
      ->condition('type', $type)
      ->execute();
  $to_return = array();
  while ($data = $list->fetchObject()) {
    $values = unserialize($data->terms);
    $solr_field = $values['solr_field_settings']['label'];
    $solr_value = $values['solr_field_settings']['value'];
    $to_return[$values['solr_field']] = "$solr_field:'$solr_value'";
  }

  return $to_return;
}
