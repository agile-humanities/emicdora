<?php

/**
 * @file
 * includes/non_sort.inc
 *
 * Non sort functions used in Emicdora module.
 */

/**
 * Remove the nonsort part with a space and add the nonsort under titleInfo.
 *
 * @param string $xml
 *   A MODS xml file.
 *
 * @return string
 *   Returns a string containing the MOD xml with additional title info.
 */
function emicdora_adjust_titleinfo($xml) {
  $str_xml = FALSE;
  $simple_mods = simplexml_load_string($xml);
  // If non sort has been manually set, we'll leave it alone.
  if ($simple_mods->titleInfo->nonSort) {
    return;
  }
  if ($simple_mods->titleInfo) {
    if ($simple_mods->titleInfo->title) {
      $result = emicdora_find_nonsort_patten($simple_mods->titleInfo->title);
      if ($result) {
        $simple_mods->titleInfo->title = $result['title'];
        $simple_mods->titleInfo->addChild("nonSort", $result['nonSort']);
      }
    }
  }

  // Let's format this nicely.
  $dom = new DOMDocument();
  $dom->preserveWhiteSpace = FALSE;
  $dom->formatOutput = TRUE;
  $dom->loadXML($simple_mods->asXML());
  $str_xml = str_replace("<@attributes/>", "", $dom->saveXML());
  return $str_xml;
}

/**
 * Find the nonsort part from a string.
 *
 * @param string $title
 *   A string in which the nonSort patten need to be found.
 *
 * @return array|bool
 *   Return array with nonSort and title if nonSort part exist or return false.
 */
function emicdora_find_nonsort_patten($title) {
  $arr = FALSE;
  $filing_article = array(
    'a ' => 2, 'an ' => 3, 'the ' => 4, 'der ' => 4, 'die ' => 4, 'das ' => 4,
    'le ' => 3, 'la ' => 3, 'les ' => 4, 'l ' => 2, 'il ' => 3, 'lo ' => 3,
    "l'" => 2, 'gli ' => 4, 'gl ' => 3, 'le ' => 3, 'el ' => 3, 'los ' => 4,
    'las ' => 4, 'den ' => 4, 'det ' => 4, 'de ' => 3,
  );
  $ignore_case_title = strtolower($title);
  foreach ($filing_article as $key => $length) {
    if (substr($ignore_case_title, 0, $length) == $key) {
      $arr['nonSort'] = substr($title, 0, $length);
      $arr['title'] = substr($title, $length);
      break;
    }
  }
  return $arr;
}

