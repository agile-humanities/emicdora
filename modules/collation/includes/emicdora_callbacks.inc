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
      $merged_deleted = emicdora_check_merged($marshalled_deleted);
      $merged_added = emicdora_check_merged($marshalled_added);
      $new_deleted = emicdora_merge($fixed_deleted, $marshalled_deleted, 'deleted');
      if ($merged_added || $merged_deleted) {
        $results = array(
          'message' => t("You have merged text in your selection.  Please unlink before resubmitting."),
        );
      }
      else {
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
      }
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

/**
 * Merges identified text.
 *
 * @param xml $fixed_content
 *   Input xml
 * @param array $marshalled_content
 *   array containing span id's and content.
 * @param string $type
 *   add or delete
 *
 * @return xml
 *   Changed panle content
 */
function emicdora_merge($fixed_content, $marshalled_content, $type) {
  $identified_text = emicdora_cleaned_post("text_$type");
  $prefix = $type[0];
  $config = emicdora_get_config();
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);
  $nodes_to_delete = array();
  $first_id = $marshalled_content[0]['id'];
  $last_id = $marshalled_content[count($marshalled_content) - 1]['id'];

  if (count($marshalled_content) > 1) {
    foreach ($marshalled_content as $data) {
      $id = $data['id'];
      $node = $dom->getElementById($id);
      if ($node) {
        $node_value = $node->nodeValue;
        $selected = $data['text'];
        if ($id == $first_id) {
          if ($node_value != $selected) {
            $offset = strrpos($node->nodeValue, $selected);
            $new_value = substr($node_value, 0, $offset);
            $length = strlen($new_value);
            // Strip linefeeds.
            if ($length == 1 && ord($new_value[0]) == 10) {
              $new_value = NULL;
            }
            if ($new_value) {
              $node->nodevalue = $new_value;
            }
            else {
              $nodes_to_delete[] = $node;
            }
            continue;
          }
        }
        if ($id == $last_id) {
          if ($node->nodeValue != $selected) {
            $new_value = preg_replace("/$selected/", '', $node->nodeValue, 1);
            $length = strlen($new_value);
            // Strip linefeeds.
            if ($length == 1 && ord($new_value[strlen($new_value) - 1]) == 10) {
              $new_value = NULL;
            }
            if ($new_value != '') {
              $node->nodevalue = $new_value;
            }
            else {
              $nodes_to_delete[] = $node;
            }
            continue;
          }
        }
        $nodes_to_delete[] = $node;
      }
    }
  }
  else {
    $starts_with = emicdora_starts_with($content, $identified_text);
    $ends_with = emicdora_ends_with($content, $identified_text);
    if ($starts_with && $ends_with) {
      $nodes_to_delete[] = $node;
      continue;
    }
    if ($starts_with) {
      $new_value = preg_replace("/$content/", '', $node->nodeValue, 1);
      $new_element = emicdora_create_span($dom, $new_value, 'merge', $prefix);
      $refnode->parentNode->insertBefore($new_element, $refnode);
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
    $class = $items->item(0)->getAttribute('class');
    $text_value = $items->item(0)->nodeValue;
    if ($text_value) {
      $marshalled_spans[] = array(
        'id' => $id,
        'text' => $text_value,
        'class' => $class,
      );
    }
  }
  return $marshalled_spans;
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

/**
 * Determine if one string matches the start of another.
 *
 * @param string $haystack
 *   String being searched
 * @param string $needle
 *   Substring being compared
 *
 * @return bool
 *   TRUE for match, FALSE for not
 */
function emicdora_starts_with($haystack, $needle) {
  return $needle === "" || strpos($haystack, $needle) === 0;
}

/**
 * Determine if one string matches the end of another.
 *
 * @param string $haystack
 *   String being searched
 * @param string $needle
 *   Substring being compared
 *
 * @return bool
 *   TRUE for match, FALSE for not
 */
function emicdora_ends_with($haystack, $needle) {
  return $needle === "" || substr($haystack, -strlen($needle)) === $needle;
}

/**
 * Creates new span to be inserted in DOM object.
 *
 * @param DOM $dom
 *   DOM representing current panel
 * @param string $contents
 *   Text value of new element
 * @param string $class_name
 *   Value of class attribute
 * @param string $prefix
 *   'a' or 'd'
 *
 * @return DOMElement
 *   Created element
 */
function emicdora_create_span($dom, $contents, $class_name, $prefix) {
  $new_element = $dom->createElement('span', $contents);
  $counter = emicdora_cleaned_post('emicdora_counter');
  if ($counter == 'undeclared') {
    $counter = emicdora_get_counter();
  }
  $_POST['emicdora_counter'] = $counter;
  $fresh_id = "{$prefix}e{$counter}";
  $new_element->setAttribute('id', $fresh_id);
  $new_element->setAttribute('class', $class_name);
  return $new_element;
}

/**
 * Tests input array for merged class.
 *
 * @param array $input_array
 *   Array to be tested
 *
 * @return bool
 *   TRUE is merged class is found
 */
function emicdora_check_merged($input_array) {
  $merged = FALSE;
  foreach ($input_array as $candidate) {
    if ($candidate['class'] == 'merged') {
      $merged = TRUE;
      break;
    }
  }
  return $merged;
}
