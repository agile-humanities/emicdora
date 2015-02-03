<?php

/**
 * @file
 * Menu callback and related helper functions for the workbench.
 */

/**
 * Display's the current users work bench.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   A Drupal renderable array, that displays the current users workbench.
 */
function emicdora_workbench_form($form, &$form_state) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  module_load_include('inc', 'islandora_bookmark', 'includes/api');
  $source_content_models = emicdora_source_content_models();
  $add_source_links = array();
  foreach ($source_content_models as $content_model => $label) {
    $add_source_links[] = array(
      'title' => t('Add @label', array('@label' => $label)),
      'href' => "workbench/add/$content_model",
    );
  }
  $form = array(
    '#tree' => TRUE,
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'emicdora') . '/css/workbench.css',
      ),
      'js' => array(
        drupal_get_path('module', 'emicdora') . '/js/workbench.js',
      ),
    ),
  );
  $form['description'] = array(
    '#type' => 'markup',
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#markup' => t('Welcome to your workbench. Here you can create new critical editions, access active editions, or remove editions from your workbench. Or you can ingest new source objects and manage collections in the Co-op.'),
  );
  $form['add_critical_edition'] = array(
    '#type' => 'submit',
    '#value' => t('Add Critical Edition'),
    '#access' => user_access(ISLANDORA_INGEST),
    '#attributes' => array(
      'class' => array(
        'emicdora-workbench-button',
      ),
    ),
  );
  $form['add_schema_transform'] = array(
    '#type' => 'submit',
    '#value' => t('Add New Schema'),
    '#access' => user_access(ISLANDORA_INGEST),
    '#attributes' => array(
      'class' => array(
        'emicdora-workbench-button',
      ),
    ),
  );
  $form['add_entities'] = array(
    '#type' => 'select',
    '#title' => t('Add Entity'),
    '#options' => array(
      'none' => t('-- Select --'),
      'person' => t('Add Person Entity'),
      'organization' => t('Add Organization Entity'),
      'title' => t('Add Title Entity'),
      'place' => t('Add Place Entity'),
    ),
    '#attributes' => array(
      'class' => array(
        'emicdora-workbench-entity-links',
      ),
    ),
    '#prefix' => '<div id="emicdora_add_entity">',
    '#suffix' => '</div>',
  );
  $form['add_sources'] = array(
    '#type' => 'select',
    '#title' => t('Add Source'),
    '#options' => array(
      'none' => t('-- Select --'),
      'islandora:bookCModel' => t('Add Text'),
      'islandora:sp-audioCModel' => t('Add Audio'),
      'islandora:sp_videoCModel' => t('Add Video'),
      'islandora:sp_large_image_cmodel' => t('Add Image'),
    ),
    '#attributes' => array(
      'class' => array(
        'emicdora-workbench-source-links',
      ),
    ),
    '#prefix' => '<div id="emicdora_add_source">',
    '#suffix' => '</div>',
  );
  // Get the user's default list and do some parsing.
  $list_id = islandora_bookmark_get_default_list();
  // User has yet to have a default list created, so let's do that now.
  if (!$list_id && variable_get('islandora_bookmark_create_user_default_lists', FALSE)) {
    global $user;
    $default_list_name = variable_get('islandora_bookmark_default_list_name', "@username's Default List");
    $default_list_name = format_string($default_list_name, array('@username' => $user->name));
    $persistent_bookmark = BookmarkDefaultDatabaseList::createNewList($default_list_name, 'bookmark_default', $user);
    if (variable_get('islandora_bookmark_share_default_list', FALSE)) {
      // The anonymous user (uid 0) is considered to be "Everyone".
      $persistent_bookmark->addUser('0');
    }
    $list_id = $persistent_bookmark->getId();
  }
  if ($list_id) {
    $list = Bookmark::getList($list_id);
    $pids = $list->getPids();
    $source_pids = array();
    $critical_pids = array();
    foreach ($pids as $pid) {
      $temp_obj = islandora_object_load($pid);
      $models = $temp_obj->models;
      $remove_button = array(
        '#type' => 'submit',
        '#name' => 'remove_pid_' . $pid,
        '#id' => 'remove_pid_' . $pid,
        '#pid' => $pid,
        '#value' => t('Remove'),
      );
      if (in_array('islandora:criticalEditionContainerCModel', $models)) {
        $critical_pids[$pid] = array(
          array(
            'data' => islandora_bookmark_generate_markup($pid),
          ),
          array(
            'data' => drupal_render($remove_button),
            'class' => 'operations',
          ),
        );
      }
      if (array_intersect(array_keys($source_content_models), $models)) {
        $source_pids[$pid] = array(
          array(
            'data' => islandora_bookmark_generate_markup($pid),
          ),
          array(
            'data' => drupal_render($remove_button),
            'class' => 'operations',
          ),
        );
      }
    }
    $form['critical_edition_bookmarks'] = array(
      '#type' => 'fieldset',
      '#title' => 'Critical Edition Bookmarks',
      '#attributes' => array(
        'class' => array(
          'emicdora-workbench-bookmark',
        ),
      ),
    );
    $form['critical_edition_bookmarks']['table'] = array(
      '#type' => 'tableselect',
      '#header' => array(
        t('Label'),
        t('Operations'),
      ),
      '#options' => $critical_pids,
      '#empty' => t('No Critical Editions present.'),
    );
    $form['critical_edition_bookmarks']['remove_selected'] = array(
      '#type' => 'submit',
      '#value' => t('Remove selected'),
      '#access' => count($critical_pids),
      '#name' => 'critical_edition_remove_selected',
    );
    $form['critical_edition_bookmarks']['remove_all'] = array(
      '#type' => 'submit',
      '#value' => t('Remove all'),
      '#access' => count($critical_pids),
      '#name' => 'critical_edition_remove_all',
    );
    $form['source_bookmarks'] = array(
      '#type' => 'fieldset',
      '#title' => 'Source Bookmarks',
      '#attributes' => array(
        'class' => array(
          'emicdora-workbench-bookmark',
        ),
      ),
    );
    $form['source_bookmarks']['table'] = array(
      '#type' => 'tableselect',
      '#header' => array(
        t('Label'),
        t('Operations'),
      ),
      '#options' => $source_pids,
      '#empty' => t('No Source objects present.'),
    );
    $form['source_bookmarks']['remove_selected'] = array(
      '#type' => 'submit',
      '#value' => t('Remove selected'),
      '#access' => count($source_pids),
      '#name' => 'source_bookmarks_remove_selected',
    );
    $form['source_bookmarks']['remove_all'] = array(
      '#type' => 'submit',
      '#value' => t('Remove all'),
      '#access' => count($source_pids),
      '#name' => 'source_bookmarks_remove_all',
    );
    $form_state['bookmark_id'] = $list_id;
  }
  else {
    drupal_set_message(t('No default Bookmark list created for this user. Please contact the administrator.'), 'warning');
  }
  return $form;
}

