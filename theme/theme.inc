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
  $display_count = 5;
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");
  drupal_add_js("$module_path/js/coop.js");
  // No Breadcrumbs on this page.
  drupal_set_breadcrumb(array());
  // Links for Add to CO-OP.
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
  $label = variable_get('islandora_solr_object_label_field', 'fgs_label_s');
  $options = array('query' => array('sort' => "$label asc"));
  $variables['browse_all_link'] = l(t('Browse All'), EMICDORA_SEARCH_COOP_PATH, $options);
  $variables['author_table'] = emicdora_build_topic_table('author', $display_count, 0);
  $variables['genre_table'] = emicdora_build_topic_table('genre', $display_count, 1);
  $variables['more_authors'] = l(t("View More »"), 'CO-OP/topics/author');
  $variables['more_genres'] = l(t("View More »"), 'CO-OP/topics/genre');
}

/**
 * Builds tables for urls.
 *
 * @param array $urls
 *   URLs to be displayed
 *
 * @return HTML
 *   Themed table
 */
function emicdora_build_table($urls) {
  $header = array(
    'title' => array('data' => t('Name'), 'class' => 'emicdora_header'),
    'counter' => array('data' => t('Counter'), 'class' => 'emicdora_header'),
  );

  $rows = array();
  foreach ($urls as $data) {
    $row = array(
      'title' => array(
        'data' => $data['url'],
        'class' => 'emicdora_title_cell',
      ),
      'counter' => array(
        'data' => $data['count'],
        'class' => 'emicdora_count_cell',
      ),
    );
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

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_coop_topic(array &$variables) {
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");
  $type = $variables['type'];
  $topic_mappings = array(
    'author' => t("Author"),
    'genre' => t("Genre"),
  );
  $topic = $topic_mappings[$type];
  $variables['page_title'] = t("Browse by @genre", array("@genre" => $topic));
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $display_count = 20;
  $variables['topic_table'] = emicdora_build_topic_table($type, $display_count, 0);
}

/**
 * Redirects menu call to themed output.
 *
 * @param string $type
 *   author or genre
 *
 * @return HTML
 *   Themed output
 */
function emicdora_get_browse_view($type) {
  return theme('emicdora_coop_topic', array('type' => $type));
}

/**
 * Builds themed table for topic page.
 *
 * @param string $type
 *   Author of Genre
 * @param int $display_count
 *   Number of entries to display
 * @param int $element
 *   Identifier to distinguish between multiple pagers
 *
 * @return HTML
 *   THemed table
 */
function emicdora_build_topic_table($type, $display_count, $element) {
  $results = emicdora_get_all($type);
  $urls = array();
  $total = count($urls);
  $current_page = pager_default_initialize($total, $display_count, $element);
  $offset = $current_page * $display_count;
  $solr_term_mappings = array(
    'author' => 'mods_name_personal_author_authority_marcrelator_namePart_ms',
    'genre' => 'mods_genre_authority_marcgt_ms',
  );
  $sort_term_mappings = array(
    'author' => 'mods_name_personal_author_authority_marcrelator_namePart_s',
    'genre' => 'mods_genre_authority_marcgt_s',
  );
  $solr_term = $solr_term_mappings[$type];
  $sort_term = $sort_term_mappings[$type];
  $options = array('query' => array('sort' => "$sort_term asc"));
  foreach ($results as $data) {
    $label = $data['label'];
    $escaped_label = "\"$label\"";
    $urls[] = array(
      'url' => l($label, "islandora/search/{$solr_term}:($escaped_label) NOT RELS_EXT_hasModel_uri_mt%3A(criticalEditionContainerCModel OR versionCModel)", $options),
      'count' => $data['count'],
    );
  }
  $total = count($urls);

  if ($total > $display_count) {
    $urls = array_slice($urls, $current_page * $display_count, $display_count);
  }
  $table = emicdora_build_table($urls);
  $table .= theme('pager', array('quantity' => $total, 'element' => $element));
  return $table;
}
