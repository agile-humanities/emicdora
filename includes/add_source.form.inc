<?php

/**
 * @file
 * Add a source object to the given Critical Edition and the CO-OP.
 */

/**
 * Form for ingesting / adding a new source object to a critical edition.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param string $content_model
 *   The identifier of the Content Model type to ingest.
 * @param string $critical_edition
 *   The identifier of the critical edition the source will be linked to.
 * @param bool $add_to_workbench
 *   Indicator of whether ingested object should be added to the workbench
 *
 * @return array
 *   A Drupal Form for ingesting a new object of the given type.
 */
function islandora_emicdora_add_source_form($form, &$form_state, $content_model, $critical_edition = NULL, $add_to_workbench = FALSE) {
  form_load_include($form_state, 'inc', 'islandora', 'includes/ingest.form');
  $add_to_workbench = $add_to_workbench == "TRUE" ? TRUE : FALSE;
  $mappings = islandora_simplified_ingest_get_mappings($content_model);
  drupal_add_css(
      drupal_get_path('module', 'emicdora') .
      '/css/emicdora_ingest.css'
  );
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $form['content_model'] = array(
    '#type' => 'hidden',
    '#value' => $content_model,
  );
  $form['critical_edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => is_object($critical_edition) ? $critical_edition->id : NULL,
  );
  if ($mappings['tn']) {
    $form = array_merge($form, emicdora_get_thumbnail_wrapper($upload_size));
  }
  if ($mappings['type'] != 'no_source') {
    $form = array_merge($form, emicdora_get_content_wrapper($mappings, $upload_size, $content_model));
  }
  $form['choice_wrapper'] = array(
    '#type' => 'fieldset',
  );
  $form['choice_wrapper']['privacy'] = array(
    '#prefix' => '<div class="report-checkboxes">',
    '#type' => 'checkbox',
    '#title' => t("Keep object private?"),
    '#default_value' => FALSE,
    '#attributes' => array(
      'class' => array('privacy-check')),
  );
  $form['choice_wrapper']['workbench'] = array(
    '#type' => 'checkbox',
    '#suffix' => '</div>',
    '#title' => t("Add object to workbench?"),
    '#default_value' => $add_to_workbench,
    '#disabled' => $add_to_workbench,
    '#attributes' => array(
      'class' => array('workbench-check')),
  );

  $form_name = $mappings['form'];
  module_load_include('module', 'xml_form_builder');
  $metadata = xml_form_builder_get_form($form, $form_state, $form_name);
  $form = array_merge($form, $metadata);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Ingest @type object", array('@type' => $mappings['label'])),
    '#process' => array('islandora_ingest_form_ingest_button_process'),
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
function islandora_emicdora_add_source_form_submit(&$form, &$form_state) {
  global $user;
  module_load_include('inc', 'xml_form_builder', 'includes/associations');
  module_load_include('inc', 'islandora_bookmark', 'includes/api');
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $mapping = islandora_simplified_ingest_get_mappings($form_state['values']['content_model']);
  $object = $repository->constructObject('emic');
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $mapping['collection']);
  if ($form_state['values']['critical_edition_pid']) {
    $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $form_state['values']['critical_edition_pid']);
  }
  $xml_form = new XMLForm($form_state);
  $document = $xml_form->submit($form, $form_state);
  $associations = xml_form_builder_get_associations(array(), array($form_state['values']['content_model']), array());
  $association = array_shift($associations);
  $created = xml_form_builder_update_object($object, $association, $document, "Object");
  // Set default label.
  $label = t("New @type", array('@type' => $mapping['label']));
  $dc = DublinCore::importFromXMLString($object['DC']->content);
  if ($dc) {
    $dc = $dc->asArray();
    $label = $dc['dc:title']['value'];
  }
  $object->label = $label;
  $xacml = FALSE;
  if ($form_state['values']['choice_wrapper']['privacy']) {
    $xacml = new IslandoraXacml($object);
    $xacml->viewingRule->addUser($user->name);
    $xacml->writeBackToFedora();
    $xacml = TRUE;
  }
  // Add thumbnail.
  if (isset($form_state['values']['tn_wrapper']['tn_file'])) {
    $tn_file = $form_state['values']['tn_wrapper']['tn_file'];
    $file = file_load($tn_file);
    if ($file) {
      emicdora_ingest_add_file($file, 'TN', $object);
    }
  }
  if (!empty($form_state['values']['content_wrapper']['content'])) {
    $file = file_load($form_state['values']['content_wrapper']['content']);

    switch ($form_state['values']['content_model']) {
      case 'islandora:bookCModel':
        $language = isset($form_state['values']['content_wrapper']['language']) ? $form_state['values']['content_wrapper']['language'] : FALSE;
        $uri = drupal_realpath($file->uri);
        emicdora_add_zipped_pages($uri, $object->id, $xacml, $language);
        break;

      case 'islandora:sp-audioCModel':
        emicdora_ingest_add_file($file, 'OBJ', $object);
        break;

      case 'islandora:sp_videoCModel':
        emicdora_ingest_add_file($file, 'OBJ', $object);
        break;

      case 'islandora:sp_large_image_cmodel':
        emicdora_ingest_add_file($file, 'OBJ', $object);
        break;
    }
  }
  islandora_add_object($object);
  if ($form_state['values']['choice_wrapper']['workbench']) {
    $list_id = islandora_bookmark_get_default_list();
    $list = Bookmark::getList($list_id);
    $list->addPid($object->id);
  }
  $form_state['redirect'] = "islandora/object/{$object->id}";
}

