<?php

/**
 * @file
 * Handles the ingestion of versionable objects.
 */

/**
 * Defines the allowed Content-Models for Source Objects.
 *
 * @return array
 *   An associative array where the keys are the Content Model PID and the
 *   values are label's for the Content Model.
 */
function emicdora_valid_source_types() {
  return array(
    'islandora:bookCModel' => t('Text'),
    'islandora:sp-audioCModel' => t('Audio'),
    'islandora:sp_videoCModel' => t('Video'),
    'islandora:sp_large_image_cmodel' => t('Image'),
  );
}

/**
 * Local menu action to present an ingest versionable object form.
 *
 * @param AbstractObject $edition
 *   The critical edition to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function emicdora_ingest_versionable_object_page(AbstractObject $edition) {
  global $user;

  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $version = $edition->repository->constructObject(islandora_get_namespace($edition->id));
  $version->owner = $user->name;
  $version->label = 'New Version';
  $version->models = 'islandora:versionCModel';
  $version->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $edition);
  drupal_set_title(t('Add Versionable Object to @edition', array('@edition' => $edition->label)));
  $form = drupal_get_form('islandora_ingest_form', array(
    'models' => array('islandora:versionCModel'),
    'objects' => array($version),
  ));

  // XXX: Allow later redirects to work properly... shouldn't actually be
  // present.
  unset($_GET['destination']);

  return $form;
}

/**
 * Autocomplete menu callback for source objects.
 */
function emicdora_versionable_object_form_source_autocomplete($type, $search) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $type = trim(urldecode($type));
  // List of approved source content model types.
  $models = array_keys(emicdora_source_content_models());
  $models = in_array($type, $models) ? array($type) : $models;
  $where_clause = '';
  foreach ($models as $model) {
    $where_clause .= (!empty($where_clause)) ? 'UNION ' : '';
    $where_clause .= "{ ?object fedora-model:hasModel <info:fedora/$model> . }\n";
  }
  $where_clause = substr($where_clause, 0, -1);
  $query = <<<EOT
    PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
    PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
    PREFIX islandora: <http://islandora.ca/ontology/relsext#>
      SELECT DISTINCT ?object ?label FROM <#ri> WHERE {
        $where_clause
        OPTIONAL { ?object fedora-model:label  ?label. }
        FILTER (regex(?label, "{$search}", "i")) .
      }
EOT;
  $tuque = islandora_get_tuque_connection();
  // @todo If time permits swap this out for a more flexable solr serach.
  $results = $tuque->repository->ri->sparqlQuery($query);
  $output = array();
  foreach ($results as $result) {
    $key = $result['object']['value'];
    $output[$key] = $result['label']['value'] . " ($key)";
  }
  drupal_json_output($output);
}

/**
 * Defines the ingest Versionable Object form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $source_id
 *   A string containing the PID of the source object for which a versionable
 *   object is being created.
 *
 * @return array
 *   The Drupal form definition.
 */
