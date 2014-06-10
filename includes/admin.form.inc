<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function emicdora_admin_settings_form(array $form, array &$form_state) {
  $form['repository'] = array(
    '#type' => 'fieldset',
    '#title' => 'Mongo Configuration',
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );

  $form['repository']['emicdora_mongo_port'] = array(
    '#type' => 'textfield',
    '#title' => t("Port"),
    '#description' => t("Mongo Database Port"),
    '#default_value' => variable_get('emicdora_mongo_port', "27017"),
  );
  $form['repository']['emicdora_mongo_host'] = array(
    '#type' => 'textfield',
    '#title' => t("Address"),
    '#description' => t("Mongo Database server address"),
    '#default_value' => variable_get('emicdora_mongo_host', "localhost"),
  );
  $form['repository']['emicdora_mongo_db'] = array(
    '#type' => 'textfield',
    '#title' => t("Database name"),
    '#description' => t("Mongo Database instance name"),
    '#default_value' => variable_get('emicdora_mongo_db', "test"),
  );
  return system_settings_form($form);
}
