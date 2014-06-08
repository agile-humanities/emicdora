<?php

/**
 * @file
 * Add a version object to the given Critical Edition / Source object.
 */

/**
 * Form to ingest / add a version object to a critical edition and source.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition to add the version to.
 * @param AbstractObject $source
 *   The source from which to derive the version.
 *
 * @return array
 *   A Drupal Form for ingesting a new version.
 */
function emicdora_add_version_form(AbstractObject $critical_edition, AbstractObject $source) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'emicdora', 'includes/breadcrumb');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  global $user;
  if (!emicdora_is_critical_edition($critical_edition) || !emicdora_is_source($critical_edition)) {
    drupal_not_found();
  }
  $tuque = islandora_get_tuque_connection();
  $object = $tuque->repository->constructObject('islandora');
  $object->owner = $user->name;
  $object->label = t('New Version of @source', array('@source' => $source->label));
  $object->models = 'islandora:versionCModel';
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $critical_edition->id);
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isDerivationOf', $source->id);
  drupal_set_title(t('New Version of @source to @edition', array('@source' => $source->label, '@edition' => $critical_edition->label)));
  emicdora_set_source_breadcrumbs($critical_edition, $source);
  return drupal_get_form('islandora_ingest_form', array(
    'models' => array('islandora:versionCModel'),
    'objects' => array($object)));
}
