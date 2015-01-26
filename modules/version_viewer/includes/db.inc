<?php

/**
 * @file
 * Defines all the callbacks this module implements.
 */

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

  if (!$results) {
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
    $pid = $results->pid;
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
 * Inserts or updates config choice.
 *
 * @param string $version_pid
 *   PID of version
 * @param int $cid
 *   Unique identifier for configuration
 */
function versionable_object_viewer_config_upsert($version_pid, $cid) {
  $query = db_select('versionable_object_config_choice')
      ->fields('versionable_object_config_choice', array('pid'))
      ->condition('pid', $version_pid, '=');
  $results = $query->execute()->fetchObject();

  if (!$results) {
    db_insert('versionable_object_config_choice')
        ->fields(array(
          'cid' => $cid,
          'pid' => $version_pid,
            )
        )
        ->execute();
  }
  else {
    db_update('versionable_object_config_choice')
        ->fields(array('pid' => $version_pid, 'cid' => $cid))
        ->condition('pid', $version_pid)
        ->execute();
  }
}