function emicdora_versionable_object_form(array $form, array &$form_state, $source_id) {
  form_load_include($form_state, 'inc', 'emicdora', 'includes/utilities');
  form_load_include($form_state, 'inc', 'islandora_paged_content', 'includes/utilities');

  $get_default = function ($field_name, $default) use ($form_state) {
    return isset($form_state['values'][$field_name]) ?
      $form_state['values'][$field_name] :
      $default;
  };
  $step_storage =& islandora_ingest_form_get_step_storage($form_state);
  $step_storage['source'] = $source_id;
  $source = islandora_object_load($source_id);

  // Build the list rows for the TEI-RDF tableselect, and count the number.
  $page_count = 0;
  $tei_rdf_objects = array();
  if ($source) {
    $versions = array_keys(emicdora_get_members($source->id, 'islandora:versionCModel', 'isDerivationOf'));
    // Not efficent we should create a separate function in
    // islandora_paged_content to compute the number of pages.
    $page_count = count(islandora_paged_content_get_pages($source));

    foreach ($versions as $version) {
      $version_object = islandora_object_load($version);
      $tei_rdf_objects[$version] = array($version, $version_object->label);
    }
  }
  $tei_rdf_source = $get_default('tei_rdf_source', 'new');
  $transcription_source = $get_default('transcription_source', 'tei-rdf');
  // Can only select page ranges for books, and if the islandora_paged_content
  // module is enabled.
  $can_limit_page_range = $source && in_array('islandora:bookCModel', $source->models);
  $limit_page_range = $can_limit_page_range && $get_default('limit_page_range', FALSE);
  // Upload Transcription.
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('txt');
  $valid_source_selected = $source ? TRUE : FALSE;
  // Opted to use #ajax over #states in most cases as #states doesn't support
  // conditionally #required values.
  return array(
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'emicdora') . '/js/ingest_versionable_object.js' => array(
          'type' => 'file',
          'weight' => 5,
        ),
      ),
    ),
    'source' => array(
      '#type' => 'hidden',
      '#value' => $source->id,
      '#access' => isset($source),
    ),
    'source_ajax_wrapper' => array(
      '#type' => 'container',
      '#prefix' => '<div id="source-ajax-container-wrapper">',
      '#suffix' => '</div>',
      'page_range' => array(
        '#access' => $can_limit_page_range,
        '#type' => 'fieldset',
        '#title' => t('Select page range'),
        '#description' => t('You may optionally limit the pages used in the TEI-RDF and transcription.'),
        '#prefix' => '<div id="page-ajax-container-wrapper">',
        '#suffix' => '</div>',
        'limit_page_range' => array(
          '#type' => 'checkbox',
          '#title' => t('Limit the page range'),
          '#default_value' => FALSE,
          '#ajax' => array(
            'callback' => 'emicdora_page_ajax_container',
            'wrapper' => 'page-ajax-container-wrapper',
            'method' => 'replace',
            'effect' => 'fade',
          ),
        ),
        'first' => array(
          '#access' => $limit_page_range,
          '#type' => 'textfield',
          '#size' => 6,
          '#title' => t('First page'),
          '#default_value' => 1,
          '#required' => TRUE,
        ),
        'last' => array(
          '#access' => $limit_page_range,
          '#type' => 'textfield',
          '#size' => 6,
          '#title' => t('Last page'),
          '#default_value' => $page_count,
          '#required' => TRUE,
        ),
      ),
      'tei_rdf_source' => array(
        '#access' => $valid_source_selected,
        '#disabled' => empty($tei_rdf_objects),
        '#description' => check_plain(empty($tei_rdf_objects) ? t('No existing TEI-RDF found.') : NULL),
        '#type' => 'radios',
        '#title' => 'TEI-RDF',
        '#required' => TRUE,
        '#options' => array('new' => t('Create New'), 'existing' => t('Use Existing')),
        '#default_value' => 'new',
        '#ajax' => array(
          'callback' => 'emicdora_tei_rdf_ajax_container',
          'wrapper' => 'tei-rdf-ajax-container-wrapper',
          'method' => 'replace',
          'effect' => 'fade',
        ),
      ),
      'tei_rdf_ajax_wrapper' => array(
        '#access' => $valid_source_selected,
        '#type' => 'container',
        '#prefix' => '<div id="tei-rdf-ajax-container-wrapper">',
        '#suffix' => '</div>',
        'tei_rdf' => array(
          '#prefix' => '<div id="version_select_table">',
          '#suffix' => '</div>',
          '#access' => $tei_rdf_source == 'existing',
          '#type' => 'tableselect',
          '#multiple' => FALSE,
          '#header' => array('PID', 'Version Title'),
          '#options' => $tei_rdf_objects,
          '#empty' => t('No TEI-RDF objects available. Please select "Create New"'),
        ),
      ),
      'transcription_label' => array(
        '#access' => $valid_source_selected,
        '#type' => 'textfield',
        '#required' => TRUE,
        '#title' => t('Transcription Label'),
      ),
      'transcription_source' => array(
        '#access' => $valid_source_selected,
        '#type' => 'radios',
        '#default_value' => 'tei-rdf',
        '#options' => array(
          'tei-rdf' => t('Create transcription from TEI-RDF Object'),
          'digital' => t('Create born-digital transcription'),
          'upload' => t('Upload transcription from file'),
        ),
      ),
      'transcription' => array(
        '#access' => $valid_source_selected,
        '#title' => t('Current Transcription'),
        '#type' => 'textarea',
        '#default_value' => t('Enter Transcription here.'),
        '#rows' => 40,
        '#cols' => 20,
        '#attributes' => array('class' => array('transcription_window')),
        '#states' => array(
          'visible' => array(
            ':input[name="transcription_source"]' => array('value' => 'digital'),
          ),
        ),
      ),
    ),
    // The pluploader doesn't respect #states, so we must wrap it in a
    // container.
    'transcription_upload' => array(
      '#title' => t('Upload Transcription'),
      '#type' => 'managed_file',
      '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="transcription_source"]' => array('value' => 'upload'),
        ),
      ),
    ),
  );
}

