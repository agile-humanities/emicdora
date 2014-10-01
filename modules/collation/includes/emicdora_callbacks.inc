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
  $config = emicdora_get_config();
  $fixed_deleted = "<div id = 'emicdora_deleted'>" . emicdora_cleaned_post('all_deleted') . "</div>";
  $fixed_added = "<div id = 'emicdora_added'>" . emicdora_cleaned_post('all_added') . "</div>";
  $action = emicdora_cleaned_post('action');
  $results = array();
  switch ($action) {
    case 'link':
      $marshalled_deleted = emicdora_marshall_spans('deleted');
      $marshalled_added = emicdora_marshall_spans('added');
      $new_deleted = emicdora_merge($fixed_deleted, $marshalled_deleted, 'deleted');
      $new_added = emicdora_merge($fixed_added, $marshalled_added, 'added');
      $outputs = array(
        'added' => $new_added,
        'deleted' => $new_deleted,
      );
      $cleaned_doms = emicdora_cleanup_merged($outputs);
      $added_node = $cleaned_doms['added']->getElementById($config['added']);
      $deleted_node = $cleaned_doms['deleted']->getElementById($config['deleted']);

      $results = array(
        'new_deleted' => $cleaned_doms['deleted']->saveXML($deleted_node),
        'new_added' => $cleaned_doms['added']->saveXML($added_node),
        'emicdora_counter' => $_POST['emicdora_counter'] + 1,
        'refresh' => 'refresh',
      );
      break;

    case 'unlink':
      $new_deleted = emicdora_unlink($fixed_deleted, 'deleted');
      $new_added = emicdora_unlink($fixed_added, 'added');
      $results = array(
        'new_deleted' => $new_deleted,
        'new_added' => $new_added,
        'emicdora_counter' => $_POST['emicdora_counter'] + 1,
        'refresh' => 'refresh',
      );
      break;

    case 'save':
      emicdora_update_collation('deleted');
      emicdora_update_collation('added');
      $results = array(
        'added' => 'success',
        'refresh' => 'false',
        'emicdora_counter' => $_POST['emicdora_counter'],
      );
  }
  if (!empty($results)) {
    echo json_encode($results);
  }
}

function emicdora_merge($fixed_content, $marshalled_content, $type) {
  $identified_text = emicdora_cleaned_post("text_$type");
  $prefix = $type[0];
  $config = emicdora_get_config();
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);
  $nodes_to_delete = array();
  foreach ($marshalled_content as $id => $content) {
    $position = array_search($id, array_keys($marshalled_content));
    $is_first = ($position == 0) ? TRUE : FALSE;
    $is_last = ($position == count($marshalled_content)) ? TRUE : FALSE;
    $node = $dom->getElementById($id);
    if ($node) {
      $content = $marshalled_content[$id];
      if ($is_first) {
        if ($node->nodeValue != $content) {
          $offset = strrpos($node->nodeValue, $content);
          $node->nodevalue = substr($node->nodevalue, 0, $offset);
          continue;
        }
      }
      if ($is_last) {
        if ($node->nodeValue != $content) {
          $node->nodevalue = preg_replace("/$content/", '', $node->nodeValue, 1);
          continue;
        }
      }
      $nodes_to_delete[] = $node;
    }
  }
  $new_element = $dom->createElement('span', $identified_text);
  $counter = emicdora_cleaned_post('emicdora_counter');
  if ($counter == 'undeclared') {
    $counter = emicdora_get_counter();
  }
  $_POST['emicdora_counter'] = $counter;
  $fresh_id = "{$prefix}e{$counter}";
  $new_element->setAttribute('id', $fresh_id);
  $new_element->setAttribute('class', 'merged');
  $delete_ids = array_keys($nodes_to_delete);
  $refnode = $nodes_to_delete[$delete_ids[0]];
  $wrapper_node = $dom->getElementById($config[$type]);
  $refnode->parentNode->insertBefore($new_element, $refnode);
  foreach ($nodes_to_delete as $victim) {
    $victim->parentNode->removeChild($victim);
  }
  return $dom->saveXML($wrapper_node);
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
  $input = emicdora_cleaned_post("context_$type");
  $spans = array_filter(explode('</span>', $input));
  $marshalled_spans = array();
  foreach ($spans as $candidate) {
    $dom = new DOMDocument();
    $dom->loadXML($candidate . '</span>');
    $items = $dom->getElementsByTagName('span');
    $id = $items->item(0)->getAttribute('id');
    $text_value = $items->item(0)->nodeValue;
    if ($text_value) {
      $marshalled_spans[$id] = $text_value;
    }
  }
  return $marshalled_spans;
}

