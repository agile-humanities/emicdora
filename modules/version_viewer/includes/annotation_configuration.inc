<?php

/**
 * @file
 * Forms to configure annotion availability.
 */

/**
 * Form for managing the the availablitly of annotations to a version.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $version
 *   The version requesting annotations.
 *
 * @return array
 *   The Drupal From definition.
 */
function islandora_emicdora_annotation_availability(array $form, array $form_state, AbstractObject $version) {
  drupal_add_css(drupal_get_path('module', 'versionable_object_viewer') . '/css/annotation_access.css');

  $query = db_select('versionable_object_access')
      ->fields('versionable_object_access', array(
        'pid',
        'allowed_users',
        'allowed_roles',
          )
      )
      ->condition('pid', $version->id, '=');
  $row = $query->execute()->fetchObject();
  $allowed_roles = isset($row->allowed_roles) ? array_values(unserialize($row->allowed_roles)) : array();
  $allowed_users = isset($row->allowed_users) ? unserialize($row->allowed_users) : array();

  $query = db_select('versionable_object_custom_config', 'v');
  $query->fields('v');
  $configs = $query->execute()->fetchAll();

  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $css_extensions = array('css');
  $xsl_extensions = array('xsl xslt');
  $users = array();
  $result = db_query('SELECT u.uid, u.name FROM {users} u');
  foreach ($result as $user) {
    $user->uid == 0 ? $users['anonymous'] = 'anonymous' : $users[$user->name] = $user->name;
    if ($user->uid == 1) {
      $admin_user = $user->name;
      $form_state['islandora_xacml']['admin_user'] = $user->name;
    }
  }

  $roles = array();
  $result = db_query('SELECT r.rid, r.name FROM {role} r');
  foreach ($result as $role) {
    $role->rid == 0 ? $roles['anonymous'] = 'anonymous' : $roles[$role->name] = $role->name;
  }

  $form = array();
  $form['version_id'] = array(
    '#type' => 'hidden',
    '#value' => $version->id,
  );
  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object Viewing'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="emicdora_access_selectors">',
    '#suffix' => '</div>',
  );

  $form['access']['users'] = array(
    '#type' => 'select',
    '#title' => t('Allowed Users'),
    '#default_value' => $allowed_users,
    '#options' => $users,
    '#multiple' => TRUE,
    '#size' => 10,
    '#prefix' => '<div class="emicdora_access_selector">',
    '#attributes' => array('class' => array('emicdora_selector')),
  );
  $form['access']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_button'), 'action' => 'cancel'),
  );

  $form['access']['roles'] = array(
    '#type' => 'select',
    '#title' => t('Allowed Roles'),
    '#options' => $roles,
    '#multiple' => TRUE,
    '#size' => 10,
    '#default_value' => $allowed_roles,
    '#prefix' => '<div class="emicdora_access_selector">',
    '#attributes' => array('class' => array('emicdora_selector')),
  );
  $form['access']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_button'), 'action' => 'save'),
  );
  $form['select_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select Transcription Display Configuration'),
    '#description' => t("Specify which Display Configuration you wish to use for this version's Reading and Diplomatic Transcription display."),
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="emicdora_config_selectors">',
    '#suffix' => '</div>',
  );
  $form['select_config']['use_custom'] = array(
    '#type' => 'checkbox',
    '#title' => t("Use Custom Display Configuration?"),
    '#default_value' => FALSE,
  );

  $form['select_config']['table_wrapper'] = array(
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="use_custom"]' => array('checked' => TRUE),
      ),
    ),
  );
  $rows = array();
  foreach ($configs as $config) {
    $css = l(t("CSS"), "islandora/version_viewer/configuration/download/{$config->cid}/css");
    $xslt = l(t("XSLT"), "islandora/version_viewer/configuration/download/{$config->cid}/xslt");
    $delete = l(t("Delete"), "islandora/version_viewer/configuration/delete/$config->cid/{$version->id}");
    $rows[] = array(
      'label' => $config->label,
      'description' => $config->description,
      'download' => "<span class = 'config_url'>$css</span> <span class = 'config_url'>$xslt</span><span class = 'config_url'>$delete</span>",
    );
  }
  $header = array(
    'label' => array('data' => t('Label')),
    'description' => array('data' => t('Description')),
    'download' => array('data' => t('Operations')),
  );
  // Build and return table element.
  $table_element = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t("This versionable object has no custom configurations."),
    '#js' => FALSE,
    '#multiple' => FALSE,
  );
  $form['select_config']['table_wrapper']['config_table'] = $table_element;
  $form['custom_forms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create Custom Transcription Display Configuration'),
    '#description' => t('Here you can specify a CSS and/or XSLT file to be used
      to customize the display of both the Reading and Diplomatic Transcription.
      The CSS will be applied to the entire page with the XSLT used to
      transform the HTML representation of the transcription, or you can choose
      an existing Configuration above.'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="emicdora_access_selectors">',
    '#suffix' => '</div>',
  );
  $form['custom_forms']['config_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
  );
  $form['custom_forms']['config_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
  );

  $form['custom_forms']['custom_css'] = array(
    '#title' => t('CSS File'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select a file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $css_extensions[0])),
    '#default_value' => isset($form_state['values']['custom_css']) ? $form_state['values']['custom_css'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $css_extensions,
      // Assume its specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['custom_forms']['custom_xslt'] = array(
    '#title' => t('XSLT File'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select a file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $xsl_extensions[0])),
    '#default_value' => isset($form_state['values']['custom_xslt']) ? $form_state['values']['custom_xslt'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $xsl_extensions,
      // Assume its specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['custom_forms']['create'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
    '#attributes' => array('class' => array('emicdora_button'), 'action' => 'create'),
  );
  return $form;
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function islandora_emicdora_annotation_availability_submit(array $form, array $form_state) {
  $action = $form_state['triggering_element']['#attributes']['action'];
  $version_pid = $form_state['values']['version_id'];
  switch ($action) {
    case 'cancel':
      drupal_goto(PATH_TO_VERSION);
      break;

    case 'save':
      $users = serialize($form_state['values']['access']['users']);
      $roles = serialize($form_state['values']['access']['roles']);
      versionable_object_viewer_access_upsert($version_pid, $users, $roles);

      break;

    case 'create':
      $css_file = file_load($form_state['values']['custom_forms']['custom_css']);
      $css = file_get_contents(drupal_realpath($css_file->uri));
      $xslt_file = file_load($form_state['values']['custom_forms']['custom_xslt']);
      $xslt = file_get_contents(drupal_realpath($xslt_file->uri));
      $label = $form_state['values']['custom_forms']['config_label'];
      $description = $form_state['values']['custom_forms']['config_description'];
      db_insert('versionable_object_custom_config')
          ->fields(array(
            'pid' => $version_pid,
            'label' => $label,
            'description' => $description,
            'css' => $css,
            'xslt' => $xslt,
              )
          )
          ->execute();

      break;
  }
}

