<?php

/**
 * @file
 * Contains term selection functionality
 */

/**
 * Emicdora Terms form.
 */
function emicdora_configure_version_links($form, &$form_state, $critical_edition) {
  module_load_include('inc', 'islandora_solr', 'includes/admin');
  module_load_include('inc', 'emicdora', 'includes/db');

  // Create form.
  $form['#attached'] = array(
    'css' => array(
      drupal_get_path('module', 'emicdora') . '/css/emicdora_terms.css',
    ),
    'library' => array(
      array('system', 'ui.dialog'),
    ),
    'js' => array(
      drupal_get_path('module', 'emicdora') . '/js/emicdora_terms.js',
    ),
    'drupal_set_title' => array(
      array(
        t('Source Material / Version Display Configuration'),
      ),
    ),
  );
  $form['critical_edition_pid'] = array(
    '#type' => 'hidden',
    '#value' => $critical_edition->id,
  );
  $form['emicdora_tabs'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 5,
  );

  // Default display settings.
  $form['emicdora_tabs']['version_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  // Display version fields.
  $version_terms = array(
    '#type' => 'item',
    '#title' => t('Display Version fields'),
    '#description' => t('Set labels for Solr fields to be included in the search results.'),
    '#tree' => TRUE,
    '#prefix' => '<div id="emicdora-version-wrapper">',
    '#suffix' => '</div>',
    '#field_type' => 'version_fields',
    '#theme' => 'emicdora_fields',
  );

  // Display source fields.
  $source_terms = array(
    '#type' => 'item',
    '#title' => t('Display Source fields'),
    '#description' => t('Set labels for Solr fields to be included in the search results.'),
    '#tree' => TRUE,
    '#prefix' => '<div id="emicdora-source-wrapper">',
    '#suffix' => '</div>',
    '#field_type' => 'source_fields',
    '#theme' => 'emicdora_fields',
  );

  // Create terms/fields.
  emicdora_settings_fields($form_state, $version_terms, 'version', $critical_edition->id);
  emicdora_settings_fields($form_state, $source_terms, 'source', $critical_edition->id);

  // Result fields.
  $form['emicdora_tabs']['version_settings']['emicdora_version_fields'] = $version_terms;
  $form['emicdora_tabs']['version_settings']['emicdora_source_fields'] = $source_terms;
  if (isset($version_terms['terms'])) {
    $form_state['version_terms'] = $version_terms['terms'];
  }

  if (isset($source_terms['terms'])) {
    $form_state['source_terms'] = $source_terms['terms'];
  }
  // The content of the popup dialog.
  $form['emicdora_dialog'] = array(
    '#theme_wrappers' => array('container'),
    '#id' => 'emicdora-admin-dialog',
    '#weight' => 50,
  );
  $form['emicdora_dialog']['title'] = array(
    '#markup' => '<h2 id="emicdora-admin-dialog-title"></h2>',
  );
  $form['emicdora_dialog']['body'] = array(
    '#theme_wrappers' => array('container'),
    '#id' => 'emicdora-admin-dialog-body',
    '#markup' => t('Default dialog text'),
  );

  // Actions.
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 0,
    '#submit' => array('emicdora_admin_settings_submit'),
  );

  if (!empty($_POST) && form_get_errors()) {
    drupal_set_message(t('Error: the settings have not been saved.'), 'error');
  }
  return $form;
}

/**
 * Populates fields.
 *
 * @param array $form_state
 *   Drupal form state
 * @param array $version_terms
 *   Terms for sorting
 * @param string $result_type
 *   Type of field
 */
