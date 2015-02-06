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
 * Generic callback that handles when we don't have a critical edition context.
 *
 * @param AbstractObject $object
 *   The object in which we generate the breadcrumbs for.
 *
 * @param string $title
 *   The title for the page, if not provided the object label is used.
 */
function emicdora_set_title_and_breadcrumbs_object(AbstractObject $object, $title = NULL) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  // Only Source objects and their children, as well as Critical Editions should
  // be visible outside the context of a Critical Edition.
  if (emicdora_is_critical_edition($object)) {
    drupal_set_breadcrumb(emicdora_get_breadcrumbs_critical_edition($object));
    // Hacking hack hack hack.. We need to check the menu paths as this callback
    // will be used for all sub tasks, etc.
    $matches = array();
    if (preg_match('/islandora\/object\/[^\/]+$/', current_path(), $matches) || preg_match('/islandora\/object\/[^\/]+\/apparatus\/./', current_path(), $matches)) {
      emicdora_set_page_title_and_subtitle($object->label, t('Critical Apparatus'));
    }
    else {
      drupal_set_title(filter_xss($object->label));
    }
  }
  else {
    // Either we are viewing a source object a child of a source object (page
    // for example).
    $breadcrumbs = emicdora_is_source($object) ?
      emicdora_get_breadcrumbs_source($object) :
      emicdora_get_breadcrumbs_source_child($object);
    drupal_set_breadcrumb($breadcrumbs);
    drupal_set_title(filter_xss($object->label));
  }
}

/**
 * Gets the default breadcrumbs when the object is a source without any context.
 *
 * @param AbstractObject $source
 *   The source object to get the default breadcrumbs for.
 *
 * @return array
 *   The default breadcrumbs as required by drupal_set_breadcrumbs().
 */
function emicdora_get_breadcrumbs_source(AbstractObject $source) {
  return array(
    l(t("CO-OP"), EMICDORA_COOP_PATH),
    filter_xss($source->label),
  );
}

/**
 * Gets the default breadcrumbs when the object is a "child" of a source object.
 *
 * This is only viewable outside the context of the critical edition.
 *
 * @param AbstractObject $child
 *   The child of a source object to get the default breadcrumbs for.
 *
 * @return array
 *   The default breadcrumbs as required by drupal_set_breadcrumbs().
 */
function emicdora_get_breadcrumbs_source_child(AbstractObject $child) {
  $query_string = <<<EOT
select ?parent ?label from <#ri> where
  (<info:fedora/{$child->id}> <fedora-rels-ext:isMemberOfCollection> ?parent
    or <info:fedora/{$child->id}> <fedora-rels-ext:isMemberOf> ?parent
    or <info:fedora/{$child->id}> <fedora-rels-ext:isPartOf> ?parent) and
  ?parent <fedora-model:label> ?label
EOT;
  $results = $child->repository->ri->itqlQuery($query_string);
  $breadcrumbs = array(l(t("CO-OP"), EMICDORA_SEARCH_COOP_PATH));
  foreach ($results as $result) {
    $breadcrumbs[] = l($result['label']['value'], "islandora/object/{$result['parent']['value']}");
  }
  $breadcrumbs[] = filter_xss($child->label);
  return $breadcrumbs;
}

/**
 * Gets the minimal required breadcrumbs for Critical Edition pages.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object to get the default breadcrumbs for.
 *
 * @return array
 *   The default breadcrumbs as required by drupal_set_breadcrumbs().
 */
function emicdora_get_breadcrumbs_critical_edition(AbstractObject $critical_edition) {
  return array(
    l(t("Editions"), EMICDORA_SEARCH_EDITIONS_PATH, array(
      'query' => array(
        'sort' => 'fgs_label_ss asc',
      ),
    )),
    l(filter_xss($critical_edition->label), "islandora/object/{$critical_edition->id}"),
  );
}

/**
 * Sets the title and breadcrumbs for the given Critical Edition and subtitle.
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param string $subtitle
 *   The subtitle to display.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param AbstractObject $source
 *   The source object used to generate the breadcrumbs and title.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param AbstractObject $version
 *   The version object used to generate the breadcrumbs and title.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param AbstractObject $source
 *   The source object used to generate the breadcrumbs and title.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param AbstractObject $transcription
 *   The transcription object used to generate the breadcrumbs and title.
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
 *
 * @param AbstractObject $critical_edition
 *   The critical edition object used to generate the breadcrumbs and title.
 *
 * @param AbstractObject $label
 *   The label of the collation object, to use as a subtitle.
 */
function emicdora_set_title_and_breadcrumbs_collation(AbstractObject $critical_edition, $label) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $label = filter_xss($label);
  $breadcrumbs[] = $label;
  drupal_set_breadcrumb($breadcrumbs);
  emicdora_set_page_title_and_subtitle($critical_edition->label, $label);
}

/**
 * Creates breadcrumbs for TEI Editior.
 *
 * @param AbstractObject $version
 *   The object in which we generate the breadcrumbs for.
 */
function emicdora_set_title_and_breadcrumbs_editor(AbstractObject $version) {
  module_load_include('inc', 'emicdora', 'includes/utilities');
  $membership = $version->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $critical_edition = islandora_object_load($membership[0]['object']['value']);
  $breadcrumbs = emicdora_get_breadcrumbs_critical_edition($critical_edition);
  $derivations = $version->relationships->get(FEDORA_RELS_EXT_URI, 'isDerivationOf');
  $source = islandora_object_load($derivations[0]['object']['value']);
  $breadcrumbs[] = l(filter_xss($source->label), "islandora/object/{$critical_edition->id}/source/{$source->id}");
  $breadcrumbs[] = l(filter_xss($version->label), "islandora/object/{$critical_edition->id}/version/{$version->id}");
  $breadcrumbs[] = t('Edit View');
  drupal_set_breadcrumb($breadcrumbs);
}
