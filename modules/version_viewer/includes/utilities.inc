<?php

/**
 * @file
 * Defines all the utility functions this module implements.
 */

/**
 * Prepares consolidated tei info.
 *
 * @param AbstractObject $object
 *   The object to prepare transformed TEI on.
 */
function versionable_object_viewer_prepare_page_tei_consolidated($object) {
  module_load_include('inc', 'php_lib', 'DOMHelpers');
  // $object is our versionable object.
  // $pageNumber is our current page
  //$pageNumber = urldecode($_GET['page']);

  // TODO: Use the '$pageNumber' and versionable object to find
  // the source, and return the matching page as display tei.
  // Return page level consolidated TEI.

//   $transformed_tei = versionable_object_viewer_display_page_tei($object, 'TEI');

//   $doc = new DOMDocument();
//   $doc->loadHTML($transformed_tei);

//   $return['transformed'] = $transformed_tei;
//   $return['pretty_print'] = dom_document_pretty_print($doc, TRUE);
dsm($object->id, "pid");
  $return['title'] = $object->label;
  $return['body'] = "This is body text to be updated.";
  return  drupal_json_output($return);
}

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
function versionable_object_viewer_display_page_tei($object, $dsid = 'CWRC') {
  module_load_include('inc', 'islandora', 'includes/authtokens');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/utilities');

  global $base_url;
  $islandora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
  $tomcat_url = str_replace('/fedora', '', $islandora_url);
  $pid = $object->id;

  $path = drupal_get_path('module', 'islandora_critical_edition_advanced');
  $xml_path = url("islandora/object/$pid/datastream/$dsid/view", array(
    'absolute' => TRUE,
    'query' => array(
      'token' => islandora_get_object_token($pid, $dsid),
    ),
  ));
  $xslt_path = "$base_url/$path/xml/EMiC_test_XSLTv2.xsl";
  return file_get_contents("$tomcat_url/saxon/SaxonServlet?source=$xml_path&style=$xslt_path");
}

/**
 * Returns detail meta data for a posted pid object.
 */
function versionable_object_viewer_prepare_detail_meta_data() {
  module_load_include('inc', 'islandora', 'includes/metadata');
  $pid = html_entity_decode(stripslashes($_GET['pid']), ENT_QUOTES, 'UTF-8');
  $versionable_object = islandora_object_load($pid);
  $data = islandora_retrieve_metadata_markup($versionable_object, TRUE);
  echo $data;//drupal_json_output();
}


function versionable_object_viewer_build_transcription_data($object) {
  $page = urldecode($_GET['page']);
  $versionable_object = islandora_object_load($object->id);
  $version_data = versionable_object_viewer_get_transcription_text($versionable_object);
  echo $version_data[$page - 1]['text'];
}

function versionable_object_viewer_build_tree($object) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  $anno_pids = islandora_image_annotation_shared_get_annotation_pids($object->id, FALSE);

  $tree_data = array();
  $count = 1;

  if (isset($anno_pids['types'])) {
    // Populate our different types of annotations.
    foreach ($anno_pids['types'] as $anno_type) {

      $tree_data[$count - 1] = array(
        "id" => $count,
        "text" => $anno_type,
        "state" => "closed",
        "attributes" => array(),
        "children" => array(),
      );

      foreach ($anno_pids['pids'] as $anno) {
        if ($tree_data[$count - 1]['text'] == $anno['type']) {
          $anno['svg_data'] = islandora_image_annotation_shared_get_annotation($anno['id'], NULL, FALSE);
          $svg_data = versionable_object_viewer_svg_data($anno['svg_data']);
          array_push(
            $tree_data[$count - 1]['children'],
            array(
              "id" => $anno['id'],
              "text" => $svg_data['label'],
              "state" => "open",
              "attributes" => $anno,
            )
          );
        }
      }
      $count++;
    }
  }

  echo drupal_json_encode($tree_data);
}

function versionable_object_viewer_svg_data($anno_svg) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');

  $annotation_xml = islandora_image_annotation_shared_transform_annotation_list($anno_svg, 'html');
  $xml = simplexml_load_string($annotation_xml);

  $labels = $xml->xpath('//dc:title');
  $type = $xml->xpath('//dc:type');
  $entity = $xml->xpath('//dcterms:relation');
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
 * @param AbstractObject $reference_object
 *   Object representing object from which this Versionable object is derived
 * @param array $variables
 *   Theme variables
 *
 * @return array
 *   Theme variables
 */
