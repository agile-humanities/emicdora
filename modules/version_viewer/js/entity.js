/*jshint browser: true, devel: true*/

/**
 * Checks if the given entity is overlapping or not.
 *
 * @param {Object} entity An object containing the entity data.
 * @param {String} entity.anchorType Either 'element' or 'offset', entities
 *   which have 'offset' are overlapping all others will be regular element
 *
 * @returns {boolean}
 *   True if the given entity is overlapping false otherwise.
 */
function isOverlappingEntity(entity) {
  'use strict';
  return entity.anchorType === 'offset';
}

/**
 * Generates a selector where all the entity's elements can be selected.
 *
 * @param {String} entID The identifier of the given entity.
 * @param {Object} entity An object containing the entity data.
 *
 * @return {String}
 *   A CSS selector that will target the appropriate element(s) for binding
 *   the tooltip to.
 */
function generateSelector(entID, entity) {
  'use strict';
  if (isOverlappingEntity(entity)) {
    return 'span.overlap-spanning-annotation.' + entID;
  }
  // Regular entity.
  return '.tei *[data-annotationid="' + entID + '"]';
}
