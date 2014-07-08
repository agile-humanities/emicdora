<?php
/**
 * @file
 * Functions that implement ajax callbacks.
 */

/**
 * Provides all the parameters necessary for CWRCWriter to operate with Fedora.
 *
 * @global User $user
 *
 * @param AbstractObject $fedora_object
 *   The object to get the info on for the CWRC writer.
 */
function tei_editor_setup(AbstractObject $fedora_object) {
  global $user;
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $stroke_width = variable_get('image_annotation_annotation_stroke_width', '1%');
  $editor_object_relationships = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');

  $editor_object_pid = $editor_object_relationships[0]['object']['value'];
  $editor_object = islandora_object_load($editor_object_pid);
  $title = $editor_object->label;
  $page_results = islandora_paged_content_get_pages($editor_object);
  $pages = array();
  foreach ($page_results as $result) {
    $pages[$result['page']] = $result['pid'];
  }
  $position = array_search($fedora_object->id, $pages);
  $results = array();
  $results['uid'] = $user->uid;
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['title'] = $title;
  $results['no_edit'] = 1;
  $results['page_count'] = count($pages);
  $results['islandora_anno_stroke_width'] = $stroke_width;
  $place_entity_collection = variable_get('islandora_entities_places_collection', 'islandora:entity_collection');
  $results['create_entity_callbacks']['places'] = url('islandora/object/' . $place_entity_collection . '/manage/overview/ingest');
  $event_entity_collection = variable_get('islandora_entities_events_collection', 'islandora:entity_collection');
  $results['create_entity_callbacks']['events'] = url('islandora/object/' . $event_entity_collection . '/manage/overview/ingest');
  $organization_entity_collection = variable_get('islandora_entities_organizations_collection', 'islandora:entity_collection');
  $results['create_entity_callbacks']['organizations'] = url('islandora/object/' . $organization_entity_collection . '/manage/overview/ingest');
  $person_entity_collection = variable_get('islandora_entities_people_collection', 'islandora:entity_collection');
  $results['create_entity_callbacks']['people'] = url('islandora/object/' . $person_entity_collection . '/manage/overview/ingest');

  drupal_json_output($results);
}

/**
 * Ajax callback for Shared Canvas.
 *
 * Supplies the basic data, including manifest URI.
 *
 * @param AbstractObject $fedora_object
 *   The Fedora object to supply the data for
 */
function tei_editor_canvas_setup(AbstractObject $fedora_object) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $stroke_width = variable_get('image_annotation_annotation_stroke_width', '1%');
  $relationships = $fedora_object->relationships->get();

  $page_object_rels = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, "isDerivationOf");
  $page_object = islandora_object_load($page_object_rels[0]['object']['value']);
  $page_relationships = $page_object->relationships->get();
  $page_object_of_relationship = $page_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $book_object = islandora_object_load($page_object_of_relationship[0]['object']['value']);
  $pages = array_keys(islandora_paged_content_get_pages($book_object));
  $position = array_search($page_object->id, $pages);

  $results = islandora_image_annotation_canvas_init($page_object->id);
  $results['islandora_anno_stroke_width'] = $stroke_width;
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['no_edit'] = !islandora_critical_edition_edit_cwrc_access($fedora_object);
  $results['entities_search_callback'] = url('islandora/entities/search');
  drupal_json_output($results);
}

/**
 * Menu callback for the editor to persist TEI data.
 *
 * @param AbstractObject $fedora_object
 *   The fedora object
 */
function tei_editor_savedata(AbstractObject $fedora_object) {
  module_load_include('inc', 'islandora_markup_editor', 'includes/utilities');
  try {
    parse_str(file_get_contents('php://input'), $put);
    // @codingStandardsIgnoreStart
    $data = $put['text'];
    $cwrc = str_replace('<br>', '<br />', $data);
    // @codingStandardsIgnoreEnd
    $cwrc = str_replace('&', '&amp;', $cwrc);
    // Update ds.
    if (!isset($fedora_object["CWRC"])) {
      $obj_ds = $fedora_object->constructDatastream('CWRC', 'M');
      $fedora_object->ingestDatastream($obj_ds);
    }
    else {
      $obj_ds = $fedora_object["CWRC"];
    }
    $obj_ds->setContentFromString($cwrc);

    // Sanitize the schema URL, incase it gets managled during transport.
    $schema = html_entity_decode(stripslashes(check_plain($put['schema'])), ENT_QUOTES, 'UTF-8');
    islandora_markup_editor_add_tei_processing_instruction($fedora_object, $schema, "CWRC");

    drupal_json_output($cwrc);
  }
  catch (Exception $e) {
    watchdog('islandora_markup_editor',
      'Failed to set CWRC datastream content for pid @pid</br>code: @code<br/>message: @msg',
      array(
        '@pid' => $fedora_object->id,
        '@code' => $e->getCode(),
        '@msg' => $e->getMessage(),
      ),
      WATCHDOG_ERROR
    );
    drupal_json_output(array("Error" => "An error occured updating "));
  }
}