function emicdora_settings_fields(&$form_state, &$version_terms, $result_type, $critical_edition) {
  module_load_include('inc', 'emicdora', 'includes/db');
  // Set variables.
  switch ($result_type) {
    case 'version':
      $field_type = 'version_fields';
      $field_type_class = "version-fields";
      $title = t("Add new version category");
      break;

    case 'source':
      $field_type = 'source_fields';
      $field_type_class = "source-fields";
      $title = t("Add new source category");
      break;
  }

  // Ways to populate the fields table:
  // 1. Saved values from the DB
  // 2. Triggered element
  // 2.a. Add field
  // 2.b. Remove field
  // 2.c. Configure field:
  // - Prepare array to populate dialog.
  // 2.d. Dialog submit:
  if ($form_state['rebuild'] == FALSE) {

    // Get values from DB unfiltered, not simplified and incremental keys.
    $records = emicdora_get_terms($critical_edition, $result_type);
    // Set fields to populate the admin table.
    $fields = array();
    foreach ($records as $key => $record) {
      $fields[$key] = array(
        'solr_field' => $record['solr_field'],
        'field_type' => $record['field_type'],
        'weight' => $record['weight'],
      );
    }
    // Move solr_field_settings into the form state.
    foreach ($records as $key => $value) {
      $solr_field = $value['solr_field'];
      $solr_field_settings = $value['solr_field_settings'];
      $form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field] = $solr_field_settings;
    }
  }
  // 2. Triggered element (Add item or remove).
  if (isset($form_state['triggering_element']) &&
      $form_state['triggering_element']['#field_type'] == $field_type) {
    // Unset terms from input array.
    if (isset($form_state['input']['emicdora_' . $field_type]['terms'])) {
      $form_state['values']['emicdora_' . $field_type]['terms'] = $form_state['input']['emicdora_' . $field_type]['terms'];
    }
    unset($form_state['input']['emicdora_' . $field_type]);

    // 2.a add field.
    if ($form_state['triggering_element']['#name'] == $field_type . '_add_more') {
      // Get new solr field value.
      $add_solr_field = $form_state['values']['emicdora_' . $field_type]['add_solr_field'];
      // Check if the value is empty.
      $solr_field_error = NULL;
      if (empty($add_solr_field)) {
        $solr_field_error = t('Field cannot be empty.');
      }
      // Add new field to values.
      if ($solr_field_error == NULL) {
        $form_state['values']['emicdora_' . $field_type]['terms'][] = array('solr_field' => $add_solr_field);
      }
    }
    // 2.b. remove field.
    if ($form_state['triggering_element']['#value'] == t('Remove')) {
      // Triggered field #.
      $field = $form_state['triggering_element']['#field'];
      $solr_field = $form_state['values']['emicdora_' . $field_type]['terms'][$field]['solr_field'];
      // Remove field.
      array_splice($form_state['values']['emicdora_' . $field_type]['terms'], $field, 1);
      // Remove field values.
      unset($form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field]);
    }
    // 2.c. configure field.
    if ($form_state['triggering_element']['#value'] == t('Configure')) {
      // Triggered field #.
      $field = $form_state['triggering_element']['#field'];
      // Get field + dialog values and merge.
      $values = isset($form_state['values']['emicdora_' . $field_type]['terms'][$field]) ? $form_state['values']['emicdora_' . $field_type]['terms'] : array();
      $solr_field = isset($form_state['values']['emicdora_' . $field_type]['terms'][$field]['solr_field']) ? $form_state['values']['emicdora_' . $field_type]['terms'][$field]['solr_field'] : '';
      $solr_field_settings = array();
      if (isset($form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field])) {
        if ($form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field]) {
          $solr_field_settings = $form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field];
        }
      }
      $values = array_merge_recursive($values, $solr_field_settings);
      // Prepare dialog values.
      $dialog_id = 'edit-emicdora-' . $field_type_class . '-terms-' . $field . '-dialog-button';
      $form_state['dialog'] = array(
        'solr_field' => $solr_field,
        'field_type' => $field_type,
        'dialog_id' => $dialog_id,
        'values' => $values,
      );
    }
    // 2.d. Dialog submit.
    if ($form_state['triggering_element']['#value'] == t('Dialog')) {
      // Triggered field #.
      $field = $form_state['triggering_element']['#field'];
      // Get $solr_field.
      $term_label = $form_state['values']['emicdora_' . $field_type]['terms'][$field]['solr_field'];
      // Get dialog values from recent dialog submit.
      $dialog_values = json_decode($form_state['input']['_table_rows'], TRUE);
      // Strip out the data from the rows that were present in the tableselect.
      foreach ($dialog_values as &$val) {
        foreach ($val as $key => $data) {
          $val[$key] = $data['data'];
        }
      }
      // Save the dialog values in the form state.
      $form_state['solr_field_settings']['emicdora_' . $field_type][$term_label] = emicdora_handle_solr_field_settings($dialog_values, $field_type);
    }
  }
  // Prepare fields array.
  if (isset($form_state['values']['emicdora_' . $field_type]['terms'])) {
    $fields = $form_state['values']['emicdora_' . $field_type]['terms'];
  }

  // If no fields available set string.
  if (empty($fields)) {
    $version_terms['no_fields'] = array(
      '#markup' => t('No fields defined'),
    );
  }
  // Else render fields.
  else {
    $term = array();
    $count = count($fields);
    $fields = array_values($fields);
    foreach ($fields as $key => $value) {
      if (!isset($value['solr_field'])) {
        continue;
      }
      $solr_field = $value['solr_field'];

      $term['draggable_handler'] = array(
        '#type' => 'item',
        '#markup' => '',
      );
      $term['solr_field_render'] = array(
        '#type' => 'item',
        '#markup' => isset($value['solr_field']) ? $value['solr_field'] : '',
      );
      $term['solr_field'] = array(
        '#type' => 'hidden',
        '#value' => isset($value['solr_field']) ? $value['solr_field'] : '',
      );
      $solr_field_settings = NULL;
      if (isset($form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field]['label'])) {
        $solr_field_settings = function_exists('i18n_string') ?
            i18n_string("islandora_solr:field_settings:$field_type:label:$solr_field", $form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field]['label']) :
            $form_state['solr_field_settings']['emicdora_' . $field_type][$solr_field]['label'];
      }

      $term['weight'] = array(
        '#type' => 'weight',
        '#default_value' => isset($value['weight']) ? $value['weight'] : $key,
        '#delta' => $count >= 10 ? $count : 10,
        '#attributes' => array('class' => array('solr-weight-' . $field_type_class)),
      );
      $term['configure'] = array(
        '#type' => 'link',
        '#title' => t('configure'),
        '#href' => '#',
        '#options' => array('attributes' => array('class' => 'emicdora-configure-link')),
      );
      $term['configure_button'] = array(
        '#type' => 'submit',
        '#value' => t('Configure'),
        '#field' => $key,
        '#field_type' => $field_type,
        '#attributes' => array('class' => array('emicdora-configure-submit')),
        '#submit' => array('emicdora_update_fields_submit'),
        '#name' => $field_type . '_configure_' . $key,
        '#id' => 'edit-emicdora-' . $field_type_class . '-terms-' . $key . '-configure-button',
        '#ajax' => array(
          'callback' => 'emicdora_field_configure',
          'wrapper' => 'emicdora-' . $result_type . '-wrapper',
          'effect' => 'none',
          'event' => 'click',
          'progress' => array('message' => '', 'type' => 'throbber'),
        ),
      );
      $term['remove'] = array(
        '#type' => 'link',
        '#title' => t('remove'),
        '#href' => '#',
        '#options' => array('attributes' => array('class' => 'emicdora-remove-link')),
      );
      $term['remove_button'] = array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#field' => $key,
        '#field_type' => $field_type,
        '#attributes' => array('class' => array('emicdora-remove-submit')),
        '#submit' => array('emicdora_update_fields_submit'),
        '#name' => $field_type . '_remove_' . $key,
        '#id' => 'edit-emicdora-' . $field_type_class . '-terms-' . $key . '-remove-button',
        '#ajax' => array(
          'callback' => 'emicdora_update_fields',
          'wrapper' => 'emicdora-' . $result_type . '-wrapper',
          'effect' => 'none',
          'event' => 'click',
          'progress' => array('message' => '', 'type' => 'throbber'),
        ),
      );
      $term['dialog_button'] = array(
        '#type' => 'submit',
        '#value' => t('Dialog'),
        '#field' => $key,
        '#field_type' => $field_type,
        '#attributes' => array('class' => array('emicdora-dialog-submit')),
        '#submit' => array('emicdora_update_fields_submit'),
        '#name' => $field_type . '_dialog_' . $key,
        '#id' => 'edit-emicdora-' . $field_type_class . '-terms-' . $key . '-dialog-button',
        '#ajax' => array(
          'callback' => 'emicdora_update_fields',
          'wrapper' => 'emicdora-' . $result_type . '-wrapper',
          'effect' => 'none',
          'event' => 'click',
          'progress' => array('message' => '', 'type' => 'throbber'),
        ),
      );

      $version_terms['terms'][] = $term;
    }
  }
  $version_terms['add_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => $title,
    '#size' => 45,
    '#default_value' => '',
  );
  if (isset($solr_field_error) && !empty($solr_field_error)) {
    $version_terms['add_solr_field']['#title'] .= '<br /><span class="solr-admin-error">' . $solr_field_error . '</span>';
    $version_terms['add_solr_field']['#attributes'] = array('class' => array('error'));
  }

  $version_terms['add_more'] = array(
    '#type' => 'button',
    '#value' => t('Add'),
    '#attributes' => array('class' => array('emicdora-add-more-submit')),
    '#submit' => array('emicdora_update_fields_submit'),
    '#name' => $field_type . '_add_more',
    '#field_type' => $field_type,
    '#id' => 'edit-emicdora-' . $field_type_class . '-add-more',
    '#ajax' => array(
      'callback' => 'emicdora_update_fields',
      'wrapper' => 'emicdora-' . $result_type . '-wrapper',
      'event' => 'click',
      'effect' => 'none',
    ),
  );
}

