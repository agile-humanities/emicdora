(function($) {
  $(document).ready(function() {
    // Assume it's always hidden to start off.
    $('.description').hide();
    $('input[name="content_source"]').click(function() {
      if ($('input[name="content_source"][value="file"]').is(':checked')) {
        $('.description').show();
      }
      else {
        // Hide the allowed file types.
        $('.description').hide();
      }
    });
    // Prevent enter submit on the source textfield as this will skip
    // validation of the #access = FALSE elements.
    $('input[name="content_source"]').keypress(function(event) {
      if (event.keyCode == '13') {
        event.preventDefault();
      }
    });
  });
})(jQuery);
