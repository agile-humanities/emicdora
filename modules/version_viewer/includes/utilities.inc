<?php

/**
 * @file
 * Defines all the utility functions this module implements.
 */

/**
 * Returns transformed from CWRC datatstream.
 *
 * @global type $base_url
 *
 * @param AbstractObject $object
 *   Object representing fedora page object
 * @param string $dsid
 *   Datastream ID of object
 *
 * @return string
 *   transformed xml
 */
function versionable_object_viewer_display_page_tei($object, $dsid = 'TEI_RDF') {
  $callable_mapping = array(
    'rd' => 'versionable_object_viewer_display_page_tei_reading',
    'dt' => 'versionable_object_viewer_display_page_tei_diplomatic',
  );
  $type = isset($_GET['type']) ? $_GET['type'] : 'rd';
  $callable = $callable_mapping[$type];

  return $callable($object[$dsid]);
}

/**
 * Render transcription markup.
 *
 * @param AbstractDatastream $datastream
 *   The datastream containing the TEI for which to produce a transcription.
 * @param string $type
 *   The type of transcription. At time of writing, we support:
 *   - reading
 *   - diplomatic
 *
 * @return string
 *   A string of HTML to output... Likely with significant styling.
 */
function versionable_object_viewer_render_transcription(AbstractDatastream $datastream, $type) {
  $path = drupal_get_path('module', 'versionable_object_viewer');
  $input = new DOMDocument();
  $input->loadXML($datastream->content);
  $wrapper_xslt_doc = new DOMDocument();
  $wrapper_xslt_doc->load("$path/tei_transforms/wrap.xslt");
  $wrapper_xslt = new XSLTProcessor();
  $wrapper_xslt->importStylesheet($wrapper_xslt_doc);
  $wrapped = $wrapper_xslt->transformToDoc($input);

  module_load_include('inc', 'versionable_object_viewer', 'includes/tei_refcounter');
  \VersionableObjectViewer\count_refs($wrapped);
  $unwrapper_xslt_doc = new DOMDocument();
  $unwrapper_xslt_doc->load("$path/tei_transforms/unwrap.xslt");
  $unwrapper_xslt = new XSLTProcessor();
  $unwrapper_xslt->importStylesheet($unwrapper_xslt_doc);
  $unwrapper_xslt->setParameter('', 'type', $type);
  $tempfile = drupal_tempnam('public://', 'tei');
  $unwrapper_xslt->transformToUri($wrapped, $tempfile);

  $islandora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  $tomcat_url = str_replace('/fedora', '', $islandora_url);
  $saxon_url = url("$tomcat_url/saxon/SaxonServlet", array(
    'absolute' => TRUE,
    'external' => TRUE,
    'query' => array(
      'style' => file_create_url("$path/tei_transforms/tei2html.xslt"),
      'source' => file_create_url($tempfile),
      'clear-stylesheet-cache' => 'yes',
      'css_base' => file_create_url("$path/tei_transforms"),
      'type' => $type,
    ),
  ));
  $result = drupal_http_request($saxon_url);

  return $result->data;
}

/**
 * Callback for versionable_object_viewer_display_page_tei for reading display.
 *
 * @param AbstractDatastream $datastream
 *   The datastrean to use to generate the display.
 *
 * @return string
 *   A string of markup to use, representing the reading display.
 */
function versionable_object_viewer_display_page_tei_reading(AbstractDatastream $datastream) {
  return versionable_object_viewer_render_transcription($datastream, 'reading');
}

/**
 * Callback for diplomatic display.
 *
 * @param AbstractDatastream $datastream
 *   The datastrean to use to generate the display.
 *
 * @return string
 *   A string of markup to use, representing the diplomatic display.
 */
function versionable_object_viewer_display_page_tei_diplomatic(AbstractDatastream $datastream) {
  return versionable_object_viewer_render_transcription($datastream, 'diplomatic');
}

/**
 * Retrieve rdf entity data.
 *
 * @param AbstractObject $object
 *   Fedora object with rdf datastream(s).
 * @param string $dsid
 *   The datastream id, defaults to 'CWRC'.
 *
 * @return array
 *   The array of rdf data entries.
 */
