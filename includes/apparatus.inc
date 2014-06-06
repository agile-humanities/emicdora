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
function emicdora_apparatus_supported_datastreams() {
  return array(
    'EPIGRAPH' => 'Epigraph',
    'ACKNOWLEDGEMENTS' => 'Acknowledgements',
    'PREFACE' => 'Preface',
    'FOREWORD' => 'Foreword',
    'INTRODUCTION' => 'Introduction',
    'EDITORIAL_PROCEDURES' => 'Editorial Procedures',
    'CHRONOLOGY' => 'Chronology',
    'ILLUSTRATIONS' => 'List of Illustrations, Figures, Graphs',
    'EPILOGUE' => 'Epilogue',
    'AFTERWORD' => 'Afterword',
    'APPENDIX' => 'Appendix',
    'BIBLIOGRAPHY' => 'Bibliography',
  );
}

/**
 * Checks if the given datastream identifier is a supported by the apparatus.
 *
 * @param string $dsid
 *   The datastream identifier to check.
 *
 * @return bool
 *   TRUE if it is supported, FALSE otherwise.
 */
function emicdora_is_apparatus_supported_datastream($dsid) {
  $supported_datastreams = emicdora_apparatus_supported_datastreams();
  return isset($supported_datastreams[$dsid]);
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
function emicdora_get_apparatus_datastreams(AbstractObject $object) {
  $apparatus_datastreams = array();
  $supported_datastreams = emicdora_apparatus_supported_datastreams();
  $supported_datastream_identifiers = array_keys($supported_datastreams);
  foreach ($object as $dsid => $datastream) {
    if (in_array($dsid, $supported_datastream_identifiers)) {
      $apparatus_datastreams[$dsid] = $datastream;
    }
  }
  return $apparatus_datastreams;
}

/**
 * Sets the content of the chosen supported apparatus datastream.
 *
 * This function will only update if the DSID is a valid apparatus datastream.
 *
 * @throws InvalidArgumentException
 *   If the given datastream is not supported by the apparatus.
 *
 * @param AbstractObject $object
 *   The object to fetch the datastreams from.
 * @param string $dsid
 *   A supported apparatus datastream identifier.
 * @param string $content
 *   The content to update the apparatus datastream with.
 */
function emicdora_set_apparatus_datastream(AbstractObject $object, $dsid, $content) {
  $supported_datastreams = emicdora_apparatus_supported_datastreams();
  if (!emicdora_is_apparatus_supported_datastream($dsid)) {
    throw new InvalidArgumentException('Attempted to set the non-supported apparatus component: ' . $dsid);
  }
  if (isset($object[$dsid])) {
    $object[$dsid]->content = $content;
  }
  else {
    $datastream = $object->constructDatastream($dsid, 'M');
    $datastream->label = $supported_datastreams[$dsid];
    $datastream->mimetype = 'text/plain';
    $datastream->setContentFromString($content);
    $object->ingestDatastream($datastream);
  }
}
