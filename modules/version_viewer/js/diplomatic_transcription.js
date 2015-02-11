/*jshint browser: true*/
/*global jQuery, Drupal*/
/**
 * @file
 * Creates a number of drupal behaviours for diplomatic transcriptions.
 */

(function ($) {
  'use strict';

  /**
   * Gets all the target elements that are part of the given group.
   *
   * @see http://www.tei-c.org/release/doc/tei-p5-doc/en/html/ref-att.pointing.html
   *
   * @param {jQuery} group
   *   jQuery object that contains a 'data-target' attribute with all the
   *   members of the substitution group.
   *
   * @return {jQuery}
   *   A jQuery object composed of all the elements in the given group element.
   */
  function getSubstitutionGroup(group) {
    var ids = group.attr('data-target').match(/(\w+)/g);
    var substitutions = $.map(ids, function(id) {
      return $('span[data-xml-id="' + id + '"]:visible');
    });
    return $(substitutions.reduce(function(result, current) {
      return result.concat(current.get());
    }, []));
  }

  /**
   * Highlights the given elements
   */
  function addHighlight(elements) {
    elements.addClass('hi');
  }

  /**
   * Removes highlights from the given elements
   */
  function removeHighlight(elements) {
    elements.removeClass('hi');
  }

  /**
   * Drupal Attach behaviours.
   */
  $(document).ready(function () {

    // Highlight all the members of a substJoin if one is clicked.
    Drupal.behaviors.highlightSubstJoin = {
      attach: function () {
        $('.tei.diplomatic .substJoin').once('highlightSubstJoin', function () {
          var substJoin = $(this),
            group = getSubstitutionGroup(substJoin),
            timeout;
          // Clicking on any member of the group highlights all members of the
          // group.
          group.click(function(event) {
            event.stopPropagation();
            addHighlight(group);
            // Highlight last for at most 5 seconds.
            timeout = setTimeout(function () {
              removeHighlight(group);
            }, 5000);
          });
          // Any clicks not on the group remove the highlight from the group if
          // they are highlighted.
          $(substJoin).parents('body').click(function () {
            if (timeout !== undefined) {
              clearTimeout(timeout);
              timeout = undefined;
            }
            removeHighlight(group);
          });
        });
      }
    };

    // Highlight a subst if it is clicked.
    Drupal.behaviors.highlightSubst = {
      attach: function () {
        $('.tei.diplomatic .subst').once('highlightSubst', function () {
          var subst = $(this);
          var timeout;
          // Clicking on the subst will highlight it.
          $(this).click(function(event) {
            event.stopPropagation();
            addHighlight(subst);
            // Highlight last for at most 5 seconds.
            timeout = setTimeout(function () {
              removeHighlight(subst);
            }, 5000);
          });
          // Any clicks not on the subst will remove the highlight.
          $(this).parents('body').click(function () {
            if (timeout !== undefined) {
              clearTimeout(timeout);
              timeout = undefined;
            }
            removeHighlight(subst);
          });
        });
      }
    };
  });
}(jQuery));
