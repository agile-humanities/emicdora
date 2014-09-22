(function ($) {
    Drupal.behaviors.emicdoraWorkbenchLink = {
        attach: function (context, settings) {
            $('.emicdora-workbench-entity-links').once().change(function(){
              var selected = $('.emicdora-workbench-entity-links option:selected').val();
              if (selected != 'none') {
                window.location = Drupal.settings.basePath + selected;
              }
            });
            $('.emicdora-workbench-source-links').once().change(function(){
                var selected = $('.emicdora-workbench-source-links option:selected').val();
                if (selected != 'none') {
                  window.location = Drupal.settings.basePath + 'emicdora/source/add/' + selected + '/FALSE/TRUE';
                }
            });
        }
    };
})(jQuery);

