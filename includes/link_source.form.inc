<?php

/**
 * @file
 * Link a source object with the given Critical Edition.
 */

/**
 * Form for ingesting / adding a new source object to a critical edition.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $critical_edition
 *   The critical edition the source will be linked to.
 *
 * @return array
 *   A Drupal Form for linking a new object of the given type.
 */
function emicdora_link_source_form(array $form, array $form_state, AbstractObject $critical_edition) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  drupal_set_title(t('Link Existing Source to @label', array('@label' => $critical_edition->label)));
  $form_state['critical_edition'] = $critical_edition->id;
  $type = isset($form_state['values']['limit_results']) ? $form_state['values']['limit_results'] : NULL;
  $form['limit_results'] = array(
    '#type' => 'radios',
    '#title' => t('Select Content type'),
    '#options' => array_merge(
        array('default' => t('All')),
        emicdora_source_content_models()
    ),
    '#default_value' => 'default',
    '#ajax' => array(
      'callback' => 'emicdora_source_select_ajax_callback',
      'wrapper' => 'source-wrapper',
      'effect' => 'fade',
      'event' => 'change',
      'method' => 'replace',
    ),
  );
  $form['source_wrapper'] = array(
    '#prefix' => '<div id="source-wrapper">',
    '#suffix' => '</div>',
  );

  $form['source_wrapper']['source_object'] = array(
    '#title' => t("Select source object"),
    '#description' => t("Selected digital asset will be associated with @critical", array('@critical' => $critical_edition->id)),
    '#type' => 'textfield',
    '#autocomplete_path' => "islandora/emicdora_source/autocomplete/$type",
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Associate this object with @label', array('@label' => $critical_edition->label)),
  );
  return $form;
}

/**
 * Links the selected source object with the given critical edition.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_link_source_form_submit(array $form, array $form_state) {
  $source_object = islandora_object_load($form_state['values']['source_object']);
  $edition_object = islandora_object_load($form_state['critical_edition']);
  if ($source_object) {
    $source_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $form_state['critical_edition']);
    drupal_set_message(t("@source has been added to @edition.", array('@source' => $source_object->label, '@edition' => $edition_object->label)));
  }
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
function emicdora_source_select_ajax_callback(array $form, array $form_state) {
  return $form['source_wrapper'];
}