/**
 * Ajax callback function for fields.
 *
 * Used by 'Add field', 'Remove' and 'Dialog' links/buttons.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 *
 * @return array
 *   The part of the form array that needs asynchronous updating.
 */
function emicdora_update_fields($form, &$form_state) {

  // Check what section the call is from.
  switch ($form_state['triggering_element']['#field_type']) {
    case 'version_fields':
      return $form['emicdora_tabs']['version_settings']['emicdora_version_fields'];

    case 'source_fields':
      return $form['emicdora_tabs']['version_settings']['emicdora_source_fields'];
  }
}

/**
 * Submit callback function for fields.
 *
 * Used by 'Add field', 'Configure', 'Remove' and 'Dialog'.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 */
function emicdora_update_fields_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * AJAX callback function for the modal dialog submit button.
 *
 * Closes the modal dialog and passes the values to the main form.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 *
 * @return array
 *   An array of AJAX commands.
 */
function emicdora_settings_dialog($form, &$form_state) {
  $dialog_id = $form_state['dialog']['dialog_id'];

  $commands = array();
  // Close dialog.
  $commands[] = ajax_command_invoke('#emicdora-admin-dialog', 'dialog', array('close'));
  // Call function which attaches the dialog form values to
  // Drupal.ajax.{dialog-button-id}.options.data.
  // After triggering the dialog ajax callback, the dialog values will be added
  // to the $form_state of the main form.
  $data = array(
    'id' => $dialog_id,
    'values' => $form_state['input'],
    'table_rows' => $form_state['complete form']['options']['table_wrapper']['table']['#options'],
  );
  $commands[] = ajax_command_invoke(NULL, 'emicdoraTermsDialogValues', array($data));
  // Trigger dialog button.
  $commands[] = ajax_command_invoke('#' . $dialog_id, 'trigger', array('click'));
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Form for result field settings.
 *
 * @params array $variables
 *   This parameter contains default values to be passed from the main form.
 */
function emicdora_settings_version_fields($form, &$form_state, $variables) {
  form_load_include($form_state, 'inc', 'islandora_solr', 'includes/admin');
  module_load_include('inc', 'islandora_solr', 'includes/admin');
  form_load_include($form_state, 'inc', 'emicdora', 'includes/configure_links');
  $form_state['dialog'] = $variables;
  $rows = array();
  if (isset($form_state['rows'])) {
    $rows = $form_state['rows'];
  }
  elseif (isset($variables['values']['fields'])) {
    foreach ($variables['values']['fields'] as $field) {
      $rows[$field['solr_field']] = array(
        'solr_field' => array('data' => $field['solr_field']),
        'value' => array('data' => $field['value']),
      );
    }
  }
  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#value'] == t('Add')) {
      if (!empty($form_state['values']['label'])) {
        $luke_result = islandora_solr_get_luke(NULL, $form_state['values']['label']);
        if (empty($luke_result['fields'])) {
          drupal_set_message(t('The field @field does not exist in Solr.', array('@field' => $form_state['values']['label'])), 'error');
        }
        else {
          if (!isset($rows[$form_state['values']['label']])) {
            if (empty($form_state['values']['value'])) {
              drupal_set_message(t('The @field must not have an empty value.', array('@field' => $form_state['values']['label'])), 'error');
            }
            else {
              $rows[$form_state['values']['label']] = array(
                'solr_field' => array('data' => $form_state['values']['label']),
                'value' => array('data' => $form_state['values']['value']),
              );
            }
          }
          else {
            drupal_set_message(t('The field @field already exists in this configuration.', array('@field' => $form_state['values']['label'])), 'error');
          }
        }
      }
      else {
        drupal_set_message(t('The Solr field value cannot be empty.'), 'error');
      }
    }
    if ($form_state['triggering_element']['#value'] == t('Remove selected')) {
      $to_remove = array_filter($form_state['values']['table']);
      if (empty($to_remove)) {
        drupal_set_message(t('Please select a field to remove.'), 'error');
      }
      else {
        foreach ($to_remove as $pid) {
          unset($rows[$pid]);
        }
      }
    }
  }
  $solr_field = $variables['solr_field'];
  $values = $variables['values'];
  $form['options'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('scroll')),
    '#id' => 'emicdora-admin-dialog-form',
  );
  $form['options']['table_wrapper'] = array(
    '#prefix' => '<div id="emicdora-solr-field-table">',
    '#suffix' => '</div>',
  );
  $form['options']['table_wrapper']['table'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'solr_field' => array('data' => t('Solr Field')),
      'value' => array('data' => t('Value')),
    ),
    '#options' => $rows,
    '#empty' => t('No fields defined for this configuration.'),
  );
  $form['options']['table_wrapper']['remove_selected'] = array(
    '#type' => 'button',
    '#value' => t('Remove selected'),
    '#ajax' => array(
      'wrapper' => 'emicdora-solr-field-table',
      'callback' => 'emicdora_field_table_ajax',
    ),
    '#access' => !empty($rows),
  );
  $form['options']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr Field'),
    '#default_value' => isset($values['label']) ? $values['label'] : '',
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    '#description' => t('Solr field.'),
  );
  $form['options']['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#default_value' => isset($values['value']) ? $values['value'] : '',
    '#description' => t('Value to search the field for. Can use any valid <a href="!url">Lucene</a> syntax', array(
      '!url' => 'http://wiki.apache.org/solr/SolrQuerySyntax',
    )),
  );
  $form['options']['add'] = array(
    '#type' => 'button',
    '#value' => t('Add'),
    '#ajax' => array(
      'wrapper' => 'emicdora-solr-field-table',
      'callback' => 'emicdora_field_table_ajax',
    ),
  );
  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-buttons')),
  );
  $path = drupal_get_path('module', 'emicdora');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Configure this field'),
    '#weight' => 5,
    '#field' => 'dialog_submit',
    '#field_type' => 'version_fields',
    '#name' => 'result-fields-dialog-submit',
    '#submit' => array('emicdora_update_fields_submit'),
    '#ajax' => array(
      'callback' => 'emicdora_settings_dialog',
      'wrapper' => 'emicdora-admin-dialog-form',
      'effect' => 'none',
      'event' => 'click',
      'progress' => array('message' => 'Working', 'type' => 'throbber'),
    ),
  );
  $form_state['rows'] = $rows;
  return $form;
}

