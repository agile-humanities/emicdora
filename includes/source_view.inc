<?php
/**
 * @file
 * Present source view in Emicdora context.
 */

/**
 * Display the sources related to this critical edition.
 *
 * @param AbstractObject $object
 *   The critical edition the sources belong to.
 *
 * @return string
 *   HTML displaying the SOLR search results containing the related sources.
 */
function emicdora_critical_edition_sources(AbstractObject $object, $content_model = NULL) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  // Only show members of the given critical edition.
  $filters[] = format_string('!membership:("info:fedora/!pid")', array(
    '!pid' => $object->id,
    '!membership' => variable_get('emicdora_critical_edition_membership_solr_field', 'RELS_EXT_isMemberOf_uri_ms')));
  // Only show source objects.
  $sources = array();
  $source_content_models = isset($content_model) ? array($content_model) : array_keys(emicdora_source_content_models());
  foreach ($source_content_models as $content_model) {
    $sources[] = format_string('"info:fedora/!pid"', array('!pid' => $content_model));
  }
  $filters[] = format_string('!model:(!sources)', array(
    '!sources' => implode(' OR ', $sources),
    '!model' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms')));
  if (isset($_GET['solr'])) {
    $filters[] = $_GET['solr'];
  }
  $_GET['f'] = array(implode(' AND ', $filters));
  $output = islandora_solr('*:*');
  return $output;
}