/**
 * Validate handler for the ingest Versionable Object form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_versionable_object_form_validate(array $form, array &$form_state) {
  $values = $form_state['values'];
  // Validate Source Object.
  $source = islandora_object_load($values['source']);
  // It must be a Fedora Object.
  if (!($source instanceof AbstractObject)) {
    form_error($form['source'], t('Source Object: "@source" is not valid object.', array(
      '@source' => $values['source'])));
  }
  else {
    $valid_source_types = emicdora_valid_source_types();
    $sources_valid_models = array_intersect(array_keys($valid_source_types), $source->models);
    // It must be of a valid source type.
    if (empty($sources_valid_models)) {
      form_error($form['source'], t('Source Object: "@source" must be one of the following types: @types.', array(
        '@source' => $values['source'],
        '@types' => implode(', ', $valid_source_types))));
    }
  }
  // Validate TEI-RDF object, if "use existing" was selected.
  if ($values['tei_rdf_source'] == 'existing') {
    if (empty($values['tei_rdf'])) {
      // We don't put the form error on the tableselect as that is not supported
      // by the Drupal Form API.
      form_error($form, t('Select an existing TEI-RDF object or create a new one.'));
    }
    else {
      $tei_rdf = islandora_object_load($values['tei_rdf']);
      // Must exist and actually be an TEI-RDF object.
      if (!$tei_rdf || !in_array('islandora:versionCModel', $tei_rdf->models)) {
        // We don't put the form error on the tableselect as that is not
        // supported by the Drupal Form API.
        form_error($form, t('TEI-RDF Object: "@tei_rdf" is not valid object.', array(
          '@tei_rdf' => $values['tei_rdf'])));
      }
    }
  }
  // Validate Page Range.
  if ($values['limit_page_range']) {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    if ($values['tei_rdf_source'] == 'new') {
      $page_count = count(islandora_paged_content_get_pages($source));
    }
    else {
      $tei_rdf = islandora_object_load($values['tei_rdf']);
      $page_count = count(islandora_paged_content_get_pages($tei_rdf));
    }
    foreach (array('first', 'last') as $element) {
      $numeric = is_numeric($values[$element]);
      $out_of_range = $numeric ? $values[$element] < 1 || $values[$element] > $page_count : FALSE;
      if ($out_of_range) {
        $element = $form['source_ajax_wrapper']['page_range'][$element];
        form_error($element, t('@name must be a number between 1 - @max.', array('@name' => $element['#title'], '@max' => $page_count)));
      }
    }
    if ((int) $values['last'] < (int) $values['first']) {
      $last = $form['source_ajax_wrapper']['page_range']['last'];
      $first = $form['source_ajax_wrapper']['page_range']['first'];
      form_error($last, t('@last must be less than @first.', array('@last' => $last['#title'], '@first' => $first['#title'])));
    }
  }
  // Validate Upload.
  if ($values['transcription_source'] == 'upload') {
    // Handels pluploader custom behavoir.
    $file_uploaded = isset($form_state['input']['edit-transcription-upload_count']) && ((int) $form_state['input']['edit-transcription-upload_count'] !== 0);
    $file_uploaded = $file_uploaded || (file_load($values['transcription_upload']) !== FALSE);
    if (!$file_uploaded) {
      // We handle the logic for #required here, allowing us to use #states on
      // the conditional transcription form elements.
      $element = $form['transcription_upload'];
      form_error($element, t('@title field is required.', array('@title' => $element['#title'])));
    }
  }
}

/**
 * Submit form for ingest Versionable Object form.
 *
 * Creates a single Versionable Object (islandora:versionCModel) and
 * either creates or links an existing TEI-RDF Object
 * (islandora:criticalEditionContainerCModel) with the Verisonable Object. Also
 * generates a Transcription Object (islandora:transcriptionCModel) from the
 * TEI-RDF Object and links it with the Versionable Object.
 *
 * It's assumed that the next form page will be the versionable objects
 * Metadata.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_versionable_object_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'emicdora', 'includes/transcriptions');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $values = $form_state['values'];
  $shared_storage =& islandora_ingest_form_get_shared_storage($form_state);
  $step_storage =& islandora_ingest_form_get_step_storage($form_state);
  $source = islandora_object_load($step_storage['source']);
  $tn = $source['TN'] ? $source['TN']->content : NULL;
  // Get the versionable object and it's critical edition.
  $versionable_object = islandora_ingest_form_get_object($form_state);
  $tn_datastream = $versionable_object->constructDatastream('TN', 'M');
  $tn_datastream->setContentFromString($tn);
  $tn = '';
  $versionable_object->ingestDatastream($tn_datastream);
  $versionable_object->relationships->add(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF, $source->id);
  $ingest_objects = &islandora_ingest_form_get_objects($form_state);
  $parents = $versionable_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $critical_edition = $parents[0]['object']['value'];

  // Inherit source XACML.
  $versionable_object->relationships->add(ISLANDORA_RELS_EXT_URI, 'inheritXacmlFrom', $source->id, RELS_TYPE_URI);

  // Link to an existing TEI-RDF Object.
  if ($values['tei_rdf_source'] == 'existing') {
    $tei_rdf = islandora_object_load($values['tei_rdf']);
    $tei_pages = array_keys(emicdora_get_members($tei_rdf->id, 'islandora:tei-rdfCModel', 'isDerivationOf'));
    $tei_pages = $values['limit_page_range'] ? array_slice($tei_pages, $values['first'] - 1, $values['last'] - $values['first'] + 1) : $tei_pages;
    // Clone existing tei-rdf if selected.
    if (!empty($tei_pages)) {
      $batch = array(
        'title' => t('Cloning selected TEI-RDF Object.'),
        'file' => drupal_get_path('module', 'emicdora') . '/includes/ingest_versionable_object.form.inc',
        'progress_message' => t('@current of @total pages processed.', array('@total' => count($tei_pages))),
        'operations' => array(),
      );
      foreach ($tei_pages as $tei_page) {
        $batch['operations'][] = array(
          'emicdora_copy_versionable_object_tei_rdf_page_batch_operation',
          array($tei_page, $versionable_object->id),
        );
      }
      batch_set($batch);
    }
  }
  // Create a new TEI-RDF object.
  else {
    $pages = array_values(islandora_paged_content_get_pages($source));
    // @todo We need to migrate the existing data at some point in the future
    // The following property should not be a literal value, but we must make
    // it so for backwards compatibility.
    // TEI-RDF objects associated with books require a batch process to be
    // created, all the files created in the batch will be added to the
    // $form_state, to be ingested when islandora_ingest_form is submited.
    $pages = $values['limit_page_range'] ? array_slice($pages, $values['first'] - 1, $values['last'] - $values['first'] + 1) : $pages;
    if (!empty($pages)) {
      $batch = array(
        'title' => t('Creating TEI-RDF Object from @source', array('@source' => $source->label)),
        'file' => drupal_get_path('module', 'emicdora') . '/includes/ingest_versionable_object.form.inc',
        'progress_message' => t('@current of @total pages processed.', array('@total' => count($pages))),
        'finished' => 'emicdora_ingest_versionable_object_tei_rdf_batch_finished',
        'operations' => array(),
      );
      foreach ($pages as $page) {
        $batch['operations'][] = array(
          'emicdora_ingest_versionable_object_tei_rdf_page_batch_operation',
          array($page, $versionable_object->id),
        );
      }
      $batch['operations'][] = array(
        'emicdora_ingest_versionable_object_tei_rdf_tei_datastream_batch_operation',
        array(),
      );
      batch_set($batch);
    }
    // TEI-RDF objects that are not associated with books require that we have
    // at least one child islandora:criticalEditionCModelPage so that the users
    // can edit TEI with the cwrc.
    else {
      $namespace = islandora_get_namespace($versionable_object->id);
      $page = $versionable_object->repository->constructObject($namespace);
      $page->label = $versionable_object->label;
      // Add all relationships.
      $page->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:tei-rdfCModel');
      $page->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object->id);
      $page->relationships->add(FEDORA_RELS_EXT_URI, 'isDerivationOf', $source->id);
      $page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageOf', $versionable_object->id);
      $page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', 1, RELS_TYPE_PLAIN_LITERAL);
      $page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageNumber', 1, RELS_TYPE_PLAIN_LITERAL);
      $page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSection', 1, RELS_TYPE_PLAIN_LITERAL);
      // Add default datastreams.
      $datastream = $page->constructDatastream("OCR", 'M');
      $datastream->mimetype = 'text/plain';
      $datastream->label = "Default Transcription";
      $datastream->setContentFromString(t("Transcription"));
      $page->ingestDatastream($datastream);
      emicdora_add_cwrc_datastream($page);
      $ingest_objects['page'] = $page;
    }
  }
  // Create a new Transcription Object.
  $transcription = emicdora_construct_transcription($versionable_object, $values['transcription_label']);
  $ingest_objects['transcription'] = $transcription;
  // Populate the Transcription Object.
  switch ($values['transcription_source']) {
    case 'tei-rdf':
      $tei_rdf = $form_state['values']['tei_rdf'];
      $tei_object = islandora_object_load($tei_rdf);
      $pages = ($values['tei_rdf_source'] == 'existing') ? islandora_paged_content_get_pages($tei_object) : islandora_paged_content_get_pages($source);
      $pages = array_values($pages);
      $pages = $values['limit_page_range'] ? array_slice($pages, $values['first'] - 1, $values['last'] - $values['first'] + 1) : $pages;
      if (!empty($pages)) {
        // If there is already a batch in place we need to append to it,
        // otherwise run from the existing object.
        $batch = array(
          'title' => t('Creating Transcription from TEI-RDF Object: @tei_rdf', array('@tei_rdf' => $versionable_object->label)),
          'file' => drupal_get_path('module', 'emicdora') . '/includes/ingest_versionable_object.form.inc',
          'progress_message' => t('@current of @total pages processed.', array('@total' => count($pages))),
          'finished' => 'emicdora_ingest_versionable_object_transcription_batch_finished',
          'operations' => array(),
        );
        // If we are using a existing TEI-RDF use it's objects CWRC datastreams.
        // Otherwise generate the CWRC datastream.
        foreach ($pages as $page) {
          $batch['operations'][] = array(
            'emicdora_ingest_versionable_object_transcription_batch_operation',
            array($page),
          );
        }
        batch_set($batch);
      }
      else {
        $datastream = $transcription->constructDatastream('TRANSCRIPTION', 'M');
        $datastream->setContentFromString(t('Transcription'));
        $transcription->ingestDatastream($datastream);
      }
      break;

    case 'digital':
      $datastream = $transcription->constructDatastream('TRANSCRIPTION', 'M');
      $datastream->setContentFromString($values['transcription']);
      $transcription->ingestDatastream($datastream);
      break;

    case 'upload':
      $file = file_load($values['transcription_upload']);
      $datastream = $transcription->constructDatastream('TRANSCRIPTION', 'M');
      $datastream->setContentFromFile(drupal_realpath($file->uri), FALSE);
      $transcription->ingestDatastream($datastream);
      break;
  }
  $shared_storage['emicdora']['destination_url'] = "islandora/object/$critical_edition/version/{$versionable_object->id}";
}

/**
 * Undoes the submit, purging any objects created by this step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_versionable_object_form_undo_submit(array $form, array &$form_state) {
  $versionable_object = islandora_ingest_form_get_object($form_state);
  $ingest_objects = &islandora_ingest_form_get_objects($form_state);
  // Forget all generated objects except the default object.
  $ingest_objects = array('versionable_object' => $versionable_object);
}

/**
 * Ajax callback for populating form elements from the selected source object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_source_ajax_container(array $form, array &$form_state) {
  global $user;
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $values = &$form_state['values'];
  $source = isset($values['source']) ? islandora_object_load($values['source']) : NULL;
  // Repopulate the create label when selecting a new Source.
  if ($source) {
    $element = &$form['source_ajax_wrapper'];
    $element['page_range']['first']['#value'] = 1;
    $element['page_range']['last']['#value'] = count(islandora_paged_content_get_pages($source));
    $element = &$form['source_ajax_wrapper']['tei_rdf_ajax_wrapper'];
    $element['tei_rdf_label']['#value'] = $source->label;
    $element = &$form['source_ajax_wrapper'];
    $element['transcription_label']['#value'] = $source->label;
  }
  else {
    $element = &$form['source_ajax_wrapper']['tei_rdf_ajax_wrapper'];
    $element['tei_rdf_label']['#value'] = "";
    $element = &$form['source_ajax_wrapper'];
    $element['transcription_label']['#value'] = "";
  }
  return $form['source_ajax_wrapper'];
}

/**
 * Ajax callback for populating form elements when limit page is checked.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_page_ajax_container(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $values = &$form_state['values'];
  $source = isset($values['source']) ? islandora_object_load($values['source']) : NULL;
  if ($source) {
    $element = &$form['source_ajax_wrapper'];
    $element['page_range']['first']['#value'] = 1;
    $element['page_range']['last']['#value'] = count(islandora_paged_content_get_pages($source));
  }
  return $form['source_ajax_wrapper']['page_range'];
}

/**
 * Ajax callback for populating form elements when TEI-RDF source changes.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function emicdora_tei_rdf_ajax_container(array $form, array &$form_state) {
  global $user;
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $values = &$form_state['values'];
  $source = isset($values['source']) ? islandora_object_load($values['source']) : NULL;
  if ($source) {
    $args = array('@source' => $source->label, '@user' => $user->name);
    $element = &$form['source_ajax_wrapper']['tei_rdf_ajax_wrapper'];
    $element['tei_rdf_label']['#value'] = t('@source: TEI-RDF (@user)', $args);
  }
  return $form['source_ajax_wrapper']['tei_rdf_ajax_wrapper'];
}

/**
 * Clones editions of the given page object.
 *
 * @param string $page_pid
 *   PID of page being cloned.
 * @param string $versionable_object
 *   The parent TEI-RDF objects PID.
 * @param array $context
 *   The Drupal Batch context.
 */
