<?php

/**
 * @file
 * Handles the management of transcription objects.
 */

/**
 * The form for creating a transcription object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_emicdora_add_transcription_form($form, &$form_state, AbstractObject $object) {

  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  drupal_add_css(drupal_get_path('module', 'emicdora') . "/css/fix_textfield_width.css");
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('txt');
  $form = array();
  $input_types = array();
  $form['#attached']['js'] = array(drupal_get_path('module', 'emicdora') . '/js/born_digital_transcription.js');
  $form['transcription'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of new transcription'),
    '#size' => 40,
    '#default_value' => t('Transcription of') . ' ' . $object->label,
  );

  $page_results = islandora_paged_content_get_pages($object);
  $count = count($page_results);

  $input_types['none'] = t('Create born-digital transcription');
  $input_types['file'] = t('Upload transcription from file');
  $input_types['tei_rdf'] = t('Create transcription from TEI-RDF Object');

  $form['version'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['page_count'] = array(
    '#type' => 'hidden',
    '#value' => $count,
  );

  $form['versionable_object_pid'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['content_source'] = array(
    '#type' => 'radios',
    '#options' => $input_types,
    '#default_value' => 'none',
  );

  $form['none'] = array(
    '#type' => 'textarea',
    '#size' => 6,
    '#title' => t('Transcription content'),
    '#default_value' => "",
    '#states' => array(
      'visible' => array(
        ':input[name="content_source"]' => array('checked' => TRUE),
      ),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="content_source"]' => array('value' => 'none'),
      ),
    ),
  );

  if ($count > 1) {
    $form['page'] = array(
      '#type' => 'fieldset',
      '#title' => t('Select page range'),
      '#description' => t('You may optionally restrict the pages used int this transcription'),
      '#states' => array(
        'visible' => array(
          ':input[name="content_source"]' => array('value' => 'tei_rdf'),
        ),
      ),
    );
    $form['page']['restricted'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use restricted page range'),
      '#default_value' => FALSE,
    );
    $form['page']['first_page'] = array(
      '#type' => 'textfield',
      '#size' => 6,
      '#title' => t('First page'),
      '#default_value' => '1',
      '#states' => array(
        'visible' => array(
          ':input[name="restricted"]' => array('checked' => TRUE),
        ),
      ),
    );
    $form['page']['last_page'] = array(
      '#type' => 'textfield',
      '#size' => 6,
      '#title' => t('Last page'),
      '#default_value' => $count,
      '#states' => array(
        'visible' => array(
          ':input[name="restricted"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $form['file'] = array(
    '#type' => 'managed_file',
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="content_source"]' => array('value' => 'file'),
      ),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Transcription'),
  );
  return $form;
}

/**
 * Validates Drupal Form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function islandora_emicdora_add_transcription_form_validate($form, &$form_state) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $versionable_object = islandora_object_load($form_state['values']['version']);
  $parents = $versionable_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $critical_edition = islandora_object_load($parents[0]['object']['value']);
  $versions = emicdora_get_raw_objects($critical_edition, 'version');
  $transcription_data = array();
  foreach ($versions as $type) {
    foreach ($type as $version) {
      $versionable_objects[] = $version;
      $version_data = emicdora_get_raw_objects(islandora_object_load($version), 'transcriptions');
      $transcription_data = array_merge($transcription_data, $version_data);
    }
  }
  $current_transcriptions = array();
  foreach ($transcription_data as $pid => $values) {
    foreach ($values as $value) {
      $current_transcriptions[] = $value['label'];
    }
  }
  $page_count = $form_state['values']['page_count'];
  if (in_array($form_state['values']['transcription'], $current_transcriptions)) {
    form_set_error('transcription', t('A transcription with this name is already associated with this versionable object.'));
  }

  if ($form_state['values']['content_source'] == 'tei_rdf' && $page_count > 1) {
    $start_page = $form_state['values']['first_page'];
    $last_page = $form_state['values']['last_page'];
    $length = $form_state['values']['last_page'];
    if (intval($start_page) == 0) {
      form_set_error('first_page', t('Number between 1 and @count required.', array('@count' => $length)));
    }
    if (intval($last_page) == 0) {
      form_set_error('last_page', t('Number between 1 and @count required.', array('@count' => $length)));
    }
    if ($last_page < $start_page) {
      form_set_error('last_page', t('Ending page number must be lower than starting page.'));
    }
  }
}

/**
 * Submit handler for Transcription creation.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form_state
 */