/**
 * Updates div contents with incoming link request.
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
function emicdora_link($fixed_content, $marshalled_content, $type) {
  $identified_text = emicdora_cleaned_post("text_$type");
  $prefix = $type[0];
  $config = emicdora_get_config();
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);
  $nodes_to_delete = array();
  foreach ($marshalled_content as $id => $content) {
    $position = array_search($id, array_keys($marshalled_content));
    $is_first = ($position == 0) ? TRUE : FALSE;
    $is_last = ($position == count($marshalled_content)) ? TRUE : FALSE;
    $node = $dom->getElementById($id);
    if ($node) {
      $node_value = $node->nodeValue;
      $is_merged = ($node->getAttribute('class') == 'merged') ? TRUE : FALSE;
      if ($node_value) {
        // Make an allowance for selected text being wholly enclosed in a span.
        // Enclosed text will have prefix or suffix moved to adjacent spans.
        $enclosed = strlen($identified_text) < strlen($content);
        if ($enclosed) {
          if (emicdora_starts_with($content, $identified_text)) {
            $remainder = str_replace($identified_text, '', $content);
            $next = $node->nextSibling;
            $next->nodeValue = $remainder . $next->nodeValue;
            $nodes_to_delete[$id] = $node;
            continue;
          }
          if (emicdora_ends_with($content, $identified_text)) {
            $remainder = str_replace($identified_text, '', $content);
            $previous = $node->previousSibling;
            if ($previous) {
              $previous->nodeValue = $previous->nodeValue . $remainder;
            }
            else {
              $new_node = $dom->createElement('span');
              $new_node->nodeValue = $remainder;
              $new_node->setAttribute('class', $type);
              $parent_node = $dom->getElementById($config[$type]);
              $parent_node->insertBefore($new_node, $node);
            }
            $nodes_to_delete[$id] = $node;
            continue;
          }
          $string_parts = explode($identified_text, $content);
          $previous = $node->previousSibling;
          $class = $previous->getAttribute('class');
          if ($class != 'merged') {
            $previous->nodeValue = $previous->nodeValue . $string_parts[0];
          }
          else {
            $new_node = $dom->createElement('span');
            $new_node->nodeValue = $string_parts[0];
            $new_node->setAttribute('class', $type);
            $parent_node = $dom->getElementById($config[$type]);
            $parent_node->insertBefore($new_node, $node);
          }
          $next = $node->nextSibling;
          $next->nodeValue = $string_parts[1] . $next->nodeValue;
          $nodes_to_delete[$id] = $node;
          continue;
        }
        // Spans containing matched text are deleted.
        if ($node_value == $content) {
          $nodes_to_delete[$id] = $node;
          continue;
        }
      }
      // If node value differs from content, node value must be adjusted.
      if ($node_value != $content) {
        // Merged spans which have been adjusted are always removed..
        if ($is_merged) {
          $nodes_to_delete[$id] = $node;
        }
        $starts_with = emicdora_starts_with($node_value, $content);
        $new_value = str_replace($content, '', $node_value);

        switch ($starts_with) {
          case FALSE:

            if ($is_merged) {
              $previous = $node->previousSibling;
              $previous->nodeValue = $previous->nodeValue . $new_value;
            }
            else {
              $refnode = $node;
              $node->nodeValue = $new_value;
            }
            break;

          case TRUE:
            $refnode = $node;
            $next = $node->nextSibling;
            $current = $next->nodeValue;
            $class = $next->getAttribute('class');
            if ($class != 'merged') {
              $next->nodeValue = $new_value . $current;
            }
            else {
              $node->node_value = $new_value;
              $new_node = $dom->createElement('span');
              $new_node->nodeValue = $new_value;
              $new_node->setAttribute('class', $type);
              $parent_node = $dom->getElementById($config[$type]);
              $parent_node->insertBefore($new_node, $node);
              $nodes_to_delete[$id] = $node;
            }
        }
      }
    }
  }

  $new_element = $dom->createElement('span', $identified_text);
  $counter = emicdora_cleaned_post('emicdora_counter');
  if ($counter == 'undeclared') {
    $counter = emicdora_get_counter();
  }
  $_POST['emicdora_counter'] = $counter;
  $fresh_id = "{$prefix}e{$counter}";
  $new_element->setAttribute('id', $fresh_id);
  $new_element->setAttribute('class', 'merged');
  if (!isset($refnode)) {
    $refnode = $dom->getElementById($id);
  }
  $delete_ids = array_keys($nodes_to_delete);
  $ref_id = $refnode->getAttribute('id');
  if (in_array($ref_id, $delete_ids)) {
    $refnode = $nodes_to_delete[$delete_ids[0]];
  }
  $wrapper_node = $dom->getElementById($config[$type]);
  $refnode->parentNode->insertBefore($new_element, $refnode);
  foreach ($nodes_to_delete as $victim) {
    $victim->parentNode->removeChild($victim);
  }
  return $dom->saveXML($wrapper_node);
}

/**
 * Updates div contents with incoming unlink request.
 *
 * @param xml $fixed_content
 *   Repaired content from viewer
 * @param string $type
 *   added or deleted
 *
 * @return xml
 *   new div contents
 */
