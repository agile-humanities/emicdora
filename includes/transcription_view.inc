<?php

/**
 * @file
 * Present transcriptions in Emicdora context.
 */

/**
 * Shows transcriptions view.
 *
 * @param AbstractObject $transcription_object
 *   The transcription object being viewed.
 *
 * @return string
 *   Themed html.
 */
function emicdora_transcription_view(AbstractObject $transcription_object) {
  drupal_set_title($transcription_object->label);
  $output = theme('emicdora_transcription_object', array('islandora_object' => $transcription_object));
  return $output;
}
