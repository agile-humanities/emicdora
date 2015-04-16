<?php

/**
 * @file
 * Examine XML with wrapped text nodes for inclusion/exclusion.
 */

namespace VersionableObjectViewer;
use \DOMXPath, \DOMElement;

/**
 * Actually flip the flags resulting from the given item.
 *
 * @param DOMXPath $xpath
 *   A DOMXPath instance, wrapping a document in which to flip things around.
 * @param DOMElement $element
 *   A DOMElement instance for which to flip flags. Should be either a tei:undo
 *   or a tei:redo.
 */
function deref_nodes(DOMXPath $xpath, DOMElement $element) {
  if ($element->hasAttribute('target')) {
    $targets = parse_target($element->getAttribute('target'));
    foreach ($targets as $target) {
      foreach ($xpath->query("//tei:*[(self::tei:undo or self::tei:redo) and '$target' = @xml:id and @xml:id]") as $wrapper) {
        // Find undo/redo elements which the current element targets, and apply
        // them.
        deref_nodes($xpath, $wrapper);
      }
      foreach ($xpath->query("//*[@xml:id and '$target' = @xml:id]//wrap:per") as $wrapper) {
        // For the nodes we specifically contain, flip the flag.
        $wrapper->setAttribute('refcount', $wrapper->getAttribute('refcount') * -1);
      }
    }
  }
  elseif ($element->hasAttribute('spanTo')) {
    $targets = parse_target($element->getAttribute('spanTo'));
    $target = reset($targets);
    foreach ($xpath->query("
      following::*[not(preceding::*[self::tei:anchor[@xml:id and '$target' = @xml:id]])]//wrap:per |
      following::wrap:per[not(preceding::*[self::tei:anchor[@xml:id and '$target' = @xml:id]])]", $element) as $wrapper) {
      // For each following wrapper text, including those further down the
      // hierarchy before the anchor, flip the flag.
      $wrapper->setAttribute('refcount', $wrapper->getAttribute('refcount') * -1);
    }
  }
}

/**
 * Helper function; parse "target" and "spanTo" values.
 *
 *
 * @param string $target
 *   A string containing the list of targets. They are understood to be a
 *   space-separated list of identifiers, each preceded by a hash (#) symbol.
 *
 * @return array
 *   An array of strings, each representing an identifier. Note: The leading
 *   hash has been stripped for convenience.
 */
function parse_target($target) {
  $targets = preg_split('/\s+/', $target);
  foreach ($targets as &$t) {
    // Naively strip the first character off (should be a "#").
    $t = substr($t, 1);
  }
  unset($t);
  return $targets;
}
