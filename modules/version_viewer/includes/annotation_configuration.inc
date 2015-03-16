<?php

/**
 * @file
 * Forms to configure annotation availability.
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
 * @param string $critical_edition_pid
 *   PID of parent critical edition
 *
 * @return array
 *   The Drupal From definition.
 */
function islandora_emicdora_annotation_availability(array $form, array &$form_state, AbstractObject $version, $critical_edition_pid = NULL) {
  drupal_add_css(drupal_get_path('module', 'versionable_object_viewer') . '/css/annotation_access.css');
  $total = db_select('versionable_object_custom_config')
      ->fields('versionable_object_custom_config')
      ->countQuery()
      ->execute()
      ->fetchField();
  $query = db_select('versionable_object_access')
      ->fields('versionable_object_access', array(
        'pid',
        'allowed_users',
        'allowed_roles',
          )
      )
      ->condition('pid', $version->id, '=');
  $row = $query->execute()->fetchObject();
  $is_restricted = $row != FALSE;
  $allowed_roles = isset($row->allowed_roles) ? array_values(unserialize($row->allowed_roles)) : array();
  $allowed_users = isset($row->allowed_users) ? unserialize($row->allowed_users) : array();

  $query = db_select('versionable_object_config_choice')
      ->fields('versionable_object_config_choice', array('pid', 'cid'))
      ->condition('pid', $version->id, '=');
  $result = $query->execute()->fetchObject();
  $chosen = $result ? $result->cid : FALSE;
  $checked = ($chosen && $chosen != 'none');
  $query = db_select('versionable_object_custom_config', 'v')
          ->fields('v')
          ->extend('PagerDefault')->limit(10);
  $configs = $query->execute()->fetchAll();
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $css_extensions = array('css');
  $xsl_extensions = array('xsl xslt');
  $users = array();
  $result = db_query('SELECT u.uid, u.name FROM {users} u');
  foreach ($result as $user) {
    $user->uid == 0 ? $users['anonymous'] = variable_get('anonymous', t('Anonymous')) : $users[$user->uid] = $user->name;
    if ($user->uid == 1) {
      $admin_user = $user->name;
      $form_state['islandora_xacml']['admin_user'] = $user->name;
    }
  }

  $roles = array();
  $result = db_query('SELECT r.rid, r.name FROM {role} r');
  foreach ($result as $role) {
    $role->rid == 0 ? $roles['anonymous'] = variable_get('anonymous', t('Anonymous')) : $roles[$role->rid] = $role->name;
  }

  $form = array();
  $form['version_id'] = array(
    '#type' => 'hidden',
    '#value' => $version->id,
  );
  $form['critical_edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => $critical_edition_pid,
  );
  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object Viewing'),
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="emicdora_access_selectors">',
    '#suffix' => '</div>',
  );
  $form['access']['annotation_access_restricted'] = array(
    '#type' => 'radios',
    '#options' => array(0 => t('Display all annotations.'), 1 => t('Only display annotations created by the Users and Roles selected below')),
    '#default_value' => intval($is_restricted),
  );
  $form['access']['access_wrapper'] = array(
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="annotation_access_restricted"]' => array('value' => '1'),
      ),
    ),
  );
  $form['access']['access_wrapper']['users'] = array(
    '#type' => 'select',
    '#title' => t('Allowed Users'),
    '#default_value' => $allowed_users,
    '#options' => $users,
    '#multiple' => TRUE,
    '#size' => 10,
    '#prefix' => '<div class="emicdora_access_selector">',
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_selector')),
  );
  $form['access']['access_wrapper']['roles'] = array(
    '#type' => 'select',
    '#title' => t('Allowed Roles'),
    '#options' => $roles,
    '#multiple' => TRUE,
    '#size' => 10,
    '#default_value' => $allowed_roles,
    '#prefix' => '<div class="emicdora_access_selector">',
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_selector')),
  );
  $form['access']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#prefix' => '<div class="emicdora_access_selector">',
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_button'), 'data-action' => 'cancel'),
  );
  $form['access']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Access Configuration'),
    '#prefix' => '<div class="emicdora_access_selector">',
    '#suffix' => '</div>',
    '#attributes' => array('class' => array('emicdora_button'), 'data-action' => 'save'),
  );
  if ($total) {
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
      '#default_value' => $checked,
    );

    $form['select_config']['table_wrapper'] = array(
      '#type' => 'fieldset',
      '#states' => array(
        'visible' => array(
          ':input[name="use_custom"]' => array('checked' => TRUE),
        ),
      ),
    );
    foreach ($configs as $config) {
      $css = t("Default");
      $xslt = t("Default");
      if ($config->css) {
        $css = l(t("View CSS"), "islandora/version_viewer/configuration/download/{$config->cid}/css");
      }
      if ($config->xslt) {
        $xslt = l(t("View XSLT"), "islandora/version_viewer/configuration/download/{$config->cid}/xslt");
      }

      $delete = l(t("Delete"), "islandora/version_viewer/configuration/delete/$config->cid/{$version->id}", array(
        'query' => drupal_get_destination(),
      ));
      $renderable_span_array = array(
        'css_span' => array(
          '#type' => 'markup',
          '#markup' => "<span class = 'config_url'>$css</span>",
        ),
        'xslt_span' => array(
          '#type' => 'markup',
          '#markup' => "<span class = 'config_url'>$xslt</span>",
        ),
        'delete_span' => array(
          '#type' => 'markup',
          '#markup' => "<span class = 'config_url'>$delete</span>",
        ),
      );
      $rows[$config->cid] = array(
        'label' => $config->label,
        'description' => $config->description,
        'download' => drupal_render($renderable_span_array),
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
      '#default_value' => $chosen,
      '#js' => FALSE,
      '#multiple' => FALSE,
    );
    $form['select_config']['table_wrapper']['config_table'] = $table_element;
    $form['select_config']['table_wrapper']['pager'] = array(
      '#type' => 'markup',
      '#markup' => theme('pager', array('quantity' => 10)),
    );
    $form['select_config_wrapper'] = array(
      '#prefix' => '<div class = "control_buttons">',
      '#suffix' => '</div>',
    );
    $form['select_config_wrapper']['select_config']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#attributes' => array('class' => array('emicdora_button'), 'data-action' => 'cancel'),
    );

    $form['select_config_wrapper']['select_config']['save_config'] = array(
      '#type' => 'submit',
      '#value' => t('Save Configuration'),
      '#attributes' => array('class' => array('emicdora_button'), 'data-action' => 'save_config'),
    );
  }

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
    '#description' => t('Select a file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $css_extensions[0])),
    '#default_value' => isset($form_state['values']['custom_css']) ? $form_state['values']['custom_css'] : NULL,
    '#upload_location' => file_default_scheme() . '://',
    '#upload_validators' => array(
      'file_validate_extensions' => $css_extensions,
      // Assume its specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['custom_forms']['custom_xslt'] = array(
    '#title' => t('XSLT File'),
    '#name' => 'files[custom_xslt]',
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select a file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $xsl_extensions[0])),
    '#default_value' => isset($form_state['values']['custom_xslt']) ? $form_state['values']['custom_xslt'] : NULL,
    '#upload_location' => file_default_scheme() . '://',
    '#upload_validators' => array(
      'file_validate_extensions' => $xsl_extensions,
      // Assume its specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['custom_forms']['create'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
    '#attributes' => array('class' => array('emicdora_button'), 'data-action' => 'create'),
  );
  return $form;
}

