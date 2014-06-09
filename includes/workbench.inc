<?php

/**
 * @file
 * Menu callback and related helper functions for the workbench.
 */

/**
 * Display's the current users work bench.
 *
 * @return array
 *   A Drupal renderable array, that displays the current users workbench.
 */
function emicdora_workbench() {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $source_content_models = emicdora_source_content_models();
  $add_source_links = array();
  foreach ($source_content_models as $content_model => $label) {
    $add_source_links[] = array(
      'title' => t('Add @label', array('@label' => $label)),
      'href' => "workbench/add/$content_model",
    );
  }
  return array(
    'description' => array(
      '#type' => 'markup',
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => t('Welcome to your workbench. Here you can create new critical editions, access active editions, or remove editions from your workbench. Or you can ingest new source objects and manage collections in the Co-op.'),
    ),
    'actions' => array(
      'add_new_critical_edition' => array(
        '#theme' => 'links',
        '#links' => array(
          array(
            'title' => t('Add Critical Edition'),
            'href' => 'workbench/add/islandora:criticalEditionCModel',
          ),
        ),
      ),
      'add_source' => array(
        '#theme' => 'links',
        '#links' => $add_source_links,
      ),
    ),
  );
}

/**
 * Defines the set of objects which are allowed to be ingestible.
 *
 * @return array
 *   An associative array where the key values are ingestible content models,
 *   and the values are human readable label for the content model.
 */
function emicdora_workbench_ingestible_content_models() {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  return array(
    'islandora:criticalEditionCModel' => t('Critical Edition'),
  ) + emicdora_source_content_models();
}

/**
 * Display form for ingesting an object of the given content model.
 *
 * @param string $content_model
 *   The PID of the Content Model type to ingest.
 *
 * @return array
 *   A Drupal Form for ingesting a new object of the given type.
 */
function emicdora_workbench_ingest_form($content_model) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $ingestible_content_models = emicdora_workbench_ingestible_content_models();
  if (!in_array($content_model, array_keys($ingestible_content_models))) {
    drupal_not_found();
  }
  $label = $ingestible_content_models[$content_model];
  $collection = emicdora_get_default_collection($content_model);
  $tuque = islandora_get_tuque_connection();
  $object = $tuque->repository->constructObject('islandora');
  $object->owner = $user->name;
  $object->label = t('New @type', array('@type' => $label));
  $object->models = $content_model;
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collection);
  drupal_set_title(t('Add New @type', array('@type' => $label)));
  return drupal_get_form('islandora_ingest_form', array(
    'parent' => $collection,
    'models' => array($content_model),
    'objects' => array($object)));
}
