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
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);
        $(".merged").css('background-color', '');
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
        $("#collation_link").click({action: 'link'}, execute_callback);
        $("#collation_unlink").click({action: 'unlink'}, execute_callback);
        $("#save_changes").click({action: 'save'}, execute_callback);

        function execute_callback(args) {
          var all_added;
          var all_deleted;
          if (args.data.action == 'save') {
            $(".merged").css('background-color', '');
            $("#emicdora_status").text("Changes saved.");
            all_added = encodeURIComponent($("#versionview-1011").html());
            all_deleted = encodeURIComponent($("#versionview-1011").html());
          }
          else {
            $("#emicdora_status").text("Unsaved changes");
            all_added = encodeURIComponent($("#versionview-1011").html());
            all_deleted = encodeURIComponent($("#versionview-1011").html());
          }

          var callback_url = Drupal.settings.basePath + 'emicdora/edit_collation/';
          $.ajax({
            url: callback_url,
            dataType: "XML",
            type: "POST",
            data: {
              action: args.data.action,
              collation_id: Drupal.settings.collation.collation_name,
              context_deleted: encodeURIComponent(context_deleted),
              text_deleted: text_deleted,
              context_added: encodeURIComponent(context_added),
              text_added: text_added,
              merged_content: encodeURIComponent(merged_content),
              all_deleted: all_deleted,
              all_added: all_added,
              emicdora_counter: emicdora_counter,
            },
            async: false,
            success: function(data, status, xhr) {
              var results = JSON.parse(data);
              emicdora_counter = results.emicdora_counter;
              if (results.refresh == "refresh") {
                $('#versionview-1010-body').html($(results.new_deleted).html());
                child_props = $(results.new_deleted).html();
                console.log(child_props)
                $('#versionview-1011-body').html($(results.new_added).html());
              }
            },
            error: function(data, status, xhd) {
              console.log("The function execute_callback has failed");
            },
          });
        }
      });
    }
  };
})(jQuery);