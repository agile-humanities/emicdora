<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess().
 */
function tei_editor_preprocess_tei_editor_viewer(&$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');

  // Get our required path's.
  $tei_editor_module_path = drupal_get_path('module', 'islandora_critical_edition');
  $islandora_markup_editor_path = drupal_get_path('module', 'islandora_markup_editor');
  $islandora_image_anno_path = drupal_get_path('module', 'islandora_image_annotation');

  // Theme our custom components to the tei editor viewer.
  $variables['anno_list_pane'] = theme('islandora_anno_list_pane');
  $variables['anno_img_pane'] = theme('islandora_anno_image_pane');

  // Use this flag to include our own init js scripts.
  $variables['custom_js'] = TRUE;
  $variables['cwrc_lib_path'] = $islandora_markup_editor_path;

  // Custom JS for our editor.
  theme('islandora_markup_editor', $variables);

  drupal_add_js($tei_editor_module_path . "/js/test.js");

  // This is gross. We cant rely on the image_annotation module to
  // add the required js at the correct time. It needs to be abstracted.
  drupal_add_js($islandora_image_anno_path . '/shared_canvas/js/jquery/jquery-ui-1.8.13.custom.min.js');
  drupal_add_js($islandora_image_anno_path . '/shared_canvas/js/jquery/jquery-1.7.js');

  // This is where we add our own init scripts, since we indicated we would with
  // the vars flag '$variables['custom_js'] = TRUE'.
  drupal_add_js($tei_editor_module_path . "/js/custom_delegate.js");
  drupal_add_js($tei_editor_module_path . '/js/writer_wrapper.js');
  drupal_add_js($tei_editor_module_path . "/js/dialog_text_image_anno.js");
  drupal_add_js($tei_editor_module_path . "/js/startup.js");
  drupal_add_js($tei_editor_module_path . "/js/init.js");

  // Include common JS/CSS for critical editions and shared canvas,
  // located in image_annotation module.
  islandora_image_annotation_include_common_js();
  islandora_image_annotation_include_common_css();
}
