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
function islandora_emicdora_add_source_form($form, $form_state, $content_model, $critical_edition) {
  $mappings = islandora_simplified_ingest_get_mappings($content_model);
  drupal_add_css(
      drupal_get_path('module', 'emicdora') .
      '/css/emicdora_ingest.css'
  );
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $form['title'] = array(
    '#type' => 'markup',
    '#markup' => '<h1>' . t('Add new @type object', array('@type' => $mappings['label'])) . '</h1>',
  );

  $form['content_model'] = array(
    '#type' => 'hidden',
    '#value' => $content_model,
  );
  $form['critical_edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => $critical_edition->id,
  );

  if ($mappings['tn']) {
    $form = array_merge($form, emicdora_get_thumbnail_wrapper($upload_size));
  }
  $form = array_merge($form, emicdora_get_content_wrapper($mappings, $upload_size));
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
    '#default_value' => FALSE,
    '#attributes' => array(
      'class' => array('workbench-check')),
  );

  $form_name = $mappings['form'];
  module_load_include('module', 'xml_form_builder');
  $metadata = xml_form_builder_get_form($form, $form_state, $form_name);
  $form = array_merge_recursive($form, $metadata);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Ingest @type object", array('@type' => $mappings['label'])),
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
function islandora_emicdora_add_source_form_submit($form, $form_state) {
  global $user;
  module_load_include('inc', 'xml_form_builder', 'includes/associations');
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $mapping = islandora_simplified_ingest_get_mappings($form_state['values']['content_model']);
  $object = $repository->constructObject('islandora');
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $mapping['collection']);
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $form_state['values']['critical_edition_pid']);
  $xml_form = new XMLForm($form_state);
  $document = $xml_form->submit($form, $form_state);
  $associations = xml_form_builder_get_associations(array(), array($form_state['values']['content_model']), array());
  $created = xml_form_builder_update_object($object, $associations[0], $document, "Object");
  $dc = DublinCore::importFromXMLString($object['DC']->content);
  if ($dc) {
    $dc = $dc->asArray();
    $label = $dc['dc:title']['value'];
  }
  $object->label = $label ? $label : t("New @type", array('@type' => $form_state['values']['type']));
  islandora_add_object($object);
  if ($form_state['values']['choice_wrapper']['privacy']) {
    $xacml = new IslandoraXacml($object);
    $xacml->viewingRule->addUser($user->name);
    $xacml->writeBackToFedora();
  }
  // Add thumbnail.
  $tn_added = FALSE;
  if (isset($form_state['values']['tn_wrapper']['tn_file'])) {
    $tn_file = $form_state['values']['tn_wrapper']['tn_file'];
    $file = file_load($tn_file);
    if ($file) {
      emicdora_ingest_add_file($file, 'TN', $object);
      $tn_added = TRUE;
    }
  }
  if (!empty($form_state['values']['content_wrapper']['content'])) {
    $file = file_load($form_state['values']['content_wrapper']['content']);

    switch ($form_state['values']['content_model']) {
      case 'islandora:bookCModel':
        $uri = drupal_realpath($file->uri);
        emicdora_add_zipped_pages($uri, $object->id);
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
 * Returns partial form defintition.
 *
 * @param int $upload_size
 *   Upload limit size
 *
 * @return array
 *   Form definition
 */
function emicdora_get_content_wrapper($mappings, $upload_size) {
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
  return $form;
}

/**
 * Returns paramaters associated with content model.
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
      'form' => 'Islandora Book MODS Form',
      'label' => t('Text'),
      'collection' => 'islandora:co-op',
      'tn' => TRUE,
      'prompt' => t("Add zipped pages to this book?"),
      'required' => FALSE,
      'upload_prompt' => t('Add zipped page images'),
      'extensions' => 'zip',
    ),
    'islandora:sp_videoCModel' => array(
      'form' => 'Video MODS form',
      'label' => t('Video'),
      'collection' => 'islandora:co-op',
      'tn' => TRUE,
      'prompt' => t("Upload video file"),
      'required' => TRUE,
      'upload_prompt' => t('Add video file'),
      'extensions' => 'ogg mp4 mov qt m4a avi',
    ),
    'islandora:sp-audioCModel' => array(
      'form' => 'Audio MODS form',
      'label' => t('Audio'),
      'collection' => 'islandora:co-op',
      'tn' => TRUE,
      'prompt' => t("Upload audio file"),
      'required' => TRUE,
      'upload_prompt' => t('Add audio file'),
      'extensions' => 'wav mp3',
    ),
    'islandora:sp_large_image_cmodel' => array(
      'form' => 'Large image MODS form',
      'label' => t('Image'),
      'collection' => 'islandora:co-op',
      'tn' => TRUE,
      'prompt' => t("Upload image file"),
      'required' => TRUE,
      'upload_prompt' => t('Add image'),
      'extensions' => 'tiff tif',
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
  if (empty($object[$dsid])) {
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
 */
function emicdora_batch_add_zipped_pages($files_to_add, $pid) {
  $batch = array(
    'title' => t('Adding pages to new book'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'emicdora') . '/includes/add_source.form.inc',
    'progress_message' => t('@current of @total pages created.'),
  );
  $page_number = 1;
  foreach ($files_to_add as $page) {
    $batch['operations'][] = array('emicdora_consolidate_add_page', array(
        'page' => $page,
        'book' => $pid,
        'page_number' => $page_number));
    $page_number++;
  }
  batch_set($batch);
  batch_process();
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
 */
function emicdora_consolidate_add_page($page, $book, $page_number) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $object = $repository->constructObject('islandora');
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:pageCModel');
  $rels_ext = $object->relationships;
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $book);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $page_number, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $page_number, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $book);
  islandora_paged_content_update_datastream($object, $page, 'OBJ', NULL, NULL, 'M', FALSE);
  islandora_add_object($object);
}

/**
 * Unzips input file and kicks off batch process.
 *
 * @param uri $zip_uri
 *   Address to zipped file
 */
function emicdora_add_zipped_pages($zip_uri, $book_pid) {
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
  emicdora_batch_add_zipped_pages($files_to_add, $book_pid);
}
