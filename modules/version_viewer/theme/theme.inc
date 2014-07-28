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
  $type = $version_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasSourceType');

  drupal_add_css("$module_path/css/version_viewer_diplomatic_tei.css");
  drupal_add_css("$module_path/css/version_viewer_reading_tei.css");
  drupal_add_css("$module_path/css/viewer.css");

    // Drop in our initilization script.
  $module_path = drupal_get_path('module', 'versionable_object_viewer');
  drupal_add_js("$module_path/js/init.js");

  // This is the Shared Canvas prefered jQuery.
  drupal_add_library('versionable_object_viewer', 'jquery_easy_ui');
  $image_module_path = drupal_get_path('module', 'islandora_image_annotation');
  drupal_add_js($image_module_path . '/shared_canvas/js/jquery/jquery-1.7.js');
  drupal_add_js($image_module_path . '/shared_canvas/js/jquery/jquery-ui-1.8.13.custom.min.js');


  $variables['content_model_type'] = $type[0]['object']['value'];
  $variables['transcription_source'] = url("islandora/object/$version_object->id/get_transcription_data");
  $variables['meta_source'] = url("islandora/cwrc_viewer/detail_meta_data");

  drupal_add_js(array('versionable_object_viewer' => array('trans_url' => $variables['transcription_source'])), 'setting');

  versionable_object_viewer_build_version_media_view($version_object, $variables);

}

function EMIC_Theme_viewer_versionable_object_viewer_preprocess_html(&$variables) {
  dsm($variables, "html preprocess");
  drupal_add_library('versionable_object_viewer', 'jquery_easy_ui');
}