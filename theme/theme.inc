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
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
}

/**
 * Implements hook_preprocess_theme().
 */
function emicdora_preprocess_emicdora_coop(array &$variables) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/emicdora.coop.css");
  // No Breadcrumbs on this page.
  drupal_set_breadcrumb(array());
  // Links for Add to COOP.
  $variables['add_source_links'] = array();
  foreach (emicdora_source_content_models() as $content_model => $label) {
    $variables['add_source_links'][] = array(
      'title' => t('Add @label', array('@label' => $label)),
      'href' => "workbench/add/$content_model",
    );
  }
  $variables['coop_icon'] = theme_image(array(
    'path' => "{$module_path}/images/coop.png",
    'attributes' => array('class' => 'coop-icon'),
  ));
  // Returns all source object(s), this assumes that solr has been configured to
  // only display source objects, version objects, and critical editions.
  $variables['browse_all_link'] = l(t('Browse All'), 'islandora/search/NOT RELS_EXT_hasModel_uri_mt:(criticalEditionContainerCModel OR versionCModel)');
}
