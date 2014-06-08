<?php

/**
 * @file
 * Add a source object to the given Critical Edition and the CO-OP.
 */

/**
 * Form for ingesting / adding a new source object to a critical edition.
 *
 * @param string $content_model
 *   The identifier of the Content Model type to ingest.
 * @param string $critical_edition
 *   The identifier of the critical edition the source will be linked to.
 *
 * @return array
 *   A Drupal Form for ingesting a new object of the given type.
 */
function emicdora_add_source_form($content_model, $critical_edition) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'emicdora', 'includes/breadcrumb');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  global $user;
  $source_content_models = emicdora_source_content_models();
  $ingestible = in_array($content_model, array_keys($source_content_models));
  if (!$ingestible || !emicdora_is_critical_edition($critical_edition)) {
    drupal_not_found();
  }
  $label = $source_content_models[$content_model];
  $collection = emicdora_get_default_collection($content_model);
  $tuque = islandora_get_tuque_connection();
  $object = $tuque->repository->constructObject('islandora');
  $object->owner = $user->name;
  $object->label = t('New @type', array('@type' => $label));
  $object->models = $content_model;
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collection);
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition->id);
  drupal_set_title(t('Add New @type to @label', array('@type' => $label, '@label' => $critical_edition->label)));
  emicdora_set_source_breadcrumbs($critical_edition);
  return drupal_get_form('islandora_ingest_form', array(
    'parent' => $collection,
    'models' => array($content_model),
    'objects' => array($object)));
}
