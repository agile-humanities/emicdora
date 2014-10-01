(function ($) {
    Drupal.behaviors.emicdoraWorkbenchLink = {
        attach: function (context, settings) {
            $('#add-to-coop').once().change(function(){
              var selected = $('#add-to-coop option:selected').val();
              if (selected != 'none') {
                window.location = Drupal.settings.basePath + selected;
              }
            });
        }
    };
})(jQuery);
//onchange="this.options[this.selectedIndex].value && (window.location = this.options[this.selectedIndex].value);"