<?php

/**
 * @file
 * Defines this modules Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function collation_preprocess_collation_compare(array &$variables) {
  $path_parts = explode('/', $_GET['q']);
  $variables['display_type'] = $path_parts[3];
  module_load_include('inc', 'collation', 'includes/calliope');
  $path = drupal_get_path('module', 'collation');
  drupal_add_js("$path/js/jquery/jquery-ui/jquery-ui.js");
  drupal_add_js("$path/js/collation_edit.js");
  drupal_add_js("$path/js/rangy-1.2.3/rangy-core.js");
  drupal_add_js("$path/js/jquery/wait_for_existence.js");
  drupal_add_css("$path/css/emicdora_collation.css");
  drupal_add_css("$path/css/jquery-ui.min.css");
  drupal_add_js(array('collation' => array('collation_name' => $variables['left'])), 'setting');
}