function islandora_emicdora_add_transcription_form_submit(array $form, array &$form_state) {
  $versionable_object_pid = $form_state['values']['versionable_object_pid'];
  $versionable_object = islandora_object_load($versionable_object_pid);
  $label = $form_state['values']['transcription'];
  $content_source = $form_state['values']['content_source'];
  $transcription = emicdora_build_transcription($versionable_object, $label);
  $datastream_id = "TRANSCRIPTION";
  $control_group = "M";
  $transcription_datastream = $transcription->constructDatastream($datastream_id, $control_group);
  $transcription_datastream->mimetype = 'text/plain';
  $transcription_datastream->label = 'Transcription';
  if ($content_source == 'tei_rdf') {
    // Cant trust form state data for page min/max range, so
    // get it agian to be sure.
    $page_results = islandora_paged_content_get_pages($versionable_object);
    $start_page = 1;
    $end_page = count($page_results);
    if (isset($form_state['values']['restricted']) && $form_state['values']['restricted']) {
      $start_page = $form_state['values']['first_page'];
      $end_page = $form_state['values']['last_page'];
    }
    islandora_add_object($transcription);
    emicdora_add_derived_transcription($page_results, $versionable_object_pid, $transcription, $transcription_datastream, $start_page, $end_page);
  }
  else {
    if ($content_source == 'file') {
      $file = file_load($form_state['values']['file']);
      $path = drupal_realpath($file->uri);
      $transcription_datastream->setContentFromFile($path);
      file_delete($file);
    }

    if ($content_source == 'none') {
      $transcription_datastream->setContentFromstring($form_state['values']['none']);
    }

    $transcription->ingestDatastream($transcription_datastream);
    islandora_add_object($transcription);
    drupal_set_message(t("Transcription object created"));
  }
}

/**
 * The form for creating a transcription object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_manage_transcriptions_form(array $form, array &$form_state, AbstractObject $object) {
  $form = array();
  $rows = array();
  $versionable_object_pid = $object->id;
  $transcriptions = islandora_critical_get_related_transcriptions($versionable_object_pid);
  $members = emicdora_get_members($object->id);
  $cwrcwriter = array_search('islandora:criticalEditionContainerCModel', $members);
  foreach ($transcriptions as $pid => $label) {
    $rows[$pid] = array(
      'pid' => $pid,
      'title' => $label,
    );
  }
  $header = array(
    'title' => array('data' => t('Transcription Title')),
    'pid' => array('data' => t('PID')),
  );
  // Build and return table element.
  $table_element = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t("This versionable object has no transcription objects."),
    '#js' => FALSE,
    '#multiple' => FALSE,
  );
  $form['intro'] = array(
    '#type' => 'markup',
    '#markup' => t('Choose transcription.'),
  );
  $form['transcription'] = $table_element;
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Transcription'),
    '#attributes' => array('source' => 'delete_transcription', 'class' => array('transcription_submit')),
  );
  $form['edit'] = array(
    '#type' => 'submit',
    '#value' => t('Edit Transcription'),
    '#attributes' => array('source' => 'edit_transcription', 'class' => array('transcription_submit')),
  );
  if ($cwrcwriter) {
    $form['cwrc_update'] = array(
      '#type' => 'submit',
      '#value' => t('Synchronize with TEI-RDF object'),
      '#attributes' => array('source' => 'cwrc_update', 'class' => array('transcription_submit')),
    );

    $form['cwrcwriter'] = array(
      '#type' => 'hidden',
      '#value' => $cwrcwriter,
    );
  }
  return $form;
}

/**
 * Drupal Form Validation.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal Form State
 */
function emicdora_manage_transcriptions_form_validate(array $form, array &$form_state) {
  if (!$form_state['values']['transcription']) {
    form_set_error('transcription', t('Please select a transcription.'));
  }
}

/**
 * Submission handler handler for transcription management form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function emicdora_manage_transcriptions_form_submit(array $form, array &$form_state) {
  module_load_include('module', 'islandora');
  $action = $form_state['triggering_element']['#attributes']['source'];
  if (isset($form_state['values']['transcription'])) {
    $transcription_pid = $form_state['values']['transcription'];
  }
  else {
    return;
  }

  switch ($action) {
    case 'delete_transcription':
      islandora_delete_object(islandora_object_load($transcription_pid));
      break;

    case 'edit_transcription':
      drupal_goto("islandora/transcription/edit/$transcription_pid");
      break;

    case 'cwrc_update':
      $cwrcwriter = $form_state['values']['cwrcwriter'];
      islandora_update_from_tei_rdf_object($cwrcwriter, $transcription_pid);
      break;
  }
}

/**
 * Form to update transcription.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form_state
 * @param abstractObject $transcription
 *   transcription object
 */
function islandora_critical_transcription_edit_form($form, &$form_state, $transcription) {
  drupal_set_title($transcription->label);
  $content = "";
  if ($transcription['TRANSCRIPTION']) {
    $content = $transcription['TRANSCRIPTION']->content;
  }
  $form = array();
  $form['transcription'] = array(
    '#title' => t('Current Transcription'),
    '#type' => 'textarea',
    '#default_value' => $content,
    '#rows' => 40,
    '#cols' => 20,
    '#attributes' => array('class' => array('transcription_window')),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $transcription->id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Transcription'),
  );
  return $form;
}

/**
 * Submit handler for transcription edit form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form
 */
