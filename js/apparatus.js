/**
 * @file
 * Ties links in Apparatus block to vertical tab display.
 */
(function($) {
  "use strict";
  Drupal.behaviors.apparatus = {

    attach: function(context, settings) {
      var chosen = undefined;
      var hash = window.location.hash;
      if (("onhashchange" in window) && !($.browser.msie)) {
        window.onhashchange = function() {
          $('.vertical-tab-button.selected').removeClass('selected');
          var choice = $('.vertical-tab-button a strong');
          choice.each(function(){
            if($(this).text().toLowerCase().replace(/[\s,]+/g, '_') == chosen.toLowerCase()) {
              $(this).closest('.vertical-tab-button').addClass('selected');
              hash = "#" + chosen;
              $('.vertical-tabs-pane').css('display', 'none');
              $(hash).css('display', 'block');
            }
          });
        }
      }

      $('.apparatus_link').click(function() {
        var value = this.toString();
        var parts = value.split('#');
        chosen = parts[1].trim();
      });
    }
  };

}(jQuery));