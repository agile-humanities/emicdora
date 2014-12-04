<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Gets the child documents and sources of the given version object.
 *
 * @param AbstractObject $object
 *   The islandora:versionCModel object whose child documents and their sources
 *   will be returned.
 * @param int $limit
 *   Limit the number of results.
 *
 * @return array
 *   An array of documents pids and source their respective source pids, such
 *   that you can list($documents, $sources) with this function.
 */
function tei_editor_get_documents_and_sources(AbstractObject $object, $limit = NULL) {
  $limit = is_int($limit) ? "LIMIT $limit" : '';
  $query = <<<EOT
PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?document ?source ?sequenceNumber
WHERE {
  ?document <fedora-rels-ext:isMemberOf> <info:fedora/{$object->id}> ;
            <fedora-model:hasModel> <info:fedora/islandora:tei-rdfCModel> ;
            <islandora-rels-ext:isSequenceNumber> ?sequenceNumber ;
            <fedora-rels-ext:isDerivationOf> ?source .
}
ORDER BY ?sequenceNumber
$limit
EOT;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->sparqlQuery($query);
  $documents = array();
  $sources = array();
  foreach ($results as $result) {
    $documents[] = $result['document']['value'];
    $sources[] = $result['source']['value'];
  }
  return array($documents, $sources);
}
