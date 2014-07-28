<?php

/**
 * @file
 * Present transcriptions in Emicdora context.
 */

/**
 * Shows transcriptions view.
 *
 * @param string $transcription_pid
 *   PID of chosen transcription
 *
 * @return string
 *   Themed html
 */
function emicdora_transcription_view($object, $transcription_pid) {
  $transcription_object = islandora_object_load($transcription_pid);
  drupal_set_title($transcription_object->label);
  $output = theme('emicdora_transcription_object', array('islandora_object' => $transcription_object));
  return $output;
}
