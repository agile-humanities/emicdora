<?php

/**
 * @file
 * Drush hook implementations and associated functions.
 */

/**
 * Implements hook_drush_command().
 */
function emicdora_drush_command() {
  $command = array();

  $command['emicdora-add-xacml-batch'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_MAX,
    'options' => array(
      'limit' => array(
        'value' => 'optional',
        'description' => dt('The max number of objects for which to query at a time. (affects memory usage, defaults to 100)'),
      ),
    ),
    'drupal dependenies' => array(
      'islandora',
      'islandora_xacml_api',
    ),
    'examples' => array(
      'drush -u 1 emicdora-add-xacml-batch',
    ),
  );

  return $command;
}

/**
 * Drush command to go add POLICYs.
 */
function drush_emicdora_add_xacml_batch() {
  $batch = array(
    'operations' => array(
      array(
        'emicdora_add_xacml_policy_batch_operation',
        array(
          drush_get_option('limit', 100),
        ),
      ),
    ),
  );
  batch_set($batch);
  drush_backend_batch_process();
}

/**
 * Batch operation to update an object in the CO-OP.
 */
function emicdora_add_xacml_policy_batch_operation($limit, &$context) {
  $sandbox =& $context['sandbox'];

  drupal_static_reset('islandora_get_tuque_connection');
  $tuque = islandora_get_tuque_connection();

  $query = <<<EOQ
SELECT DISTINCT ?object ?created
WHERE {
  ?object <fedora-model:createdDate> ?created .
  {
    ?object <fedora-rels-ext:isMemberOfCollection>+ <info:fedora/emic:co-op> .
  }
  UNION {
    ?coll <fedora-rels-ext:isMemberOfCollection>+ <info:fedora/emic:co-op> .
    ?object <fedora-rels-ext:isMemberOf>+ ?coll .
  }
  OPTIONAL {
      ?object <fedora-view:disseminates> ?ds .
      ?ds <fedora-view:disseminationType> <info:fedora/*/POLICY> .
   }
   FILTER(!bound(?ds) && regex(str(?object), 'emic'))
   !filter
}
ORDER BY ?created

EOQ;
  if (!isset($sandbox['total'])) {
    $sandbox['progress'] = 0;
    $sandbox['total'] = $tuque->repository->ri->countQuery(format_string($query, array(
      '!filter' => '',
    )), 'sparql');
    if ($sandbox['total'] == 0) {
      $context['finished'] = 1;
      $context['message'] = t('Nothing to fix.');
      return;
    }
    $sandbox['result_stash'] = array();
  }

  if (empty($sandbox['result_stash'])) {
    $limited_query = format_string($query, array(
      '!filter' => (isset($sandbox['last_date']) ?
          "FILTER(?created > \"{$sandbox['last_date']}\"^^<xsd:dateTime>)" :
          ''),
    )) . <<<EOQ
LIMIT $limit
EOQ;
    drush_log(dt('Querying for @count objects.', array('@count' => $limit)));
    $sandbox['result_stash'] = $tuque->repository->ri->sparqlQuery($limited_query);
    if (empty($sandbox['result_stash'])) {
      // Ran out of items early?
      $context['finished'] = 1;

      $context['message'] = t('Updated @count PID(s) and ran out of items early... Somebody manually updated a document?', array(
        '@count' => $sandbox['total'],
      ));
      return;
    }
  }

  $result = array_shift($sandbox['result_stash']);
  $updating_pid = $result['object']['value'];
  $sandbox['last_date'] = $result['created'];
  $object_to_update = islandora_object_load($updating_pid);

  module_load_include('inc', 'islandora_xacml_api', 'includes/islandora_xacml');
  $module_path = drupal_get_path('module', 'emicdora');
  $xacml = new IslandoraXacml($object_to_update, file_get_contents("$module_path/data/objects/co_op_xacml_policy.xml"));
  $xacml->writeBackToFedora();

  watchdog('emicdora', 'Updated POLICY for @pid.', array(
    '@pid' => $updating_pid,
  ));

  $sandbox['progress'] = min($sandbox['total'], $sandbox['progress'] + 1);
  $context['finished'] = $sandbox['progress'] / $sandbox['total'];

  $context['message'] = t('Updated @progress of @total.', array(
    '@progress' => $sandbox['progress'],
    '@total' => $sandbox['total'],
  ));
}
