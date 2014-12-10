(function ($) {
  Drupal.behaviors.ViewerFullWindow = {
    attach: function (context, settings){
      $('#wb_max_min').click(function() {
        $('#eui_window').toggleClass('eui-window-fullscreen');
        if ($(this).hasClass('win-switch-full')) {
            $('#admin-menu-wrapper').hide();
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
            $('#eui_window').layout('resize', {
              width:'100%',
              height:'100%',
            });
        }
        else {
          $('#admin-menu-wrapper').show();
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
        }
      });
    }
  };
  // Listen for the 'esc' key.
  $(document).keyup(function(e) {
    if (e.keyCode == 27) {
      if ($("#wb_max_min").hasClass('win-switch-norm')) {
        $('#eui_window').toggleClass('eui-window-fullscreen');
        $('#admin-menu-wrapper').show();
        $("#wb_max_min").removeClass("win-switch-norm").addClass("win-switch-full");
        $("#wb_max_min").attr('title', "Maximize Viewer");
        var element = $('#eui_window').detach();
        $('#content').append(element);
        $('#eui_window').css('max-height', '729px');
        if ($('#eui_window').layout('panel', 'south').is(":visible")) {
          $('#eui_window').layout('panel', 'south').panel('resize', {height: '678'});
        }
        $('#eui_window').layout('resize', {
          width:'100%',
          height:'550px',
        });
        $(window).trigger('resize');
      }
    }
  });
})(jQuery);
