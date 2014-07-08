<?php
/**
 * @file
 * Defines all the theme funcitons this module implements.
 */

/**
 * Implements hook_preprocess().
 */
function versionable_object_viewer_preprocess_versionable_object_viewer(&$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');
  module_load_include('inc', 'php_lib', 'DOMHelpers');

  $module_path = drupal_get_path('module', 'versionable_object_viewer');

  dom_document_pretty_print_include_files();

  $version_object = $variables['islandora_object'];

  drupal_add_css("$module_path/css/islandora_critical_edition_diplomatic_tei.css");
  drupal_add_css("$module_path/css/islandora_critical_edition_reading_tei.css");

  $type = $version_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasSourceType');
  $variables['content_model_type'] = $type[0]['object']['value'];
  $variables['transcription_source'] = url("islandora/object/$version_object->id/get_transcription_data");
  $variables['meta_source'] = url("islandora/cwrc_viewer/detail_meta_data");

  drupal_add_js(array('versionable_object_viewer' => array('trans_url' => $variables['transcription_source'])), 'setting');

  versionable_object_viewer_build_version_media_view($version_object, $variables);
}
