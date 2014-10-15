<?php

/**
 * @file
 * Reconfigures Calliope inputs.
 */

/**
 * Reconfigures stock Calliope json.
 *
 * @global url $base_url
 *
 * @param string $collation
 *   Collation name
 */
function emicdora_fix_calliope_inputs($collation) {
  global $base_url;
  $collation = str_replace(' ', '_', $collation);
  $mvd_url = variable_get('mvd_server_url', $base_url) . variable_get('austese_server_prefix', '');
  try {
    $json = file_get_contents("$mvd_url/json/list/$collation");
  }
  catch (Exception $exc) {
    watchdog_exception("emicdora", $exc);
  }

  $results = json_decode($json, TRUE);
  foreach ($results['versions'] as &$version) {
    $file_name = str_replace(' ', '_', $version['version']);
    $version['longname'] = str_replace('/Base/', '', $file_name);
  }
  echo json_encode($results);
}