/**
 * Helper submit to redirect to the add Schema / Transform form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function emicdora_workbench_add_schema_transform_redirect($form, &$form_state) {
  $form_state['redirect'] = 'islandora/object/cwrc:schemaCollection/manage/overview/ingest';
}

/**
 * Validation function for the workbench form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function emicdora_workbench_form_validate($form, &$form_state) {
  // Due to how the buttons are rendered triggering element is misreporting
  // what is being selected. This first check sees if the user has chosen
  // to remove an individual pid.
  if (!emicdora_workbench_find_pid_to_remove($form_state)) {
    $critical_selected_parents = array(
      'critical_edition_bookmarks',
      'remove_selected',
    );
    $source_selected_parents = array(
      'source_bookmarks',
      'remove_selected',
    );
    if ($form_state['triggering_element']['#parents'] == $critical_selected_parents) {
      $checkboxes = array_filter($form_state['values']['critical_edition_bookmarks']['table']);
      if (empty($checkboxes)) {
        form_error($form['critical_edition_bookmarks']['table'], t('Please select a Critical Edition bookmark to remove.'));
      }
    }
    if ($form_state['triggering_element']['#parents'] == $source_selected_parents) {
      $checkboxes = array_filter($form_state['values']['source_bookmarks']['table']);
      if (empty($checkboxes)) {
        form_error($form['source_bookmarks']['table'], t('Please select a Source bookmark to remove.'));
      }
    }
  }
}

/**
 * Submit handler for the workbench form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function emicdora_workbench_form_submit($form, &$form_state) {
  $critical_remove = array(
    'critical_edition_bookmarks',
  );
  $source_remove = array(
    'source_bookmarks',
  );
  $critical_selected_parents = array_merge($critical_remove, array('remove_selected'));
  $source_selected_parents = array_merge($source_remove, array('remove_selected'));
  $critical_all_parents = array_merge($critical_remove, array('remove_all'));
  $source_all_parents = array_merge($source_remove, array('remove_all'));
  $individual_pid = emicdora_workbench_find_pid_to_remove($form_state);
  $list = Bookmark::getList($form_state['bookmark_id']);

  if ($individual_pid) {
    $pid = reset($individual_pid);
    $object = islandora_object_load($pid);
    $type = t('Source');
    if (in_array('islandora:criticalEditionContainerCModel', $object->models)) {
      $type = t('Critical Edition');
    }
    emicdora_workbench_remove_individual_pid($pid, $list, $type);
  }
  else {
    if ($form_state['triggering_element']['#parents'] == $critical_selected_parents) {
      $pids = array_filter($form_state['values']['critical_edition_bookmarks']['table']);
      emicdora_workbench_remove_selected_pids($pids, $list, t('Critical Edition'));
    }

    if ($form_state['triggering_element']['#parents'] == $source_selected_parents) {
      $pids = array_filter($form_state['values']['source_bookmarks']['table']);
      emicdora_workbench_remove_selected_pids($pids, $list, t('Source'));
    }

    if ($form_state['triggering_element']['#parents'] == $critical_all_parents) {
      emicdora_workbench_remove_all_pids($list, t('Critical Edition'));
    }

    if ($form_state['triggering_element']['#parents'] == $source_all_parents) {
      emicdora_workbench_remove_all_pids($list, t('Source'));
    }

    if ($form_state['triggering_element']['#parents'] == array('add_critical_edition')) {
      $form_state['redirect'] = 'emicdora/source/add/islandora:criticalEditionContainerCModel/FALSE/TRUE';
    }

    if ($form_state['triggering_element']['#parents'] == array('add_schema_transform')) {
      $form_state['redirect'] = 'islandora/object/cwrc:schemaCollection/manage/overview/ingest';
    }
  }
}

/**
 * Finds the remove button that was pressed.
 *
 * @param array $form_state
 *   An array representing the Drupal form state.
 *
 * @return array
 *   An array containing the PID.
 */
