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
  // No Title on this page.
  drupal_set_title('');
  // No Breadcrumbs on this page.
  drupal_set_breadcrumb(array());
  // Links for Add to COOP.
  foreach (emicdora_source_content_models() as $content_model => $label) {
    $add_source_links[] = array(
      'title' => t('Add @label', array('@label' => $label)),
      'href' => "workbench/add/$content_model",
    );
  }
  $variables['add_source_links'] = $add_source_links;
}