/**
 * Returns partial form defintition.
 *
 * @param int $upload_size
 *   Upload limit size
 *
 * @return array
 *   Form definition
 */
function emicdora_get_thumbnail_wrapper($upload_size) {
  $form['tn_upload'] = array(
    '#type' => 'checkbox',
    '#title' => t("Add thumbnail image?"),
    '#default_value' => FALSE,
  );

  $form['tn_wrapper'] = array(
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="tn_upload"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['tn_wrapper']['tn_file'] = array(
    '#title' => t('Add thumbnail images.'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('jpg', 'jpeg'),
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  return $form;
}

/**
 * Returns partial form definition.
 *
 * @param array $mappings
 *   Parameter mappings
 * @param int $upload_size
 *   Upload size limit
 * @param string $content_model
 *   PID of content model
 *
 * @return array
 *   Drupal partartial form
 */
function emicdora_get_content_wrapper($mappings, $upload_size, $content_model) {
  module_load_include('inc', 'islandora_ocr', 'includes/utilities');
  $form['content_upload'] = array(
    '#type' => 'checkbox',
    '#title' => $mappings['prompt'],
    '#default_value' => $mappings['required'],
  );

  $form['content_wrapper'] = array(
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="content_upload"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['content_wrapper']['content'] = array(
    '#title' => $mappings['upload_prompt'],
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array($mappings['extensions']),
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $languages = module_exists('islandora_ocr') ? islandora_ocr_get_enabled_tesseract_languages() : array('eng' => t('English'));
  if ($mappings['type'] == 'text') {
    $form['content_wrapper']['language'] = array(
      '#title' => t('Select language for OCR creation'),
      '#type' => 'select',
      '#description' => t('Select language from source page for more accurate results.'),
      '#options' => $languages,
    );
  }
  return $form;
}

/**
 * Returns parameters associated with content model.
 *
 * @param string $content_model
 *   Configuration type
 *
 * @return array
 *   key => value configuration options
 */
function islandora_simplified_ingest_get_mappings($content_model) {
  $mappings = array(
    'islandora:bookCModel' => array(
      'form' => 'EMiC Source MODS form',
      'label' => t('Text'),
      'type' => 'text',
      'collection' => EMICDORA_SOURCE_COLLECTION,
      'tn' => FALSE,
      'prompt' => t("Add zipped pages?"),
      'required' => FALSE,
      'upload_prompt' => t('Add zipped page images'),
      'extensions' => 'zip',
    ),
    'islandora:sp_videoCModel' => array(
      'form' => 'EMiC Source MODS form',
      'label' => t('Video'),
      'type' => 'video',
      'collection' => EMICDORA_SOURCE_COLLECTION,
      'tn' => TRUE,
      'prompt' => t("Upload video file"),
      'required' => TRUE,
      'upload_prompt' => t('Add video file'),
      'extensions' => 'ogg mp4 mov qt m4a avi',
    ),
    'islandora:sp-audioCModel' => array(
      'form' => 'EMiC Source MODS form',
      'label' => t('Audio'),
      'type' => 'audio',
      'collection' => EMICDORA_SOURCE_COLLECTION,
      'tn' => TRUE,
      'prompt' => t("Upload audio file"),
      'required' => TRUE,
      'upload_prompt' => t('Add audio file'),
      'extensions' => 'wav mp3',
    ),
    'islandora:sp_large_image_cmodel' => array(
      'form' => 'EMiC Source MODS form',
      'label' => t('Image'),
      'type' => 'image',
      'collection' => EMICDORA_SOURCE_COLLECTION,
      'tn' => FALSE,
      'prompt' => t("Upload image file"),
      'required' => TRUE,
      'upload_prompt' => t('Add image'),
      'extensions' => 'tiff tif',
    ),
    'islandora:criticalEditionContainerCModel' => array(
      'form' => 'EMiC Critical Edition MODS form',
      'label' => t('Critical Edition'),
      'collection' => EMICDORA_EDITION_COLLECTION,
      'tn' => FALSE,
      'prompt' => NULL,
      'upload_prompt' => NULL,
      'type' => 'no_source',
      'extensions' => NULL,
      'required' => FALSE,
    ),
  );
  return $mappings[$content_model];
}

/**
 * Adds file contents to object.
 *
 * @param file $file
 *   Input file
 * @param string $dsid
 *   DSID to be created
 * @param AbstractObject $object
 *   Host object
 */
function emicdora_ingest_add_file($file, $dsid, $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!isset($object[$dsid])) {
    $ds = $object->constructDatastream($dsid, 'M');
    $ds->setContentFromFile($file->uri, FALSE);
    $mime_detect = new MimeDetect();
    $ds->label = $file->filename;
    $ds->mimetype = $mime_detect->getMimetype($file->filename);
    $object->ingestDatastream($ds);
  }
}

/**
 * Adds zipped pages to book object.
 *
 * @param array $files_to_add
 *   URIs of page source files
 * @param string $pid
 *   PID of host book
 * @param string $language
 *   Chosen language for OCR process
 */
function emicdora_batch_add_zipped_pages($files_to_add, $pid, $xacml, $language) {
  $batch = array(
    'title' => t('Adding pages to new text object'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/add_source.form.inc',
    'progress_message' => t('@current of @total pages created.'),
  );
  $page_number = 1;
  foreach ($files_to_add as $page) {
    $batch['operations'][] = array('emicdora_consolidate_add_page', array(
        'page' => $page,
        'book' => $pid,
        'page_number' => $page_number,
        'xacml' => $xacml,
        'language' => $language,
      ),
    );
    $page_number++;
  }
  batch_set($batch);
}

/**
 * Adds individual page to book object.
 *
 * @param string $page
 *   Page PID
 * @param string $book
 *   Book PID
 * @param int $page_number
 *   page number
 * @param bool $xacml
 *   Whether the pages need the base XACML policy or not.
 * @param string $language
 *   Chosen language for OCR process
 */
function emicdora_consolidate_add_page($page, $book, $page_number, $xacml, $language) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  global $user;

  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $object = $repository->constructObject('emic');
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:pageCModel');
  $book_object = islandora_object_load($book);
  $object->label = "{$book_object->label} - page $page_number";
  $rels_ext = $object->relationships;
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $book);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $page_number, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $page_number, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $book);
  islandora_paged_content_update_datastream($object, $page, 'OBJ', NULL, NULL, 'M', FALSE);
  if ($language) {
    islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'hasLanguage', $language, TRUE);
  }
  if ($xacml) {
    $xacml = new IslandoraXacml($object);
    $xacml->viewingRule->addUser($user->name);
    $xacml->writeBackToFedora();
  }
  islandora_add_object($object);
}

/**
 * Unzips input file and kicks off batch process.
 *
 * @param string $zip_uri
 *   Address to zipped file
 * @param string $book_pid
 *   The PID of the book we are adding the pages to.
 * @param bool $xacml
 *   Whether the pages need the base XACML policy or not.
 * @param bool $language
 *   Slected language for OCR.
 */
function emicdora_add_zipped_pages($zip_uri, $book_pid, $xacml = FALSE, $language = FALSE) {
  $zip = new ZipArchive();
  $zip->open($zip_uri);
  $tmp_dir = uniqid();
  $destination_dir = drupal_realpath("temporary://$tmp_dir");

  // Extract zipped file to named directory.
  if (!$zip->extractTo($destination_dir)) {
    drupal_set_message(t('Ingest failed.'), 'warning');
    return;
  }
  $zip->close();
  $allowed_extensions = array('tif', 'tiff', 'jpg', 'jpeg', 'jp2');
  $callback = function ($element) use ($allowed_extensions) {
    $ext = pathinfo($element, PATHINFO_EXTENSION);
    $ext = drupal_strtolower($ext);
    // An allowed extension and does /not/ contain __MACOSX.
    return in_array($ext, $allowed_extensions) && preg_match('/__MACOSX/', $element) === 0;
  };

  $file_objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($destination_dir), RecursiveIteratorIterator::SELF_FIRST);
  foreach ($file_objects as $file => $file_object) {
    $unfiltered[] = $file;
  }
  $files_to_add = array_values(array_filter($unfiltered, $callback));
  // Sort files based on name.
  $comparator = function ($a, $b) {
    $file_a = pathinfo($a, PATHINFO_FILENAME);
    $file_b = pathinfo($b, PATHINFO_FILENAME);
    return ($file_a < $file_b) ? -1 : 1;
  };
  usort($files_to_add, $comparator);
  emicdora_batch_add_zipped_pages($files_to_add, $book_pid, $xacml, $language);
}
