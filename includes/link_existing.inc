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
 * @param string $title
 *   Ttitle filter for search
 *
 * @return HTML
 *   Themed page
 */
function emicdora_add_existing($object, $title = NULL) {
  $form = drupal_get_form('emicdora_sources_update_form');
  $output = drupal_render($form);
  $output .= theme('emicdora_add_source_page', array('object' => $object, 'title' => $title));
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
 * @param string $title
 *   Title filter
 *
 * @return array
 *   Discovered results
 */
function emicdora_get_all_source_objects($critical_edition_pid, $start = 0, $rows = 1000, $title = NULL) {
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
  if ($title) {
    $title = strtolower($title);
    $qp->solrParams['fq'][] = "mods_titleInfo_title_t:\"$title\"";
  }
  $qp->solrParams['fl'] = "PID $term RELS_EXT_hasModel_uri_s mods_genre_authority_marcgt_s RELS_EXT_isMemberOf_uri_ms";
  $qp->solrParams['facet.limit'] = 100000;
  $qp->solrParams['facet.field'] = array($term, 'RELS_EXT_hasModel_uri_s');
  $qp->solrParams['sort'] = "mods_titleInfo_title_t asc";
  $qp->solrParams['facet.mincount'] = 1;
  $qp->solrStart = $start;
  $qp->solrLimit = $rows;
  $qp->executeQuery(FALSE);
  $solr_results = $qp->islandoraSolrResult['response'];
  $total_objects = $solr_results['numFound'];
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
 *
 * @return array
 *   Drupal Form
 */
function emicdora_sources_update_form(array $form, array $form_state) {
  $form['filter'] = array(
    '#type' => 'textfield',
    '#title' => t('Filter'),
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
function emicdora_sources_update_form_submit(array $form, array $form_state) {
  unset($_GET['destination']);
  $filter = $form_state['values']['filter'];
  drupal_goto("emicdora/contextual-links/source/islandora%3Asample-critical_edition/link/$filter");
}

/**
 * Ajax callback to add source objects to critical edition.
 */
function emicdora_add_source_to_edition() {
  $source_pid = urldecode(filter_xss($_POST['source_pid']));
  $critical_edition_pid = urldecode(filter_xss($_POST['critical_edition_pid']));
  $source_object = islandora_object_load($source_pid);
  if ($source_object) {
    $rels = $source_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    foreach ($rels as $rel) {
      if ($rel['object']['value'] == $critical_edition_pid) {
        $message = t("@label is already associated with this Critical Edition", array('@label' => $source_object->label));
      }
    }
    if (!isset($message)) {
      $source_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition_pid);
      $message = t("Successfully added @label to Critical Edition", array('@label' => $source_object->label));
    }

  }
  $results = array(
    'message' => $message,
    'added' => "$critical_edition_pid/source/$source_pid",
  );
  echo json_encode($results);
}
