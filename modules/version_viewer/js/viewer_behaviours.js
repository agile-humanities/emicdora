(function ($) {
  Drupal.behaviors.ViewerFullWindow = {
    attach: function (context, settings){
      $('#wb_max_min').once('viewer-full-window', function () {
        $(this).click(function() {
          $('#eui_window').toggleClass('eui-window-fullscreen');
          if ($(this).hasClass('win-switch-full')) {
            $("#wb_max_min").removeClass("win-switch-full").addClass("win-switch-norm");
            $("#wb_max_min").attr('title', "Minimize Viewer");
            $('#eui_window').css('max-height', 'none');
            var element = $('#eui_window').detach();
            $('body').append(element);
            // If the metadate south panel is open resize it with window size.
            if ($('#eui_window').layout('panel', 'south').is(":visible")) {
              // Use window height and subtract 50px for the menu bar display.
              var height = $(window).height() - 50;
              $('#eui_window').layout('panel', 'south').panel('resize', {height: height});
            }
            resizepage();
          }
          else {
            $("#wb_max_min").removeClass("win-switch-norm").addClass("win-switch-full");
            $("#wb_max_min").attr('title', "Maximize Viewer");
            var element = $('#eui_window').detach();
            $('#content').append(element);
            $('#eui_window').css('max-height', '729px');
            if ($('#eui_window').layout('panel', 'south').is(":visible")) {
              $('#eui_window').layout('panel', 'south').panel('resize', {height: '678'});    // resize the panel
            }
            $('#eui_window').layout('resize', {
              width:'100%',
              height:'550px',
            });
            $(window).trigger('resize');
            $('#eui_window').css("top", "0px");
          }
          $(window).resize(function () {
            resizepage();
          });

        function resizepage() {
          if ($('#wb_max_min').hasClass('win-switch-norm')) {
            var bar_top = 0;
            if ($('#admin-menu').length > 0) {
              var bar_top = $('#admin-menu-wrapper').height();
            }
            $('.eui-window-fullscreen').css("top", bar_top + "px");
            $('#eui_window').layout('resize', {
              width:'100%',
              height: "100%",
            });
          }
        }

        });
      });
    }
  };

  // Listen for the 'esc' key.
  $(document).keyup(function(e) {
    if (e.keyCode == 27) {
      if ($("#wb_max_min").hasClass('win-switch-norm')) {
        $('#wb_max_min').trigger('click');
      }
    }
  });
})(jQuery);