function islandora_critical_transcription_edit_form_submit($form, &$form_state) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $transcription_object = islandora_object_load($form_state['values']['pid']);
  $success = FALSE;
  $content = $form_state['values']['transcription'];

  try {
    if ($transcription_object['TRANSCRIPTION']) {
      $success = $transcription_object['TRANSCRIPTION']->content = $content;
    }
    else {
      $datastream_id = "TRANSCRIPTION";
      $control_group = "M";
      $transcription_datastream = $transcription_object->constructDatastream($datastream_id, $control_group);
      $transcription_datastream->mimetype = 'text/plain';
      $transcription_datastream->label = 'Transcription';
      $transcription_datastream->setContentFromstring($content);
      $success = $transcription_object->ingestDatastream($transcription_datastream);
    }
  }
  catch (Exception $exc) {
    watchdog_exception("islandora critical edition advanced", $exc);
  }
  if ($success) {
    drupal_set_message(t("Transcription updated."));
    emicdora_calliope_write_file($transcription_object->id);
  }
}

/**
 * Adds transcription from CWRC datastreams of CWRCWriter page objects.
 *
 * @param AbstractObject $pages
 *   An indexed array of pages
 * @param string $versionable_object_pid
 *   PID of Versionable Object
 * @param AbstractObject $transcription
 *   Object representing Transcription Object
 * @param IslandoraFedoraDatastream $transcription_datastream
 *   Datastream object to be updated
 */
function emicdora_add_derived_transcription($pages, $versionable_object_pid, $transcription, $transcription_datastream, $start_page, $end_page) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');

  $offset = $start_page - 1;
  $range = $end_page - $offset;
  $pages = array_slice($pages, $offset, $range);

  $batch = array(
    'title' => t('Creating Transcription from TEI-RDF Object'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/add_transcription.form.inc',
    'progress_message' => t('@current of @total pages processed.'),
  );
  $flat_pid = islandora_escape_pid_for_function($versionable_object_pid);

  $filename = file_create_filename($flat_pid . "tempcwrc", 'temporary://');
  foreach ($pages as $page) {
    $batch['operations'][] = array('emicdora_build_text_file', array(
        'page_object' => islandora_object_load($page['pid']),
        'filename' => $filename));
  }
  $batch['operations'][] = array('emicdora_transcription_file_add',
    array(
      'transcription' => $transcription,
      'transcription_datastream' => $transcription_datastream,
      'filename' => $filename));
  batch_set($batch);
  batch_process();
}

/**
 * Builds temp file with TEI-RDF data from page level objects.
 *
 * @param AbstractObject $object
 *   Object representing page
 * @param string $filename
 *   Temp file to hold data
 */
function emicdora_build_text_file($object, $filename) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  if (isset($object['CWRC'])) {
    $text = emicdora_convert_tei_to_plain_text($object['CWRC']->content);
    file_put_contents($filename, $text, FILE_APPEND | LOCK_EX);
  }
}

/**
 * Adds completed file to Transcription Object.
 *
 * @param AbstractObject $transcription
 *   Uningested object representing transcription
 * @param IslandoraFedoraDatastream $transcription_datastream
 *   Transcription datastream
 * @param string $filename
 *   file to be added
 */
function emicdora_transcription_file_add($transcription, $transcription_datastream, $filename) {
  module_load_include('inc', 'collation', 'includes/calliope');
  $transcription_datastream->setContentFromFile($filename);
  $transcription->ingestDatastream($transcription_datastream);
  emicdora_calliope_write_file($transcription->id);
  unlink($filename);
}

/**
 * Populates transcription with current text from TEI-RDF object.
 *
 * @param string $tei_rdf_object_pid
 *   PID of RDF-TEI Object
 * @param string $transcription_pid
 *   PID of Transcription Object being updated
 */
function islandora_update_from_tei_rdf_object($tei_rdf_object_pid, $transcription_pid) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tei_rdf_object = islandora_object_load($tei_rdf_object_pid);
  $transcription = islandora_object_load($transcription_pid);
  $pages = array_keys(islandora_paged_content_get_pages($tei_rdf_object));
  $batch = array(
    'title' => t('Creating Transcription from TEI-RDF Object'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'emicdora_advanced') . '/includes/transcriptions.inc',
    'progress_message' => t('@current of @total pages processed.'),
  );

  $flat_pid = islandora_escape_pid_for_function($transcription_pid);
  $filename = file_create_filename($flat_pid . "tempcwrc", 'temporary://');
  foreach ($pages as $page) {
    $batch['operations'][] = array('emicdora_build_text_file', array(
        'page_object' => islandora_object_load($page),
        'filename' => $filename));
  }
  $batch['operations'][] = array('islandora_transcription_file_update',
    array(
      'transcription' => $transcription,
      'filename' => $filename));
  batch_set($batch);
  batch_process();
}

/**
 * Updates completed file to Transcription Object.
 *
 * @param AbstractObject $transcription
 *   Transcription Object
 * @param string $filename
 *   file to be added
 */
function islandora_transcription_file_update($transcription, $filename) {
  $transcription['TRANSCRIPTION']->setContentFromFile($filename);
  unlink($filename);
}

/**
 * Builds and ingest transcription object.
 *
 * @param AbstractObject $versionable_object
 *   object representing enclising Versionable Object
 * @param string $label
 *   Label for new Transcription
 *
 * @return AbsgtractObject
 *   Transcription object
 */
function emicdora_build_transcription($versionable_object, $label) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $transcription = emicdora_construct_transcription($versionable_object, $label);
  return $transcription;
}
