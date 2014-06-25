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
  $pageNumber = urldecode($_GET['page']);

  // TODO: Use the '$pageNumber' and versionable object to find
  // the source, and return the matching page as display tei.
  // Return page level consolidated TEI.
  module_load_include('inc', 'islandora_critical_edition_advanced', 'includes/consolidate_tei');
  $transformed_tei = islandora_critical_edition_advanced_display_page_tei($object, 'TEI');

  $doc = new DOMDocument();
  $doc->loadHTML($transformed_tei);

  $return['transformed'] = $transformed_tei;
  $return['pretty_print'] = dom_document_pretty_print($doc, TRUE);
  return drupal_json_output($return);
}

/**
 * Returns detail meta data for a posted pid object.
 */
function versionable_object_viewer_prepare_detail_meta_data() {
  module_load_include('inc', 'islandora', 'includes/metadata');
  $pid = html_entity_decode(stripslashes($_GET['pid']), ENT_QUOTES, 'UTF-8');
  $versionable_object = islandora_object_load($pid);
  $data = islandora_retrieve_metadata_markup($versionable_object, TRUE);
  dsm($data, "data");
  echo $data;//drupal_json_output();
}


function versionable_object_viewer_build_transcription_data($object) {
  $page = urldecode($_GET['page']);
  //dsm($object->id, "object");
  $versionable_object = islandora_object_load($object->id);
  $version_data = versionable_object_viewer_get_transcription_text($versionable_object);
  //dsm($version_data, "vdata");
  //dsm($page, "data");
  echo $version_data[$page - 1]['text'];
}

function versionable_object_viewer_build_tree($object) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  $anno_pids = islandora_image_annotation_shared_get_annotation_pids($object->id, FALSE);

  $tree_data = array();
  $count = 1;

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
