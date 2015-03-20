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
  $base_doc = <<<EOQ
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
  </head>
<body>
!body
</body>
</html>
EOQ;
  $fixed_deleted = format_string($base_doc, array('!body' => "<div id='emicdora_deleted'>" . emicdora_cleaned_post('all_deleted') . "</div>"));
  $fixed_added = format_string($base_doc, array('!body' => "<div id='emicdora_added'>" . emicdora_cleaned_post('all_added') . "</div>"));
  $action = emicdora_cleaned_post('action');
  $counter = emicdora_cleaned_post('emicdora_counter');
  // Let's grab the two versions so there's context for Mongo to query on.
  $name_added = filter_input(INPUT_POST, 'name_added');
  $name_deleted = filter_input(INPUT_POST, 'name_deleted');
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
        $added_node = emicdora_get_element_by_id($cleaned_doms['added'], $config['added']);
        $deleted_node = emicdora_get_element_by_id($cleaned_doms['deleted'], $config['deleted']);

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
      // XXX: Due to how Calliope handles requests the parameters need to be
      // flipped.
      emicdora_update_collation('deleted', $name_added, $name_deleted);
      emicdora_update_collation('added', $name_deleted, $name_added);
      $results = array(
        'added' => 'success',
        'refresh' => 'false',
        'emicdora_counter' => $counter,
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
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->loadHTML($fixed_content);
  $range_info = filter_input(INPUT_POST, $type, FILTER_DEFAULT, FILTER_REQUIRE_ARRAY);

  $find_text_node = function ($node_info) use ($dom) {
    // Iterate over text nodes in the given element until we find the one in
    // which our offset occurs.
    // XXX: Very much based on the overlapping entities work, for the
    // "versionable object viewer".
    $remaining = $node_info['offset'];
    $element = emicdora_get_element_by_id($dom, $node_info['id']);
    if ($element) {
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
        return array(
          'remaining' => drupal_strlen($element->lastChild->nodeValue),
          'node' => $element->lastChild,
        );
      }
    }
    throw new CollationException('Failed to find text node.');
  };

  // Identify and split out our selection in the start and ending elements.
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
  $start_node = $start_suffix;
  $end_node = $end_info['node'];

  // Build up spans which will replace those whose text we are "merging".
  // "prefix", meaning those in the starting span but outside the selection,
  // "content", meaning the selection and,
  // "suffix", meaning those nodes in the ending span, but outside the
  // selection.
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
  $start_xpath = $start_node->getNodePath();
  $end_xpath = $end_node->getNodePath();

  // XXX: Some XPath magic... Unioning a node into a set in which it is already
  // present should not affect the size of the set. We exploit this to perform
  // a set intersection here.
  // @see http://stackoverflow.com/q/7178471
  $query = "($start_xpath/following::node()[count(. | $end_xpath/preceding::node()) = count($end_xpath/preceding::node())] | $start_xpath/.. | $end_xpath/..)[(contains(@class, 'merged') or contains(@class, 'variant'))]";
  $overlapping_nodes = $xpath->query($query);
  if ($overlapping_nodes->length > 0) {
    throw new CollationSelectionOverlapException(t('The "@type" selection overlaps with an existing merged or variant tag. Unlink the existing tag as necessary.', array(
      '@type' => $type,
    )));
  }

  // Couple flags to help track our traversal.
  $found_start = FALSE;
  $found_end = FALSE;
  // Track those spans whose content is being slotted into one of our three.
  $to_nuke = array();
  $replacement_nodes = array(
    'prefix' => array(),
    'content' => array(),
    'suffix' => array(),
  );

  $traverse_container = function (DOMNodeList $nodes) use (&$replacement_nodes, &$found_start, &$found_end, $start_node, $end_node) {
    // Traverse /inside/ the containing elements, slotting the content into one
    // of our three spans accordingly.

    // XXX: Yes, this could be done with XPath, though due to "missing" set
    // operations in XPath 1.0, it gets very messy.
    foreach ($nodes as $node) {
      $found_start = $found_start || $start_node->isSameNode($node);

      if (!$found_start) {
        // Initially, thing go into "prefix".
        $replacement_nodes['prefix'][] = $node;
      }
      elseif ($found_start && !$found_end) {
        // When we've found our starting node, we've found our content.
        $replacement_nodes['content'][] = $node;
      }
      elseif ($found_start && $found_end) {
        // And when we've hit our end node, our content has been exhausted.
        $replacement_nodes['suffix'][] = $node;
      }

      $found_end = $found_end || ($found_start && $end_node->isSameNode($node));
    }
  };
  $traverse = function (DOMElement $element) use (&$found_end, $traverse_container, &$to_nuke) {
    // Traverse across the "containing" elements... Should be the spans
    // themselves.
    $current = $element;
    while (!$found_end && $current->nextSibling !== NULL) {
      $traverse_container($current->childNodes);
      $to_nuke[] = $current;
      if ($current->nextSibling === NULL) {
        if (!$found_end) {
          throw new Exception(t('Ran out of elements before finding ending text node.'));
        }
        break;
      }
      do {
        $current = $current->nextSibling;
      } while ($current->childNodes === NULL);
    }
  };

  // Kick off the traversal.
  $traverse($start_suffix->parentNode);

  // Add each node to its replacements. Note: This also removes the node from
  // its original location in the document.
  foreach ($replacement_nodes as $type_thing => $nodes) {
    array_map(array($replacements[$type_thing], 'appendChild'), $nodes);
  }

  // We use the first affected span to know where to locate where in the
  // document our replacement spans should go.
  $first_nuked = reset($to_nuke);
  foreach ($replacements as $type_thing => &$node) {
    if ($node->hasChildNodes()) {
      $node = $first_nuked->parentNode->insertBefore($node, $first_nuked);
    }
  }
  foreach ($to_nuke as $nuke) {
    // Nuke all the "replaced" spans.
    $nuke->parentNode->removeChild($nuke);
  }
  return $dom;
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
  $element = emicdora_get_element_by_id($dom, $id);
  $element->setAttribute('class', $type);
  $element->removeAttribute('style');
  $node = emicdora_get_element_by_id($dom, $config[$type]);
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
 * @param string $name_added
 *   The name of the collation added.
 * @param string $name_deleted
 *   The name of the collation deleted.
 */
