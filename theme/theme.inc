<?php

/**
 * @file
 * Defines this modules Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_transcription_object(array &$variables) {
  $multiple = FALSE;
  if (isset($variables['transcriptions']) && count($variables['transcriptions']) > 1) {
    $multiple = TRUE;
  }
  $variables['multiple'] = $multiple;

  $objects = array();
  if (isset($variables['transcriptions'])) {
    foreach ($variables['transcriptions'] as $transcription) {
      $flat_pid = islandora_escape_pid_for_function($transcription);
      $objects[$flat_pid] = islandora_object_load($transcription);
    }
  }
  // Fallback if Transcription object is viewed directly.
  else {
    $objects[] = $variables['islandora_object'];
  }
  $object = $objects[key($objects)];
  if ($object['TRANSCRIPTION']) {
    $transcription = str_replace("\n", "<br />", $object['TRANSCRIPTION']->content);
  }
  else {
    $transcription = t("This file has no transcription datastream");
  }
  $variables['transcription'] = $transcription;
}

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_coop(array &$variables) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $titles = emicdora_get_all('title');
  $title_urls = array();
  foreach ($titles as $title) {
    $escaped_title = "\"$title\"";
    $title_urls[] = l($title, "islandora/search/mods_titleInfo_title_ms:($escaped_title) NOT RELS_EXT_hasModel_uri_mt%3A(criticalEditionContainerCModel OR versionCModel)");
  }
  $title_table = emicdora_build_table($title_urls, t('Titles'));
  $authors = emicdora_get_all('author');
  $author_urls = array();
  foreach ($authors as $author) {
    $escaped_author = "\"$author\"";
    $author_urls[] = l($author, "islandora/search/mods_name_personal_author_authority_marcrelator_namePart_ms:($escaped_author) NOT RELS_EXT_hasModel_uri_mt%3A(criticalEditionContainerCModel OR versionCModel)");
  }
  $author_table = emicdora_build_table($author_urls, t('Authors'));
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");
  drupal_add_js("$module_path/js/coop.js");
  // No Breadcrumbs on this page.
  drupal_set_breadcrumb(array());
  // Links for Add to COOP.
  $variables['add_source_links'] = array();
  $source_content_models = emicdora_source_content_models();
  unset($source_content_models['islandora:criticalEditionContainerCModel']);
  foreach ($source_content_models as $content_model => $label) {
    $variables['add_source_links'][] = array(
      'title' => t('Add @label', array('@label' => $label)),
      'href' => "emicdora/source/add/$content_model/FALSE/FALSE",
    );
  }
  $variables['coop_icon'] = theme_image(array(
    'path' => "{$module_path}/images/coop.png",
    'attributes' => array('class' => 'coop-icon'),
  ));
  // Returns all source object(s), this assumes that solr has been configured to
  // only display source objects, version objects, and critical editions.
  $options = array('query' => array('sort' => 'mods_titleInfo_title_s asc'));
  $variables['browse_all_link'] = l(t('Browse All'), EMICDORA_SEARCH_COOP_PATH, $options);
  $variables['title_table'] = $title_table;
  $variables['author_table'] = $author_table;
  $pager_1 = pager_default_initialize(count($title_urls), 20, 0);
  $pager_2 = pager_default_initialize(count($author_urls), 20, 1);

  $variables['collection_pager'] = theme('pager', array('quantity' => 2));
}

/**
 * Builds display tabnles for urls.
 *
 * @param array $urls
 *   URLS to be displayed
 * @param string $title
 *   header
 */
function emicdora_build_table($urls, $title) {
  $header = array(
    array('data' => $title),
  );
  $rows = array();
  foreach ($urls as $url) {
    $row = array('data' => $url);
    $rows[] = array('data' => $row, 'class' => array('draggable'));
  }

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No results found.'),
    'attributes' => array(
      'class' => array('emicdora-url-table'),
    ),
  ));

  return $output;
}
