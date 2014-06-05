<?php

/**
 * @file
 * Defines forms and functions related to the Critical Apparatus.
 */

/**
 * Gets the list of datastreams the critical apparatus supports.
 *
 * @return array
 *   The DSID's that make up the critical apparatus.
 */
function emicdora_critical_apparatus_supported_datastreams() {
  return array(
    'EPIGRAPH',
    'ACKNOWLEDGEMENTS',
    'PREFACE',
    'FOREWORD',
    'INTRODUCTION',
    'EDITORIAL_PROCEDURES',
    'CHRONOLOGY',
    'ILLUSTRATIONS',
    'EPILOGUE',
    'AFTERWORD',
    'APPENDIX',
    'BIBLIOGRAPHY',
  );
}

/**
 * Gets the apparatus related datastreams that the given object has.
 *
 * @param AbstractObject $object
 *   The object to fetch the datastreams from.
 *
 * @return array
 *   The apparatus related datastreams that the object has index by their DSID.
 */
function emicdora_critical_get_apparatus_datastreams(AbstractObject $object) {
  $apparatus_datastreams = array();
  $supported_datastreams = emicdora_critical_apparatus_supported_datastreams();
  foreach ($object as $dsid => $datastream) {
    if (in_array($dsid, $supported_datastreams)) {
      $apparatus_datastreams[$dsid] = $datastream;
    }
  }
  return $apparatus_datastreams;
}

/**
 * Defines the edit apparatus form.
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
function emicdora_edit_apparatus_form(array $form, array $form_state, AbstractObject $object) {
  return $form;
}
