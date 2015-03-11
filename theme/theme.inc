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
  $display_count = $variables['display_count'];
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
  $options = array('query' => array('sort' => "fgs_label_ss asc"));
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
    'counter' => array('data' => t('Count'), 'class' => 'emicdora_header'),
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
  $variables['topic_table'] = emicdora_build_topic_table($type, $variables['display_count'], 0);
}

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_add_source_page(array &$variables) {
  module_load_include('inc', 'emicdora', 'includes/link_existing');

  $object = $variables['islandora_object'];
  $return_label = t("Return to @label", array('@label' => $object->label));
  $variables['return_button'] = "<button class = 'emicdora_button return'>$return_label</button>";
  $title = isset($variables['filter_term']) ? $variables['filter_term'] : NULL;
  $field = isset($variables['field']) ? $variables['field'] : NULL;
  $display_count = $variables['display_count'];
  $count_data = emicdora_get_all_source_objects($object->id, 0, 0, $title, $field);
  $total = $count_data['count'];
  $current_page = pager_default_initialize($total, $display_count);
  $start = $current_page * $display_count;
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");
  drupal_add_js("$module_path/js/emicdora_add_source_callback.js");
  $critical_edition = $variables['islandora_object'];
  drupal_add_css("$module_path/css/emicdora.coop.css");
  drupal_add_js(
      array(
        'critical_edition' => $critical_edition->id,
      ), 'setting'
  );
  $source_data = emicdora_get_all_source_objects($object->id, $start, $display_count, $title, $field);
  $source_objects = $source_data['results'];
  $output = "";
  foreach ($source_objects as $item_details) {
    $item = theme('emicdora_add_source_item', array('object' => $object, 'item_details' => $item_details));
    $output .= $item;
  }
  $variables['item_table'] = $output;
  $variables['pager'] = theme('pager');
}

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_add_source_item(array &$variables) {
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");

  $variables['item_table'] = emicdora_item_table($variables['item_details']);
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
 *   Themed table
 */
function emicdora_build_topic_table($type, $display_count, $element) {
  $results = emicdora_get_all($type);
  $urls = array();
  $total = count($results);
  $current_page = pager_default_initialize($total, $display_count, $element);
  $solr_term_mappings = array(
    'author' => 'mods_name_personal_author_authority_marcrelator_namePart_ms',
    'genre' => 'mods_genre_authority_marcgt_ms',
  );
  $solr_term = $solr_term_mappings[$type];
  $options = array('query' => array('sort' => "fgs_label_ss asc", 'query_type' => 'coop'));
  foreach ($results as $data) {
    $label = $data['label'];
    $escaped_label = "\"$label\"";
    $urls[] = array(
      'url' => l($label, "islandora/search/{$solr_term}:($escaped_label) NOT RELS_EXT_hasModel_uri_mt%3A(criticalEditionContainerCModel OR versionCModel)", $options),
      'count' => $data['count'],
    );
  }
  if ($total > $display_count) {
    $urls = array_slice($urls, $current_page * $display_count, $display_count);
  }
  $table = emicdora_build_table($urls);
  $table .= theme('pager', array('quantity' => 10, 'element' => $element));
  return $table;
}

/**
 * Build source table.
 *
 * @param array $item_details
 *   Values for table items
 *
 * @return HTML
 *   Themed table
 */
function emicdora_item_table($item_details) {
  $header = NULL;
  $rows = array();
  $image = theme_image(array(
    'path' => "islandora/object/{$item_details['pid']}/datastream/TN/view",
    'alt' => '',
    'height' => '150px',
    'attributes' => array('class' => 'emicdora_thumbnail'),
  ));

  $add_class = !$item_details['added'] ? 'add_source' : NULL;
  $label = !$item_details['added'] ? t("Add") : t("Added");
  $button = "<button class = 'emicdora_button $add_class' target='{$item_details['pid']}'>$label</button>";
  $rows[] = array(
    'image' => array(
      'data' => $image,
      'class' => 'emicdora_cell_image',
      'rowspan' => 3,
    ),
    'title' => array(
      'data' => t('Title'),
      'class' => 'emicdora_cell_title',
    ),
    'label' => array(
      'data' => $item_details['title'],
      'class' => 'emicdora_cell_label',
    ),
  );
  $rows[] = array(
    'title' => array(
      'data' => t('Type'),
      'class' => 'emicdora_cell_title',
    ),
    'label' => array(
      'data' => $item_details['type'],
      'class' => 'emicdora_cell_label',
    ),
  );
  $rows[] = array(
    'title' => array(
      'data' => t('Description'),
      'class' => 'emicdora_cell_title',
    ),
    'label' => array(
      'data' => $item_details['description'],
      'class' => 'emicdora_cell_label',
    ),
  );
  $rows[] = array(
    'image' => array(
      'data' => $button,
      'class' => 'emicdora_cell_image',
    ),
    'title' => array(
      'data' => t('Genre'),
      'class' => 'emicdora_cell_title',
    ),
    'label' => array(
      'data' => $item_details['genre'],
      'class' => 'emicdora_cell_label',
    ),
  );

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No results found.'),
    'attributes' => array(
      'class' => array('emicdora-link-source-table'),
    ),
  ));
  return $output;
}
