<?php

/**
 * @file
 * Functions to display Add Existing Sources page..
 */

/**
 * Generate Add Sources page.
 *
 * @param AbstactObject $object
 *   Object representing Critical Edition
 * @param string $filter_term
 *   Term for search
 *
 * @return HTML
 *   Themed page
 */
function emicdora_add_existing($object, $filter_term = NULL, $field = NULL) {
  $form = drupal_get_form('emicdora_sources_update_form', $object->id);
  $output = drupal_render($form);
  $output .= theme('emicdora_add_source_page', array(
    'islandora_object' => $object,
    'filter_term' => $filter_term,
    'field' => $field,
      )
  );
  return $output;
}

/**
 * Retrieve all instances of a given term in Solr.
 *
 * @param string $critical_edition_pid
 *   PID of critical edition
 * @param int $start
 *   Solr Offset
 * @param int $rows
 *   Solr Limit
 * @param string $filter_term
 *   Title filter
 *
 * @return array
 *   Discovered results
 */
function emicdora_get_all_source_objects($critical_edition_pid, $start = 0, $rows = 1000, $filter_term = NULL, $field = NULL) {
  // Solr doesn't like having someone else tell if what $_GET['page'] means.
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $type_mappings = emicdora_source_content_models();
  $temp_get = $_GET;
  unset($_GET);
  $filter = 'RELS_EXT_hasModel_uri_ms:("info:fedora/islandora:bookCModel" OR "info:fedora/islandora:sp-audioCModel" OR "info:fedora/islandora:sp_videoCModel" OR "info:fedora/islandora:sp_large_image_cmodel")';
  $term = 'mods_titleInfo_title_ms';
  $qp = new IslandoraSolrQueryProcessor();
  $page_model = '"info:fedora/islandora:pageCModel"';
  $current_member = "\"info:fedora/$critical_edition_pid\"";

  $qp->buildQuery("$term:[* TO *] -RELS_EXT_hasModel_uri_ms:$page_model -RELS_EXT_isMemberOf_uri_ms:$current_member");
  $qp->solrParams['fq'][] = $filter;
  if ($filter_term && $field) {
    $filter_term = strtolower($filter_term);
    $qp->solrParams['fq'][] = "$field:\"$filter_term\"";
  }
  $qp->solrParams['fl'] = "PID $term RELS_EXT_hasModel_uri_s mods_genre_authority_marcgt_s RELS_EXT_isMemberOf_uri_ms mods_name_personal_author_authority_marcrelator_namePart_mlt mods_abstract_ms";
  $qp->solrParams['facet.limit'] = 100000;
  $qp->solrParams['facet.field'] = array($term, 'RELS_EXT_hasModel_uri_s');
  $qp->solrParams['sort'] = "mods_titleInfo_title_t asc";
  $qp->solrParams['facet.mincount'] = 1;
  $qp->solrStart = $start;
  $qp->solrLimit = $rows;
  $qp->executeQuery(FALSE);
  $solr_results = $qp->islandoraSolrResult['response'];
  $results = array();
  foreach ($solr_results['objects'] as $object) {
    $details = $object['solr_doc'];
    $added = FALSE;
    if (isset($details['RELS_EXT_isMemberOf_uri_ms'])) {
      if (in_array("info:fedora/$critical_edition_pid", $details['RELS_EXT_isMemberOf_uri_ms'])) {
        $added = TRUE;
      }
    }
    $content_model = str_replace('info:fedora/', '', $details['RELS_EXT_hasModel_uri_s']);
    $object_results = array();
    $object_results['pid'] = $details['PID'];
    $object_results['added'] = $added;
    $object_results['genre'] = isset($details['mods_genre_authority_marcgt_s']) ? $details['mods_genre_authority_marcgt_s'] : t('Undefined');
    $object_results['type'] = $type_mappings[$content_model];
    $object_results['title'] = isset($details['mods_titleInfo_title_ms'][0]) ? $details['mods_titleInfo_title_ms'][0] : t('Undefined');
    $object_results['description'] = isset($details['mods_abstract_ms'][0]) ? $details['mods_abstract_ms'][0] : '';
    $results[] = $object_results;

  }
  $_GET = $temp_get;
  return array(
    'results' => $results,
    'count' => $solr_results['numFound'],
  );
}

/**
 * Form to update Sources Add page.
 *
 * @param array $form
 *   Drupal Form definition
 * @param array $form_state
 *   Drupal Form_state definition
 * @param string $pid
 *   PID of the current source.
 *
 * @return array
 *   Drupal Form
 */
function emicdora_sources_update_form(array $form, array &$form_state, $pid) {
  $path_parts = explode('/', $_GET['q']);
  $filter = isset($path_parts[5]) ? $path_parts[5] : '';
  $form['filter'] = array(
    '#type' => 'textfield',
    '#title' => t('Search for'),
    '#default_value' => $filter,
  );
  $field = isset($path_parts[6]) ? $path_parts[6] : 'mods_titleInfo_title_t';
  $form['field'] = array(
    '#title' => t('filter by'),
    '#type' => 'radios',
    '#options' => array(
      'mods_titleInfo_title_t' => t('Title'),
      'mods_name_personal_author_authority_marcrelator_namePart_mt' => t('Creator'),
    ),
    '#default_value' => $field,
  );
  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  return $form;
}

/**
 * Submit function for on filter choice.
 *
 * @param array $form
 *   Drupal Form definition
 * @param array $form_state
 *   Drupal Form_state definition
 */
function emicdora_sources_update_form_submit(array $form, array &$form_state) {
  unset($_GET['destination']);
  $filter = $form_state['values']['filter'];
  $field = $form_state['values']['field'];
  drupal_goto("emicdora/contextual-links/source/{$form_state['values']['pid']}/link/$filter/$field");
}

/**
 * Ajax callback to add source objects to critical edition.
 */
function emicdora_add_source_to_edition() {
  $source_pid = urldecode(filter_xss($_POST['source_pid']));
  $critical_edition_pid = urldecode(filter_xss($_POST['critical_edition_pid']));
  $source_object = islandora_object_load($source_pid);
  $results = array();
  if ($source_object) {
    $rels = $source_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    foreach ($rels as $rel) {
      if ($rel['object']['value'] == $critical_edition_pid) {
        $results['message'] = t("@label is already associated with this Critical Edition", array('@label' => $source_object->label));
      }
    }
    if (!isset($results['message'])) {
      $source_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition_pid);
    }
  }
  $results['added'] = "$critical_edition_pid/source/$source_pid";
  drupal_json_output($results);
}
