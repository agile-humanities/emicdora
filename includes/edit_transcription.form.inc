<?php

/**
 * @file
 * Handles the management of transcription objects.
 */

/**
 * The form for editiing a transcription.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $transcription
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   An array containing the transcription form to be rendered.
 */
function emicdora_edit_transcription_form($form, &$form_state, AbstractObject $transcription) {
  drupal_set_title(t('Edit @transcription', array('@transcription' => $transcription->label)));
  $content = "";
  if ($transcription['TRANSCRIPTION']) {
    $content = $transcription['TRANSCRIPTION']->content;
  }
  $form = array();
  $form['transcription'] = array(
    '#title' => t('Current Transcription'),
    '#type' => 'textarea',
    '#default_value' => $content,
    '#rows' => 40,
    '#cols' => 20,
    '#attributes' => array('class' => array('transcription_window')),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $transcription->id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Transcription'),
  );
  return $form;
}

/**
 * Submit handler for transcription edit form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form
 */
function emicdora_edit_transcription_form_submit($form, &$form_state) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $transcription_object = islandora_object_load($form_state['values']['pid']);
  $success = FALSE;
  $content = $form_state['values']['transcription'];

  try {
    if ($transcription_object['TRANSCRIPTION']) {
      $success = $transcription_object['TRANSCRIPTION']->content = $content;
    }
    else {
      $datastream_id = "TRANSCRIPTION";
      $control_group = "M";
      $transcription_datastream = $transcription_object->constructDatastream($datastream_id, $control_group);
      $transcription_datastream->mimetype = 'text/plain';
      $transcription_datastream->label = 'Transcription';
      $transcription_datastream->setContentFromstring($content);
      $success = $transcription_object->ingestDatastream($transcription_datastream);
    }
  }
  catch (Exception $exc) {
    watchdog_exception("emicdora", $exc);
  }
  if ($success) {
    drupal_set_message(t("Transcription updated."));
    emicdora_calliope_write_file($transcription_object->id);
  }
}