function versionable_object_viewer_build_version_media_view($reference_object, &$variables) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'islandora_critical_edition', 'theme/theme');
  module_load_include('inc', 'islandora_critical_edition', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  if ($variables['content_model'] == 'islandora:sp-audioCModel') {
    if (isset($reference_object['PROXY_MP3'])) {
      $audio_params = versionable_object_viewer_get_audio_params($reference_object);
    }
    // Thumbnail.
    if (isset($reference_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $reference_object['TN'])) {
      $tn_url = url("islandora/object/{$reference_object->id}/datastream/TN/view");
      $params = array(
        'title' => $reference_object->label,
        'path' => $tn_url,
      );
      $variables['islandora_thumbnail_img'] = theme('image', $params);
      $audio_params['tn'] = $tn_url;
    }
    //drupal_add_css("$critical_edition_module_path/css/islandora_critical_edition.css");
    $viewer = islandora_get_viewer($audio_params, 'islandora_audio_viewers', $reference_object);
    $variables['params'] = $audio_params;
    if ($viewer) {
      $variables['islandora_content'] = $viewer;
    }
    elseif (isset($variables['islandora_thumbnail_img']) && isset($islandora_object['PROXY_MP3']) &&
        islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['PROXY_MP3'])) {

      $variables['islandora_content'] = l($variables['islandora_thumbnail_img'], $audio_url, array('html' => TRUE));
    }
    elseif (isset($islandora_object['PROXY_MP3']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['PROXY_MP3'])) {
      $variables['islandora_content'] = l($islandora_object->label, $audio_url);
    }
  }
  if ($variables['content_model'] == 'islandora:sp_videoCModel') {
    $video_params = versionable_object_viewer_get_video_params($reference_object);
    $viewer = islandora_get_viewer($video_params, 'islandora_video_viewers', $reference_object);
    $variables['params'] = $video_params;
    if ($viewer) {
      $variables['islandora_content'] = $viewer;
    }
  }
  if ($variables['content_model'] == 'islandora:bookCModel') {

    $module_path = drupal_get_path('module', 'versionable_object_viewer');

    $pages = islandora_paged_content_get_pages($reference_object);
    $pids = array_keys($pages);

    // Ensure page pids have a 'CWRC' datastream.
    foreach ($pids as $pid) {
      $obj = islandora_object_load($pid);
      if (!isset($obj['CWRC'])) {
        //dsm($pid, "no cwrc data stream");
      }
    }
    dsm($pids, "pids");
    $variables['page_pids'] = $pids;

    $islandora_image_anno_path = drupal_get_path('module', 'islandora_image_annotation');

    $variables['tree_source'] = url("islandora/object/$pids[0]/get_tree_data");
    $variables['islandora_content'] = theme('islandora_anno_image_pane');

    drupal_add_js(array('islandora_image_annotation' => array('PID' => $pids[0])), 'setting');
    islandora_image_annotation_include_common_js();
    islandora_image_annotation_include_common_css();
    drupal_add_js($module_path . '/js/islandora_image_annotation_init.js');
  }
  return $variables;
}

function versionable_object_viewer_process_version_object_source(AbstractObject $versionable_object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
            PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
              SELECT ?obj ?type FROM <#ri> WHERE {
                <info:fedora/$versionable_object->id> <fedora-rels-ext:isDerivationOf> ?obj . ?obj <fedora-model:hasModel> ?type
              FILTER (?type != <info:fedora/fedora-system:FedoraObject-3.0>)
              }";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  return $results;
}

/**
 * Prepare the advanced view.
 *
 * @param unknown $object
 *   Fedora object to prepare the advanced view.
 */
function versionable_object_viewer_prepare_advanced($object) {
  global $base_url;
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/consolidate_tei');
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/utilities');

  $module_path = drupal_get_path('module', 'islandora_critical_edition_advanced');

  $data = $object->id;

  // Gather all transcriptions for this versionable object.
  $transcriptions_data = islandora_critical_get_related_transcriptions($data);

//   $transcriptions = array();
//   foreach ($transcriptions_data as $key => $value) {
//     $trans = islandora_object_load($key);
//     $transcription_details['title'] = $transcriptions_data[$key];
//     $transcription_details['text'] = $trans['TRANSCRIPTION']->content;
//     array_push($transcriptions, $transcription_details);
//   }
  $reference_object;

  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/consolidate_tei');

  $tei_rdfs = array_keys(islandora_critical_edition_get_members($data));
  // $player_params;
  if (!empty($tei_rdfs)) {
    $media_cmodels = array(
      'islandora:sp_videoCModel',
      'islandora:sp-audioCModel',
    );
    $tei_rdf = islandora_object_load($tei_rdfs[0]);
    $output;
    $rels = $tei_rdf->relationships->get(ISLANDORA_RELS_EXT_URI, ISLANDORA_CRITICAL_EDITION_IS_CRITICAL_EDITION_OF);
    if (is_array($rels)) {
      $reference_object = islandora_object_load($rels[0]['object']['value']);
      if ($reference_object) {
        $content_models = $reference_object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
        $content_model = $content_models[0]['object']['value'];
        if ($content_model == 'islandora:bookCModel') {
          // This is a ....sigh.. iFrame.
          $output = drupal_json_output(theme("islandora_critical_edition_advanced_detail_viewer", array(
            'islandora_object' => $tei_rdf,
            'media_viewer' => NULL,
            'player_params' => NULL,
            //'transcription_text' => $transcriptions,
            'module_base' => url($module_path),
          )));
          drupal_json_output($output);
          drupal_exit();
        }
        else {
          $variables['content_model'] = $content_model;
          $viewer = versionable_object_viewer_build_version_media_view($reference_object, $variables);
          drupal_json_output(theme("islandora_critical_edition_advanced_detail_viewer", array(
            'islandora_object' => $data,
            'media_viewer' => $viewer['islandora_content'],
            'player_params' => $viewer['params'],
            //'transcription_text' => $transcriptions,
            'module_base' => url($module_path),
          )));
          drupal_exit();
        }
      }
    }
  }
  // This is a catch all, to handle anything else with a basic TN.
  drupal_json_output(theme("islandora_critical_edition_advanced_detail_viewer", array(
    'islandora_object' => $data,
    'media_viewer' => url("islandora/object/$data/datastream/TN/view"),
    'player_params' => NULL,
    //'transcription_text' => $transcriptions,
    'module_base' => url($module_path),
    'player_ready' => FALSE,
  )));
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
