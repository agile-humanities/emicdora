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
  Drupal.behaviors.emicdoraWorkbenchEntityLinks = {
    attach: function (context, settings) {
      $('.emicdora-workbench-entity-links').once().change(function(){
        var selected, collections;
        selected = $('.emicdora-workbench-entity-links option:selected').val();
        collections = {
          person: 'islandora_cwrc_writer_person_entity_collection',
          organization: 'islandora_cwrc_writer_organization_entity_collection',
          title: 'islandora_cwrc_writer_title_entity_collection',
          place: 'islandora_cwrc_writer_place_entity_collection'
        };
        if (selected !== 'none') {
          window.location = Drupal.settings.basePath + 'islandora/object/' + collections[selected] + '/manage/overview/ingest';
        }
      });
    }
  };
})(jQuery);

