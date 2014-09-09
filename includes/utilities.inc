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
  $return = FALSE;
  // Ensure we are looking at the default edition view.
  if (isset($path_parts[2]) && !isset($path_parts[3])) {
    // Confirm the given objects content model.
    $object = islandora_object_load($path_parts[2]);
    if (in_array('islandora:criticalEditionContainerCModel', $object->models)) {
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
  $path = drupal_get_path('module', 'emicdora');
  drupal_add_js("$path/js/apparatus.js");
  $datastreams = emicdora_get_apparatus_datastreams($object);
  $content = array(
    'apparatus' => array(
      '#type' => 'vertical_tabs',
    ),
  );
  foreach ($datastreams as $dsid => $datastream) {
    // Do some shenanigans to make fragments nice.
    $fragment = strtolower(preg_replace('/[\s,]+/', '_', $datastream->label));
    $content['apparatus'][$dsid] = array(
      '#title' => $datastream->label,
      '#type' => 'fieldset',
      '#id' => $fragment,
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
 * Gets all versionable objects associated with critical edition.
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
  $markup_editor_module_path = drupal_get_path('module', 'islandora_markup_editor');
  $schema = file_create_url("$markup_editor_module_path/CWRC-Writer/src/schema/CWRC-TEIBasic.rng");
  $prefix = '<?xml version="1.0" encoding="UTF-8"?>
<?xml-model href="' . $schema . '" type="application/xml" schematypens="http://relaxng.org/ns/structure/1.0"?>
<TEI xml:id="struct_35" xmlns="http://www.tei-c.org/ns/1.0">
  <teiHeader xml:id="struct_530">
    <fileDesc xml:id="struct_531">
      <titleStmt xml:id="struct_532">
        <title xml:id="struct_533">Sample Document Title</title>
      </titleStmt>
      <publicationStmt xml:id="struct_534">
        <p xml:id="struct_535"/>
      </publicationStmt>
      <sourceDesc xml:id="struct_536" sameAs="http://www.cwrc.ca">
        <p xml:id="struct_537">Created from islandora book ingest unless otherwise noted.</p>
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
  $content = trim(preg_replace("/\n/", '<lb/>', $content));

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
