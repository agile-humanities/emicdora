<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_tei_editor_viewer().
 *
 * Prepares variables for the CWRC viewer.
 * $variables['islandora_object']
 * $variables['anno_list_pane']
 * $variables['anno_img_pane']
 * $variables['images_path']
 * $variables['custom_js']
 */
function tei_editor_preprocess_tei_editor_viewer(&$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');

  $tei_editor_module_path = drupal_get_path('module', 'tei_editor');
  $islandora_markup_editor_path = drupal_get_path('module', 'islandora_markup_editor');
  $islandora_image_anno_path = drupal_get_path('module', 'islandora_image_annotation');

  // Theme our custom components to the tei_editor viewer.
  $variables['anno_list_pane'] = theme('islandora_anno_list_pane');
  $type = $variables['islandora_object']->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasSourceType');
  $variables['content_model_type'] = $type[0]['object']['value'];

  versionable_object_viewer_build_version_media_view($variables['islandora_object'], $variables);
  $variables['images_path'] = url($islandora_markup_editor_path . '/CWRC-Writer/src/img/',
    array('absolute' => TRUE));

  // Use this flag to include your own init .js scripts.
  $variables['custom_js'] = "$islandora_markup_editor_path/js";
  $variables['cwrc_lib_path'] = $islandora_markup_editor_path;
  $variables['sc_img_path'] = url("$islandora_image_anno_path/shared_canvas/imgs/small-logo.png");

  // Theme the default markup editor.
  theme('islandora_markup_editor', $variables);
  // Custom JS for our tei editor.
  tei_editor_add_js($variables);

  // This is gross. We cant rely on the image_annotation module to
  // add the required js at the correct time. It needs to be abstracted.
  drupal_add_js($islandora_image_anno_path . '/shared_canvas/js/jquery/jquery-ui-1.8.13.custom.min.js');
  drupal_add_js($islandora_image_anno_path . '/shared_canvas/js/jquery/jquery-1.7.js');

  // Drop our loader in first.
  drupal_add_js("$tei_editor_module_path/js/loader/js/loading.js");
  drupal_add_css("$tei_editor_module_path/js/loader/css/loading.css");

  // This is where we add our own init scripts, since we indicated we would with
  // the vars flag '$variables['custom_js'] = TRUE'.
  drupal_add_css($tei_editor_module_path . "/css/tei_editor.css");
  drupal_add_css($tei_editor_module_path . "/css/tei_editor_markup.css");
  drupal_add_css($tei_editor_module_path . "/css/tei_editor_dialogs.css");
  drupal_add_js($tei_editor_module_path . "/js/custom_delegate.js");
  drupal_add_js($tei_editor_module_path . '/js/writer_wrapper.js');
  drupal_add_js($tei_editor_module_path . "/js/dialog_text_image_anno.js");
  drupal_add_js($tei_editor_module_path . "/js/startup.js");
  drupal_add_js($tei_editor_module_path . "/js/init.js");

  // Include common JS/CSS for critical editions and shared canvas.
  islandora_image_annotation_include_common_js();
  islandora_image_annotation_include_common_css();

  drupal_add_js($islandora_markup_editor_path . '/CWRC-Writer/src/js/lib/jquery/jquery-ui-1.10.3.custom.js');
}

/**
 * Abstraction of preprocessing functions.
 *
 * @global String $base_url
 * @global User $user
 *
 * @param array $variables
 *   Display variables
 */
function tei_editor_add_js(&$variables, $media = FALSE) {
  global $base_url;
  global $user;

  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_markup_editor', 'includes/utilities');
  module_load_include('inc', 'tei_editor', 'includes/utilities');
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  $tei_editor_module_path = drupal_get_path('module', 'tei_editor');

  $image_loader = file_create_url("$tei_editor_module_path/js/loader/img/ajax-loader.gif");
  // Islandora Image Annotation base path, required module.
  $image_annotation_module_path = drupal_get_path('module', 'islandora_image_annotation');
  $islandora_markup_editor_module_path = drupal_get_path('module', 'islandora_markup_editor');

  $pages = islandora_paged_content_get_pages($variables['islandora_object']);
  $pids = array_keys($pages);
  $schema_object = array();
  $page_obj = islandora_object_load($pids[0]);
  $doc_process_instructions = islandora_markup_editor_fetch_tei_processing_instruction($page_obj, 'CWRC');
  $image_anno_images = $image_annotation_module_path . '/shared_canvas/imgs/';
  $validate_path = variable_get('islandora_markup_editor_validate_url', '/validator/validate.html');

  $bp = tei_editor_get_object_of_tei_rdf($variables['islandora_object']);

  // This is silly. We need to override the default tei schema with our own.
  $schema_object['schemas']['tei'] = array(
    'cssUrl' => file_create_url("$islandora_markup_editor_module_path/CWRC-Writer/src/css/tei_converted.css"),
    'name' => 'tei',
    'url' => $doc_process_instructions['href'],
  );

  $type = $variables['islandora_object']->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasSourceType');
  // To avoid hard coded values in JavaScript, this is the best option.
  drupal_add_js(
    array(
      'islandora_markup_editor' => array(
        'image_anno_img' => $image_anno_images,
        'book_pid' => $bp,
        'type' => $type[0]['object']['value'],
        'module_base' => $variables['cwrc_lib_path'],
        'schema_object' => $schema_object,
        'images_path' => $variables['images_path'],
        'base_url' => $base_url,
        'validate_path' => $validate_path,
        'page_pid' => $pids[0],
        'image_loader' => $image_loader,
      ),
      'current_user' => array(
        'id' => $user->uid,
      ),
    ),
    'setting'
  );
}
