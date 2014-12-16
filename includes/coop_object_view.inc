<?php

/**
 * @file
 * Present source objects in Emicdora context.
 */

/**
 * Presents object view in CO-OP context.
 *
 * @param AbstractObject $source_object
 *   Object to be displayed
 *
 * @return HTML
 *   Themed view of object
 */
function emicdora_show_object(AbstractObject $source_object) {
  module_load_include('inc', 'emicdora', 'includes/breadcrumb');
  emicdora_set_title_and_breadcrumbs_object($source_object);
  $models = $source_object->models;
  $mappings = emicdora_source_themes();
  $content_models = array_keys($mappings);
  $found = array_intersect($models, array_keys($mappings));
  $theme = $mappings[$found[0]];
  $output = theme($theme['function'], array($theme['object'] => $source_object));
  return $output;
}

/**
 * Defines the source content models themes.
 *
 * @return array
 *   The content models and associated theme functions.
 */
function emicdora_source_themes() {
  return array(
    'islandora:bookCModel' => array('function' => 'islandora_book_book', 'object' => 'object'),
    'islandora:sp-audioCModel' => array('function' => 'islandora_audio', 'object' => 'islandora_object'),
    'islandora:sp_videoCModel' => array('function' => 'islandora_video', 'object' => 'object'),
    'islandora:sp_large_image_cmodel' => array('function' => 'islandora_large_image', 'object' => 'islandora_object'),
  );
}
