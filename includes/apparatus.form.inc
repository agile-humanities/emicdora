<?php

/**
 * @file
 * Defines the form that allows users to add / edit the apparatus.
 */

/**
 * Defines the form that allows users to add or edit sections of the apparatus.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The apparatus to be edited.
 *
 * @return array
 *   The Drupal form definition.
 */
function emicdora_apparatus_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'emicdora', 'includes/apparatus');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'emicdora', 'includes/breadcrumb');
  emicdora_set_title_and_breadcrumbs_object($object);
  $form_state['pid'] = $object->id;
  $supported_datastreams = emicdora_apparatus_supported_datastreams();
  $default_selected_datastream = key($supported_datastreams);
  $selected_datastream = isset($form_state['values']['datastream']) ? $form_state['values']['datastream'] : $default_selected_datastream;
  $content = t('Enter text for @datastream', array('@datastream' => $supported_datastreams[$selected_datastream]));
  if (isset($object[$selected_datastream])) {
    $content = $object[$selected_datastream]->content;
  }
  return array(
    'datastream' => array(
      '#title' => t('Select Component'),
      '#type' => 'select',
      '#options' => $supported_datastreams,
      '#default_value' => $selected_datastream,
      '#ajax' => array(
        'callback' => 'emicdora_apparatus_form_ajax_callback',
        'wrapper' => 'content-wrapper',
        'effect' => 'fade',
        'event' => 'change',
        'method' => 'replace',
      ),
    ),
    $selected_datastream => array(
      '#title' => t('Enter text'),
      '#prefix' => '<div id="content-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'text_format',
      '#base_type' => 'textarea',
      '#rows' => 30,
      '#default_value' => $content,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Create / Update'),
    ),
  );
}

/**
 * Ajax callback for rendering the content section.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 *
 * @return array
 *   element to be re-rendered
 */
function emicdora_apparatus_form_ajax_callback(array $form, array $form_state) {
  $selected_datastream = $form_state['values']['datastream'];
  return $form[$selected_datastream];
}

/**
 * Submit handler will create / update the selected apparatus component.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function emicdora_apparatus_form_submit(array $form, array &$form_state) {

  module_load_include('inc', 'emicdora', 'includes/apparatus');
  $object = islandora_object_load($form_state['pid']);
  $selected_datastream = $form_state['values']['datastream'];
  $content = $form_state['values'][$selected_datastream];
  $content = is_array($content) ? $content['value'] : $content;
  if ($content) {
    emicdora_set_apparatus_datastream($object, $selected_datastream, $content);
  }
  else {
    $object->purgeDatastream($selected_datastream);
  }
  drupal_set_message(t('Updated @component', array('@component' => $form['datastream']['#options'][$selected_datastream])));
  unset($_GET['destination']);
  drupal_static_reset('drupal_get_destination');
  drupal_get_destination();
}