/**
 * AJAX callback to return the portion of the form to be re-rendered.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   The portion of the form to be rendered.
 */
function emicdora_field_table_ajax($form, &$form_state) {
  return $form['options']['table_wrapper'];
}

/**
 * Validation for configuring links that kick off facet queries.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function emicdora_configure_version_links_validate($form, &$form_state) {
  if ($form_state['triggering_element']['#array_parents'] == array('actions', 'submit')) {
    module_load_include('inc', 'islandora_solr', 'includes/luke');
    $terms['version'] = isset($form_state['solr_field_settings']['emicdora_version_fields']) ? $form_state['solr_field_settings']['emicdora_version_fields'] : array();
    $terms['source'] = isset($form_state['solr_field_settings']['emicdora_source_fields']) ? $form_state['solr_field_settings']['emicdora_source_fields'] : array();
    foreach ($terms as $term_type => $fields) {
      if (!isset($form_state['values']["emicdora_{$term_type}_fields"]['terms'])) {
        continue;
      }
      elseif (empty($fields)) {
        form_set_error($term_type, t('Configuration must be defined for fields in @type.', array('@type' => $term_type)));
      }
    }
  }
}

/**
 * Submit handler for term selection.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal Form State
 */
function emicdora_admin_settings_submit($form, &$form_state) {
  module_load_include('inc', 'emicdora', 'includes/db');
  $critical_edition = $form_state['values']['critical_edition_pid'];
  emicdora_delete_terms($critical_edition);
  $terms['version'] = isset($form_state['solr_field_settings']['emicdora_version_fields']) ? $form_state['solr_field_settings']['emicdora_version_fields'] : array();
  $terms['source'] = isset($form_state['solr_field_settings']['emicdora_source_fields']) ? $form_state['solr_field_settings']['emicdora_source_fields'] : array();
  foreach ($terms as $term_type => $values) {
    $form_field = "emicdora_{$term_type}_fields";
    if (!isset($form_state['values'][$form_field]['terms'])) {
      continue;
    }
    $weights = array();
    foreach ($form_state['values'][$form_field]['terms'] as $category => $category_values) {
      $weights[intval($category_values['weight'])] = $category_values['solr_field'];
    }
    ksort($weights);
    foreach ($weights as $index => $weight) {
      $solr_field_settings = array();
      foreach ($values[$weight] as $label) {
        foreach ($label as $field) {
          $solr_field_settings['fields'][] = array(
            'solr_field' => $field['solr_field'],
            'value' => $field['value'],
          );
        }
      }
      $fields = array(
        'solr_field' => $weight,
        'field_type' => 'unused',
        'weight' => $index,
        'solr_field_settings' => $solr_field_settings,
      );
      emicdora_insert_terms($critical_edition, $term_type, $fields);
    }
  }
}

