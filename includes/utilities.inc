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
    'islandora:bookCModel' => t('Text'),
    'islandora:sp-audioCModel' => t('Audio'),
    'islandora:sp_videoCModel' => t('Video'),
    'islandora:sp_large_image_cmodel' => t('Image'),
  );
}

/**
 * Defines all content models.
 *
 * @return array
 *   All content models needing menu paths.
 */
function emicdora_get_all_content_models() {
  return array_merge(emicdora_source_content_models(), array('islandora:criticalEditionContainerCModel' => t('Critical Edition'))
  );
}

/**
 * Get the plain text source type of the isDerivationOf rel.
 *
 * @param AbstractObject $version_object
 *   The versionable object to be processed
 *
 * @return string
 *   text, audio, video, image or NULL.
 */
function emicdora_get_source_type(AbstractObject $version_object) {
  $derivation = $version_object->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
  $derivation_pid = $derivation[0]['object']['value'];
  $object = islandora_object_load($derivation_pid);
  $type = NULL;
  if (in_array('islandora:bookCModel', $object->models)) {
    $type = "text";
  }
  if (in_array('islandora:sp-audioCModel', $object->models)) {
    $type = "audio";
  }
  if (in_array('islandora:sp_videoCModel', $object->models)) {
    $type = "video";
  }
  if (in_array('islandora:sp_large_image_cmodel', $object->models)) {
    $type = "image";
  }
  return $type;
}

/**
 * Determin if the Apparatus block should be shown.
 *
 * @return bool
 *   TRUE if the apparatus should be shown, FALSE otherwise.
 */
function emicdora_show_apparatus_block() {
  $path_parts = explode('/', $_GET['q']);
  $acceptable = array(
    'source',
    'sources',
    'version',
    'versions',
    'transcription',
    'apparatus',
    'compare',
  );
  $show_apparatus = FALSE;
  if (!isset($path_parts[3])) {
    $show_apparatus = TRUE;
  }
  else {
    if (in_array($path_parts[3], $acceptable)) {
      $show_apparatus = TRUE;
    }
  }

  $return = FALSE;
  // Ensure we are looking at the default edition view.
  if (isset($path_parts[2]) && $show_apparatus) {
    // Confirm the given objects content model.
    $object = islandora_object_load($path_parts[2]);
    $allowed_cmodels = array('islandora:criticalEditionContainerCModel', 'islandora:versionCModel');
    if (array_intersect($allowed_cmodels, $object->models)) {
      // Correct content model, and correct path, i say
      // lets show the Apparatus block.
      $return = TRUE;
    }
  }
  return $return;
}

/**
 * Builds a transcription object.
 *
 * @param AbstractObject $versionable_object
 *   object representing enclising Versionable Object
 * @param string $label
 *   Label for new Transcription
 *
 * @return AbsgtractObject
 *   Transcription object
 */
