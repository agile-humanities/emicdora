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

  dsm(array(), "hit setup");
  $stroke_width = variable_get('image_annotation_annotation_stroke_width', '1%');
  $critical_edition_relationships = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');

  dsm($critical_edition_relationships, "relationships");

  $critical_edition = $critical_edition_relationships[0]['object']['value'];
  $critical_edition_object = islandora_object_load($critical_edition);
  $title = $critical_edition_object->label;
  $page_results = islandora_paged_content_get_pages($critical_edition_object);
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
  $results['no_edit'] = !islandora_critical_edition_edit_cwrc_access($fedora_object);
  $results['page_count'] = count($pages);
  $results['islandora_anno_stroke_width'] = $stroke_width;

  dsm($results, "results");
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