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
function emicdora_link_version_form($form, &$form_state, $critical_edition) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $module_path = drupal_get_path('module', 'emicdora');
  $destination_parts = explode('?', $_GET['destination']);
  $path_parts = explode('/', $destination_parts[0]);
  $source_pid = $path_parts[4];
  $rows = array();
  $header = array(
    'title' => array('data' => t('Version'), 'class' => array('class' => 'version_title')),
    'pid' => array('data' => t('PID'), 'class' => array('class' => 'version_pid')),
  );
  // We check the date here being bound as having a FILTER inside the
  // OPTIONAL as well as that the end was yielding some weird behavior.
  $versions_query = <<<EOQ
SELECT ?pid ?label FROM <#ri> WHERE {
  ?pid <fedora-rels-ext:isDerivationOf> <info:fedora/$source_pid> ;
       <fedora-model:label> ?label
  OPTIONAL {
    ?pid <fedora-rels-ext:isMemberOf> <info:fedora/$critical_edition->id> ;
         <fedora-model:createdDate> ?date
  }
  FILTER(!bound(?date))
}
EOQ;
  $results = $critical_edition->repository->ri->sparqlQuery($versions_query);
  foreach ($results as $result) {
    $pid = $result['pid']['value'];
    $rows[$pid] = array(
      'title' => array('data' => $result['label']['value'], 'class' => array('class' => 'version_title')),
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
    '#value' => t("Link version to @crit", array('@crit' => $critical_edition->label)),
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
function emicdora_link_version_form_submit($form, &$form_state) {
  $version_id = $form_state['values']['versions'];
  if ($version_id) {
    $version = islandora_object_load($version_id);
    $version->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $form_state['values']['edition_pid']);
  }
}