function emicdora_update_collation($type, $name_added, $name_deleted) {
  module_load_include('inc', 'austese_collation', 'includes/emicdora');
  $query = emicdora_get_file_query($type, $name_added, $name_deleted);
  $file = emicdora_calliope_get_collation_file($query);
  $counter = 1;
  if (isset($_POST['emicdora_counter']) && filter_input(INPUT_POST, 'emicdora_counter') != 'undeclared') {
    $counter = emicdora_cleaned_post('emicdora_counter');
  }
  $file_params = array(
    'collation' => $file['emic_data']['collation'],
    'version1' => $file['emic_data']['version1'],
    'version2' => $file['emic_data']['version2'],
    'diff_kind' => $type,
    'emicdora_count' => $counter,
  );
  $contents = emicdora_cleaned_post("all_$type");
  emicdora_calliope_write_collation($file_params, $contents);
}

/**
 * Gets mongo file query.
 *
 * @param string $type
 *   Added or deleted
 * @param string $name_added
 *   The name of the collation added.
 * @param string $name_deleted
 *   The name of the collation deleted.
 *
 * @return array
 *   An array representing a constructed query to pass of to Mongo.
 */
function emicdora_get_file_query($type, $name_added, $name_deleted) {
  return array(
    'emic_data.collation' => filter_input(INPUT_POST, 'collation_id'),
    'emic_data.diff_kind' => $type,
    'emic_data.version1' => $name_deleted,
    'emic_data.version2' => $name_added,
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
  $query = emicdora_get_file_query('deleted', filter_input(INPUT_POST, 'name_added'), filter_input(INPUT_POST, 'name_deleted'));
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
 * @param array $doms
 *   An array of DOMDocuments containing the XML to be cleaned up.
 *
 * @return array
 *   DOM
 */
function emicdora_cleanup_merged($doms) {
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
      $test_node = emicdora_get_element_by_id($doms[$opposite_type], $opposite_id);
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

/**
 * This is a replace for $dom->getElementById().
 *
 * As it does not work in earlier versions (<=5.3.3) of PHP with HTML.
 *
 * @param DOMDocument $dom
 * @param string $id
 *
 * @return DOMElement|NULL
 *   The element if found, NULL otherwise.
 */
function emicdora_get_element_by_id($dom, $id) {
  $xpath = new DOMXPath($dom);
  $results = $xpath->query("//*[@id='{$id}']");
  return $results->item(0);
}

class CollationException extends Exception {}
class CollationSelectionOverlapException extends CollationException {}
