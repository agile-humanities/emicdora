/**
 * @file
 *  Handles the data collection for collation editting.
 */
(function($) {

  Drupal.behaviors.emicdoraEdit = {
    attach: function(context, settings) {
      var emicdora_counter = 'undeclared';
      var range_deleted = null;
      var range_added = null;
      var selection_deleted = null;
      var selection_added = null;
      var context_deleted = "";
      var context_added = "";
      var text_deleted = "";
      var text_added = "";
      var merged_content = "";

      // Forces related spans to display next to each to each other.
      function emicdora_sync_spans(id) {
        var type = id.charAt(0);
        var qualifier = id.slice(1);
        var aposition = $('#a' + qualifier).position().top;
        var ascrollTop = $("#versionview-1011-body").scrollTop();
        var dposition = $('#d' + qualifier).position().top;
        var dscrollTop = $("#versionview-1010-body").scrollTop();
        if (type == 'a') {
          $("#versionview-1010-body").scrollTop(dscrollTop + dposition - aposition);
        }
        else {
          $("#versionview-1011-body").scrollTop(ascrollTop + aposition - dposition);
        }
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
      $(document).delegate('span.merged_selected, span.variant_selected', 'click', function(e) {
        $('.merged_selected, .variant_selected').removeClass('merged_selected variant_selected');
        merged_content = '';
        $('#merged_text').text('');
      });
      $(document).delegate('span.merged:not(.merged_selected), span.variant:not(.variant_selected)', 'click', function() {
        $('.merged_selected, .variant_selected').removeClass('merged_selected variant_selected');
        var qualifier = $(this).attr('id').slice(1);
        var $left = $('#d' + qualifier);
        var merged = $left.hasClass('merged');
        $left.add('#a' + qualifier).addClass(merged ? 'merged_selected' : 'variant_selected');
        $('#merged_text').text($left.text());
        var $wrapped_content = $left.wrap('<span/>');
        merged_content = $wrapped_content.parent().html();
        $wrapped_content.unwrap();
        emicdora_sync_spans($(this).attr('id'));
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
        $('.emicdora_input').text('');
        $("#save_changes").hide();
        $('.x-panel-body').hover(
            function() {
              $(this).data('active_pane', 1);
            },
            function() {
              $(this).data('active_pane', 0);
            }
        );
        var sync_on = true;
        $('.emicdora_sync_button').click(function() {
          sync_on = !sync_on;
          opacity = sync_on ? 1 : .5;
          $('.emicdora_sync_button').css('opacity', opacity);
        });

        // Listening to the change event of the combo boxes was being blocked.
        setInterval(function() {
          $("#top-label").text($('#combobox-1029-inputEl').val());
          $("#bottom-label").text($('#combobox-1031-inputEl').val());
        }, 500);

        // Adds html to context_deleted.
        $('#versionview-1010-body').mouseup(function(evt) {
          selection_deleted = rangy.getSelection();
          // If multiple ranges ignore the selection.
          var invalid_range = selection_deleted.rangeCount !== 1;
          // If either the start or the end of the selection is outside of this
          // panel ignore it.
          var invalid_anchor = $(selection_deleted.anchorNode).parents('#versionview-1010-body').length !== 1;
          var invalid_focus = $(selection_deleted.focusNode).parents('#versionview-1010-body').length !== 1;
          if (invalid_range || invalid_anchor || invalid_focus) {
            selection_deleted = null;
            return;
          }
          range_deleted = selection_deleted._ranges[0]
          text_deleted = selection_deleted.toHtml();
          $("#diff_l").html(text_deleted);
        });
        // Adds html to context_added.
        $('#versionview-1011-body').mouseup(function(evt) {
          selection_added = rangy.getSelection();
          // If multiple ranges ignore the selection.
          var invalid_range = selection_added.rangeCount !== 1;
          // If either the start or the end of the selection is outside of this
          // panel ignore it.
          var invalid_anchor = $(selection_added.anchorNode).parents('#versionview-1011-body').length !== 1;
          var invalid_focus = $(selection_added.focusNode).parents('#versionview-1011-body').length !== 1;
          if (invalid_range || invalid_anchor || invalid_focus) {
            selection_added = null;
            return;
          }
          range_added = selection_added._ranges[0]
          text_added = selection_added.toHtml();
          $("#diff_r").html(text_added);
        });

        /**
         * Helper to make an element visible by scrolling.
         *
         * @param $element
         *   A jQuery object with the element to make visible.
         * @param $parent
         *   A jQuery object with the element to scroll. Defaults to the
         *   window, if not provided.
         */
        function scroll($element, $parent) {
          if (typeof $parent === "undefined") {
            $parent = $(window);
          }
          var cTop = $element.position().top;
          var cHeight = $element.outerHeight(true);
          var windowTop = $parent.scrollTop();
          var visibleHeight = $parent.height();

          if (cTop < 0) {
            $parent.scrollTop(windowTop + cTop);
          }
          else if ((cTop + cHeight) > visibleHeight) {
            $parent.scrollTop(windowTop + cTop - visibleHeight + cHeight);
          }
        }

        /**
         * Helper to select an merged/variant pair.
         *
         * @param $next
         *   A jQuery object with the element to select.
         */
        function select($next) {
          scroll($next, $next.parent());
          $next.click();
        };

        // Map the previous/next merged/variant to the relevant body.
        var map = {
          'button-1013': '1010',
          'button-1014': '1010',
          'button-1022': '1011',
          'button-1023': '1011',
        };

        // Add functionality to arrow keys.
        $(".emicdora_next_button").click(function() {
          var $current = $('#versionview-' + map[this.id] + '-body').find('.variant_selected');
          var $next = $current.nextAll('.variant').first();
          if ($current.length === 0 || $next.length === 0) {
            // Select first occurence.
            $next = $('.variant').first();
          }
          if ($next.length === 0) {
            alert(Drupal.t('No items over which to iterate.'));
          }
          select($next);
        });

        $(".emicdora_previous_button").click(function() {
          var $current = $('#versionview-' + map[this.id] + '-body').find('.variant_selected');
          var $next = $current.prevAll('.variant').first();
          if ($current.length === 0 || $next.length === 0) {
            // Select last occurence.
            $next = $('.variant').last();
          }
          if ($next.length === 0) {
            alert(Drupal.t('No items over which to iterate.'));
          }
          select($next);
        });
        // Synchronize scolling.
        $("#versionview-1010-body").scroll(function(e) {
          if ($("#versionview-1010-body").data('active_pane') == 1 && sync_on == true) {
            $("#versionview-1011-body").scrollTop($("#versionview-1010-body").scrollTop());
          }
        });
        $("#versionview-1011-body").scroll(function(e) {
          if ($("#versionview-1011-body").data('active_pane') == 1 && sync_on == true) {
            $("#versionview-1010-body").scrollTop($("#versionview-1011-body").scrollTop());
          }
        });
        $("#collation_link").click({action: 'link'}, execute_callback);
        $("#collation_variant").click({action: 'variant'}, execute_callback);
        $("#collation_unlink").click({action: 'unlink'}, execute_callback);
        $("#save_changes").click({action: 'save'}, execute_callback);
        function execute_callback(args) {
          var all_added;
          var all_deleted;
          var requires_dual_selection = args.data.action != "save" && args.data.action != "unlink";
          if ((selection_deleted === null || selection_added === null) && requires_dual_selection) {
            alert(Drupal.t('You must select text from both the left and right panes.'));
            return;
          }
          if (args.data.action == 'link' || args.data.action == 'variant') {
            if (text_added.length < 1 || text_deleted.length < 1) {
              alert(Drupal.t('Text to link must be selected from both panes.'))
              return;
            }
          }
          if (args.data.action == 'unlink') {
            if ($('#merged_text').text() == '') {
              alert(Drupal.t('No text selected to unmerge.'))
              return;
            }
          }
          if (args.data.action == 'save') {
            if ($("#save_changes").text() == 'Saving..') {
              return;
            }
            $("#save_changes").text("Saving..");
            var $right = $("#versionview-1011-body");
            var $left = $("#versionview-1010-body");
          }
          else {
            $("#save_changes").show();
            var $right = $("#versionview-1011");
            var $left = $("#versionview-1010");
          }

          // Find any selected parts.
          var $selected = $left.add($right).find('.variant_selected, .merged_selected');
          var merged = $selected.hasClass('merged_selected');
          // Remove the class used for selecting, so it will not get pushed to
          // the backend.
          $selected.removeClass('variant_selected merged_selected');
          all_added = encodeURIComponent($right.html());
          all_deleted = encodeURIComponent($left.html());
          // Restore the class, for GUI consistency.
          $selected.addClass(merged ? 'merged_selected' : 'variant_selected');

          var callback_url = Drupal.settings.basePath + 'emicdora/edit_collation/';
          var build_selection = function(range) {
            var total_offset = function(node, node_offset) {
              // We need the text offset inside of a given node, so let's
              // sum up our previous siblings.
              var total = node_offset;
              var current = node;
              while (current.previousSibling != null) {
                current = current.previousSibling;
                if (current.nodeType === Node.TEXT_NODE) {
                  total += current.length;
                }
              }
              return total;
            };
            var to_return = {
              start: {
                // The ID of the element in which the selection started.
                id: range.startContainer.parentNode.id,
                // The number of characters from the beginning of the parent
                // node, until we hit the start of the selection.
                offset: total_offset(range.startContainer, range.startOffset)
              },
              end: {
                // The ID of the element in which the selection ended.
                id: range.endContainer.parentNode.id,
                // The number of characters from the beginning of the parent
                // node, until we hit the end of the selection.
                offset: total_offset(range.endContainer, range.endOffset)
              }
            };

            // XXX: Try to account for some oddities in rangy... Placing the
            // selection endpoints inside of element which do not contain any
            // of the selected text (just outside it, really).
            if (range.startOffset === range.startContainer.length) {
              to_return.start.id = range.startContainer.parentNode.nextSibling.id;
              to_return.start.offset = 0;
            }
            if (to_return.end.offset === 0) {
              var alternate = range.endContainer.parentNode.previousSibling;
              to_return.end.id = alternate.id;
              to_return.end.offset = total_offset(alternate.lastChild, alternate.lastChild.length);
            }
            return to_return;
          };
          $.ajax({
            url: callback_url,
            type: "POST",
            data: {
              action: args.data.action,
              collation_id: Drupal.settings.collation.collation_name,
              text_deleted: text_deleted,
              text_added: text_added,
              merged_content: encodeURIComponent(merged_content),
              all_deleted: all_deleted,
              all_added: all_added,
              emicdora_counter: emicdora_counter,
              deleted: requires_dual_selection ? build_selection(range_deleted) : null,
              added: requires_dual_selection ? build_selection(range_added) : null,
              name_deleted: Ext.ComponentQuery.query('#versionSelector1')[0].getValue(),
              name_added: Ext.ComponentQuery.query('#versionSelector2')[0].getValue()
            },
            async: false,
            success: function(results, status, xhr) {
              if (results.hasOwnProperty('message')) {
                alert(results.message)
              }
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
                text_deleted = '';
                text_added = '';
                merged_content = '';
                selection_deleted = null;
                selection_added = null;
                range_deleted = null;
                range_added = null;
                $('#merged_text').text("");
                $('#diff_l').text("");
                $('#diff_r').text("");
              }
              if (results.added == 'success') {
                $("#save_changes").text("Save Changes")
                $("#save_changes").hide();
                $(".emicdora_input").text('');
                $('#merged_text').text("");
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
        contents.find("#logo").attr('title', 'Copyright © 2010-2013 The Interedition Development Group. All rights reserved.');
        contents.find("#examples").closest('.form-element').hide();
        contents.find("#graphml").closest('.yui3-g').hide();
        contents.find("#footer").hide();
        var $head = contents.find("head");
        $head.append($("<link/>", {
          rel: "stylesheet",
          href: Drupal.settings.collation.cssPath,
          type: "text/css"
        }
        ));
        var footer_bar = contents.find(".yui3-skin-sam div#input div.yui-3 div.yui3-u");
        footer_bar.append("<a target='_blank' href='http://collatex.net/' class='collate-x-click' ><span class='collate-x-logo'></span></a>");
      });
    }

  };
})(jQuery);
