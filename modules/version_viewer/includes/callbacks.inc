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
  $rels = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageNumber');
  $page_number = $rels[0]['object']['value'];
  try {
    $contents = $object['CWRC']->content;
    $contents = preg_replace('/<\?xml-model .*\?>/', '', $contents);
    $doc = new DOMDocument();
    $doc->loadXML($contents);
    if ($page_number != '1') {
      $headers = $doc->getElementsByTagName('teiHeader');
      foreach ($headers as $header) {
        $header->parentNode->removeChild($header);
      }
    }
    $rdf_nodes = $doc->getElementsByTagNameNS('http://www.w3.org/1999/02/22-rdf-syntax-ns#', 'RDF');
    foreach ($rdf_nodes as $node) {
      $node->parentNode->removeChild($node);
    }
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
function versionable_object_viewer_prepare_detail_metadata() {
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
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $page = urldecode($_GET['page']);
  $members = versionable_object_viewer_get_members($object);
  $pids = array();
  // Ensure page pids have a 'TEI_RDF' datastream.
  foreach ($members as $member) {
    array_push($pids, $member['obj']['value']);
  }
  $derivation_object = islandora_object_load($pids[$page - 1]);
  $tran_data = versionable_object_viewer_display_page_tei($object, $derivation_object, 'CWRC');
  $return['body'] = $tran_data;
  $return['title'] = $derivation_object->label . " (page $page)";
  drupal_json_output($return);
}

/**
 * Construct the version objects viewers tree (callback).
 *
 * @param AbstractObject $object
 *   The versionable object in use in the viewer.
 * @param AbstractObject $derivation_object
 *   The versionable objects source object to use.
 */
function versionable_object_viewer_build_tree($object, $derivation_object, $version_pid = NULL) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  module_load_include('inc', 'versionable_object_viewer', 'includes/utilities');

  $rdf_annotations = versionable_object_viewer_read_rdf($derivation_object);
  $tree_data = versionable_object_viewer_flatten_rdf_annos($rdf_annotations);
  $text_to_image_annotations = array();
  foreach ($tree_data as $folder) {
    foreach ($folder['children'] as $child) {
      $cwrc_attributes = $child['attributes']['cwrcAttributes'];
      if (isset($cwrc_attributes->attributes->id)) {
        $text_to_image_annotations[] = $cwrc_attributes->attributes->id;
      }
    }
  }
  $anno_pids = islandora_image_annotation_shared_get_annotation_pids($object->id, FALSE);
  if (isset($anno_pids['pids'])) {
    $query = db_select('versionable_object_access')
        ->fields('versionable_object_access', array(
          'pid',
          'allowed_users',
          'allowed_roles',
            )
        )
        ->condition('pid', $version_pid, '=');
    $row = $query->execute()->fetchObject();
    $size = count($tree_data);
    foreach ($anno_pids['pids'] as $key => $plain_annotation) {
      $annotation = islandora_object_load($plain_annotation['id']);
      if ($row) {
        $users = unserialize($row->allowed_users);
        $roles = unserialize($row->allowed_roles);
        $owner = user_load_by_name($annotation->owner);
        $owner_roles = isset($owner->roles) ? $owner->roles : array();
        $user_not_in_list = in_array($owner->uid, $users) ? FALSE : TRUE;
        $roles_not_in_list = array_intersect($roles, array_keys($owner_roles)) ? FALSE : TRUE;
        if ($user_not_in_list && $roles_not_in_list) {
          unset($anno_pids['pids'][$key]);
          $size = count($tree_data);
          continue;
        }
      }
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
            $tree_data[$size]["children"], $child
        );
      }
    }
  }
  drupal_json_output($tree_data);
}
