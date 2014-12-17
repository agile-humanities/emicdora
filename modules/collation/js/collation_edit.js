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
      var variant_selected = false;
      function emicdora_get_variants() {
        var variant_counts = $.map($('.variant'), function(el) {
          return  $(el).data('variant');
        });
        $.unique(variant_counts);
        return variant_counts.sort();
      }
      $(document).keyup(function(e) {
        if (e.keyCode == 27) {
          if ($('#full-window-button').val() == Drupal.t('Exit Full Window')) {
            $('#collatex_iframe').toggleClass('emicdora-collation_fullwindow');
            $('#compareviewer-1009').show();
            $('#admin-menu-wrapper').show();
            $('#cwrc_wrapper').css({
              height: '600',
            });
            $('#full-window-button').val(Drupal.t('Full Window'))
          }
          if ($('#button-1027-btnIconEl').hasClass('exitFullscreenIcon')) {
            $('#collatex_iframe').show();
            $('.region-sidebar-first').show();
            $('#admin-menu-wrapper').show();
            $('#button-1027-btnEl').trigger('click');
          }
        }
      });
      $(document).delegate('span.merged', 'click', function() {
        var qualifier = $(this).attr('id').slice(1);
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);
        if ($(this).hasClass('merged_selected')) {
          merged_content = '';
          $('#merged_text').val('');
          $(".merged").removeClass('merged_selected');
        } else {
          $(".merged").removeClass('merged_selected');
          if (left.hasClass('merged') && right.hasClass('merged')) {
            left.addClass('merged_selected');
            right.addClass('merged_selected');
            $('#merged_text').text($(left).text());
            wrapped_content = left.wrap('<span/>');
            merged_content = $(wrapped_content).parent().html();
            $(wrapped_content).unwrap();
          }
        }
      });

      $(document).delegate('span.variant', 'click', function() {
        var qualifier = $(this).attr('id').slice(1);
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);
        if ($(this).hasClass('variant_selected')) {
          merged_content = '';
          $('#merged_text').val('');
          $(".variant").removeClass('variant_selected');
          variant_selected = false;
        } else {
          $(".variant").removeClass('variant_selected');
          left.addClass('variant_selected');
          right.addClass('variant_selected');
          variant_selected = $(this).data('variant');
          $('#merged_text').text($(left).text());
          wrapped_content = left.wrap('<span/>');
          merged_content = $(wrapped_content).parent().html();
          $(wrapped_content).unwrap();
        }

      });
      $(".collation_resize").resizable();
      $('#full-window-button').click(function() {
        $('#collatex_iframe').toggleClass('emicdora-collation_fullwindow');
        if ($(this).val() == Drupal.t('Full Window')) {
          window.scrollTo(0, 0);
          $('#admin-menu-wrapper').hide();
          window.scrollTo(0, 0);
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
      });

      waitUntilExists("versionview-1010", function() {
        $('#button-1027-btnEl').click(function() {
          if ($('#button-1027-btnIconEl').hasClass('fullscreenIcon')) {
            $('#admin-menu-wrapper').hide();
            $('#collatex_iframe').hide();
            $('.region-sidebar-first').hide();
          }
          else {
            $('#admin-menu-wrapper').show();
            $('#collatex_iframe').show();
            $('.region-sidebar-first').show();
          }
        });

        $('#merge_container').hide();
        $('#unmerge_container').hide();
        $('#merge_label').click(function() {
          $('#merge_container').toggle();
        });
        $('#unmerge_label').click(function() {
          $('#unmerge_container').toggle();
        });
        var contents = $("#emicdora_collatex_iframe").contents();
        contents.find("#logo").hide();
        contents.find("#examples").closest('.form-element').hide();
        contents.find("#graphml").closest('.yui3-g').hide();
        var $head = contents.find("head");
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
          $("#top-label").text($('#combobox-1026-inputEl').val());
          selection_deleted = rangy.getSelection();
          text_deleted = selection_deleted.toString();
          if (selection_deleted.toHtml() !== '') {
            context_deleted = selection_deleted.toHtml();
          }
          if (context_deleted.indexOf('<span') === -1) {
            context_deleted = window.getSelection().anchorNode.parentNode.outerHTML;
          }
          $("#diff_l").text(text_deleted);
        });
        // Adds html to context_added.
        $('#versionview-1011-body').mouseup(function(evt) {
          $("#bottom-label").text($('#combobox-1027-inputEl').val());
          selection_added = rangy.getSelection();
          text_added = selection_added.toString();
          if (selection_added.toHtml() !== '') {
            context_added = selection_added.toHtml();
          }
          if (context_added.indexOf('<span') === -1) {
            context_added = window.getSelection().anchorNode.parentNode.outerHTML;
          }
          $("#diff_r").text(text_added);
        });

        // Add functionality to arrow keys.
        $(".emicdora_next_button").click(function() {
          if (typeof(variant_counts) === 'undefined') {
            var variant_counts = emicdora_get_variants();
          }
          if (variant_counts.length > 0) {
            current_index = $.inArray(variant_selected, variant_counts);
            current_count = variant_counts[current_index];
            next_index = (current_index === -1) ? 0 : ++current_index;
            next_count = variant_counts[next_index];
            current_selector = '[data-variant=' + current_count + ']';
            next_selector = '[data-variant=' + next_count + ']';
            $(current_selector).removeClass('variant_selected');
            $(next_selector).addClass('variant_selected');
            variant_selected = $(next_selector).data('variant');
          }
        });

        $(".emicdora_previous_button").click(function() {
          if (typeof(variant_counts) === 'undefined') {
            var variant_counts = emicdora_get_variants();
          }
          if (variant_counts.length > 0) {
            current_index = $.inArray(variant_selected, variant_counts);
            current_count = variant_counts[current_index];
            previous_index = (current_index === -1) ? 0 : --current_index;
            previous_count = variant_counts[previous_index];
            current_selector = '[data-variant=' + current_count + ']';
            previous_selector = '[data-variant=' + previous_count + ']';
            $(current_selector).removeClass('variant_selected');
            $(previous_selector).addClass('variant_selected');
            variant_selected = $(previous_selector).data('variant');
          }
        });

        $("#collation_link").click({action: 'link'}, execute_callback);
        $("#collation_variant").click({action: 'variant'}, execute_callback);
        $("#collation_unlink").click({action: 'unlink'}, execute_callback);
        $("#save_changes").click({action: 'save'}, execute_callback);
        function execute_callback(args) {
          var all_added;
          var all_deleted;
          if (args.data.action == 'link' || args.data.action == 'variant') {
            if (text_added.length < 1 || text_deleted.length < 1) {
              alert('Text to link must be selected from both panes.')
              return;
            }
          }
          if (args.data.action == 'unlink') {
            if ($('#merged_text').text() == '') {
              alert('No text selected to unmerge.')
              return;
            }
          }
          if (args.data.action == 'save') {
            if ($("#save_changes").text() == 'Saving..') {
              return;
            }
            $("#save_changes").text("Saving..")
            $(".merged").css('background-color', '');
            all_added = encodeURIComponent($("#versionview-1011-body").html());
            all_deleted = encodeURIComponent($("#versionview-1010-body").html());
          }
          else {
            $("#save_changes").show();
            all_added = encodeURIComponent($("#versionview-1011").html());
            all_deleted = encodeURIComponent($("#versionview-1010").html());
          }

          var callback_url = Drupal.settings.basePath + 'emicdora/edit_collation/';
          $.ajax({
            url: callback_url,
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
              emicdora_counter: emicdora_counter
            },
            async: false,
            success: function(data, status, xhr) {
              var results = JSON.parse(data);
              if (results.hasOwnProperty('message')) {
                alert(results.message)
              }
              variant_counts = emicdora_get_variants();
              emicdora_counter = results.emicdora_counter;
              if (results.refresh == "refresh") {
                $('#versionview-1010-body').html($(results.new_deleted).html());
                $('#versionview-1010-body span').each(function() {
                  if ($(this).text().indexOf('<br>') >= 0) {
                    ($(this).html($(this).text()));
                  }
                });
                $('#versionview-1011-body').html($(results.new_added).html());
                $('#versionview-1011-body span').each(function() {
                  if ($(this).text().indexOf('<br>') >= 0) {
                    ($(this).html($(this).text()));
                  }
                });
                $('#merged_text').text("");
              }
              if (results.added == 'success') {
                $("#save_changes").text("Save Changes")
                $("#save_changes").hide();
                $(".emicdora_input").text('');
                $('#diff_l').text("");
                $('#diff_r').text("");
              }
            },
            error: function(data, status, xhd) {
              console.log("The function execute_callback has failed");
            }
          });
        }
      });
      $('#emicdora_collatex_iframe').load(function() {
        var contents = $("#emicdora_collatex_iframe").contents();
        contents.find("#logo").hide();
        contents.find("#examples").closest('.form-element').hide();
        contents.find("#graphml").closest('.yui3-g').hide();
        var $head = contents.find("head");
        $head.append($("<link/>", {
          rel: "stylesheet",
          href: Drupal.settings.basePath + "sites/all/modules/emicdora/modules/collation/css/emicdora_collatex.css",
          type: "text/css"
        }
        ));
      });
    }
  };
})(jQuery);