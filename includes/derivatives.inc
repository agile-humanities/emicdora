<?php
/**
 * @file
 * Derivative callbacks for EMiC.
 */

/**
 * Updates the label of an object for a place entity.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param bool $force
 *   Whether the derivative is being forcefully created or not.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function emicdora_update_object_label_from_place_entity(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/entity');
  return emicdora_update_object_label_derivative($object, islandora_cwrc_writer_cwrc_api_get_entity_label('place', $object['PLACE']->content));
}

/**
 * Updates the label of an object for a title entity.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param bool $force
 *   Whether the derivative is being forcefully created or not.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function emicdora_update_object_label_from_title_entity(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/entity');
  return emicdora_update_object_label_derivative($object, islandora_cwrc_writer_cwrc_api_get_entity_label('title', $object['TITLE']->content));
}

/**
 * Updates the label of an object for a person entity.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param bool $force
 *   Whether the derivative is being forcefully created or not.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function emicdora_update_object_label_from_person_entity(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/entity');
  return emicdora_update_object_label_derivative($object, islandora_cwrc_writer_cwrc_api_get_entity_label('person', $object['PERSON']->content));
}

/**
 * Updates the label of an object for a organization entity.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param bool $force
 *   Whether the derivative is being forcefully created or not.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function emicdora_update_object_label_from_organization_entity(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/entity');
  return emicdora_update_object_label_derivative($object, islandora_cwrc_writer_cwrc_api_get_entity_label('organization', $object['ORGANIZATION']->content));
}

/**
 * Updates the label of an entity object.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param string $label
 *   The label to be set.
 * @return array
 *   An array describing the outcome of the derivative operation.
 */
function emicdora_update_object_label_derivative(AbstractObject $object, $label) {
  if ($label && $object->label != $label) {
    $object->label = $label;
  }
  return array(
    'success' => TRUE,
    'messages' => array(
      array(
        'message' => t('Updated the label for @pid.'),
        'message_sub' => array(
          '@pid' => $object->id,
        ),
        'type' => 'dsm',
      ),
    ),
  );
}