function emicdora_construct_transcription(AbstractObject $versionable_object, $label) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $namespace = islandora_get_namespace($versionable_object->id);
  $transcription = $versionable_object->repository->constructObject($namespace);
  $transcription->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object->id);
  $transcription->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:transcriptionCModel');
  $transcription->label = $label;
  $transcription->owner = $versionable_object->owner;
  return $transcription;
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
  $source_content_models = array_keys(emicdora_source_content_models());
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
  if ($content_model == 'islandora:criticalEditionContainerCModel') {
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
 * @param string $relationship
 *   fedora relationship
 *
 * @return array
 *   associative array of PID => Content Model pairs.
 */
function emicdora_get_members($object_pid, $content_model = NULL, $relationship = 'isMemberOfCollection') {
  $qualifier = $content_model ? "fm:hasModel <info:fedora/$content_model>;" : "";
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">
            SELECT ?subject ?object
            FROM <#ri>
            WHERE {
              {?subject fm:state fm:Active;
                        fr:$relationship <info:fedora/$object_pid>;
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
  $tei_rdf = preg_replace('/<\?xml-model .*\?>/', '', $tei_rdf);
  $tei_rdf_document->loadXML($tei_rdf);
  $tei_rdf_text_node = $tei_rdf_document->getElementsByTagNameNS(EMICDORA_TEI_NAMESPACE, 'text')->item(0);
  foreach ($tei_rdf_text_node->childNodes as $tei_rdf_content_node) {
    if (isset(${$tei_rdf_content_node->nodeName})) {
      $tei_rdf_fragment = $tei_document->createDocumentFragment();
      foreach ($tei_rdf_content_node->childNodes as $tei_rdf_content_node_child) {
        $tei_rdf_fragment->appendXML($tei_rdf_document->saveXML($tei_rdf_content_node_child));
      }
      ${$tei_rdf_content_node->nodeName}->appendChild($tei_rdf_fragment);
    }
  }
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
  // @ignore style_lowercase_html:function
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
function emicdora_get_apparatus_as_content($object, $dsid = NULL) {
  $path = drupal_get_path('module', 'emicdora');
  $dsid = isset($dsid) ? strtoupper($dsid) : NULL;
  $datastreams = emicdora_get_apparatus_datastreams($object);
  if (count($datastreams) == 0) {
    if (islandora_object_access(ISLANDORA_ADD_DS, $object)) {
      $link = l(t("create one"), "emicdora/contextual-links/apparatus/{$object->id}/edit");
      return t("No apparatus defined, please !link.", array('!link' => $link));
    }
    return t("No apparatus defined");
  }
  if ($dsid && isset($datastreams[$dsid])) {
    $datastream = $datastreams[$dsid];
  }
  else {
    $datastream = array_shift($datastreams);
  }

  $content['apparatus'][$datastream->id] = array(
    '#title' => $datastream->label,
    '#type' => 'fieldset',
    '#id' => $dsid,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    'markup' => array(
      '#markup' => filter_xss($datastream->content),
    ),
  );

  return $content;
}

/**
 * Gets all versionable objects associated with critical edition.
 *
 * @return array
 *   PIDs of currently associated versionable objects
 */
function emicdora_get_source_autocomplete() {
  // @ignore style_lowercase_html:function
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

/**
 * Returns PID and title of all critical editions associated with an object.
 *
 * @param AbstractObject $fedora_object
 *   The base object to check for related critical editions.
 *
 * @return array
 *   The list of critical editions.
 */
function emicdora_get_critical_editions(AbstractObject $fedora_object) {
  // @ignore style_lowercase_html:function
  $edition_pids = array();

  $query = "
select \$subject \$title from <#ri>
where (\$subject <" . ISLANDORA_RELS_EXT_URI . EMICDORA_ISDERIVATIONOF . ">'" . $fedora_object->id . "'
and \$subject <" . FEDORA_MODEL_URI . "state> <" . FEDORA_MODEL_URI . "Active>
and \$subject <dc:title> \$title)
";

  $objects = $fedora_object->repository->ri->itqlQuery($query, 'unlimited');
  if (!empty($objects)) {
    foreach ($objects as $object) {
      $edition_pids[$object['subject']['value']] = $object['title']['value'];
    }
  }

  return $edition_pids;
}

/**
 * Constructs CWRC datastream on an object if none exists.
 *
 * @param AbstractObject $object
 *   The object to create the CWRC datastream for. It should have an OCR
 *   datastream.
 */
function emicdora_add_cwrc_datastream(AbstractObject $object) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  try {
    $cwrc_stream = emicdora_wrap_plain_text_in_tei($object['OCR']->content);
    if (!$object['CWRC']) {
      $cwrc_datastream = $object->constructDatastream('CWRC');
    }
    else {
      $cwrc_datastream = $object['CWRC'];
    }
    if ($cwrc_datastream->label != 'CWRC') {
      $cwrc_datastream->label = 'CWRC';
    }
    if ($cwrc_datastream->mimetype != 'text/plain') {
      $cwrc_datastream->mimetype = 'text/plain';
    }
    $cwrc_datastream->setContentFromString($cwrc_stream);
    $object->ingestDatastream($cwrc_datastream);
  }
  catch (Exception $e) {
    drupal_set_message(t(
            'An error occured while creating the CWRC datastream on @pid with message: "@message"', array(
              '@pid' => $object->id,
              '@message' => $e->getMessage(),
            )
        ), 'error'
    );
    watchdog(
        'islandora_critical_edition', 'An error occured while creating the CWRC datastream on @pid with message: "@message" and stack: @trace', array(
          '@pid' => $object->id,
          '@message' => $e->getMessage(),
          '@trace' => $e->getTraceAsString(),
        ), WATCHDOG_ERROR
    );
  }
}

/**
 * Converts the given TEI xml document to plain text.
 *
 * @param string $tei
 *   A document containing TEI xml.
 */
function emicdora_convert_tei_to_plain_text($tei) {
  // @codingStandardsIgnoreStart
  $replace_candidates = array(
    '<br>',
    '<br />',
    '<lb></lb>',
    '<lb/>',
    '</l>',
    '</lg>',
  );
  // @codingStandardsIgnoreEnd
  $text = @simplexml_load_string($tei)->text[0]->asXML();
  foreach ($replace_candidates as $candidate) {
    $text = str_replace($candidate, "$candidate\n", $text);
  }
  return strip_tags($text);
}

/**
 * Retrieves all member objects.
 *
 * @param AbstractObject $object
 *   Critical Edition object
 * @param string $type
 *   The type of objects being searched for.
 * @param string $source_id
 *   The PID of the source object if present.
 *
 * @return array
 *   Sorted member objects
 */
function emicdora_get_raw_objects($object, $type = 'source', $source_id = NULL) {
  module_load_include('inc', 'emicdora', 'includes/db');
  $query = 'RELS_EXT_isMemberOf_uri_mt:"' . $object->id . '"';
  $filters[] = format_string('!membership:("info:fedora/!pid")', array(
    '!pid' => $object->id,
    '!membership' => variable_get('emicdora_critical_edition_membership_solr_field', 'RELS_EXT_isMemberOf_uri_ms')));
  switch ($type) {
    case 'source':
      $source_content_models = emicdora_source_content_models();
      break;

    case 'version':
      $source_content_models = array('islandora:versionCModel' => 0);
      break;

    case 'source_versions':
      $configs = emicdora_get_terms($object->id, 'version');
      $categories = array();
      foreach ($configs as $config) {
        $categories[$config['solr_field']] = $config['solr_field'];
      }
      $source_content_models = array('islandora:versionCModel' => 0);
      break;

    case 'transcriptions':
      $source_content_models = array('islandora:transcriptionCModel' => 0);
      break;
  }

  $sources = array();
  foreach (array_keys($source_content_models) as $content_model) {
    $sources[] = format_string('"info:fedora/!pid"', array('!pid' => $content_model));
  }
  $filters[] = format_string('!model:(!sources)', array(
    '!sources' => implode(' OR ', $sources),
    '!model' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_mt')));
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  $fq = $qp->solrParams['fq'];
  // Lifted directly out of the QP class on how it handles base filters.
  $base_filters = preg_split("/\\r\\n|\\n|\\r/", variable_get('islandora_solr_base_filter', ''), -1, PREG_SPLIT_NO_EMPTY);
  foreach ($base_filters as $filter) {
    $key = array_search($filter, $fq);
    unset($fq[$key]);
  }
  $qp->solrParams['fq'] = array_merge($fq, $filters);
  $qp->executeQuery(FALSE);
  $sorted_results = array();
  foreach ($qp->islandoraSolrResult['response']['objects'] as $result) {
    $candidate = $result['solr_doc'];
    switch ($type) {

      case 'version':
      case 'source':
        $configs = emicdora_get_terms($object->id, $type);
        $version_facets = array();
        foreach ($configs as $config) {
          $version_facets[$config['solr_field']] = array(
            'fields' => $config['solr_field_settings']['fields'],
          );
        }
        $matched = FALSE;
        $added = FALSE;
        foreach ($version_facets as $label => $fields) {
          foreach ($fields as $solr_field => $value) {
            if (isset($candidate[$solr_field])) {
              if (is_array($candidate[$solr_field])) {
                $search_array = array_map('strtolower', $candidate[$solr_field]);
                if (in_array(strtolower($value), $search_array)) {
                  $matched = TRUE;
                }
              }
              else {
                if (strtolower($candidate[$solr_field]) == strtolower($value)) {
                  $matched = TRUE;
                }
              }
            }
          }
          if ($matched) {
            $sorted_results[$label][] = $candidate['PID'];
            $matched = FALSE;
            $added = TRUE;
          }
        }
        if (!$added) {
          $sorted_results[t('Unclassified')][] = $candidate['PID'];
        }
        break;

      case 'source_versions':
        $version_facets = array();
        foreach ($configs as $config) {
          $version_facets[$config['solr_field']] = array(
            'fields' => $config['solr_field_settings']['fields'],
          );
        }
        $type_parts = array();
        if (isset($candidate['RELS_EXT_hasSourceType_uri_t'])) {
          $type_parts = explode('/', $candidate['RELS_EXT_hasSourceType_uri_t']);
        }
        $type_part = isset($type_parts[1]) ? $type_parts[1] : t("unclassified");
        if (isset($candidate['RELS_EXT_isDerivationOf_uri_s']) && $candidate['RELS_EXT_isDerivationOf_uri_s'] == "info:fedora/$source_id") {
          $sorted_results[$type_part][] = $candidate['PID'];
        }

        break;

      case 'transcriptions':
        $type_parts = array();
        if (isset($candidate['RELS_EXT_hasSourceType_uri_t'])) {
          $type_parts = explode('/', $candidate['RELS_EXT_hasSourceType_uri_t']);
        }
        $type_part = isset($type_parts[1]) ? $type_parts[1] : t("unclassified");
        $sorted_results[$object->id][] = array('label' => $candidate['dc.title'][0], 'pid' => $candidate['PID']);
        break;
    }
  }
  return $sorted_results;
}

/**
 * Generates a TEI document from the given object's OCR datastream.
 *
 * @param AbstractObject $object
 *   The object to generate the TEI document from, it should have an OCR
 *   datastream.
 */
function emicdora_generate_tei_from_ocr(AbstractObject $object) {
  $content = isset($object['OCR']) ? $object['OCR']->content : NULL;
  if (isset($object['OCR'])) {
    return emicdora_wrap_plain_text_in_tei($object['OCR']->content);
  }
  // Return the Default blank text.
  return emicdora_wrap_plain_text_in_tei();
}

/**
 * Generates a TEI document with the given text content.
 *
 * This function is used to generate the CWRC datastream content from the
 * objects OCR datastream.
 *
 * @param string $content
 *   The plain text to be wrapped in a TEI document.
 */
function emicdora_wrap_plain_text_in_tei($content = 'Blank Page') {
  // @ignore style_lowercase_html:function
  $cwrc_library_path = libraries_get_path('CWRC-Writer');
  $prefix = '<?xml version="1.0" encoding="UTF-8"?>
<TEI xmlns="http://www.tei-c.org/ns/1.0">
  <teiHeader>
    <fileDesc>
      <titleStmt>
        <title>Sample Document Title</title>
      </titleStmt>
      <publicationStmt>
        <p/>
      </publicationStmt>
      <sourceDesc sameAs="http://www.cwrc.ca">
        <p>Created from Islandora book ingest unless otherwise noted.</p>
      </sourceDesc>
    </fileDesc>
  </teiHeader>
  <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:w="http://cwrctc.artsrn.ualberta.ca/#"></rdf:RDF>
  <text>
    <body><ab>';
  $suffix = '</ab></body></text></TEI>';
  // Correct common problems with the incoming text, and replace line breaks
  // with there TEI equivalent.
  $content = trim(preg_replace('/&£/', '£', $content));
  $content = htmlspecialchars($content);

  return $prefix . $content . $suffix;
}

/**
 * Retrieves created user facets for versions and sources.
 *
 * @param string $pid
 *   The PID of the critical edition that the facets were made under.
 * @param string $type
 *   The type of facets being retrieved either 'version' or 'source'.
 *
 * @return array
 *   An array of arrays where the key is the Solr field and value and the values
 *   contain:
 *   -display_name: A string representing the field name for display in the UI.
 *   -count: An integer of the number of occurences of the facet.
 */
function emicdora_retrieve_user_created_facets($pid, $type) {
  module_load_include('inc', 'emicdora', 'includes/db');
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('*:*');
  $filters[] = format_string('!membership:("info:fedora/!pid")', array(
    '!pid' => $pid,
    '!membership' => variable_get('emicdora_critical_edition_membership_solr_field', 'RELS_EXT_isMemberOf_uri_ms')));
  // Only show source objects.
  if ($type == 'source') {
    foreach (emicdora_source_content_models() as $content_model => $label) {
      $sources[] = format_string('"info:fedora/!pid"', array('!pid' => $content_model));
    }
    $filters[] = format_string('!model:(!sources)', array(
      '!sources' => implode(' OR ', $sources),
      '!model' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms')));
  }
  // Only show versions objects.
  else {
    $filters[] = format_string('!model:("info:fedora/islandora:versionCModel")', array(
      '!model' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms')));
  }
  $qp->solrParams['fq'] = $filters;
  $qp->solrParams['fl'] = '*';
  $qp->solrParams['facet.limit'] = 100000;
  $qp->solrParams['facet.mincount'] = variable_get('islandora_solr_facet_min_limit', '2');
  $qp->solrParams['facet.field'] = array();
  $configs = emicdora_get_terms($pid, $type);
  $formatted_facets = array();
  foreach ($configs as $config) {
    $facet = array();
    foreach ($config['solr_field_settings']['fields'] as $field) {
      $facet_query = format_string('@solr_field:@solr_value', array(
        '@solr_field' => $field['solr_field'],
        '@solr_value' => $field['value'],
      ));
      $facet[] = $facet_query;
    }
    $facets = implode(' AND ', $facet);
    $formatted_facets[$facets] = array(
      'display_name' => $config['solr_field'],
    );
    $qp->solrParams['facet.query'][] = $facets;
  }
  $qp->executeQuery(FALSE);
  foreach ($qp->islandoraSolrResult['facet_counts']['facet_queries'] as $field_val => $count) {
    $formatted_facets[$field_val]['count'] = $count;
  }
  return $formatted_facets;
}

/**
 * Removes source object from Critical Edition.
 *
 * @param AbstractObject $source_object
 *   Source Object to be removed from Critical Edition.
 */
function emicdora_remove_source(AbstractObject $source_object = NULL) {
  $destination = $_GET['destination'];
  $path_parts = explode('/', $destination);
  // If $_GET['destination'] is set, page redirects will fail.
  unset($_GET['destination']);
  $critical_edition_pid = $path_parts[2];
  $critical_edition = islandora_object_load($critical_edition_pid);
  $source_object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition_pid);
  $versions = array_keys(emicdora_get_members($critical_edition_pid, 'islandora:versionCModel'));
  foreach ($versions as $version_pid) {
    $version = islandora_object_load($version_pid);
    $sources = $version->relationships->get(FEDORA_RELS_EXT_URI, 'isDerivationOf');
    foreach ($sources as $source) {
      if ($source['object']['value'] == $source_object->id) {
        $version->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition_pid);
      }
    }
  }
  drupal_set_message(t("@source is no longer associated with @edition", array('@source' => $source_object->label, '@edition' => $critical_edition->label)));
  drupal_goto("islandora/object/$critical_edition_pid");
}

/**
 * Removes Version from Critical Edition.
 *
 * @param string $critical_edition_pid
 *   PID of Critical Edition
 * @param string $version_object_pid
 *   PID of Version Object.
 */
function emicdora_remove_version($critical_edition_pid, $version_object_pid) {
  $version = islandora_object_load($version_object_pid);
  $critical_edition = islandora_object_load($critical_edition_pid);
  $version->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition_pid);
  drupal_set_message(t("@version is no longer associated with @edition", array('@version' => $version->label, '@edition' => $critical_edition->label)));
  drupal_goto("islandora/object/$critical_edition_pid");
}

/**
 * Fetches the source object the given version object was derived from.
 *
 * This function makes no checks to see if the object given is in fact a
 * version. Or if the version in not valid (ex. is missing
 * relationships).
 *
 * @param AbstractObject $version
 *   The version object to get the source from.
 *
 * @return AbstractObject
 *   The source object the given version object was derived from.
 */
function emicdora_get_version_source(AbstractObject $version) {
  $results = $version->relationships->get(FEDORA_RELS_EXT_URI, 'isDerivationOf');
  // Assumes the relation is always present and valid.
  return islandora_object_load($results[0]['object']['value']);
}

/**
 * Fetches the given transcription's objects parent version object.
 *
 * This function makes no checks to see if the object given is in fact a
 * transcription. Or if the transcription in not valid (ex. is missing
 * relationships).
 *
 * @param AbstractObject $transcription
 *   The transcription object to get the version from.
 *
 * @return AbstractObject
 *   The given transcription objects parent version object.
 */
function emicdora_get_transcription_version(AbstractObject $transcription) {
  $results = $transcription->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  // Assumes the relation is always present and valid.
  return islandora_object_load($results[0]['object']['value']);
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
function emicdora_is_critical_edition(AbstractObject $object) {
  return in_array('islandora:criticalEditionContainerCModel', $object->models);
}

/**
 * Retrieve all instances of a given term in Solr.
 *
 * @param string $type
 *   Type of input
 *
 * @return array
 *   Discovered results
 */
function emicdora_get_all($type) {
  // Solr doesn't like having someone else tell if what $_GET['page'] means.
  $temp_get = $_GET;
  unset($_GET);
  $mappings = array(
    'author' => 'mods_name_personal_author_authority_marcrelator_namePart_ms',
    'title' => 'mods_titleInfo_title_ms',
    'genre' => 'mods_genre_authority_marcgt_ms',
  );
  $filter = 'RELS_EXT_hasModel_uri_ms:("info:fedora/islandora:bookCModel" OR "info:fedora/islandora:sp-audioCModel" OR "info:fedora/islandora:sp_videoCModel" OR "info:fedora/islandora:sp_large_image_cmodel")';
  $term = $mappings[$type];
  $qp = new IslandoraSolrQueryProcessor();
  $page_model = '"info:fedora/islandora:pageCModel"';
  $qp->buildQuery("$term:[* TO *] -RELS_EXT_hasModel_uri_ms:$page_model");
  $qp->solrParams['fq'][] = $filter;
  $qp->solrParams['fl'] = "PID $term";
  $qp->solrParams['facet.limit'] = 100000;
  $qp->solrParams['facet.field'] = array($term, 'RELS_EXT_hasModel_uri_s');
  $qp->solrParams['sort'] = "mods_titleInfo_title_ss asc";
  $qp->solrParams['facet.mincount'] = 1;
  $qp->executeQuery(FALSE);

  $results = array();
  foreach ($qp->islandoraSolrResult['facet_counts']['facet_fields'][$term] as $value => $count) {
    $results[] = array('label' => $value, 'count' => $count);
  }
  $_GET = $temp_get;
  sort($results);
  return $results;
}

/**
 * Rewrites the destination query.
 *
 * @param AbstractObject $source_object
 *   Tuque object representing source
 */
function emicdora_contextual_redirect($source_object, $delta) {
  $destination = '';
  if (isset($_GET['destination'])) {
    $destination = $_GET['destination'];
    $destination_parts = explode('/', $destination);
  }
  $get_parts = explode('/', $_GET['q']);

  $options = array('query' => array('destination' => $destination));
  unset($_GET['destination']);
  $pid = $source_object->id;
  switch ($delta) {
    case "edit":
      drupal_goto("islandora/edit_form/$pid/MODS", $options);
      break;

    case "refresh_transcription":
      $mvd = $destination_parts[4];
      drupal_goto("collation/$pid/refresh/$mvd/compare", $options);
      break;

    case "delete_mvd":
      $mvd = $destination_parts[4];
      drupal_goto("collation/$pid/delete/$mvd/compare", $options);
      break;

    case "delete_transcription":
      drupal_goto("islandora/object/$pid/delete", $options);
      break;

    case "edit_tei_rdf":
      drupal_goto("islandora/object/$pid/version/{$get_parts[3]}/edit");
      break;

    case "view_tei_rdf":
      drupal_goto("islandora/object/$pid/version/{$destination_parts[4]}");
      break;

    case "download_tei":
      drupal_goto("/islandora/object/{$get_parts[3]}/datastream/TEI/download");
      break;

    case "configure":
      drupal_goto("islandora/version_viewer/annotation/$pid/{$destination_parts[2]}");
      break;

    case "delete_tei":
      $versionable_object_pid = $get_parts[3];
      $versionable_object = islandora_object_load($get_parts[3]);
      $critical_editions = $versionable_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
      $critical_edition_pid = $critical_editions[0]['object']['value'];
      drupal_goto("emicdora/remove/version/$critical_edition_pid/$versionable_object_pid");
      break;

    case "purge_version":
      drupal_goto("islandora/object/$pid/delete", $options);
      break;

    default:
      return "This function has not yet been implemented";
  }
}
