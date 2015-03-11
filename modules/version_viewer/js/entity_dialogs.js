/*jshint browser: true*/
/*global jQuery, Drupal, generateSelector */

Drupal.versionViewer = Drupal.versionViewer || {};
Drupal.versionViewer.dialogs = Drupal.versionViewer.dialogs || {};

(function ($) {
  'use strict';

  /**
   * Entity can display a dialog.
   *
   * @param {Object} entity An object containing the entity data.
   * @param {String} entity.dialogMarkup The content of the dialog box.
   * @param {Object} entity.cwrcAttributes Entity attributes from the RDF.
   * @param {Object} entity.cwrcAttributes.cwrcInfo Additional info.
   * @param {String} entity.cwrcAttributes.cwrcInfo.name The name of the Entity.
   * @return {boolean} True if the entity can display a dialog, false otherwise.
   */
  Drupal.versionViewer.dialogs.canDisplayEntity = function (entity) {
    var hasMarkup = entity.dialogMarkup !== undefined && entity.dialogMarkup !== null;
    var hasName = entity.cwrcAttributes !== undefined &&
      entity.cwrcAttributes !== null &&
      entity.cwrcAttributes.cwrcInfo !== undefined &&
      entity.cwrcAttributes.cwrcInfo !== null &&
      entity.cwrcAttributes.cwrcInfo.name !== undefined &&
      entity.cwrcAttributes.cwrcInfo.name !== null;
    return hasMarkup && hasName;
  };

  /**
   *
   * @param {String} entID The identifier of the given entity.
   * @return {boolean} True if the entity has a dialog, false otherwise.
   */
  Drupal.versionViewer.dialogs.entityHasDialog = function (entID) {
    return $('#ent_dialog_' + entID).length !== 0;
  };

  /**
   * Reset the dialog content scrollable area to the top.
   *
   * @param {String} entID The identifier of the given entity.
   */
  function scrollDialogContentToTop(entID) {
    $('#ent_dialog_' + entID).scrollTop(0);
  }

  /**
   * Gets the target element in which to bind the dialog box to.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   *
   * @return {jQuery}
   *   The element to bind the dialog to.
   */
  function getDialogTargets(entID, entity) {
    var selector = generateSelector(entID, entity);
    // We filter out the super / sub / line magic elements if they are hidden.
    return $(selector).filter(function() {
      return $(this).css('visibility') !== 'hidden';
    });
  }

  /**
   * Show an Entity dialog when the given entity is clicked.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   */
  Drupal.versionViewer.dialogs.createEntityDialog = function (entID, entity) {
    var targets = getDialogTargets(entID, entity);
    var data = {
      entID: entID,
      entity: entity
    };
    targets.bind('click', data, Drupal.versionViewer.dialogs.showEntityDialogEvent);
  };

  /**
   * Shows the entity dialog described by the event.
   *
   * @param {Object} event The click event to display an entity.
   */
  Drupal.versionViewer.dialogs.showEntityDialogEvent = function (event) {
    var entID = event.data.entID;
    var entity = event.data.entity;
    var selector = '#ent_dialog_' + entID;
    if ($(selector).length === 0) {
      // We add the dialog markup to the bottom of the page, but it will be
      // immediately moved into the content of the dialog box below when called.
      $('#content').append(entity.dialogMarkup);
    }
    $(selector).dialog({
     title: entity.cwrcAttributes.cwrcInfo.name,
     width: 400,
     height: 200,
     closed: false,
     cache: false,
     resizeable: true,
     collapsible: true,
     modal: false
    });
    scrollDialogContentToTop(entID);
  };

  /**
   * Removes the click events from Entity elements and destroys the Dialog Box.
   *
   * @param {String} entID The identifier of the given entity.
   * @param {Object} entity An object containing the entity data.
   */
  Drupal.versionViewer.dialogs.destroyEntityDialog = function (entID, entity) {
    var selector = '#ent_dialog_' + entID;
    var targets = getDialogTargets(entID, entity);
    targets.unbind('click', Drupal.versionViewer.dialogs.showEntityDialogEvent);
    $(selector).dialog('destroy');
  };

}(jQuery));
