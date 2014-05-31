<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Checks if the given object is a critical edition object.
 *
 * @param AbstractObject $object
 *   The object to check.
 *
 * @return bool
 *   TRUE if it is a critical edition, FALSE otherwise.
 */
function emicdora_is_critical_edition(AbstractObject $object) {
  return in_array('islandora:criticalEditionCModel', $object->models);
}

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