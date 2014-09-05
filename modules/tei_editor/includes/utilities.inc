<?php
/**
 * @file
 * Utility functions.
 */

/**
 * Helper function to find related editions of pages.
 *
 * @param AbstractObject $fedora_object
 *   The fedora object in question.
 * @param string $ontology
 *   The ontology to use, defaults to isDerivationOf.
 */
function tei_editor_get_object_of_tei_rdf(AbstractObject $fedora_object, $ontology = EMICDORA_ISDERIVATIONOF) {
  $relationships = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, $ontology);
  if (in_array("islandora:versionCModel", $fedora_object->models)) {
    $relationships = $fedora_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  }
  return $relationships[0]['object']['value'];
}