function emicdora_copy_versionable_object_tei_rdf_page_batch_operation($page_pid, $versionable_object, array &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('module', 'islandora');
  $batch = &batch_get();
  $ingest_objects = &islandora_ingest_form_get_objects($batch['form_state']);
  $page_object = islandora_object_load($page_pid);
  $namespace = islandora_get_namespace($versionable_object);
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $tei_rdf_page = $repository->constructObject($namespace);
  $derivation_results = $page_object->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
  $sequence_results = $page_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  $page_number_results = $page_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageNumber');

  // Add all relationships.
  $tei_rdf_page->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageOf', $versionable_object);
  $tei_rdf_page->relationships->add(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF, $derivation_results[0]['object']['value']);
  $tei_rdf_page->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:tei-rdfCModel');
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', $sequence_results[0]['object']['value'], RELS_TYPE_PLAIN_LITERAL);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageNumber', $page_number_results[0]['object']['value'], RELS_TYPE_PLAIN_LITERAL);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSection', 1, RELS_TYPE_PLAIN_LITERAL);

  $dsids = array('OCR', 'TN', 'CWRC');
  foreach ($dsids as $dsid) {
    if (isset($page_object[$dsid])) {
      $datastream = $tei_rdf_page->constructDatastream($dsid, 'M');
      $unique_id = uniqid();
      $file = file_save_data('', "temporary://${unique_id}_datastream_$dsid");
      $filename = drupal_realpath($file->uri);
      // Since the file must persist across multiple requests, we must not
      // delete it, until it has successfully been ingested.
      $page_object[$dsid]->getContent($filename);
      $datastream->setContentFromFile($filename, FALSE);
      $tei_rdf_page->ingestDatastream($datastream);
    }
  }
  $ingest_objects[] = $tei_rdf_page;
}