/**
 * Theme function to create an admin table for result fields.
 *
 * @param array $variables
 *   Array containing form elements to be themed.
 *
 * @return string
 *   Rendered table.
 */
function theme_emicdora_fields($variables) {
  $form = $variables['form'];

  $field_type = $form['#field_type'];

  // Render class name from field type.
  $field_type_class = str_replace('_', '-', $field_type);

  $rows = array();

  // If no_fields is set, we don't render all the fields.
  if (isset($form['no_fields'])) {
    $row = array();
    $row[] = array('data' => drupal_render($form['no_fields']), 'colspan' => 10);

    // Add to rows.
    $rows[] = array('data' => $row);
  }
  else {
    foreach ($form['terms'] as $key => $element) {
      if (is_array($element) && element_child($key)) {
        $row = array();
        $row[] = array('data' => drupal_render($form['terms'][$key]['draggable_handler']), 'class' => 'emicdora-multiple-drag');
        $row[] = array('data' => drupal_render($form['terms'][$key]['solr_field_render']), 'class' => 'emicdora-solr-field-render');
        $row[] = array('data' => drupal_render($form['terms'][$key]['weight']));
        $row[] = array('data' => drupal_render($form['terms'][$key]['configure']), 'class' => 'emicdora-operations');
        $row[] = array('data' => drupal_render($form['terms'][$key]['configure_button']), 'class' => 'emicdora-operations emicdora-operations-hidden');
        $row[] = array('data' => drupal_render($form['terms'][$key]['remove']), 'class' => 'emicdora-operations');
        $row[] = array('data' => drupal_render($form['terms'][$key]['remove_button']), 'class' => 'emicdora-operations emicdora-operations-hidden');
        $row[] = array('data' => drupal_render($form['terms'][$key]['dialog_button']), 'class' => 'emicdora-operations emicdora-operations-hidden');

        // Add to rows.
        $rows[] = array('data' => $row, 'class' => array('draggable'));
      }
    }
  }

  // Create header array.
  $header = array();
  $header[] = array(
    'data' => t('Label'),
    'colspan' => 2,
    'class' => 'emicdora-solr-field-header',
  );
  $header[] = array('data' => t('Weight'), 'class' => 'emicdora-solr-field-weight');
  $header[] = array('data' => t('Operations'), 'colspan' => 4);

  // Controls table.
  $rows_controls = array();
  // Add another item form elements.
  $rows_controls[] = array(
    'data' => array(
      array(
        'data' => drupal_render($form['add_solr_field']) . drupal_render($form['add_more']),
        'colspan' => 10,
      ),
    ),
    'no_striping' => 1,
    'class' => array('add-item-controls'),
  );

  // Render form.
  $output = '';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'emicdora-' . $field_type_class,
      'class' => array('emicdora-fields-table'),
    ),
  ));
  $output .= theme('table', array(
    'header' => array(),
    'rows' => $rows_controls,
    'attributes' => array('class' => array('emicdora-table-controls')),
  ));
  $output .= drupal_render_children($form);
  // Add tabledrag features.
  drupal_add_tabledrag('emicdora-' . $field_type_class, 'order', 'sibling', 'solr-weight-' . $field_type_class);

  return $output;
}

