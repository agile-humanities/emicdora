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

  $count = 0;
  $tree_data = array();
  $rdf_annotations = versionable_object_viewer_read_rdf($derivation_object);
  if (count($rdf_annotations) > 0) {
    $size = count($tree_data);
    foreach ($rdf_annotations as $rdf_anno) {
      // For non TextImgLink annotations.
      $child_type = $size;
      if (isset($rdf_anno->annotationId)) {
        $child;
        // Entities can now be text image links.
        if ($rdf_anno->cwrcType == "textimagelink") {
          $child_type = $size;
          if (!isset($tree_data[$child_type])) {
            $tree_data[$child_type] = array(
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
            "id" => $rdf_anno->annotationId,
            "text" => $rdf_anno->cwrcAttributes->cwrcInfo->name,
            "state" => "open",
            "attributes" => $rdf_anno,
          );
        }
        else {
          $child_type = $size;
          if (!isset($tree_data[$child_type])) {
            $tree_data[$child_type] = array(
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
            "id" => $rdf_anno->annotationId,
            "text" => $rdf_anno->cwrcAttributes->cwrcInfo->name,
            "state" => "open",
            "attributes" => $rdf_anno,
          );
        }
        array_push(
          $tree_data[$child_type]["children"],
          $child
        );
      }
    }
  }

  $anno_pids = islandora_image_annotation_shared_get_annotation_pids($object->id, FALSE);
  if (isset($anno_pids['pids'])) {
    $size = count($tree_data);
    foreach ($anno_pids['pids'] as $plain_annotation) {
      if ($plain_annotation['type'] != "TextImageLink") {
        if (!isset($tree_data[$size])) {
          $tree_data[$size] = array(
            "id" => "tree_imageannotations",
            "text" => "Image Annotations",
            "state" => "closed",
            "attributes" => array(
              "root" => TRUE,
            ),
            "children" => array(),
          );
        }
        $ds_content = islandora_image_annotation_get_annotation_data($plain_annotation['id']);
        $xml = simplexml_load_string($ds_content);
        $body_urn_array = $xml->xpath("//oa:hasBody");
        $body_urn = (string) $body_urn_array[0]->attributes('rdf', 'about');
        $content_node = $xml->xpath("//rdf:Description[@rdf:about = '$body_urn']//cnt:chars");
        // Description.
        $description = (string) $content_node[0][0];
        $real_target = $xml->xpath("//rdf:Description");
        // UUID.
        $uuid = (string) $real_target[0]->attributes('rdf', 'about');
        $uuid = str_replace("urn:uuid:", "", $uuid);
        $created_array = $xml->xpath("//dcterms:created");
        // Created.
        $created = (string) $created_array[0];
        $title_array = $xml->xpath("//dc:title");
        // Title.
        $title = (string) $title_array[0];
        $type_array = $xml->xpath("//dc:type");
        // Type.
        $type = (string) $type_array[0];
        $target_array = $xml->xpath("//oa:hasTarget");
        // target.
        $target = (string) $target_array[0]->attributes('rdf', 'about');
        // Build the child image annotation.
        $child = array(
          "id" => $uuid,
          "text" => $title,
          "state" => "open",
          "attributes" => array(
            'uuid' => $uuid,
            'created' => $created,
            'title' => $title,
            'type' => $type,
            'cwrcInfo' => array(
              'description' => $description,
            ),
            'cwrcType' => 'imageannotation',
          ),
        );
        array_push(
          $tree_data[$size]["children"],
          $child
        );
      }
    }
  }

  drupal_json_output($tree_data);
  drupal_exit();
}
