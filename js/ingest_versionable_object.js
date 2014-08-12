/**
 * @file
 *  Handles the versionable object ingest form specialy javascript nonsense.
 */
(function($) {
  "use strict";

  // Hide uploader.
  $(document).ready(function() {
    // Assume it's always hidden to start off.
    $('.form-type-plupload').hide();
    $('input[name="transcription_source"]').click(function() {
      if ($('input[name="transcription_source"][value="upload"]').is(':checked')) {
        $('.form-type-plupload').show();
      }
      else {
        // Hide the uplaod header and allowed files if a user is
        // clicking around the radio buttons.
        $('.form-type-plupload').hide();
      }
    });

    // Prevent enter submit on the source textfield as this will skip
    // validation of the #access = FALSE elements.
    $('input[name="source"]').keypress(function(event) {
      if (event.keyCode == '13') {
        event.preventDefault();
      }
    });
  });
})(jQuery);