function versionable_object_viewer_read_rdf($object, $dsid = 'CWRC') {
  $entities = array();
  // RDF is embedded in the TEI.
  $xml = simplexml_load_string($object[$dsid]->content);
  $xml->registerXPathNamespace('rdf', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
  $rdf = $xml->xpath('//rdf:RDF');
  if (!$rdf) {
    return array();
  }
  $xml = simplexml_load_string(reset($rdf)->asXML());
  // Fetches each annotation.
  $annotations = $xml->xpath("//rdf:Description[rdf:type/@rdf:resource = 'http://www.w3.org/ns/oa#Annotation']");
  foreach ($annotations as $annotation) {
    $body = $annotation->xpath('oa:hasBody/@rdf:resource');
    $body = (string) reset($body);
    $type = $xml->xpath("//rdf:Description[@rdf:about = '$body']/rdf:type[@rdf:resource != 'http://www.w3.org/ns/oa#SemanticTag']/@rdf:resource");
    $type = (string) reset($type);
    $target = $annotation->xpath('oa:hasTarget/@rdf:resource');
    $target = (string) reset($target);
    $selector = $xml->xpath("//rdf:Description[@rdf:about = '$target']/oa:hasSelector/@rdf:resource");
    $selector = (string) reset($selector);
    $selection = $xml->xpath("//rdf:Description[@rdf:about = '$selector']/rdf:value");
    $selection = (string) reset($selection);
    $cwrc_info = $annotation->xpath('cw:cwrcInfo');
    $cwrc_info = json_decode((string) reset($cwrc_info));
    $cwrc_attributes = $annotation->xpath('cw:cwrcAttributes');
    $cwrc_attributes = json_decode((string) reset($cwrc_attributes));
    // JS code expects the following variables to be in these places.
    $cwrc_attributes->cwrcInfo = $cwrc_info;
    $cwrc_attributes->attributes->uuid = $cwrc_attributes->attributes->id;
    $matches = array();
    preg_match('/@annotationId="(.*)"/', $selection, $matches);
    $entities[] = array(
      'id' => $matches[1],
      'type' => $type,
      'text' => $cwrc_info->name,
      'attributes' => array(
        'annotationId' => $matches[1],
        'cwrcType' => $type == 'http://xmlns.com/foaf/0.1/Image' ? 'textimagelink' : '',
        'cwrcAttributes' => $cwrc_attributes,
      ),
    );
  }
  return $entities;
}

/**
 * Extract svg data for a given svg datastreams content.
 *
 * @param string $anno_svg
 *   The svg annotaton data to parse
 *
 * @return array
 *   The svg data as 'type', 'label' and 'text' (in array).
 */
function versionable_object_viewer_svg_data($anno_svg) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');

  $annotation_xml = islandora_image_annotation_shared_transform_annotation_list($anno_svg, 'html');
  $xml = simplexml_load_string($annotation_xml);

  $labels = $xml->xpath('//dc:title');
  $type = $xml->xpath('//dc:type');
  $text = $xml->xpath('//cnt:chars');

  $anno_type = dom_import_simplexml($type[0])->textContent;
  $anno_label = dom_import_simplexml($labels[0])->textContent;
  $anno_text = dom_import_simplexml($text[0])->textContent;

  $svg_info = array(
    'type' => $anno_type,
    'label' => $anno_label,
    'text' => $anno_text,
  );

  return $svg_info;
}

/**
 * Builds viewer for Media objects.
 *
 * @param AbstractObject $version
 *   Object representing Versionable object
 * @param array $variables
 *   Theme variables
 *
 * @return array
 *   Theme variables
 */