function emicdora_unlink($fixed_content, $type) {
  $merged_content = emicdora_cleaned_post('merged_content');
  $span_dom = new DOMDocument();
  $span_dom->loadXML($merged_content);
  $items = $span_dom->getElementsByTagName('span');
  $id = $items->item(0)->getAttribute('id');
  $qualifier = substr($id, 1);
  $id = $type[0] . $qualifier;

  $config = array(
    'added' => 'versionview-1011-body',
    'deleted' => 'versionview-1010-body',
  );
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);
  $element = $dom->getElementById($id);
  $previous = $element->previousSibling;
  $added = FALSE;
  if ($previous) {
    $class = $previous->getAttribute('class');
    if ($class != 'merged') {
      $previous->nodeValue = $previous->nodeValue . $element->nodeValue;
      $added = TRUE;
    }
  }
  if (!$added) {
    $element->nextSibling->nodeValue = $element->nodeValue . $element->nextSibling->nodeValue;
  }

  $element->parentNode->removeChild($element);
  $node = $dom->getElementById($config[$type]);
  return $dom->saveXML($node);
}

/**
 * Utility function to unencode $_POST variables.
 *
 * @param string $term
 *   array key
 *
 * @return string
 *   Decoded input
 */
function emicdora_cleaned_post($term) {
  return (isset($_POST[$term])) ? urldecode(filter_xss($_POST[$term])) : '';
}

/**
 * Persists updated collation.
 *
 * @param string $type
 *   added or deleted
 */
function emicdora_update_collation($type) {
  module_load_include('inc', 'austese_collation', 'includes/emicdora');
  $query = emicdora_get_file_query($type);
  $file = emicdora_calliope_get_collation_file($query);
  $counter = 1;
  if (isset($_POST['emicdora_counter']) && $_POST['emicdora_counter'] != 'undeclared') {
    $counter = emicdora_cleaned_post('emicdora_counter');
  }
  $file_params = array(
    'collation' => $file['emic_data']['collation'],
    'version1' => $file['emic_data']['version1'],
    'version2' => $file['emic_data']['version2'],
    'diff_kind' => $type,
    'emicdora_count' => isset($_POST['emicdora_counter']) ? $_POST['emicdora_counter'] : 1,
  );
  $contents = emicdora_cleaned_post("all_$type");
  emicdora_calliope_write_collation($file_params, $contents);
}

/**
 * Gets mongo file query.
 *
 * @param string $type
 *   added or deleted
 *
 * @return array
 *   returned query
 */
function emicdora_get_file_query($type) {
  return array(
    'emic_data.collation' => $_POST['collation_id'],
    'emic_data.diff_kind' => $type,
  );
}

/**
 * Returns current counter for emic generated tags.
 *
 * @return int
 *   current emicdora count
 */
function emicdora_get_counter() {
  module_load_include('inc', 'collation', 'includes/calliope');
  $query = emicdora_get_file_query('deleted');
  $file = emicdora_calliope_get_collation_file($query);
  return $file['emic_data']['emicdora_count'];
}

/**
 * Gets matching id from opposite display pane.
 *
 * @param string $id
 *   id of current span
 *
 * @return string
 *   id of opposite span
 */
function emicdora_flip_id($id) {
  $suffix = substr($id, 1);
  if ($id[0] == 'd') {
    return "a$suffix";
  }
  else {
    return "d$suffix";
  }
}

/**
 * Removes orphaned merged spans.
 *
 * @param array $inputs
 *   input HTML
 *
 * @return array
 *   DOM
 */
function emicdora_cleanup_merged($inputs) {
  $doms = array();
  foreach ($inputs as $type => $text) {
    $dom = new DOMDocument();
    $dom->loadHTML($text);
    $doms[$type] = $dom;
  }
  foreach ($doms as $type => $dom) {
    $spans[$type] = $dom->getElementsByTagName('span');
  }
  foreach ($spans as $type => $items) {
    $opposite_type = $type == 'added' ? 'deleted' : 'added';
    foreach ($items as $item) {
      if ($item->getAttribute('class') != 'merged') {
        continue;
      }
      $opposite_id = emicdora_flip_id($item->getAttribute('id'));
      $test_node = $doms[$opposite_type]->getElementById($opposite_id);
      if ($test_node) {
        continue;
      }
      $incomplete = TRUE;
      $previous_sibling = $item->previousSibling;
      if ($previous_sibling) {
        $class = $previous_sibling->getAttribute('class');
        if ($class != 'merged') {
          $previous_sibling->nodeValue = $previous_sibling->nodeValue . $item->nodeValue;
          $item->parentNode->removeChild($item);
          $incomplete = FALSE;
        }
      }
      if ($incomplete) {
        $next_sibling = $item->nextSibling;
        if ($next_sibling) {
          $class = $next_sibling->getAttribute('class');
          if ($class != 'merged') {
            $next_sibling->nodeValue = $item->nodeValue . $next_sibling->nodeValue;
            $item->parentNode->removeChild($item);
            $incomplete = FALSE;
          }
        }
      }
      if ($incomplete) {
        $item->removeAttribute('class');
        $item->removeAttribute('style');
        $item->setAttribute('class', $opposite_type);
      }
    }
  }
  return $doms;
}

/**
 * Gets body ids.
 *
 * @return array
 *   Config values
 */
function emicdora_get_config() {
  return array(
    'added' => 'versionview-1011-body',
    'deleted' => 'versionview-1010-body',
  );
}
