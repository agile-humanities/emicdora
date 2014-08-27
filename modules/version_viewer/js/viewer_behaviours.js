(function ($) {
  Drupal.behaviors.ViewerFullWindow = {
    attach: function (context, settings){
      $('#wb_max_min').click(function() {
        $('#eui_window').toggleClass('eui-window-fullscreen');
        if ($(this).hasClass('win-switch-full')) {
            $('#admin-menu-wrapper').hide();
            $("#wb_max_min").removeClass("win-switch-full").addClass("win-switch-norm");
            $("#wb_max_min").attr('title', "Minimize Viewer");
            $('#eui_window').layout();
        }
        else {
          $('#admin-menu-wrapper').show();
          $("#wb_max_min").removeClass("win-switch-norm").addClass("win-switch-full");
          $("#wb_max_min").attr('title', "Maximize Viewer");
          $('#eui_window').layout();
        }
      });
    }
  };
  // Listen for the 'esc' key.
  $(document).keyup(function(e) {
    if (e.keyCode == 27) {
      if($("#wb_max_min").hasClass('win-switch-norm')) {
        $('#eui_window').toggleClass('eui-window-fullscreen');
        $('#admin-menu-wrapper').show();
        $("#wb_max_min").removeClass("win-switch-norm").addClass("win-switch-full");
        $("#wb_max_min").attr('title', "Minimize Viewer");
        $('#eui_window').layout();
      }
    }
  });
})(jQuery);