/**
 * AJAX callback function for 'Configure' link/button.
 *
 * Passes values from the field, loads the right form and populates and
 * triggers the modal dialog.
 *
 * @param array $form
 *   An associative array containing the form definition.
 * @param array $form_state
 *   An associative array containing the form state.
 *
 * @return array
 *   An array of AJAX commands.
 */
function emicdora_field_configure($form, &$form_state) {
  $variables = $form_state['dialog'];
  $field_type = $form_state['dialog']['field_type'];
  $solr_field = $form_state['dialog']['solr_field'];
  $form_function = emicdora_get_form_function($field_type);

  $commands = array();

  // Open dialog.
  $commands[] = ajax_command_invoke('#emicdora-admin-dialog', 'dialog', array('open'));
  // Dialog title.
  $title = t('Configure field:') . ' ' . $solr_field;
  $commands[] = ajax_command_invoke('#emicdora-admin-dialog-title', 'html', array($title));
  // Dialog body.
  $form = drupal_get_form($form_function, $variables);
  $body = drupal_render($form);
  $commands[] = ajax_command_invoke('#emicdora-admin-dialog-body', 'html', array($body));
  // Attach behaviors (re-attaches javascript).
  $commands[] = ajax_command_invoke(NULL, 'emicdoraTermsAttachBehaviors', array());

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Function which returns the form function name associated with a field type.
 *
 * @param string $field_type
 *   Field type to get the form function name for.
 *
 * @return bool|string
 *   A form function name associated with the field type, or FALSE if field
 *   type is not given or does not exist.
 */
function emicdora_get_form_function($field_type = NULL) {
  if (!empty($field_type)) {
    switch ($field_type) {
      case 'version_fields':
        $form_callback = 'emicdora_settings_version_fields';
        break;

      case 'source_fields':
        $form_callback = 'emicdora_settings_version_fields';
        break;

      default:
        return FALSE;
    }
    return $form_callback;
  }
  return FALSE;
}

/**
 * Select the fields from the raw input values the modal form.
 *
 * AJAX call fun.
 *
 * @todo This should probably be removed and values we don't need maybe should
 *   be filtered out before sending it back to the main form.
 */
function emicdora_handle_solr_field_settings($solr_field_settings = NULL, $field_type = NULL) {
  $to_return = FALSE;
  if (!empty($solr_field_settings) && !empty($field_type)) {
    switch ($field_type) {
      case 'version_fields':
        $to_return = array();
        foreach ($solr_field_settings as $row) {
          $to_return['fields'][] = $row;
        }
        break;

      case 'source_fields':
        $to_return = array();
        foreach ($solr_field_settings as $row) {
          $to_return['fields'][] = $row;
        }
        break;
    }
  }

  return $to_return;
}