/**
 * Create editions of the given page object.
 *
 * @param array $page
 *   A page from an array create by islandora_paged_content_get_pages.
 * @param string $versionable_object
 *   The parent TEI-RDF objects PID.
 * @param array $context
 *   The Drupal Batch context.
 */
function emicdora_ingest_versionable_object_tei_rdf_page_batch_operation(array $page, $versionable_object, array &$context) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $namespace = islandora_get_namespace($versionable_object);
  $tei_rdf_page = islandora_get_tuque_connection()->repository->constructObject($namespace);
  $tei_rdf_page->label = $page['label'];
  // Add all relationships.
  $tei_rdf_page->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:tei-rdfCModel');
  $tei_rdf_page->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $versionable_object);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageOf', $versionable_object);
  $tei_rdf_page->relationships->add(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF, $page['pid']);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', $page['page'], RELS_TYPE_PLAIN_LITERAL);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isPageNumber', $page['page'], RELS_TYPE_PLAIN_LITERAL);
  $tei_rdf_page->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSection', '1', RELS_TYPE_PLAIN_LITERAL);
  // Add a minimal set of datastreams from original page object.
  $page = islandora_object_load($page['pid']);
  $dsids = array('OCR', 'TN', 'CWRC');
  foreach ($dsids as $dsid) {
    if (isset($page[$dsid])) {
      $datastream = $tei_rdf_page->constructDatastream($dsid, 'M');
      $unique_id = uniqid();
      $file = file_save_data('', "temporary://${unique_id}_datastream_$dsid");
      $filename = drupal_realpath($file->uri);
      // Since the file must persist across multiple requests, we must not
      // delete it, until it has successfully been ingested.
      $page[$dsid]->getContent($filename);
      $datastream->setContentFromFile($filename, FALSE);
      $tei_rdf_page->ingestDatastream($datastream);
    }
  }
  // Create new CWRC datastream if there is none.
  if (!isset($tei_rdf_page['CWRC']) && isset($tei_rdf_page['OCR'])) {
    emicdora_add_cwrc_datastream($tei_rdf_page);
  }
  $context['results']['pages'][] = $tei_rdf_page;
  $context['results']['objects'][$tei_rdf_page->id] = $tei_rdf_page;
}

