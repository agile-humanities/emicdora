<?php

/**
 * @file
 * Defines this modules Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function collation_preprocess_collation_compare(array &$variables) {
  $path = drupal_get_path('module', 'collation');
  $image_module_path = drupal_get_path('module', 'islandora_image_annotation');
  drupal_add_js($image_module_path . '/shared_canvas/js/jquery/jquery-1.7.js');
  drupal_add_js("$path/js/collation_edit.js");
  drupal_add_js("$path/js/jquery-selection/src/jquery.selection.js");
  drupal_add_js("$path/js/jquery-textrange/jquery-textrange.js");
  drupal_add_js("$path/js/jquery/wait_for_existence.js");
}
