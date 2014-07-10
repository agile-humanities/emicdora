<?php

/**
 * @file
 * Defines all the callbacks this module implements.
 */

/**
 * Prepare the page level consolidated TEI for the version viewer.
 *
 * @param AbstractObject $object
 *   The versionable object to process.
 */
function versionable_object_viewer_prepare_page_tei_consolidated($object) {
  module_load_include('inc', 'php_lib', 'DOMHelpers');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');
  try {
    $transformed_tei = versionable_object_viewer_display_page_tei($object, "CWRC");

    $doc = new DOMDocument();
    $doc->loadXML($transformed_tei);

    $pretty_printed = dom_document_pretty_print($doc, TRUE);

    $return['body'] = $pretty_printed;
    $return['title'] = $object->label;

    return drupal_json_output($return);
  }
  catch (Exception $e) {
    return drupal_json_output($e);
  }
}

/**
 * Returns detail meta data for a posted pid object.
 */
function versionable_object_viewer_prepare_detail_meta_data() {
  module_load_include('inc', 'islandora', 'includes/metadata');
  $pid = html_entity_decode(stripslashes($_GET['pid']), ENT_QUOTES, 'UTF-8');
  $versionable_object = islandora_object_load($pid);
  $data = islandora_retrieve_metadata_markup($versionable_object, TRUE);
  echo $data;
}

/**
 * Gather transcription data for a given versionable object.
 *
 * @param AbstractObject $object
 *   The versionable object to process transcriptions.
 */
function versionable_object_viewer_build_transcription_data($object) {
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');

  $page = urldecode($_GET['page']);
  $type = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasSourceType');
  $transcriptions = versionable_object_viewer_get_transcription_text($object);
  switch ($type[0]['object']['value']) {
    case "audio":
      $return['body'] = $transcriptions[$page - 1]['text'];
      $return['title'] = $transcriptions[$page - 1]['title'];
      echo json_encode($return);
      break;

    case "text":
      $members = versionable_object_viewer_get_members($object);
      $pids = array();
      // Ensure page pids have a 'TEI_RDF' datastream.
      foreach ($members as $member) {
        array_push($pids, $member['obj']['value']);
      }
      $derivation_object = islandora_object_load($pids[$page - 1]);
      $tran_data = versionable_object_viewer_display_page_tei($derivation_object, 'CWRC');
      $return['body'] = $tran_data;
      $return['title'] = $derivation_object->label . " (page $page)";
      $return['transcriptions'] = $transcriptions;
      echo json_encode($return);
      break;

    case "video":
      $return['body'] = $transcriptions[$page - 1]['text'];
      $return['title'] = $transcriptions[$page - 1]['title'];
      echo json_encode($return);
      break;

    case "image":
      $return['body'] = $transcriptions[$page - 1]['text'];
      $return['title'] = $transcriptions[$page - 1]['title'];
      echo json_encode($return);
      break;

  }
}

/**
 * Construct the version objects viewers tree (callback).
 *
 * @param AbstractObject $object
 *   The versionable object in use in the viewer.
 * @param AbstractObject $derivation_object
 *   The versionable objects source object to use.
 */
function versionable_object_viewer_build_tree($object, $derivation_object) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');

  $anno_pids = islandora_image_annotation_shared_get_annotation_pids($object->id, FALSE);
  $count = 0;
  $tree_data = array();

  // Construct our image annotations.
  if (isset($anno_pids['types'])) {
    // Populate our different types of annotations.
    foreach ($anno_pids['types'] as $anno_type) {
      if (count($anno_pids['pids']) > 0) {
        $tree_data[$count] = array(
          "id" => $count,
          "text" => $anno_type,
          "state" => "closed",
          "attributes" => array(
            "root" => "true",
          ),
          "children" => array(),
        );
        foreach ($anno_pids['pids'] as $anno) {
          if ($tree_data[$count]['text'] == $anno['type']) {
            $anno['svg_data'] = islandora_image_annotation_shared_get_annotation_data($anno['id'], NULL);
            $svg_data = versionable_object_viewer_svg_data($anno['svg_data']);
            array_push(
              $tree_data[$count]['children'],
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
  }
  $rdf_annotations = versionable_object_viewer_read_rdf($derivation_object);
  if (count($rdf_annotations) > 0) {
    $size = count($tree_data);
    $tree_data[$size] = array(
      "id" => "tree_entities",
      "text" => "Entities",
      "state" => "closed",
      "attributes" => array(
        "root" => "true",
      ),
      "children" => array(),
    );
    foreach ($rdf_annotations as $rdf_anno) {
      // For non TextImgLink annotations.
      if (isset($rdf_anno->annotationId)) {
        array_push(
          $tree_data[$size]["children"],
          array(
            "id" => $rdf_anno->annotationId,
            "text" => $rdf_anno->cwrcAttributes->cwrcInfo->name,
            "state" => "open",
            "attributes" => $rdf_anno,
          )
        );
      }

    }
  }
  drupal_json_output($tree_data);
  drupal_exit();
}