/**
 * Creates the TEI datastream on the TEI-RDF object.
 *
 * @param array $context
 *   The Drupal Batch context.
 */
function emicdora_ingest_versionable_object_tei_rdf_tei_datastream_batch_operation(array &$context) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $batch = &batch_get();
  $ingest_objects = &islandora_ingest_form_get_objects($batch['form_state']);
  $tei_rdf = &$ingest_objects[0];
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($context['results']['pages']);
    // Create the template TEI to be populated.
    $context['sandbox']['content'] = emicdora_get_blank_tei($tei_rdf->label);
  }
  $index = $context['sandbox']['progress'];
  $page = isset($context['results']['pages'][$index]) ? $context['results']['pages'][$index] : array();
  if (isset($page['CWRC'])) {
    $context['sandbox']['content'] = emicdora_append_tei_rdf_text_to_tei($context['sandbox']['content'], $page['CWRC']->content);
  }
  $context['sandbox']['progress']++;
  $context['message'] = t('Converting page @page of @total into a TEI ...', array(
    '@page' => $context['sandbox']['progress'],
    '@total' => $context['sandbox']['total']));
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
  else {
    $datastream = $tei_rdf->constructDatastream('TEI', 'M');
    $datastream->mimetype = 'text/xml';
    $datastream->setContentFromString($context['sandbox']['content']);
    $datastream->label = "Consolidated TEI";
    $tei_rdf->ingestDatastream($datastream);
  }
}

