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
  module_load_include('inc', 'emicdora', 'includes/breadcrumb');
  drupal_set_title(t('Link Existing Source to @label', array('@label' => $critical_edition->label)));
  emicdora_set_source_breadcrumbs($critical_edition);
  $form_state['critical_edition'] = $critical_edition->id;
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
  // @todo Implement.
}
