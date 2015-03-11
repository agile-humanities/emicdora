
/**
 * @file
 * Script for emicdora terms.
 */
(function($) {

  Drupal.behaviors.emicdoraRedirect = {
    attach: function(context, settings) {
      $(".return_button").click(function() {
        var location = window.location.origin + Drupal.settings.basePath + 'islandora/object/' + Drupal.settings.critical_edition;
        window.location.replace(location);
      });
      $(".add_source").click(function() {
        var target = $(this).attr('target');
        var callback_url = Drupal.settings.basePath + 'emicdora/add_source_callback'
        $(this).text('Added');
        $(this).removeClass('add_source');
        $.ajax({
          url: callback_url,
          type: "POST",
          data: {
            source_pid: target,
            critical_edition_pid: Drupal.settings.critical_edition,
          },
          async: true,
          success: function(data, status, xhr) {
            var results = data;
            if (results.hasOwnProperty('message')) {
              alert(results.message)
            }
          },
          error: function(data, status, xhd) {
            console.log("The function execute_callback has failed");
          }
        });
      });
    }
  };
})(jQuery);