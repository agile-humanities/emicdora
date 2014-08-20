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
  $deleted = $_POST['context_deleted'];
  $deleted_spans = explode('</span>', $deleted);
  $marshalled_deleted = emicdora_marshall_spans('deleted');
  $marshalled_added = emicdora_marshall_spans('added');
  $query = array(
    'emic_data.collation' => $_POST['collation_id'],
    'emic_data.diff_kind' => 'deleted'
  );
  $deleted_pane_contents = $_POST['all_deleted'];
  $spans_deleted = explode('</span>', $deleted_pane_contents);
  $fixed_deleted = "<div id = 'emicdora_deleted'>" . emicdora_fix_calliope($spans_deleted, 'deleted') . "</div>";
  $added_pane_contents = $_POST['all_added'];
  $spans_added = explode('</span>', $added_pane_contents);
  $fixed_added = "<div id = 'emicdora_added'>" . emicdora_fix_calliope($spans_added, 'added') . "</div>";
  $results = array(
    'new_deleted' => emicdora_updated_div_contents($fixed_deleted, $marshalled_deleted, 'deleted'),
    'new_added' => emicdora_updated_div_contents($fixed_added, $marshalled_added, 'added'),
  );
  echo json_encode($results);
}

/**
 * Repairs possibly damaged Calliope.
 *
 * @param array $spans
 *   array of spans from file
 * @param string $type
 *   'delete' or 'added'
 *
 * @return xml
 *   Repaired xml
 */
function emicdora_fix_calliope($spans, $type) {
  $repaired = array();
  foreach ($spans as $index => $span) {
    $span_pos = strpos(trim($span), '<span');
    if (strpos(trim($span), '<span') !== 0) {
      $repaired[$index] = "<span class='$type'>$span</span> ";
    }
    else {
      $repaired[$index] = "$span</span>";
    }
  }
  return implode('', $repaired);
}

/**
 * Pairs content with identified span tags.
 *
 * @param string $type
 *   'added' or 'deleted'
 *
 * @return array
 *   Associative array id => content
 */
function emicdora_marshall_spans($type) {
  $input = $_POST["context_$type"];
  $spans = array_filter(explode('</span>', $input));
  foreach ($spans as $candidate) {
    $dom = new DOMDocument;
    $dom->loadXML($candidate . '</span>');
    $items = $dom->getElementsByTagName('span');
    $id = $items->item(0)->getAttribute('id');
    $text_value = $items->item(0)->nodeValue;
    $marshalled_spans[$id] = $text_value;
  }
  return $marshalled_spans;
}

/**
 * Updates div contents with incoming request.
 *
 * @param xml $fixed_content
 *   Repaired content from viewer
 * @param array $marshalled_content
 *   id => tag content
 * @param string $type
 *   added or deleted
 *
 * @return xml
 *   new div contents
 */
function emicdora_updated_div_contents($fixed_content, $marshalled_content, $type) {
  $config = array(
    'added' => 'versionview-1011-body',
    'deleted' => 'versionview-1010-body',
  );
  $dom = new DOMDocument;
  $dom->loadHTML($fixed_content);
  $unmerged = TRUE;
  foreach ($marshalled_content as $id => $content) {
    $node = $dom->getElementById($id);
    if ($node) {
      $node_value = $node->nodeValue;
      $class = $node->getAttribute('class');
      if ($node_value && $unmerged) {
        if ($node_value == $content) {
          $node->nodeValue = $_POST["text_$type"];
          $node_value = $_POST["text_$type"];
          $node->setAttribute('class', 'merged');
          $node->removeAttribute('data-variant');
          $unmerged = FALSE;
          continue;
        }
      }
      if ($node_value != $content) {
        $new_value = str_replace($content, '', $node_value);
        $node->nodeValue = str_replace($content, '', $new_value);
      }
    }
  }
  $node = $dom->getElementById($config[$type]);
  return $dom->saveXML($node);
}
