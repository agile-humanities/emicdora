<?php

/**
 * @file
 * Link a source object with the given Critical Edition.
 */

/**
 * Form for associating an existing version to a critical edition.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $critical_edition
 *   The critical edition the version will be linked to.
 *
 * @return array
 *   A Drupal Form for linking a new object of the given type.
 */
function emicdora_link_version_form($form, $form_state, $critical_edition) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $module_path = drupal_get_path('module', 'emicdora');
  drupal_add_css("$module_path/css/version_table.css");
  $destination_parts = explode('?', $_GET['destination']);
  $path_parts = explode('/', $destination_parts[0]);
  $source_pid = $path_parts[4];
  $rows = array();
  $header = array(
    'title' => array('data' => t('Version'), 'class' => array('class' => 'version_title')),
    'pid' => array('data' => t('PID'), 'class' => array('class' => 'version_pid')),
  );
  $existing_versions = emicdora_get_raw_objects($critical_edition, 'versions');
  $current = array();
  foreach ($existing_versions as $type) {
    foreach ($type as $pid) {
      $current[] = $pid;
    }
  }
  foreach (emicdora_get_versions($source_pid) as $label => $pid) {
    if (in_array($pid, $current)) {
      continue;
    }
    $rows[$pid] = array(
      'title' => array('data' => $label, 'class' => array('class' => 'version_title')),
      'pid' => array('data' => $pid, 'class' => array('class' => 'version_pid')),
    );
  }

  $form['edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => $critical_edition->id,
  );

  $form['versions'] = array(
    '#type' => 'tableselect',
    '#title' => t('Select Versions'),
    '#description' => t('Selected version(s) will be associated with your Critical Edition'),
    '#multiple' => FALSE,
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No TEI-RDF objects available. Please select "Create New"'),
    '#attributes' => array('class' => array('version_select_table')),
    '#id' => 'version_select_table',
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Link version to critical edition"),
  );
  return $form;
}

/**
 * Submit handler.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_link_version_form_submit($form, $form_state) {
  $version_id = $form_state['values']['versions'];
  if ($version_id) {
    $version = islandora_object_load($version_id);
    $version->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $form_state['values']['edition_pid']);
  }
}
