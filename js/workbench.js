/*jslint browser: true*/
/*global jQuery, Drupal*/
/**
 * @file
 * Core class acts as a central store of annotations.
 *
 * Makes use of the jQuery RDF plugin, which we seem to have a later version 1.1
 * in which no one else in the world has...
 * @see https://code.google.com/p/rdfquery/
 */
(function ($) {
  'use strict';
  Drupal.behaviors.emicdoraWorkbenchLink = {
    attach: function (context, settings) {
      $('.emicdora-workbench-entity-links').once('emicdora-workbench-entity-links', function() {
        $(this).change(function(){
          var selected, collections;
          selected = $('.emicdora-workbench-entity-links option:selected').val();
          collections = {
            person: 'cwrc:personEntityCollection',
            organization: 'cwrc:organizationEntityCollection',
            title: 'cwrc:titleEntityCollection',
            place: 'cwrc:placeEntityCollection'
          };
          if (selected !== 'none') {
            window.location = Drupal.settings.basePath + 'islandora/object/' + collections[selected] + '/manage/overview/ingest';
          }
        });
      });
      $('.emicdora-workbench-source-links').once('emicdora-workbench-source-links', function () {
        $(this).change(function(){
          var selected = $('.emicdora-workbench-source-links option:selected').val();
          if (selected !== 'none') {
            window.location = Drupal.settings.basePath + 'emicdora/source/add/' + selected + '/FALSE/TRUE';
          }
        });
      });
    }
  };
})(jQuery);