function versionable_object_viewer_build_version_media_view($version, &$variables) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  // Use this to determine what to show by default.
  $variables['hide_components'] = array(
    'n' => 'false',
    's' => 'false',
    'e' => 'false',
    'w' => 'false',
  );
  drupal_add_js(array('versionable_object_viewer' => array('mode' => $variables['content_model_type'])), 'setting');
  if ($variables['content_model_type'] == 'audio') {
    $data = $version->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
    $source = islandora_object_load($data[0]['object']['value']);
    $variables['show_components']['w'] = 'true';
    if (isset($source['PROXY_MP3'])) {
      $audio_params = versionable_object_viewer_get_audio_params($source);
    }
    // Thumbnail.
    if (isset($source['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $source['TN'])) {
      $tn_url = url("islandora/object/{$source->id}/datastream/TN/view");
      $params = array(
        'title' => $source->label,
        'path' => $tn_url,
      );
      $variables['islandora_thumbnail_img'] = theme('image', $params);
      $audio_params['tn'] = $tn_url;
    }

    $viewer = islandora_get_viewer($audio_params, 'islandora_audio_viewers', $source);
    $variables['params'] = $audio_params;
    if ($viewer) {
      $variables['islandora_content'] = $viewer;
    }
    elseif (isset($variables['islandora_thumbnail_img']) && isset($source['PROXY_MP3']) &&
        islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $source['PROXY_MP3'])) {

      $variables['islandora_content'] = l($variables['islandora_thumbnail_img'], $audio_params['url'], array('html' => TRUE));
    }
    elseif (isset($source['PROXY_MP3']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $source['PROXY_MP3'])) {
      $variables['islandora_content'] = l($source->label, $audio_params['url']);
    }
    $variables['pager_data'] = versionable_object_viewer_get_transcription_text($version);
  }
  if ($variables['content_model_type'] == 'video') {
    $variables['show_components']['w'] = 'true';
    $data = $version->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
    $source = islandora_object_load($data[0]['object']['value']);

    $video_params = versionable_object_viewer_get_video_params($source);
    $viewer = islandora_get_viewer($video_params, 'islandora_video_viewers', $source);
    $variables['params'] = $video_params;
    if ($viewer) {
      $variables['islandora_content'] = $viewer;
    }
    $variables['pager_data'] = versionable_object_viewer_get_transcription_text($version);
  }
  if ($variables['content_model_type'] == 'image') {

    $variables['show_components']['w'] = 'true';
    $module_path = drupal_get_path('module', 'versionable_object_viewer');

    $data = $version->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
    $source = islandora_object_load($data[0]['object']['value']);
    $variables['islandora_content'] = theme('islandora_anno_image_pane');
    $variables['pager_data'] = versionable_object_viewer_get_transcription_text($version);
    drupal_add_js(array('islandora_image_annotation' => array('PID' => $source->id)), 'setting');

    islandora_image_annotation_include_common_js();
    islandora_image_annotation_include_common_css();

    drupal_add_js($module_path . '/js/islandora_image_annotation_init.js');
  }
  if ($variables['content_model_type'] == 'text') {

    $module_path = drupal_get_path('module', 'versionable_object_viewer');
    $members = versionable_object_viewer_get_members($variables['islandora_object']);
    $pids = array();
    // Ensure page pids have a 'TEI_RDF' datastream.
    foreach ($members as $member) {
      array_push($pids, $member['obj']['value']);
    }
    $derivation_pids = array();
    foreach ($pids as $pid) {
      $page_derivation = islandora_object_load($pid);
      $page_derivation_source = $page_derivation->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
      $pg_derivation_pid = $page_derivation_source[0]['object']['value'];
      array_push($derivation_pids, $pg_derivation_pid);
    }
    $variables['pager_data'] = $pids;
    drupal_add_js(array('versionable_object_viewer' => array('tei_rdf_pids' => $pids)), 'setting');
    drupal_add_js(array('versionable_object_viewer' => array('pids' => $derivation_pids)), 'setting');
    $variables['tree_source'] = url("islandora/object/$derivation_pids[0]/get_tree_data/{$version->id}");
    $variables['islandora_content'] = theme('islandora_anno_image_pane');
    drupal_add_js(array('islandora_image_annotation' => array('PID' => $derivation_pids[0])), 'setting');
    islandora_image_annotation_include_common_js();
    islandora_image_annotation_include_common_css();
    drupal_add_js($module_path . '/js/islandora_image_annotation_init.js');
  }
  return $variables;
}

/**
 * Find all 'isMemberOf' relationships for the given object.
 *
 * @param AbstractObject $object
 *   Fedora object instance.
 * @param string $content_model
 *   OPTIONAL, defaults to 'islandora:tei-rdfCModel'.
 *
 * @return array
 *   The query results as an array.
 */
function versionable_object_viewer_get_members(AbstractObject $object, $content_model = 'islandora:tei-rdfCModel') {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
            PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
            SELECT ?obj FROM <#ri> WHERE {
              ?obj <fedora-rels-ext:isMemberOf> <info:fedora/$object->id> .
              ?obj <fedora-model:hasModel> <info:fedora/$content_model>.
            }";
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  return $results;
}

/**
 * Find all 'isDerivationOf' relations for a given versionable object.
 *
 * @param AbstractObject $versionable_object
 *   An instance of a versionable object.
 *
 * @return array
 *   Query results in an array.
 */
function versionable_object_viewer_process_version_object_source(AbstractObject $versionable_object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
            PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
              SELECT ?obj ?type FROM <#ri> WHERE {
              <info:fedora/$versionable_object->id> <fedora-rels-ext:isDerivationOf> ?obj . ?obj <fedora-model:hasModel> ?type
              FILTER (?type != <info:fedora/fedora-system:FedoraObject-3.0>)
              }";
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  return $results;
}

/**
 * Get audio params for jwplayer.
 *
 * @param AbstractObject $reference_object
 *   object representing fedora audio object
 *
 * @return array
 *   Associative array containing params.
 */
function versionable_object_viewer_get_audio_params($reference_object) {
  $audio_url = url("islandora/object/{$reference_object->id}/datastream/PROXY_MP3/view", array('absolute' => TRUE));
  $audio_params = array(
    "pid" => $reference_object->id,
    "url" => $audio_url,
    "mime" => 'audio/mpeg',
    "width" => "100%",
  );
  return $audio_params;
}

