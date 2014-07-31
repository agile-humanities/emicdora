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
  drupal_add_js("$path/js/collation_edit.js");
}
