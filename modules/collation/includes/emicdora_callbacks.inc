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
  $counter = emicdora_cleaned_post('emicdora_counter');
  if ($counter == 'undeclared' || $counter == '') {
    $counter = emicdora_get_counter();
  }
  $results = array();
  $linkage_type = 'merged';
  switch ($action) {
    case 'variant':
      $linkage_type = 'variant';
    case 'link':
      $marshalled_deleted = emicdora_marshall_spans('deleted');
      $marshalled_added = emicdora_marshall_spans('added');
      $merged_deleted = emicdora_check_merged($marshalled_deleted);
      $merged_added = emicdora_check_merged($marshalled_added);
      if ($merged_added || $merged_deleted) {
        $results = array(
          'message' => t("You have associated text in your selection.  Please unlink before resubmitting."),
          'emicdora_counter' => $_POST['emicdora_counter'],
        );
      }
      else {
        $new_deleted = emicdora_merge($fixed_deleted, $marshalled_deleted, 'deleted', $linkage_type);
        $new_added = emicdora_merge($fixed_added, $marshalled_added, 'added', $linkage_type);
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
        'emicdora_counter' => $counter + 1,
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
 * @param XML $fixed_content
 *   Current pane contents
 * @param array $marshalled_content
 *   Array of span ids matched to span contents
 * @param string $type
 *   add or delete
 * @param string $linkage_type
 *   'merge' or 'variant'
 *
 * @return XML
 *   Revised content
 */
function emicdora_merge($fixed_content, $marshalled_content, $type, $linkage_type) {
  $identified_text = emicdora_cleaned_post("text_$type");
  $created_elements = array();
  $counter = emicdora_cleaned_post('emicdora_counter');
  if ($counter == 'undeclared') {
    $counter = emicdora_get_counter();
  }
  $prefix = $type[0];
  $config = emicdora_get_config();
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);
  $nodes_to_delete = array();

  foreach ($marshalled_content as $data) {
    $id = $data['id'];
    $node = $dom->getElementById($id);
    if ($node) {
      $current = emicdora_get_innerHTML($node);
      $selected = $data['text'];
      if (strlen($identified_text) < strlen($selected)) {
        $selected = $identified_text;
      }

      if ($current == $selected) {
        $nodes_to_delete[$id] = $node;
        continue;
      }
      if ($current != $selected) {
        $nodes_to_delete[$id] = $node;
        $string_parts = explode($selected, $current);
        $count = count($string_parts);
        if ($count == 1) {
          $new_value = $string_parts[0];
          // Orphan only.
          $new_id = "{$prefix}oo{$counter}";
          $element = emicdora_create_span($dom, $new_value, $data['class'], $new_id);
          if (emicdora_ends_with($current, $selected)) {
            // Inserted before current.
            $refnode = $node;
          }
          else {
            $refnode = $node->nextSibling;
          }
          $refnode->parentNode->insertBefore($element, $refnode);
        }
        else {
          $prefix_text = emicdora_ends_with($current, $selected) ? substr($current, 0, -strlen($selected)) : reset($string_parts);
          $suffix_text = emicdora_starts_with($current, $selected) ? substr($current, strlen($selected)) : end($string_parts);

          // Orphan prefix.
          $prefix_id = "{$prefix}opref{$counter}";
          if (in_array($prefix_id, $created_elements)) {
            $prefix_id .= '-b';
          }
          $created_elements[] = $prefix_id;
          $first_element = emicdora_create_span($dom, $prefix_text, $data['class'], $prefix_id);
          $node->parentNode->insertBefore($first_element, $node);
          // Orphan suffix.
          $suffix_id = "{$prefix}osuf{$prefix}{$counter}";
          if (in_array($suffix_id, $created_elements)) {
            $suffix_id .= '-b';
          }
          $created_elements[] = $suffix_id;
          $second_element = emicdora_create_span($dom, $suffix_text, $data['class'], $suffix_id);
          $node->parentNode->insertBefore($second_element, $node->nextSibling);
        }
      }
    }
  }
  $new_element = $dom->createElement('span', $identified_text);
  $fresh_id = "{$prefix}e{$counter}";
  $new_element->setAttribute('id', $fresh_id);
  $new_element->setAttribute('class', $linkage_type);
  $new_element->setAttribute("data-$linkage_type", $counter);
  if (!isset($refnode)) {
    $refnode = reset($nodes_to_delete);
  }
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
  $tagname = "span";
  $pattern = "/<$tagname ?.*>(.*)<\/$tagname>/";
  preg_match($pattern, $input, $matches);
  if (isset($matches[0])) {
    $input = $matches[0];
  }
  $spans = array_filter(explode('</span>', $input));
  $marshalled_spans = array();
  foreach ($spans as $candidate) {
    // @codingStandardsIgnoreStart
    $fixed_span = str_replace('<br>', '<br />', $candidate) . '</span>';
    // @codingStandardsIgnoreEnd
    $dom = new DOMDocument();
    $dom->loadXML($fixed_span);
    $items = $dom->getElementsByTagName('span');
    $id = $items->item(0)->getAttribute('id');
    $class = $items->item(0)->getAttribute('class');
    $text_value = emicdora_get_innerHTML($items->item(0));
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
  // @codingStandardsIgnoreStart
  $merged_content = str_replace('<br>', '', $merged_content);
  // @codingStandardsIgnoreEnd
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
  $element->setAttribute('class', $type);
  $element->removeAttribute('style');
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
  $counter = 0;
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
      $node_type = get_class($previous_sibling);
      if ($node_type != 'DOMText') {
        $class = $previous_sibling->getAttribute('class');
        if ($class != 'merged') {

          $temp_node = $previous_sibling->cloneNode(FALSE);
          $temp_children = $temp_node->childNodes;
          while ($temp_children->length > 0) {
            $temp_node->removeChild($temp_children->item(0));
          }
          $previous_sibling_childen = $previous_sibling->childNodes;
          $current_children = $item->childNodes;
          foreach ($previous_sibling_childen as $child) {
            $temp_node->appendChild($child);
          }
          foreach ($current_children as $child) {
            $temp_node->appendChild($child);
          }
          $parent = $item->parentNode;
          $parent->replaceChild($temp_node, $previous_sibling);
          $item->parentNode->removeChild($item);
          $incomplete = FALSE;
        }
      }
      if ($incomplete) {
        $next_sibling = $item->nextSibling;
        if ($next_sibling) {
          $class = $next_sibling->getAttribute('class');
          if ($class != 'merged') {
            $temp_node = $next_sibling->cloneNode(FALSE);
            $temp_children = $temp_node->childNodes();
            while ($temp_children->length > 0) {
              $temp_node->removeChild($temp_children->item(0));
            }
            $next_sibling_childen = $next_sibling->childNodes;
            $current_children = $item->childNodes;
            foreach ($current_children as $child) {
              $temp_node->appendChild($child);
            }
            foreach ($next_sibling_childen as $child) {
              $temp_node->appendChild($child);
            }
            $parent->replaceChild($temp_node, $next_sibling);
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
 * Creates new DomElement.
 *
 * @param DOMDocument $dom
 *   Representation of current panel
 * @param string $contents
 *   Contents of new element
 * @param string $class_name
 *   class attribute for new element
 * @param string $prefix
 *   'a' or 'd'
 *
 * @return DOMElement
 *   New element
 */
function emicdora_create_span($dom, $contents, $class_name, $prefix) {
  $contents = nl2br($contents, FALSE);
  // The collation viewer doesn't handle xhtml break tags well.
  // @codingStandardsIgnoreStart
  $contents = str_replace('<br/>', '<br>', $contents);
  // @codingStandardsIgnoreEnd
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
    if ($candidate['class'] == 'merged' || $candidate['class'] == 'variant') {
      $merged = TRUE;
      break;
    }
  }
  return $merged;
}

/**
 * Gets innerHTML of DOMElement.
 *
 * @param DOMElement $element
 *   Element being examined
 *
 * @return string
 *   InnerHTML
 */
function emicdora_get_innerHTML($element) {
  $html = '';
  foreach ($element->childNodes as $child) {
    $html .= $child->ownerDocument->saveXML($child);
  }
  return $html;
}
