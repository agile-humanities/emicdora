<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Gets the PIDs of the pages for the given islandora:versionCModel object.
 *
 * @param AbstractObject $object
 *   The islandora:versionCModel object whose page PIDs will be returned.
 * @param int $limit
 *   Limit the number of results.
 *
 * @return array
 *   An array of zero or more PID's
 */
function tei_editor_get_page_pids(AbstractObject $object, $limit = NULL) {
  $limit = is_int($limit) ? "LIMIT $limit" : '';
  $query = <<<EOT
PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?pid ?sequenceNumber
WHERE {
  ?pid <fedora-rels-ext:isMemberOf> <info:fedora/{$object->id}> ;
       <fedora-model:hasModel> <info:fedora/islandora:tei-rdfCModel> ;
       <islandora-rels-ext:isSequenceNumber> ?sequenceNumber .
}
ORDER BY ?sequenceNumber
$limit
EOT;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->sparqlQuery($query);
  $pids = array();
  foreach ($results as $result) {
    $pids[] = $result['pid']['value'];
  }
  return $pids;
}

/**
 * Gets the PID of the first page of the given islandora:versionCModel object.
 *
 * @param AbstractObject $object
 *   The islandora:versionCModel object whose first page PID will be returned.
 *
 * @return string|NULL
 *   The PID if found, otherwise NULL;
 */
function tei_editor_get_first_page_pid(AbstractObject $object) {
  $pids = tei_editor_get_page_pids($object, 1);
  return array_shift($pids);
}

/**
 * Should be a utility function for EMiC.
 *
 * @param array $derivatives
 *   The set of TEI-RDF objects in which we need to fetch the sources of.
 *
 * @return array
 *   The sources of the given $derivatives.
 */
function tei_editor_fetch_sources(array $derivatives) {
  // Fetch all the source content our documents were derived from.
  $filters = array();
  foreach($derivatives as $pid) {
    $filters[] = "{ <info:fedora/{$pid}> <fedora-rels-ext:isDerivationOf> ?object. }";
  }
  $filters = implode(' UNION ', $filters);
  $query = <<<EOT
PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
SELECT ?object FROM <#ri> WHERE {
  $filters
}
EOT;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->sparqlQuery($query);
  $output = array();
  foreach($results as $result) {
    $output[] = $result['object']['value'];
  }
  return $output;
}
