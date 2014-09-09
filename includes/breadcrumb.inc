<?php

/**
 * @file
 * Helper functions for setting the breadcrumbs on various pages.
 */

/**
 * Set the page title and subtitle.
 *
 * This is used primarily in the context of viewing a Critical Edition Object.
 * The Critical Edition has the title and any other object form is represented
 * by the subtitle.
 *
 * @param string $title
 *   The text to use as the page title.
 *
 * @param string $subtitle
 *   The text to use as the page subtitle.
 */
function emicdora_set_page_title_and_subtitle($title, $subtitle) {
  drupal_set_title(t('<h1>!title</h1><h2>!subtitle</h2>', array(
    '!title' => filter_xss($title),
    '!subtitle' => filter_xss($subtitle),
  )), PASS_THROUGH);
}

/**
 * Generic callback that handles when we don't have any context.
 */
function emicdora_set_title_and_breadcrumbs_object(AbstractObject $object, $title = NULL) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  // Only Source objects and Critical Editions should be visible outside the
  // context of a Critical Edition.
  if (emicdora_is_source($object)) {
    drupal_set_breadcrumb(emicdora_get_breadcrumbs_source($object));
    drupal_set_title(filter_xss($object->label));
  }
  else {
    drupal_set_breadcrumb(emicdora_get_breadcrumbs_critical_edition($object));
    // Hacking hack hack hack.. We need to check the menu paths as this callback
    // will be used for all sub tasks, etc.
    $matches = array();
    if (preg_match('/islandora\/object\/[^\/]+$/', current_path(), $matches)) {
      emicdora_set_page_title_and_subtitle($object->label, t('Critical Apparatus'));
    }
    else {
      drupal_set_title(filter_xss($object->label));
    }
  }
}

/**
 * Gets the default breadcrumbs when the object is a source without any context.
 */
function emicdora_get_breadcrumbs_source(AbstractObject $source) {
  return array(
    l(t("COOP"), EMICDORA_SEARCH_COOP_PATH),
    filter_xss($source->label),
  );
}


/**
 * Gets the minimal required breadcrumbs for Critical Edition pages.
 */
function emicdora_get_breadcrumbs_critical_edition(AbstractObject $critical_edition) {
  return array(
    l(t("Editions"), EMICDORA_SEARCH_EDITIONS_PATH),
    l(filter_xss($critical_edition->label), "islandora/object/{$critical_edition->id}"),
  );
}

/**
 * Sets the title and breadcrumbs for the given Critical Edition and subtitle.
 */
function emicdora_set_title_and_breadcrumbs_critical_edition(AbstractObject $critical_edition, $subtitle = NULL) {
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, isset($subtitle) ? $subtitle : t('Critical Apparatus'));
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Source" page.
 *
 * islandora/object/critical_edition:pid/source/source:pid.
 */
function emicdora_set_title_and_breadcrumbs_source(AbstractObject $critical_edition, AbstractObject $source) {
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $breadcrumbs[] = filter_xss($source->label);
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, $source->label);
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Sources" page.
 *
 * islandora/object/critical_edition:pid/sources.
 */
function emicdora_set_title_and_breadcrumbs_sources(AbstractObject $critical_edition) {
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $breadcrumbs[] = t("Sources");
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, t("Search Source(s)"));
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Version" page.
 *
 * islandora/object/critical_edition:pid/version/version:pid.
 */
function emicdora_set_title_and_breadcrumbs_version(AbstractObject $critical_edition, AbstractObject $version) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $base_path = "islandora/object/{$critical_edition->id}";
  $source = emicdora_get_version_source($version);
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $breadcrumbs[] = l(filter_xss($source->label), "{$base_path}/source/{$source->id}");
  $breadcrumbs[] = filter_xss($version->label);
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, $version->label);
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Versions" page.
 *
 * islandora/object/critical_edition:pid/versions
 * islandora/object/critical_edition:pid/versions/source:pid
 */
function emicdora_set_title_and_breadcrumbs_versions(AbstractObject $critical_edition, AbstractObject $source = NULL) {
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  if (isset($source)) {
    $breadcrumbs[] = l(filter_xss($source->label), "islandora/object/{$critical_edition->id}/source/{$source->id}");
  }
  $breadcrumbs[] = t("Versions");
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, t("Search Versions(s)"));
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Transcription" page.
 *
 * islandora/object/critical_edition:pid/transcription/transcription:pid
 */
function emicdora_set_title_and_breadcrumbs_transcription(AbstractObject $critical_edition, AbstractObject $transcription) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $base_path = "islandora/object/{$critical_edition->id}";
  $version = emicdora_get_transcription_version($transcription);
  $source = emicdora_get_version_source($version);
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $breadcrumbs[] = l(filter_xss($source->label), "{$base_path}/source/{$source->id}");
  $breadcrumbs[] = l(filter_xss($version->label), "{$base_path}/version/{$version->id}");
  $breadcrumbs[] = filter_xss($transcription->label);
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, $transcription->label);
}

/**
 * Sets the title / subtitle / breadcrumbs for the "Collation" page.
 *
 * islandora/object/critical_edition:pid/compare
 */
function emicdora_set_title_and_breadcrumbs_collation(AbstractObject $critical_edition) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $breadcrumbs[] = t('Collation');
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, t('Collation'));
}
