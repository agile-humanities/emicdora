/**
 * @file
 * Ties links in Apparatus block to vertical tab display.
 */
(function($) {
  "use strict";
  Drupal.behaviors.apparatus = {

    attach: function(context, settings) {
      var chosen = undefined;
      var oldhash = window.location.hash;
      var newhash = oldhash;
      if (("onhashchange" in window) && !($.browser.msie)) {
        window.onhashchange = function() {
          $('.vertical-tab-button.selected').removeClass('selected');
          var choice = $('.vertical-tab-button a strong');
          choice.each(function(){
            if($(this).text().toLowerCase() == chosen.toLowerCase()) {
              $(this).closest('.vertical-tab-button').addClass('selected');
              newhash = "#" + chosen;
              $(oldhash).css('display', 'none');
              $(newhash).css('display', 'block');
              oldhash = newhash;
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