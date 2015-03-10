<?php

/**
 * @file
 * Defines all the theme functions this module implements.
 */

/**
 * Implements hook_preprocess().
 */
function versionable_object_viewer_preprocess_versionable_object_viewer(&$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');
  module_load_include('inc', 'php_lib', 'DOMHelpers');
  $version = $variables['islandora_object'];
  $module_path = drupal_get_path('module', 'versionable_object_viewer');
  dom_document_pretty_print_include_files();
  $version_object = $variables['islandora_object'];
  $type = emicdora_get_source_type($version_object);
  drupal_add_css("$module_path/css/viewer.css");
  $config = version_viewer_get_config($version_object->id);
  $custom_css = FALSE;
  if ($config && $config->css) {
    $custom_css = file_create_url($config->css);
  }

  // Drop in our initilization script.
  $module_path = drupal_get_path('module', 'versionable_object_viewer');
  drupal_add_js("$module_path/js/entity.js");
  drupal_add_js("$module_path/js/entity_tooltips.js");
  drupal_add_js("$module_path/js/entity_dialogs.js");
  drupal_add_js("$module_path/js/init.js");
  drupal_add_js("$module_path/js/viewer_behaviours.js");
  drupal_add_js("$module_path/js/diplomatic_transcription.js");
  // This is the Shared Canvas prefered jQuery.
  drupal_add_js(
    array(
      'versionable_object_viewer' => array(
        'version_pid' => $version->id,
      ),
    ), 'setting'
  );
  drupal_add_library('versionable_object_viewer', 'jquery_easy_ui');
  $image_module_path = drupal_get_path('module', 'islandora_image_annotation');
  drupal_add_js($image_module_path . '/shared_canvas/js/jquery/jquery-1.7.js');
  drupal_add_js($image_module_path . '/shared_canvas/js/jquery/jquery-ui-1.8.13.custom.min.js');

  $variables['content_model_type'] = emicdora_get_source_type($version_object);
  $variables['transcription_source'] = url("islandora/object/$version_object->id/get_transcription_data");
  $variables['meta_source'] = url("islandora/cwrc_viewer/detail_metadata");
  drupal_add_js(array('versionable_object_viewer' => array('trans_url' => $variables['transcription_source'])), 'setting');
  $variables['version_pid'] = $version->id;
  versionable_object_viewer_build_version_media_view($version_object, $variables);

  if ($custom_css) {
    drupal_add_css($custom_css, array('type' => 'external'));
  }
  else {
    drupal_add_css("$module_path/tei_transforms/stylesheets/screen.css", array(
      'media' => 'screen, projection',
    ));
  }

  drupal_add_css("$module_path/tei_transforms/stylesheets/print.css", array(
    'media' => 'print',
  ));
  drupal_add_css("$module_path/tei_transforms/stylesheets/ie.css", array(
    'media' => 'screen, projection',
    'browsers' => array(
      '!IE' => FALSE,
      'IE' => TRUE,
    ),
  ));
}
