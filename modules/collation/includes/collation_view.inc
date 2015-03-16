<?php

/**
 * @file
 * Present collations in Emicdora context.
 */

/**
 * Shows compare view.
 *
 * @global type $base_path
 *
 * @param string $right
 *   Page data
 * @param string $left
 *   Page data
 *
 * @return string
 *   Themed html
 */
function emicdora_compare_view($right = NULL, $left = NULL) {
  // Add JS/CSS to render array.
  global $base_path;
  $module_path = drupal_get_path('module', 'collationtools');
  drupal_add_js("$module_path/js/emicdora_customizations.js");
  drupal_add_css(drupal_get_path('module', 'repository') . '/ui/css/tei.css');
  drupal_add_css($module_path . '/ui/app.css');
  drupal_add_js('sites/all/libraries/ext-4.1.1a/ext-all.js');
  drupal_add_js(
      array(
        'collationtools' => array(
          'base_url' => $base_path,
          'module_path' => drupal_get_path('module', 'repository'),
        ),
      ), 'setting'
  );
  drupal_add_js($module_path . '/ui/app/controller/CompareAppController.js');
  drupal_add_js($module_path . '/ui/compareapp.js');
  $contents = theme('collation_compare', array('left' => $left, 'right' => $right));
  return "<div id='emicdora_collation_container'>$contents</div>";
}
