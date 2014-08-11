/**
 * @file
 *  Handles the versionable object ingest form specialty javascript nonsense.
 */
(function($) {

  Drupal.behaviors.emicdoraEdit = {
    attach: function(context, settings) {
      waitUntilExists("versionview-1010", function() {
        $('#versionview-1010-body').mouseup(function() {
          var text = "";
          if (window.getSelection) {
            selection = window.getSelection();
            text = selection.toString();
            range1 = selection.getRangeAt(0);
          } else if (document.selection && document.selection.type != "Control") {
            text = document.selection.createRange().text;
          }
          $("#emicdora_1").val(text);
        });
        $('#versionview-1011-body').mouseup(function() {
          var text = "";
          if (window.getSelection) {
            selection = window.getSelection();
            text = selection.toString();
            range2 = selection.getRangeAt(0);
          } else if (document.selection && document.selection.type != "Control") {
            text = document.selection.createRange().text;
          }
          $("#emicdora_2").val(text);
        });
      });



    }
  };
})(jQuery);