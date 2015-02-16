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
  $fixed_deleted = "<div id='emicdora_deleted'>" . emicdora_cleaned_post('all_deleted') . "</div>";
  $fixed_added = "<div id='emicdora_added'>" . emicdora_cleaned_post('all_added') . "</div>";
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
      try {
        $new_deleted = emicdora_merge($fixed_deleted, 'deleted', $linkage_type, $counter);
        $new_added = emicdora_merge($fixed_added, 'added', $linkage_type, $counter);
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
          'emicdora_counter' => $counter + 1,
          'refresh' => 'refresh',
        );
      }
      catch (CollationException $e) {
        $results = array(
          'message' => $e->getMessage(),
          'emicdora_counter' => $counter,
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
    drupal_json_output($results);
  }
}

/**
 * Merges identified text.
 *
 * @param XML $fixed_content
 *   Current pane contents
 * @param string $type
 *   add or delete
 * @param string $linkage_type
 *   'merge' or 'variant'
 * @param int $counter
 *   The "ID number" of this merge operation.
 *
 * @return XML
 *   Revised content
 */
function emicdora_merge($fixed_content, $type, $linkage_type, $counter) {
  $prefix = $type[0];
  $config = emicdora_get_config();
  $dom = new DOMDocument();
  $dom->loadHTML($fixed_content);

  $range_info = $_POST[$type];

  $find_text_node = function ($node_info) use ($dom) {
    $remaining = $node_info['offset'];
    $element = $dom->getElementById($node_info['id']);
    foreach ($element->childNodes as $child) {
      if ($child->nodeType === XML_TEXT_NODE) {
        $length = drupal_strlen($child->nodeValue);
        if ($remaining >= $length) {
          $remaining -= $length;
        }
        else {
          return array(
            'remaining' => $remaining,
            'node' => $child,
          );
        }
      }
    }
    if ($remaining === 0) {
      // XXX: The selection provided from the client side "rangy" code can
      // point to the very end of an element, which means that you could end up
      // selecting an empty text node which is actually outside the selection,
      // so let's instead try to point at the next text node. NOTE: This needs
      // to be tested for how it works at the /end/ of the selection... likely
      // end up in the next span...
      $get_next_text_node = function (DOMNode $node) {
        foreach ($node->nextSibling->childNodes as $child) {
          if ($child->nodeType === XML_TEXT_NODE) {
            return $child;
          }
        }
        throw new CollationException('Next node fallback failed.');
      };
      return array(
        'remaining' => $remaining,
        'node' => $get_next_text_node($element),
      );
    }
    throw new CollationException('Failed to find text node.');
  };


  $start_location_info = $range_info['start'];
  $start_info = $find_text_node($start_location_info);
  $start_suffix = drupal_strlen($start_info['node']->nodeValue) === $start_info['remaining'] ?
    $start_info['node'] :
    $start_info['node']->splitText($start_info['remaining']);
  $end_location_info = $range_info['end'];
  $end_info = $find_text_node($end_location_info);
  $end_suffix = drupal_strlen($end_info['node']->nodeValue) === $end_info['remaining'] ?
    NULL :
    $end_info['node']->splitText($end_info['remaining']);

  $content_id = "{$prefix}e{$counter}";
  $replacements = array();
  $replacements['prefix'] = $dom->createElement('span');
  $replacements['prefix']->setAttribute('id', "{$prefix}opref{$counter}e{$content_id}");
  $replacements['content'] = $dom->createElement('span');
  $replacements['content']->setAttribute('id', $content_id);
  $replacements['content']->setAttribute('class', $linkage_type);
  $replacements['content']->setAttribute("data-$linkage_type", $counter);
  $replacements['suffix'] = $dom->createElement('span');
  $replacements['suffix']->setAttribute('id', "{$prefix}osuf{$counter}e{$content_id}");

  $xpath = new DOMXPath($dom);
  $start_xpath = $start_suffix->getNodePath();
  $end_xpath = $end_info['node']->getNodePath();
  $query = "//div[@id='emidora_$type']//span[following::*[$start_xpath] and preceding::*[$end_xpath] and (contains(@class, 'merged') or contains(@class, 'variant'))]";
  $overlapping_nodes = $xpath->query($query);
  dsm($query);
  $dump = function($name, $nodelist) {
    dsm($name);
    foreach ($nodelist as $node) {
      dsm($node->ownerDocument->saveHTML($node), $name);
    }
  };
  if ($overlapping_nodes->length > 0) {
    $dump('overlap', $overlapping_nodes);
    throw new CollationSelectionOverlapException(t('The "@type" selection overlaps with an existing merged or variant tag. Unlink the existing tag as necessary.', array(
      '@type' => $type,
    )));
  }

  $found_start = FALSE;
  $found_end = FALSE;
  $to_nuke = array();
  $start_node = $start_suffix;
  $end_node = $end_info['node'];
  $replacement_nodes = array(
    'prefix' => array(),
    'content' => array(),
    'suffix' => array(),
  );
  $prefix_nodes = $xpath->query("preceding-sibling::* | preceding-sibling::text()", $start_node);
  $content_nodes = $xpath->query("$start_xpath/following::node()[count(. | $end_xpath/preceding::node()) = count($end_xpath/preceding::node())] | $start_xpath | $end_xpath");
  $suffix_nodes = $xpath->query("following-sibling::* | following-sibling::text()", $end_node);
  $dump('prefix', $prefix_nodes);
  $dump('content', $content_nodes);
  $dump('suffix', $suffix_nodes);

  $traverse_container = function (DOMNodeList $nodes) use (&$replacement_nodes, &$found_start, &$found_end, $start_node, $end_node) {
    foreach ($nodes as $node) {
      $found_start = $found_start || $start_node->isSameNode($node);

      if (!$found_start) {
        $replacement_nodes['prefix'][] = $node;
      }
      elseif ($found_start && !$found_end) {
        $replacement_nodes['content'][] = $node;
      }
      elseif ($found_start && $found_end) {
        $replacement_nodes['suffix'][] = $node;
      }

      $found_end = $found_end || ($found_start && $end_node->isSameNode($node));
    }
  };
  $traverse = function (DOMElement $element) use (&$found_end, $traverse_container, &$to_nuke) {
    $current = $element;
    while (!$found_end && $current->nextSibling !== NULL) {
      $traverse_container($current->childNodes);
      $to_nuke[] = $current;
      if ($current->nextSibling === NULL) {
        if (!$found_end) {
          // Ran out of elements!?
          throw new Exception('Kaboom!');
        }
        break;
      }
      do {
        $current = $current->nextSibling;
      } while ($current->childNodes === NULL);
    }
  };
  $traverse($start_suffix->parentNode);

  foreach ($replacement_nodes as $type_thing => $nodes) {
    foreach($nodes as $node) {
    dsm($dom->saveHTML($node), $type_thing);
    }
    array_map(array($replacements[$type_thing], 'appendChild'), $nodes);
  }

  $first_nuked = reset($to_nuke);
  $wrapper_node = $dom->getElementById($config[$type]);
  dsm($dom->saveHTML($wrapper_node), 'all');
  dsm($dom->saveHTML($first_nuked), 'replacement');
  foreach ($replacements as $type_thing => &$node) {
    dsm($dom->saveHTML($node), $type_thing);
  }
  foreach ($replacements as $type_thing => &$node) {
    if ($node->hasChildNodes()) {
    dsm($dom->saveHTML($node), $type_thing);
      $node = $first_nuked->parentNode->insertBefore($node, $first_nuked);
    }
  }
  foreach ($to_nuke as $nuke) {
    $nuke->parentNode->removeChild($nuke);
  }

  $wrapper_node = $dom->getElementById($config[$type]);
  return $dom->saveHTML($wrapper_node);
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
  return urldecode(filter_input(INPUT_POST, $term));
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
          if ($temp_node->hasChildNodes()) {
            $temp_children = $temp_node->childNodes();
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
      }
      if ($incomplete) {
        $next_sibling = $item->nextSibling;
        if ($next_sibling) {
          $class = $next_sibling->getAttribute('class');
          if ($class != 'merged') {
            $temp_node = $next_sibling->cloneNode(FALSE);
            if ($temp_node->hasChildNodes()) {
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

class CollationException extends Exception {}
class CollationSelectionOverlapException extends CollationException {}