/**
 * Inserts or updates access permissions.
 *
 * @param string $version_pid
 *   PID of version
 * @param string $users
 *   Serialized array of allowed users
 * @param string $roles
 *   Serialized array of allowed roles
 */
function versionable_object_viewer_access_upsert($version_pid, $users, $roles) {
  $query = db_select('versionable_object_access')
      ->fields('versionable_object_access', array('pid'))
      ->condition('pid', $version_pid, '=');
  $results = $query->execute()->fetchObject();

  if (empty($results)) {
    db_insert('versionable_object_access')
        ->fields(array(
          'allowed_users' => $users,
          'allowed_roles' => $roles,
          'pid' => $version_pid,
            )
        )
        ->execute();
  }
  else {
    $pid = $results[0]->pid;
    db_update('versionable_object_access')
        ->fields(array('allowed_users' => $users, 'allowed_roles' => $roles))
        ->condition('pid', $pid)
        ->execute();
  }
}

/**
 * Deletes custom configuration.
 *
 * @param int $cid
 *   Unique identifier of configuration record
 * @param string $pid
 *   PID for redirection
 */
function versionable_object_viewer_delete_config($cid, $pid) {
  db_delete('versionable_object_custom_config')
      ->condition('cid', $cid)
      ->execute();
  drupal_goto("islandora/version_viewer/annotation/$pid");
}

/**
 * Downloads selected file.
 *
 * @param int $cid
 *   Unique identifier for record
 * @param string $type
 *   CSS or XSLT
 */
function versionable_object_viewer_download($cid, $type) {
  $query = db_select('versionable_object_custom_config')
      ->fields('versionable_object_custom_config', array($type))
      ->condition('cid', $cid, '=');
  $results = $query->execute()->fetchObject();
  header('Content-Type: text/xml', TRUE);
  header('Content-Disposition: attachment; filename="File.' . $type . '"');
  switch ($type) {
    case 'css':
      echo $results->css;
      break;

    case 'xslt':
      echo $results->xslt;
      break;
  }
}
