<?php

/**
 * @file
 * Handles the server side of collation editing.
 */

/**
 * Ajax callback for collation editing.
 */
function emicdora_collation_edit() {
  module_load_include('inc', 'collation', 'includes/calliope');
  $left_window = $_POST['context_deleted'];
  $right_window = $_POST['context_added'];
  $query = array(
    'emic_data.collation' => $_POST['collation_id'],
    'emic_data.diff_kind' => 'deleted'
  );
  $file = emicdora_calliope_get_collation_file($query);
  $deleted_panes = explode('-->', $file['emic_data']['contents']);
  $css = $deleted_panes[0];
  $xml = $deleted_panes[1];
  $nodes = new SimpleXMLElement($xml, LIBXML_NOCDATA, true);
  echo $deleted_panes[1];
}