/**
 * Validation for form.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal Form State
 */
function islandora_emicdora_annotation_availability_validate(array $form, array &$form_state) {
  $action = $form_state['triggering_element']['#attributes']['data-action'];
  switch ($action) {
    case 'save':
      if ($form_state['values']['annotation_access_restricted'] && empty($form_state['values']['users']) && empty($form_state['values']['roles'])) {
        form_set_error('users', t('Please select at least one entry from either Users or Roles'));
      }

      break;

    case 'create':
      if (empty($form_state['values']['custom_forms']['custom_css']) && empty($form_state['values']['custom_forms']['custom_xslt'])) {
        form_set_error('custom_css', t('CSS or XSLT file is required.'));
      }
      if (empty($form_state['values']['custom_forms']['config_label'])) {
        form_set_error('config_label', t('Label is required.'));
      }
      if (empty($form_state['values']['custom_forms']['config_description'])) {
        form_set_error('config_description', t('Description is required.'));
      }
  }
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function islandora_emicdora_annotation_availability_submit(array &$form, array &$form_state) {
  module_load_include('inc', 'versionable_object_viewer', 'includes/db');
  $action = $form_state['triggering_element']['#attributes']['data-action'];
  $version_pid = $form_state['values']['version_id'];
  $critical_edition_pid = $form_state['values']['critical_edition_pid'];
  switch ($action) {
    case 'cancel':
      $form_state['redirect'] = "islandora/object/$critical_edition_pid/version/$version_pid";
      break;

    case 'save':
      $restricted = $form_state['values']['annotation_access_restricted'];
      if ($restricted) {
        $users = serialize($form_state['values']['users']);
        $roles = serialize($form_state['values']['roles']);
        versionable_object_viewer_access_upsert($version_pid, $users, $roles);
      }
      else {
        db_delete('versionable_object_access')
            ->condition('pid', $version_pid)
            ->execute();
      }
      $form_state['redirect'] = "islandora/object/$critical_edition_pid/version/$version_pid";
      break;

    case 'save_config':
      $cid = 'none';
      if ($form_state['values']['use_custom']) {
        $cid = $form_state['values']['config_table'];
        versionable_object_viewer_config_upsert($version_pid, $cid);
      }
      else {
        db_delete('versionable_object_config_choice')
            ->condition('pid', $version_pid)
            ->execute();
      }

      break;

    case 'create':
      $css_url = NULL;
      if ($form_state['values']['custom_forms']['custom_css']) {
        $css_file = file_load($form_state['values']['custom_forms']['custom_css']);
        $css = file_get_contents(drupal_realpath($css_file->uri));
        // A bug in the pluploader causes it to ignore #upload_location.

        $public_css_file = drupal_tempnam(file_default_scheme() . '://', 'css');
        $css_file = file_save_data($css, $public_css_file);
        $css_file->status = 1;
        file_save($css_file);
        $css_url = file_create_url($css_file->uri);
      }
      $xslt_url = NULL;
      if ($form_state['values']['custom_forms']['custom_xslt']) {
        $xslt_file = file_load($form_state['values']['custom_forms']['custom_xslt']);
        $xslt = file_get_contents(drupal_realpath($xslt_file->uri));
        $public_xslt_file = drupal_tempnam(file_default_scheme() . '://', 'xslt');
        $xslt_file = file_save_data($xslt, $public_xslt_file);
        $xslt_file->status = 1;
        file_save($xslt_file);
        $xslt_url = file_create_url($xslt_file->uri);
      }

      $label = $form_state['values']['custom_forms']['config_label'];
      $description = $form_state['values']['custom_forms']['config_description'];
      $cid = db_insert('versionable_object_custom_config')
          ->fields(array(
            'pid' => $version_pid,
            'label' => $label,
            'description' => $description,
            'css' => $css_url,
            'xslt' => $xslt_url,
              )
          )
          ->execute();
      versionable_object_viewer_config_upsert($version_pid, $cid);
      drupal_set_message(t('Configuration saved and associated with this version.'));
      $form_state['redirect'] = "islandora/object/$critical_edition_pid/version/$version_pid";
      break;
  }
}

/**
 * The delete datastream configuration form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param int $cid
 *   The configuration to be deleted.
 * @param string $pid
 *   PID associated with config to be deleted.
 *
 * @return array
 *   The drupal form definition.
 */
function versionable_object_viewer_delete_config_form(array $form, array &$form_state, $cid, $pid) {
  $form['cid'] = array(
    '#type' => 'hidden',
    '#value' => $cid,
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  return confirm_form($form, t('Are you sure you want to delete this configuration?'), "islandora/version_viewer/annotation/$pid", t('This action cannot be undone.'), t('Delete'), t('Cancel')
  );
}

/**
 * Submit function for configuration deletion.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function versionable_object_viewer_delete_config_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'versionable_object_viewer', 'includes/db');
  $cid = $form_state['values']['cid'];
  $pid = $form_state['values']['pid'];
  versionable_object_viewer_delete_config($cid, $pid);
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
  $mime_map = array(
    'css' => 'text/css',
    'xslt' => 'text/xml',
  );
  $mimetype = $mime_map[$type];
  $query = db_select('versionable_object_custom_config')
      ->fields('versionable_object_custom_config', array($type))
      ->condition('cid', $cid, '=');
  $results = $query->execute()->fetchObject();
  header("Content-Type: $mimetype", TRUE);
  header('Content-Disposition: attachment; filename="File.' . $type . '"');
  switch ($type) {
    case 'css':
      echo file_get_contents($results->css);
      break;

    case 'xslt':
      echo file_get_contents($results->xslt);
      break;
  }
}
