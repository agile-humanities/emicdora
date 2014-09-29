/**
 * @file
 *  Handles the data collection for collation editting.
 */
(function($) {

  Drupal.behaviors.emicdoraEdit = {
    attach: function(context, settings) {
      var emicdora_counter = 'undeclared';
      var context_deleted = "";
      var context_added = "";
      var text_deleted = "";
      var text_added = "";
      var merged_content = "";
      $(document).delegate('span.merged', 'click', function() {
        var qualifier = $(this).attr('id').slice(1);
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);

        if (left.css("background-color") != 'transparent') {
          merged_content = '';
          $('#merged_text').val('');
          $(".merged").css('background-color', '');
        } else {
          $(".merged").css('background-color', '');
          left.css("background-color", 'green');
          right.css("background-color", 'green');
          $('#merged_text').val($(left).text());
          wrapped_content = left.wrap('<span/>');
          merged_content = $(wrapped_content).parent().html();
          $(wrapped_content).unwrap();
        }

      });
      $(".collation_resize").resizable();
      $('#full-window-button').click(function() {
        $('#collatex_iframe').toggleClass('emicdora-collation_fullwindow');
        if ($(this).val() == Drupal.t('Full Window')) {
          $('#admin-menu-wrapper').hide();
          $(this).val(Drupal.t('Exit Full Window'));
          $('#compareviewer-1009').hide();
          $('.x-css-shadow').hide();
          $('#cwrc_wrapper').css({
            height: '100%',
          });
        }
        else {
          $(this).val(Drupal.t('Full Window'));
          $('#compareviewer-1009').show();
          $('#admin-menu-wrapper').show();
          $('#cwrc_wrapper').css({
            height: '600',
          });
        }
        $('#cwrc_wrapper').layout().resizeAll();
      });

      waitUntilExists("versionview-1010", function() {
        var $head = $("#emicdora_collatex_iframe").contents().find("head");
        $head.append($("<link/>", {
          rel: "stylesheet",
          href: Drupal.settings.basePath + "sites/all/modules/emicdora/modules/collation/css/emicdora_collatex.css",
          type: "text/css"
        }
        ));
        $('.emicdora_input').text('');
        $("#save_changes").hide();
        // Adds html to context_deleted.
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
          $("#diff_l").text(text_deleted);
        });
        // // Adds html to context_added.
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
          $("#diff_r").text(text_added);
        });
        $("#collation_link").click({action: 'link'}, execute_callback);
        $("#collation_unlink").click({action: 'unlink'}, execute_callback);
        $("#save_changes").click({action: 'save'}, execute_callback);
        function execute_callback(args) {
          var all_added;
          var all_deleted;
          if (args.data.action == 'link') {
            if (text_added.length < 1 || text_deleted.length < 1) {
              alert('Text to link must be selected from both panes.')
              return;
            }
          }
          if (args.data.action == 'save') {
            $(".merged").css('background-color', '');
            $("#save_changes").hide();
            all_added = encodeURIComponent($("#versionview-1011-body").html());
            all_deleted = encodeURIComponent($("#versionview-1010-body").html());
          }
          else {
            $("#emicdora_status").text("Unsaved changes");
            $("#save_changes").show();
            all_added = encodeURIComponent($("#versionview-1011").html());
            all_deleted = encodeURIComponent($("#versionview-1010").html());
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
                $('#versionview-1011-body').html($(results.new_added).html());
              }
              if (results.added == 'success') {
                $(".emicdora_input").text('');
                $('#diff_l').text("");
                $('#diff_r').text("");
                $("#emicdora_status").text("Changes saved.");
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