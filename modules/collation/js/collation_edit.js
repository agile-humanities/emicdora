/**
 * @file
 *  Handles the data collection for collation editting.
 */
(function($) {
  function adjustRange(range) {
    range = range.cloneRange();

    // Expand range to encompass complete element if element's text is all selected by the range
    var container = range.commonAncestorContainer;
    var parentElement = container.nodeType == 3 ? container.parentNode : container;
    if (parentElement.textContent == range.toString()) {
      range.selectNode(parentElement);
    }

    return range;
  }

  function getSelectionHtml() {
    var html = "", sel, range;
    if (typeof window.getSelection != "undefined") {
      sel = window.getSelection();
      if (sel.rangeCount) {
        var container = document.createElement("div");
        for (var i = 0, len = sel.rangeCount; i < len; ++i) {
          range = adjustRange(sel.getRangeAt(i));
          container.appendChild(range.cloneContents());
        }
        html = container.innerHTML;
      }
    } else if (typeof document.selection != "undefined") {
      if (document.selection.type == "Text") {
        html = document.selection.createRange().htmlText;
      }
    }
    return html;
  }
  Drupal.behaviors.emicdoraEdit = {
    attach: function(context, settings) {

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

      $(document).delegate('span.merged', 'click', function() {
        var qualifier = $(this).attr('id').slice(1);
        $(".merged").css('background-color', '');
        left = $('#d' + qualifier);
        right = $('#a' + qualifier);
        left.css("background-color", 'green');
        right.css("background-color", 'green');
        $('#merged_text').val($(left).text());
        merged_content = left.wrap('<span/>').parent().html();
        alert(merged_content);
      });

      waitUntilExists("versionview-1010", function() {

        $('#versionview-1010-body').mouseup(function() {
          selection_deleted = rangy.getSelection();
          text_deleted = selection_deleted.toString();
          if (selection_deleted.toHtml() !== '') {
            context_deleted = selection_deleted.toHtml();
          }
          console.log(context_deleted);
          $("#diff_l").val(text_deleted);
        });

        $('#versionview-1011-body').mouseup(function() {
          selection_added = rangy.getSelection();
          text_added = selection_added.toString();
          context_added = selection_added.toHtml();
          $("#diff_r").val(text_added);
        });
        $("#collation_link").click(function() {
          var callback_url = Drupal.settings.basePath + 'emicdora/edit_collation/';
          $.ajax({
            url: callback_url,
            dataType: "text/xml",
            type: "POST",
            data: {
              collation_id: Drupal.settings.collation.collation_name,
              context_deleted: context_deleted,
              text_deleted: text_deleted,
              context_added: context_added,
              text_added: text_added,
              merged_content :merged_content,
            },
            async: false,
            success: function(data, status, xhr) {
              console.log(data);
            },
            error: function(data, status, xhd) {
              console.log("failure");
            },
          });
        });
      });



    }
  };
})(jQuery);