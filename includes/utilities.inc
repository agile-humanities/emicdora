<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Defines the set source content models.
 *
 * @return array
 *   The content models which are considered source objects.
 */
function emicdora_source_content_models() {
  return array(
    'islandora:bookCModel' => t('Textual Content'),
    'islandora:sp-audioCModel' => t('Audio'),
    'islandora:sp_videoCModel' => t('Video'),
    'islandora:sp_large_image_cmodel' => t('Image'),
  );
}

/**
 * Checks if the given content model is a source content model.
 *
 * @param string $content_model
 *   The identifier of the content model.
 *
 * @return bool
 *   TRUE if the given content model belongs to the set of source content
 *   models.
 */
function emicdora_is_source_content_model($content_model) {
  $source_content_models = emicdora_source_content_models();
  return isset($source_content_models[$content_model]);
}

/**
 * Checks if the given object has a source content model.
 *
 * @param AbstractObject $object
 *   The object to check.
 *
 * @return bool
 *   TRUE if one of the given object's content models belongs to the set of
 *   source content models.
 */
function emicdora_is_source(AbstractObject $object) {
  $source_content_models = emicdora_source_content_models();
  foreach ($object->models as $content_model) {
    if (in_array($content_model, $source_content_models)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Checks if the given object has content model.
 *
 * @param AbstractObject $object
 *   The object to check.
 * @param string $content_model
 *   PID of content model
 *
 * @return bool
 *   TRUE if it is a critical edition, FALSE otherwise.
 */
function emicdora_has_content_model(AbstractObject $object, $content_model) {
  return in_array($content_model, $object->models);
}

/**
 * Gets the default collection to ingest the given content model type into.
 *
 * @param string $content_model
 *   A content model identifier.
 *
 * @return string|bool
 *   The default collection identifier in which to ingest the given content
 *   model type if one exists, otherwise FALSE.
 */
function emicdora_get_default_collection($content_model) {
  if ($content_model == 'islandora:criticalEditionCModel') {
    return EMICDORA_EDITION_COLLECTION;
  }
  elseif (emicdora_is_source_content_model($content_model)) {
    return EMICDORA_SOURCE_COLLECTION;
  }
  return FALSE;
}

/**
 * Convenience function used to create objects in hook_required_objects().
 *
 * @param IslandoraTuque $connection
 *   The connection to the fedora repository.
 * @param array $args
 *   An associative array defining the objects properties and datastreams"
 *   - pid: The identifier for the new object.
 *   - label: The new object's label.
 *   - model: The content model of the new object.
 *   - datastreams: An associative array of properties for each datastream where
 *     each key represents the datastream's identifier:
 *     - control_group: The control group for this datastream defaults to 'M'.
 *     - label: The datastream's label.
 *     - file: The file path to the datastream's content.
 *     - mimetype: The mimetype of the given file. Defaults to 'text/xml'.
 *
 * @return AbstractObject
 *   A new Fedora Object, that has yet to be persisted to the repository.
 */
function emicdora_create_required_object(IslandoraTuque $connection, array $args) {
  $object = $connection->repository->constructObject($args['pid']);
  $object->owner = 'fedoraAdmin';
  $object->label = $args['label'];
  // Tuque is broken in that if you set the model then set the content of the
  // RELS-EXT datastream afterwards it will be ignored.
  if (isset($args['model'])) {
    $object->models = $args['model'];
  }
  foreach ($args['datastreams'] as $dsid => $properties) {
    $control_group = isset($properties['control_group']) ? $properties['control_group'] : 'M';
    $datastream = $object->constructDatastream($dsid, $control_group);
    $datastream->label = isset($properties['label']) ? $properties['label'] : $dsid;
    $datastream->mimetype = isset($properties['mimetype']) ? $properties['mimetype'] : 'text/xml';
    $datastream->setContentFromFile($properties['file'], FALSE);
    $object->ingestDatastream($datastream);
  }
  return $object;
}

/**
 * Returns associated objects.
 *
 * @param string $object_pid
 *   PID of object
 * @param string $content_model
 *   Optional Content Model to restrict results.
 *
 * @return array
 *   associative array of PID => Content Model pairs.
 */
function emicdora_get_members($object_pid, $content_model = NULL) {
  $qualifier = $content_model ? "fm:hasModel <info:fedora/$content_model>;" : "";
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">
            SELECT ?subject ?object
            FROM <#ri>
            WHERE {
              {?subject fm:state fm:Active;
                        fr:isMemberOfCollection <info:fedora/$object_pid>;
                        fm:hasModel ?object;
                        $qualifier

              }
            UNION
              {?subject fm:state fm:Active;
                        fr:isMemberOf <info:fedora/$object_pid>;
                        fm:hasModel ?object;
                        $qualifier
              }
            }
            ORDER BY ?subject";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    if ($result['object']['value'] != 'fedora-system:FedoraObject-3.0') {
      $objects[$result['subject']['value']] = $result['object']['value'];
    }
  }
  return $objects;
}

/**
 * Append the given TEI-RDF documents text to the given TEI Document.
 *
 * This function is used to generate a consolidated TEI document from the
 * TEI-RDF page objects.
 *
 * @param string $tei
 *   The TEI document to append the TEI-RDF document to.
 * @param string $tei_rdf
 *   The TEI-RDF document to append.
 *
 * @return string
 *   The given TEI document with the appended TEI-RDF document.
 */
function emicdora_append_tei_rdf_text_to_tei($tei, $tei_rdf) {
  define('TEI_NAMESPACE', 'http://www.tei-c.org/ns/1.0');
  $tei_document = new DOMDocument();
  $tei_document->formatOutput = TRUE;
  $tei_document->preserveWhiteSpace = TRUE;
  $tei_document->loadXML($tei);
  $front = $tei_document->getElementsByTagName('front')->item(0);
  $body = $tei_document->getElementsByTagName('body')->item(0);
  $back = $tei_document->getElementsByTagName('back')->item(0);
  $tei_rdf_document = new DOMDocument();
  $tei_rdf_document->formatOutput = TRUE;
  $tei_rdf_document->preserveWhiteSpace = TRUE;
  $tei_rdf_document->loadXML($tei_rdf);
  $tei_rdf_text_node = $tei_rdf_document->getElementsByTagNameNS(TEI_NAMESPACE, 'text')->item(0);
  foreach ($tei_rdf_text_node->childNodes as $tei_rdf_content_node) {
    if (isset(${$tei_rdf_content_node->nodeName})) {
      $tei_rdf_fragment = $tei_document->createDocumentFragment();
      foreach ($tei_rdf_content_node->childNodes as $tei_rdf_content_node_child) {
        $tei_rdf_fragment->appendXML($tei_rdf_document->saveXML($tei_rdf_content_node_child));
      }
      ${$tei_rdf_content_node->nodeName}->appendChild($tei_rdf_fragment);
    }
  }
  $my_doc = $tei_document->saveHTML();
  return $tei_document->saveXML();
}

/**
 * Returns empty TEI form.
 *
 * @param string $title
 *   Title for TEI
 *
 * @return string
 *   Blank TEI
 */
function emicdora_get_blank_tei($title) {
  // @codingStandardsIgnoreStart
  $tei = <<<EOD
<TEI xmlns="http://www.tei-c.org/ns/1.0">
    <teiHeader>
        <fileDesc>
            <titleStmt>
                <title>$title</title>
            </titleStmt>
            <publicationStmt>
                <p/>
            </publicationStmt>
            <sourceDesc>
                <p>Created from islandora book ingest unless otherwise noted.</p>
            </sourceDesc>
        </fileDesc>
    </teiHeader>

    <text>
        <front></front>
        <body></body>
        <back></back>
    </text>
</TEI>
EOD;
// @codingStandardsIgnoreEnd
  return $tei;
}

/**
 * Returns apparatus for display.
 *
 * @param AbstractObject $object
 *   Object representing Critical Edition
 *
 * @return array
 *   array of form elements
 */
function emicdora_get_apparatus_as_content($object) {
  $datastreams = emicdora_get_apparatus_datastreams($object);
  $content = array(
    'apparatus' => array(
      '#type' => 'vertical_tabs',
    ),
  );
  foreach ($datastreams as $dsid => $datastream) {
    $content['apparatus'][$dsid] = array(
      '#title' => $datastream->label,
      '#type' => 'fieldset',
      '#id' => strtolower($dsid),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      'markup' => array(
        '#markup' => filter_xss($datastream->content),
      ),
    );
  }
  return $content;
}

/**
 * Retrieves all source objects.
 *
 * @param AbstractObject $object
 *   Critical Edition object
 *
 * @return array
 *   Sorted member objects
 */
function emicdora_get_raw_sources($object, $offset = 0, $limit = 25) {
  $query = 'RELS_EXT_isMemberOf_uri_mt:"' . $object->id . '"';
  $filters[] = format_string('!membership:("info:fedora/!pid")', array(
    '!pid' => $object->id,
    '!membership' => variable_get('emicdora_critical_edition_membership_solr_field', 'RELS_EXT_isMemberOf_uri_ms')));
  // Only show source objects.
  $source_content_models = emicdora_source_content_models();
  $sources = array();
  foreach (array_keys($source_content_models) as $content_model) {
    $sources[] = format_string('"info:fedora/!pid"', array('!pid' => $content_model));
  }
  $filters[] = format_string('!model:(!sources)', array(
    '!sources' => implode(' OR ', $sources),
    '!model' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_mt')));
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);

  try {
    $results = $solr->search($query, $offset, $limit, array('fq' => $filters));
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('emicdora', $e, 'Got an exception while searching for collection.', array(), WATCHDOG_ERROR);
  }
  $sorted = array();
  foreach ($json['response']['docs'] as $source) {
    $content_model_parts = explode('/', $source['RELS_EXT_hasModel_uri_t']);
    $content_model = $content_model_parts[1];
    $sorted_results[$content_model][] = $source['PID'];
  }
  return $sorted_results;
}

/**
 * Gets all versionable objects assicated with critical edition.
 *
 * @return array
 *   PIDs of currently associated versionable objects
 */
function emicdora_get_source_autocomplete() {
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">
            SELECT ?subject ?label
            FROM <#ri>
            WHERE {
              {?subject     fm:state fm:Active;
                            fr:isMemberOfCollection <info:fedora/islandora:co-op>;
              }
              OPTIONAL{?subject fm:label ?label}
             }";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    $objects[$result['label']['value']] = $result['subject']['value'];
  }
  return $objects;
}
