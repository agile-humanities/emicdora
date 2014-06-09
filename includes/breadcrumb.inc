<?php

/**
 * @file
 * Helper functions for setting the breadcrumbs on various pages.
 */

/**
 * Gets the breadcrumbs for the critical edition object display.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object.
 *
 * @return array
 *   An array of links to use as breadcrumbs.
 */
function emicdora_get_critical_edition_breadcrumbs(AbstractObject $critical_edition) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  $breadcrumbs = islandora_get_breadcrumbs($critical_edition);
  // Discard "Home" as it's redundant.
  array_shift($breadcrumbs);
  // Discard "Islandora Repository" as we don't want user navigating to it.
  array_shift($breadcrumbs);
  return $breadcrumbs;
}

/**
 * Sets the breadcrumbs for the critical edition object display.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition to which the breadcrumbs will navigate to.
 */
function emicdora_set_critical_edition_breadcrumbs(AbstractObject $critical_edition) {
  $breadcrumbs = emicdora_get_critical_edition_breadcrumbs($critical_edition);
  $breadcrumbs[] = $critical_edition->label;
  drupal_set_breadcrumb($breadcrumbs);
}

/**
 * Sets the breadcrumbs for the apparatus.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition to which the breadcrumbs will navigate to.
 */
function emicdora_set_apparatus_breadcrumbs(AbstractObject $critical_edition) {
  $breadcrumbs = emicdora_get_critical_edition_breadcrumbs($critical_edition);
  $breadcrumbs[] = l($critical_edition->label, "islandora/object/{$critical_edition->id}");
  drupal_set_breadcrumb($breadcrumbs);
}

/**
 * Sets the breadcrumbs for the source.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition to which the breadcrumbs will navigate to.
 */
function emicdora_set_source_breadcrumbs(AbstractObject $critical_edition) {
  emicdora_set_apparatus_breadcrumbs($critical_edition);
}

/**
 * Sets the breadcrumbs for the version.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition this version belongs to.
 * @param AbstractObject $source
 *   The source this version is derived from.
 */
function emicdora_set_version_breadcrumbs(AbstractObject $critical_edition, AbstractObject $source) {
  $breadcrumbs = emicdora_get_critical_edition_breadcrumbs($critical_edition);
  $breadcrumbs[] = l($critical_edition->label, "islandora/object/{$critical_edition->id}");
  $breadcrumbs[] = l($source->label, "islandora/object/{$source->id}");
  drupal_set_breadcrumb($breadcrumbs);
}

/**
 * Sets the breadcrumbs for the transcription.
 *
 * @param AbstractObject $version
 *   The version this transcription belongs to.
 */
function emicdora_set_transcription_breadcrumbs(AbstractObject $version) {
  // @todo Implement.
}
