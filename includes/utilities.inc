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
