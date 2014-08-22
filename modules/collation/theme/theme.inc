<?php

/**
 * @file
 * Defines this modules Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function collation_preprocess_collation_compare(array &$variables) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $library_path = libraries_get_path('rangy-1.2.3');
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  $path = drupal_get_path('module', 'collation');
  drupal_add_js("$path/js/collation_edit.js");
  drupal_add_js("$library_path/rangy-core.js");
  drupal_add_js("$path/js/jquery/wait_for_existence.js");
  drupal_add_js(array('collation' => array('collation_name' => $variables['left'])), 'setting');
  drupal_add_js($module_path . '/shared_canvas/js/jquery/jquery-1.7.js');
}
