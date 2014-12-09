/*jslint browser: true*/
/*global jQuery, Drupal, IIAUtils*/
/**
 * @file
 * Defines the Islandora Image Annotation widget and it's Drupal behaviour.
 *
 * Requires jQuery 1.7+.
 * Requires jQuery UI.
 */
(function ($) {
  'use strict';

  /**
   * The DOM element that represents the Singleton Instance of this class.
   * @type {string}
   */
  var base = '#full-window-button';

  /**
   * Initialize the Tabs section of the Image annotation widget.
   *
   * We can only have one Tabs per page.
   */
  Drupal.behaviors.teiEditorFullWindow = {
    attach: function (context, settings) {
      $(base, context).once('teiEditorFullWindow', function () {
        var $wrapper, $cwrc, expand, collapse;
        $wrapper = $('#tei_editor_cwrc_writer_wrapper');
        $cwrc = $('#cwrc_wrapper');
        expand = function () {
          $('#admin-menu-wrapper').hide();
          $cwrc.css({
            height: '100%'
          });
          require(['jquery'], function ($) {
            $('#cwrc_wrapper').layout().resizeAll();
          });
        };
        collapse = function () {
          $('#admin-menu-wrapper').show();
          $cwrc.css({ height: '700' });
          require(['jquery'], function ($) {
            $('#cwrc_wrapper').layout().resizeAll();
          });
        };
        $(base).click(function () {
          if ($wrapper.hasClass('tei-editor-full-window')) {
            collapse();
          } else {
            expand();
          }
          $wrapper.toggleClass('tei-editor-full-window');
        });

        // Listen for the 'esc' key.
        $(document).keyup(function (event) {
          if (event.keyCode === 27) {
            if ($wrapper.hasClass('tei-editor-full-window')) {
              $wrapper.removeClass('tei-editor-full-window');
              collapse();
            }
          }
        });
      });
    }
  };
}(jQuery));