/**
 * Finished call-back for the create TEI-RDF batch process.
 */
function emicdora_ingest_versionable_object_tei_rdf_batch_finished($success, array $results, array $operations) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $batch = &batch_get();
  // We have to unset the cached files here due to a bug in core. Basically when
  // we come back from the batch function drupal_build_form() gets the
  // $form_state back from the session rather than the database, thus skipping
  // $form_get_cache which loads all of the required files. We forget the
  // required files now, so that we can dynamically load the required files
  // during drupal_rebuild_form().
  $batch['form_state']['build_info']['files'] = array();
  if ($success) {
    drupal_set_message(t('Succesfully created TEI-RDF Object.'));
    $ingest_objects = &islandora_ingest_form_get_objects($batch['form_state']);
    if (!empty($results['objects'])) {
      $ingest_objects = array_merge($ingest_objects, $results['objects']);
    }
  }
  else {
    drupal_set_message(t('Failed to create the TEI-RDF object.'), 'error');
    // Mock the form as it's not used, then return to the form step,
    // that launched this batch process.
    $form = array();
    islandora_ingest_form_previous_submit($form, $batch['form_state']);
  }
}

/**
 * Create transcription of the given page object.
 *
 * @param string $page
 *   The page object's PID.
 * @param array $context
 *   The Drupal Batch context.
 */
function emicdora_ingest_versionable_object_transcription_batch_operation($page, array &$context) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  // Even though this include is not needed by this function it must be included
  // here the Batch API won't be able to Find the submit handler:
  // islandora_ingest_form_next_submit() as it doesn't respect form_load_include
  // and doesn't check load any files required by submit handles.
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  if (empty($context['results'])) {
    $file_id = uniqid();
    $file = file_save_data('', "temporary://{$file_id}transcription.txt");
    $context['results']['filename'] = drupal_realpath($file->uri);
  }
  $page = islandora_object_load($page['pid']);
  $filename = $context['results']['filename'];
  $text = '';
  if (isset($page['CWRC'])) {
    $text = emicdora_convert_tei_to_plain_text($page['CWRC']->content);
  }
  elseif (isset($page['OCR'])) {
    $text = $page['OCR']->content;
  }
  file_put_contents($filename, $text, FILE_APPEND | LOCK_EX);
}

/**
 * Finished call-back for the create TEI-RDF batch process.
 */
function emicdora_ingest_versionable_object_transcription_batch_finished($success, array $results, array $operations) {
  module_load_include('inc', 'collation', 'includes/calliope');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $batch = &batch_get();
  // We have to unset the cached files here due to a bug in core. Basically when
  // we come back from the batch function drupal_build_form() gets the
  // $form_state back from the session rather than the database, thus skipping
  // $form_get_cache which loads all of the required files. We forget the
  // required files now, so that we can dynamically load the required files
  // during drupal_rebuild_form().
  $batch['form_state']['build_info']['files'] = array();
  if ($success) {
    $ingest_objects = &islandora_ingest_form_get_objects($batch['form_state']);
    $transcription = $ingest_objects['transcription'];
    $datastream = $transcription->constructDatastream('TRANSCRIPTION', 'M');
    $contents = file_get_contents($results['filename']);
    $datastream->setContentFromString($contents);
    $transcription->ingestDatastream($datastream);
    unlink($results['filename']);
    drupal_set_message(t('Succesfully created Transcription Object.'));
  }
  else {
    drupal_set_message(t('Failed to create Transcription object.'), 'error');
    // Mock the form as it's not used, then return to the form step,
    // that launched this batch process.
    $form = array();
    islandora_ingest_form_previous_submit($form, $batch['form_state']);
  }
}