/**
 * Get video params for jwplayer.
 *
 * @param AbstractObject $reference_object
 *   object representing fedora audio object
 *
 * @return array
 *   Associative array containing params.
 */
function versionable_object_viewer_get_video_params($reference_object) {
  if (stristr($_SERVER['HTTP_USER_AGENT'], 'chrome') !== FALSE) {
    $viewer_dsid = 'OGG';
  }
  else {
    $viewer_dsid = 'MP4';
  }

  $video_params = array(
    'pid' => $reference_object->id,
  );
  // Video player.
  if (isset($reference_object[$viewer_dsid]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $reference_object[$viewer_dsid])) {
    $video_url = url("islandora/object/{$reference_object->id}/datastream/$viewer_dsid/view");
    $video_params += array(
      'mime' => 'video/mp4',
      'url' => $video_url,
      'width' => "100%",
    );
  }
  // Thumbnail.
  if (isset($reference_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $reference_object['TN'])) {
    $video_params += array(
      'tn' => url("islandora/object/{$reference_object->id}/datastream/TN/view", array('absolute' => TRUE)),
    );
  }
  return $video_params;
}

/**
 * Gets all transcriptions related to this object.
 *
 * @param string $versionable_object_pid
 *   PID of object
 *
 * @return array
 *   Array of pid=>label pairs
 */
function versionable_object_viewer_get_related_transcriptions($versionable_object_pid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "
            PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">

            SELECT DISTINCT ?subject ?label FROM <#ri>
                WHERE {
                        {?subject fm:hasModel <info:fedora/islandora:transcriptionCModel>;
                                  fr:isMemberOf <info:fedora/$versionable_object_pid>;
                       }
                 OPTIONAL{
                           ?subject fm:label  ?label;
                          }
                       }";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    $objects[$result['subject']['value']] = $result['label']['value'];
  }
  return $objects;
}

/**
 * Get all transcriptions for a given versionable object.
 *
 * @param AbstractObject $object
 *   A versionable object.
 *
 * @return array
 *   An array of transcriptions, with 'title' and 'text' transcription keys.
 */
function versionable_object_viewer_get_transcription_text($object) {
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');

  // Gather all transcriptions for this versionable object.
  $transcriptions_data = versionable_object_viewer_get_related_transcriptions($object->id);
  $transcriptions = array();
  foreach ($transcriptions_data as $key => $value) {
    $trans = islandora_object_load($key);
    $transcription_details['title'] = $transcriptions_data[$key];
    $transcription_details['text'] = $trans['TRANSCRIPTION']->content;
    array_push($transcriptions, $transcription_details);
  }
  return $transcriptions;
}

/**
 * Flatten all annotations for use with the version viewer 'tree'.
 *
 * @param array $annotations
 *   A zero based indexed array of entities/annotations.
 *
 * @return array
 *   An indexed array of annotations for use with the version viewer.
 */
function versionable_object_viewer_flatten_rdf_annos($annotations) {
  $tree_data = array();
  foreach ($annotations as $annotation) {
    if ($annotation['attributes']['cwrcType'] == 'textimagelink') {
      if (!isset($tree_data["textimagelink"])) {
        $tree_data["textimagelink"] = array(
          "id" => "tree_textimagelinks",
          "text" => "Text Image Annotations",
          "state" => "closed",
          "attributes" => array(
            "root" => TRUE,
          ),
          "children" => array(),
        );
      }
      $child = array(
        "id" => $annotation['id'],
        "text" => $annotation['text'],
        "state" => "open",
        "attributes" => $annotation['attributes'],
      );
      array_push(
          $tree_data["textimagelink"]["children"], $child
      );
    }
    else {
      if (!isset($tree_data['entity'])) {
        $tree_data['entity'] = array(
          "id" => "tree_entities",
          "text" => "Entities",
          "state" => "closed",
          "attributes" => array(
            "root" => TRUE,
          ),
          "children" => array(),
        );
      }
      $child = array(
        "id" => $annotation['id'],
        "text" => $annotation['text'],
        "state" => "open",
        "attributes" => $annotation['attributes'],
      );
      array_push(
          $tree_data['entity']["children"], $child
      );
    }
  }
  // The following sillyness ensures keys are numeric.
  foreach ($tree_data as $k => $v) {
    $new_key = array_search($k, array_keys($tree_data));
    $tree_data[$new_key] = $v;
  }
  unset($tree_data['entity'], $tree_data['textimagelink']);
  return $tree_data;
}
