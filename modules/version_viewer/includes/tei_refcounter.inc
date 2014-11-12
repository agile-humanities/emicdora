<?php

namespace VersionableObjectViewer;
use \DOMXPath, \DOMElement;

function count_refs($doc) {
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('wrap', 'http://islandora.ca/ontology/tei-text-wrapper');
  $xpath->registerNamespace('tei', 'http://www.tei-c.org/ns/1.0');

  foreach ($xpath->query('//tei:undo | //tei:redo') as $element) {
    deref_nodes($xpath, $element);
  }
}

function deref_nodes(DOMXPath $xpath, DOMElement $element) {
  if ($element->hasAttribute('target')) {
    $targets = parse_target($element->getAttribute('target'));
    foreach ($targets as $target) {
      foreach ($xpath->query("//tei:*[(self::tei:undo or self::tei:redo) and '$target' = @xml:id and @xml:id]") as $wrapper) {
        deref_nodes($xpath, $wrapper);
      }
      foreach ($xpath->query("//*[@xml:id and '$target' = @xml:id]//wrap:per") as $wrapper) {
        $wrapper->setAttribute('refcount', $wrapper->getAttribute('refcount') * -1);
      }
    }
  }
  elseif ($element->hasAttribute('spanTo')) {
    $targets = parse_target($element->getAttribute('spanTo'));
    $target = reset($targets);
    foreach ($xpath->query("
      following-sibling::*[not(preceding-sibling::*[self::tei:anchor[@xml:id and '$target' = @xml:id]])]//wrap:per |
      following-sibling::wrap:per[not(preceding-sibling::*[self::tei:anchor[@xml:id and '$target' = @xml:id]])]", $element) as $wrapper) {
      // Each following wrapper text, including those further down the hierarchy
      // before the anchor.
      $wrapper->setAttribute('refcount', $wrapper->getAttribute('refcount') * -1);
    }
  }
}

function parse_target($target) {
  $targets = preg_split('/\s+/', $target);
  foreach ($targets as &$t) {
    $t = substr($t, 1);
  }
  unset($t);
  return $targets;
}