function emicdora_workbench_find_pid_to_remove($form_state) {
  $to_remove = preg_grep('/remove_pid_.*/', array_keys($form_state['input']));
  return preg_replace('/remove_pid_(.*)/', '$1', $to_remove);
}

/**
 * Removes an individual pid from a list.
 *
 * @param string $pid
 *   The PID to be removed.
 * @param Bookmark $list
 *   A Bookmark representing the list.
 * @param string $type
 *   The type of list being removed from.
 */
function emicdora_workbench_remove_individual_pid($pid, $list, $type) {
  $list->removePid($pid);
  emicdora_remove_pid_message($pid, $type);
}

/**
 * Removes the selected pids from a list.
 *
 * @param array $pids
 *   An array containing strings of pids to be removed.
 * @param Bookmark $list
 *   A Bookmark representing the list.
 * @param string $type
 *   The type of list being removed from.
 */
function emicdora_workbench_remove_selected_pids($pids, $list, $type) {
  $list->removePids($pids);
  emicdora_remove_pid_message($pids, $type);
}

/**
 * Removes all pids from a list.
 *
 * @param Bookmark $list
 *   A Bookmark representing the list.
 * @param string $type
 *   The type of list being removed from.
 */
function emicdora_workbench_remove_all_pids($list, $type) {
  $pids = $list->getPids();
  $pids_to_remove = array();
  $source_content_models = emicdora_source_content_models();
  foreach ($pids as $pid) {
    $object = islandora_object_load($pid);
    if ($type == t('Critical Edition')) {
      if (in_array('islandora:criticalEditionContainerCModel', $object->models)) {
        $pids_to_remove[$pid] = $pid;
      }
    }
    else {
      if (array_intersect(array_keys($source_content_models), $object->models)) {
        $pids_to_remove[$pid] = $pid;
      }
    }
  }
  $list->removePids($pids_to_remove);
  drupal_set_message(t('All bookmarks have been removed from @type Bookmarks.', array('@type' => $type)));
}

/**
 * Helper function to display feedback to the user on removal.
 *
 * @param array $pids
 *   An array containing strings of pids to be removed.
 * @param string $list_type
 *   The type of list being removed from.
 */
function emicdora_remove_pid_message($pids, $list_type) {
  if (!is_array($pids)) {
    $pids = array($pids);
  }
  foreach (array_map('islandora_object_load', $pids) as $removed_object) {
    drupal_set_message(t('The @type @objectname has been removed from @list Bookmarks.', array(
      '@type' => variable_get('islandora_bookmark_type', 'bookmark'),
      '@objectname' => $removed_object->label,
      '@list' => $list_type,
    )));
  }
}
