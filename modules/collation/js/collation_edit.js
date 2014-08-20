/**
 * @file
 *  Handles the data collection for collation editting.
 */
(function($) {

  Drupal.behaviors.emicdoraEdit = {
    attach: function(context, settings) {
      var emicdora_counter = 1;
      var context_deleted = "";
      var context_added = "";
      var text_deleted = "";
      var text_added = "";
      var merged_content = "";
      $(document).delegate('span.deleted', 'click', function() {
        var qualifier = $(this).prev().attr('id').slice(1);
        $('#diff_l').val($(this).text());
        $('#diff_r').val($('#a' + qualifier).next().text());
      });

      $(document).delegate('span.added', 'click', function() {
        var qualifier = $(this).prev().attr('id').slice(1);
        $('#diff_r').val($(this).text());
        $('#diff_l').val($('#d' + qualifier).next().text());
      });

      $(document).delegate('span.merged', 'click mouseup', function() {
        var qualifier = $(this).attr('id').slice(1);
        $(".merged").css('background-color', '');
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);
        left.css("background-color", 'green');
        right.css("background-color", 'green');
        $('#merged_text').val($(left).text());
        wrapped_content = left.wrap('<span/>');
        merged_content = $(wrapped_content).parent().html();
        $(wrapped_content).unwrap();

      });

      waitUntilExists("versionview-1010", function() {

        $('#versionview-1010-body').mouseup(function(evt) {
          selection_deleted = rangy.getSelection();
          text_deleted = selection_deleted.toString();
          if (selection_deleted.toHtml() !== '') {
            context_deleted = selection_deleted.toHtml();
          }
          if (context_deleted.indexOf('<span') === -1) {
            var elem = document.elementFromPoint(evt.clientX, evt.clientY);
            context_deleted = elem.outerHTML;
          }
          $("#diff_l").val(text_deleted);
        });

        $('#versionview-1011-body').mouseup(function(evt) {
          selection_added = rangy.getSelection();
          text_added = selection_added.toString();
          if (selection_added.toHtml() !== '') {
            context_added = selection_added.toHtml();
          }
          if (context_added.indexOf('<span') === -1) {
            var elem = document.elementFromPoint(evt.clientX, evt.clientY);
            context_added = elem.outerHTML;
          }
          $("#diff_r").val(text_added);
        });
        $("#collation_link").click(function() {
          var callback_url = Drupal.settings.basePath + 'emicdora/edit_collation/';
          $.ajax({
            url: callback_url,
            dataType: "XML",
            type: "POST",
            data: {
              collation_id: Drupal.settings.collation.collation_name,
              context_deleted: context_deleted,
              text_deleted: text_deleted,
              context_added: context_added,
              text_added: text_added,
              merged_content: merged_content,
              all_deleted: $("#versionview-1010").html(),
              all_added: $("#versionview-1011").html(),
              emicdora_counter: emicdora_counter,
            },
            async: false,
            success: function(data, status, xhr) {
              var results = JSON.parse(data);
              emicdora_counter = results.emicdora_counter;
              $('#versionview-1010-body').html($(results.new_deleted).html());
              $('#versionview-1011-body').html($(results.new_added).html());
            },
            error: function(data, status, xhd) {
              console.log("failure");
            },
          });
        });
        function execute_callback() {

        };
      });
    }
  };
})(jQuery);