/**
 * Attempt to grok out the relevant source object.
 *
 * @param array $form_state
 *   A $form_state array, as passed along during hook_islandora_ingest_steps().
 *
 * @return string|bool
 *   A string containing the PID of the object if found; otherwise, boolean
 *   FALSE.
 */
function emicdora_determine_source_for_versionable_object_ingest($form_state) {
  $step_storage = islandora_ingest_form_get_step_storage($form_state, 'emicdora_versionable_object');

  if (isset($step_storage['source'])) {
    return $step_storage['source'];
  }
  elseif (isset($_GET['d'])) {
    $destination_url = filter_input(INPUT_GET, 'd');
    $fourth_arg = arg(4, $destination_url);
    $custom_params['limit_page_range'] = TRUE;
    if ($fourth_arg === NULL) {
      $version_pid = arg(2, $destination_url);
      $version_object = islandora_object_load($version_pid);
      $sources = $version_object->relationships->get(FEDORA_RELS_EXT_URI, EMICDORA_ISDERIVATIONOF);
      $source = reset($sources);
      if ($source) {
        return $source['object']['value'];
      }
    }
    else {
      return $fourth_arg;
    }
  }

  return FALSE;
}

/**
 * Ingest step callback; defer ingest of objects into a batch.
 *
 * Sets a batch to process objects returned by
 * islandora_ingest_form_get_objects().
 *
 * @param array $form_state
 *   The ingest step form state, from which the objects are to be ingest.
 * @param bool $skip_first
 *   A boolean flag. If TRUE, the first object in the the list of objects will
 *   be left intact, to be ingested by the core ingest step process. If FALSE,
 *   we will try to ingest everything in the batch.
 */
function emicdora_defer_ingest_to_batch(&$form_state, $skip_first) {
  $all_objects =& islandora_ingest_form_get_objects($form_state);
  $batch = array(
    'title' => t('Ingesting objects'),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/ingest_versionable_object.form.inc',
    'operations' => array(
      array(
        'emicdora_deferred_batch_ingest_operation',
        array(
          ($skip_first ?
            array_slice($all_objects, 1) :
            $all_objects),
        ),
      ),
    ),
    'finished' => 'emicdora_deferred_batch_finished',
  );
  if ($skip_first) {
    $all_objects = array_slice($all_objects, 0, 1);
  }
  else {
    $all_objects = array();
  }
  batch_set($batch);
}

/**
 * Batch operation; ingest an object from an array.
 *
 * @param array $objects
 *   The initial array of objects to ingest.
 * @param array $context
 *   The batch context.
 */
function emicdora_deferred_batch_ingest_operation($objects, &$context) {
  $sandbox =& $context['sandbox'];

  if (!isset($sandbox['objects'])) {
    $sandbox['objects'] = $objects;
    $sandbox['total'] = count($objects);
    $sandbox['progress'] = 0.0;
    if ($sandbox['total'] === 0) {
      return;
    }
  }

  $sandbox['progress']++;
  $object = array_shift($sandbox['objects']);
  islandora_add_object($object);

  $context['message'] = t('Ingested @pid: @progress/@total', array(
    '@progress' => $sandbox['progress'],
    '@total' => $sandbox['total'],
    '@pid' => $object->id,
  ));
  $context['finished'] = $sandbox['progress'] / $sandbox['total'];
}

/**
 * Batch "finished" callback; hack around form/batch stuffs.
 */
function emicdora_deferred_batch_finished($success, array $results, array $operations) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $batch = &batch_get();
  // We have to unset the cached files here due to a bug in core. Basically when
  // we come back from the batch function drupal_build_form() gets the
  // $form_state back from the session rather than the database, thus skipping
  // $form_get_cache which loads all of the required files. We forget the
  // required files now, so that we can dynamically load the required files
  // during drupal_rebuild_form().
  $batch['form_state']['build_info']['files'] = array();
  if ($success) {
    // No-op.
  }
  else {
    drupal_set_message(t('Failed to ingest some objects.'), 'error');
    // Mock the form as it's not used, then return to the form step,
    // that launched this batch process.
    $form = array();
    islandora_ingest_form_previous_submit($form, $batch['form_state']);
  }
}
