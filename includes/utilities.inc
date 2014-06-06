<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Checks if the given object is a critical edition object.
 *
 * @param AbstractObject $object
 *   The object to check.
 *
 * @return bool
 *   TRUE if it is a critical edition, FALSE otherwise.
 */
function emicdora_is_critical_edition(AbstractObject $object) {
  return in_array('islandora:criticalEditionCModel', $object->models);
}

/**
 * Convenience function used to create objects in hook_required_objects().
 *
 * @param IslandoraTuque $connection
 *   The connection to the fedora repository.
 * @param array $args
 *   An associative array defining the objects properties and datastreams"
 *   - pid: The identifier for the new object.
 *   - label: The new object's label.
 *   - model: The content model of the new object.
 *   - datastreams: An associative array of properties for each datastream where
 *     each key represents the datastream's identifier:
 *     - control_group: The control group for this datastream defaults to 'M'.
 *     - label: The datastream's label.
 *     - file: The file path to the datastream's content.
 *     - mimetype: The mimetype of the given file. Defaults to 'text/xml'.
 * @return AbstractObject
 *   A new Fedora Object, that has yet to be persisted to the repository.
 */
function emicdora_create_required_object(IslandoraTuque $connection, array $args) {
  $object = $connection->repository->constructObject($args['pid']);
  $object->owner = 'fedoraAdmin';
  $object->label = $args['label'];
  $object->models = $args['model'];
  foreach ($args['datastreams'] as $dsid => $properties) {
    $control_group = isset($properties['control_group']) ? $properties['control_group'] : 'M';
    $datastream = $object->constructDatastream($dsid, $control_group);
    $datastream->label = isset($properties['label']) ? $properties['label'] : $dsid;
    $datastream->mimetype = isset($properties['mimetype']) ? $properties['mimetype'] : 'text/xml';
    $datastream->setContentFromFile($properties['file'], FALSE);
    $object->ingestDatastream($datastream);
  }
  return $object;
}