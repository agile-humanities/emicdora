(function($) {
  $(document).ready(function() {
    $('body').click(function(){
      // Hide any active tooltips (sometimes they do not clear).
      $('.tooltip-f').tooltip('hide');
    });
    // Initilize our layout per versionable obj type.
    switch (Drupal.settings.versionable_object_viewer.mode) {
      case "text":
        update_tree_data();
        break;
      case "audio":
        $('#wb_tei_markup').hide();
        $('#wb_show_annos').hide();
        $('#wb_show_til').hide();
        break;
      case "image":
        $('#wb_tei_markup').hide();
        $('#wb_show_annos').hide();
        $('#wb_show_til').hide();
        break;
      case "video":
        $('#wb_tei_markup').hide();
        $('#wb_show_annos').hide();
        $('#wb_show_til').hide();
        break;
    }

    var is_toggled = false;

    // Setup the initial menu 'look'.
    $('#wb_show_til').addClass('annos');
    $('#wb_show_annos').addClass('annos');

    $('#wb_image').addClass('img_selected');
    $('#wb_reading').addClass('img_selected');
    $('#wb_show_til').addClass('img_selected');
    $('#wb_show_annos').addClass('img_selected');

    // jQuery EasyUI tree controller.
    // Use this to control image anotations.
    $("#easyui_tree").tree({
      onCheck: function(node, checked) {
        var nodes = (node['attributes']['root'] ? node['children'] : []);
        if (nodes.length == 0) {
          nodes.push(node);
        }
        if (checked) {
          show_annotations(nodes);
        }
        else {
          hide_annotations(nodes);
        }
      }
    });

    function update_tree_data() {
      var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
      var dpid = Drupal.settings.versionable_object_viewer.tei_rdf_pids[pageNumber - 1];
      var pid = Drupal.settings.versionable_object_viewer.pids[pageNumber - 1];
      var version_pid = Drupal.settings.versionable_object_viewer.version_pid;
      $.ajax({
        url: Drupal.settings.basePath + 'islandora/object/' + pid + '/get_tree_data/' + dpid + '/' + version_pid,
        async: false,
        success: function(data, status, xhr) {
          $('#easyui_tree').tree({
            data: data
          });
          // Resize content.
          $('#eui_window').layout('resize', {
            width: '100%'
          });
        },
        error: function(data, status, xhd) {
        }
      });
    }

    $('#ui-easy-paginator').pagination({
      onSelectPage: function(pageNumber, pageSize) {
        show_transcription(pageNumber);
        update_tree_data();
      }
    });

    function show_annotations(nodes) {
      // Hide any active tooltips (sometimes they do not clear).
      $('.tooltip-f').tooltip('hide');

      if (nodes.length > 0 && nodes[0]['attributes']['urn']) {
        for (var i = 0; i < nodes.length; i++) {
          var anno_id = nodes[i]['attributes']['urn'].replace("urn:uuid:", "");
          paint_commentAnnoTargets(null, 'canvas_0', anno_id, nodes[i]['attributes']['type']);
        }
      }
      else {
        for (var i = 0; i < nodes.length; i++) {
          var ent_id = nodes[i]['attributes']['annotationId'];
          if (nodes[i]['attributes']['anchorType'] == 'offset') {
            var start = nodes[i]['attributes']['offsets']['start'];
            var end = nodes[i]['attributes']['offsets']['end'];
            var get_offset_element = function (offset) {
              var selector = '.tei .line-magic .text .' + offset['element'] + "[data-" + offset['id_attribute'].toLowerCase() + "='" + offset['id'] + "']";
              return document.querySelector(selector);
            };

            // Get start offset_id
            var start_element = get_offset_element(start);
            var end_element = get_offset_element(end);

            var find_text_node_at_offset = function (element, offset) {
              var info = {
                remaining: offset,
                node: null
              };
              var elements =  $.unique(
                $(element)
                  .children('.overlap-spanning-annotation')
                  .find('.overlap-spanning-annotation')
                  .addBack()
                  .add(element)
                  .contents()
                  .get()
              );
              $(elements)
                .filter(function (element) {
                  return this.nodeType == Node.TEXT_NODE && this.data != ' ';
                })
                .each(function(index, element) {
                  if (info.remaining >= this.data.length) {
                    info.remaining -= this.data.length;
                  }
                  else {
                    info.node = this;
                    return false;
                  }
                });
              return info;
            };

            // Custom handle overlap tooltips and "painting".
            // Find and split "start" node.
            var start_info = find_text_node_at_offset(start_element, start['offset']);
            var start_suffix = start_info.remaining == start_info.node.length ?
              start_info.node :
              start_info.node.splitText(start_info.remaining);
            // Find and split "end" node.
            var end_info = find_text_node_at_offset(end_element, end['offset']);
            // Split text nodes.
            var end_suffix = end_info.remaining == end_info.node.length ?
              null :
              end_info.node.splitText(end_info.remaining);

            // Tag relevant text nodes between the start and end elements
            // (exclusive).
            var found_start = false;
            var found_end = false;
            var temp_linked_overlaps = [];
            var linked_overlaps = [];
            // No nice way to recursively consider text nodes...
            var unique = $.unique($('.tei .line-magic .text *')
              .contents()
              .get());
            $(unique)
              .filter(function (index, element) {
                if (this.nodeType != Node.TEXT_NODE) {
                  return false;
                }
                if (this == start_suffix) {
                  found_start = true;
                  return true;
                }
                else if (start_suffix && this == end_info.node) {
                  found_end = true;
                  return true;
                }
                else {
                  return found_start && !found_end;
                }
              })
              .each(function(){
                var parents = $.unique($(this).parent('span.overlap-spanning-annotation:not(.tooltip-f)').get());
                $(parents).each(function(){ temp_linked_overlaps.push($(this).prop("className").replace(" ", ".")); });;
              })
              .wrap('<span class="overlap-spanning-annotation ' + ent_id + '"></span>');
            temp_linked_overlaps = $.unique(temp_linked_overlaps);
            // Associate the linked_overlaps with current overlap.
            for (var index in temp_linked_overlaps) {
              if (temp_linked_overlaps[index]) {
                var tooltip_selector = temp_linked_overlaps[index].replace(" ", ".");
                linked_overlaps.push(tooltip_selector);
                // Get current data and append to it.
                var data_overlap_attr = $('overlap-spanning-annotation.' + ent_id).attr('data-linked-overlaps');
                var current_overlaps = [];
                if (typeof data_overlap_attr !== typeof undefined && data_overlap_attr !== false) {
                  var current_overlaps = data_overlap_attr.split(",");
                }
                current_overlaps.push('overlap-spanning-annotation.' + ent_id);
                $("span." + tooltip_selector).attr('data-linked-overlaps', current_overlaps.join());
              }
            }

            $('span.overlap-spanning-annotation.' + ent_id)
              .css('text-decoration', 'underline')
              .attr('data-linked-overlaps', linked_overlaps.join());

            show_entity_tooltip(nodes[i]['attributes'], ent_id);

            var checked = $("#easyui_tree").tree('getChecked');
            for (var j = 0; j < checked.length; j++) {
              if (checked[j]['attributes']['annotationId'] == start['id'] || checked[j]['attributes']['annotationId'] == end['id']) {
                var tooltips_elements = [];
                if (checked[j]['attributes'].hasOwnProperty('nestedTooltips')) {
                  tooltips_elements = checked[j]['attributes']['nestedTooltips'];
                }
                tooltips_elements.push('span.overlap-spanning-annotation.' + ent_id);
                var temp_attributes = checked[j]['attributes'];
                temp_attributes['nestedTooltips'] = tooltips_elements;
                $("#easyui_tree").tree('update', {
                  target: checked[j].target,
                  attributes: temp_attributes
                });
                // Reset the parent entity to have it update the tooltip code.
                var selector = ".tei *[data-annotationid='" + checked[j]['attributes']['annotationId'] + "']";
                $(selector).off();
                show_entity_tooltip(checked[j]['attributes'], checked[j]['attributes']['annotationId']);
              }
            }
          } else {
            $("span[data-annotationid='" + ent_id + "']").css('background-color', 'red');
            show_entity_tooltip(nodes[i]['attributes'], ent_id);
            if (nodes[i]['attributes']['cwrcType'] == 'textimagelink') {
              var anno_id = nodes[i]['attributes']['cwrcAttributes']['attributes']['uuid'].replace
              paint_commentAnnoTargets(null, 'canvas_0', anno_id, "comment");
            }
            if (nodes[i]['attributes']['cwrcType'] == 'imageannotation') {
              var anno_id = nodes[i]['attributes']['uuid'];
              paint_commentAnnoTargets(null, 'canvas_0', anno_id, "comment");
            }
          }
        }
      }
    }
    function hide_annotations(nodes) {
      // Hide any active tooltips (sometimes they do not clear).
      $('.tooltip-f').tooltip('hide');

      if (nodes.length > 0 && nodes[0]['attributes']['urn']) {
        for (var i = 0; i < nodes.length; i++) {
          var anno_id = nodes[i]['attributes']['urn'].replace("urn:uuid:", "");
          $('.svg_' + anno_id).remove();
        }
      }
      else {
        // Hide Entities.
        for (var i = 0; i < nodes.length; i++) {
          var ent_id = nodes[i]['attributes']['annotationId'];
          if (nodes[i]['attributes']['anchorType'] == 'offset') {
            $('span.overlap-spanning-annotation.' + ent_id)
              .css('text-decoration', 'inherit')
              .contents()
              .unwrap();

            var checked = $("#easyui_tree").tree('getChecked');
            for (var j = 0; j < checked.length; j++) {
              if (checked[j]['attributes']['annotationId'] == nodes[i]['attributes']['offsets']['start']['id'] || checked[j]['attributes']['annotationId'] == nodes[i]['attributes']['offsets']['start']['id']) {
                if (checked[j]['attributes'].hasOwnProperty('nestedTooltips')) {
                  var tooltips_elements = checked[j]['attributes']['nestedTooltips'];
                  var value = 'span.overlap-spanning-annotation.' + ent_id;
                  for (var index in tooltips_elements) {
                    if (tooltips_elements[index] == value) {
                      tooltips_elements.splice(index, 1);
                      break;
                    }
                  }
                  var temp_attributes = checked[j]['attributes'];
                  temp_attributes['nestedTooltips'] = tooltips_elements;
                  $("#easyui_tree").tree('update', {
                    target: checked[j].target,
                    attributes: temp_attributes
                  });
                }
              }
            }
          }
          else {
            var selector = "span[data-annotationid='" + ent_id + "']";
            $(selector).css('background-color', 'initial');
            // Clear all click and tooltip events.
            $(selector).off();
            if (nodes[i]['attributes']['cwrcType'] == 'textimagelink') {
              var anno_id = nodes[i]['attributes']['cwrcAttributes']['attributes']['uuid'].replace("urn:uuid:", "");
              $('.svg_' + anno_id).remove();
            }
            if (nodes[i]['attributes']['cwrcType'] == 'imageannotation') {
              var anno_id = nodes[i]['attributes']['uuid'];
              $('.svg_' + anno_id).remove();
            }
          }
        }
      }
    }

    function show_entity_tooltip(data, ent_id) {
      console.log(data);
      var descriptive_note = data['descriptiveNote'];
      var positions = ['left', 'right', 'bottom'];
      if (data.hasOwnProperty('cwrcAttributes')) {
        var colour = "red";
        if (data['cwrcAttributes']['attributes']['Colour']) {
          colour = data['cwrcAttributes']['attributes']['Colour'];
        }
        var selector = ".tei *[data-annotationid='" + ent_id + "']";
        if (data['anchorType'] == 'offset') {
          selector = 'span.overlap-spanning-annotation.' + ent_id +  ':first';
          colour = 'inherit';
        }
        $(selector).css('background-color', colour);
        if (descriptive_note !== undefined && descriptive_note !== null && descriptive_note.length > 0) {
          $(selector).tooltip({
            position: 'top',
            hideEvent: 'mouseleave',
            content: function() {
              var tool_tip_content = data['cwrcAttributes']['cwrcInfo']['name'];
              if (data['cwrcAttributes']['cwrcInfo'].hasOwnProperty('description')) {
                tool_tip_content = data['cwrcAttributes']['cwrcInfo']['description'];
              }
              return '<div class="easyui-panel" style="overflow:scroll; width:100px;height:100px;padding:10px;">' +
                  tool_tip_content +
                  '</div>';
            },
            onShow: function() {
              // Allow a mouse to enter and leave the tooltip,
              // for the purposes of scrolling and selecting
              // a tooltips contents.
              var this_tip = $(this);
              this_tip.tooltip('tip').unbind().bind('mouseenter',
                function(){
                  this_tip.tooltip('show');
                }).bind('mouseleave',
                function(){
                  this_tip.tooltip('hide');
                });

              var display_count = 0;
              if (data.hasOwnProperty('nestedTooltips')) {
                var tooltips_elements = data['nestedTooltips'];
                for (var index in tooltips_elements) {
                  if (tooltips_elements[index]) {
                    $(tooltips_elements[index] + ":first").tooltip({
                      position: positions[display_count % positions.length],
                      hideEvent: 'mouseleave'
                    });
                    $(tooltips_elements[index] + ":first").tooltip('show');
                    display_count++;
                  }
                }
              } else {
                // Overlaps need to display other overlaps that collide with it.
                positions = ['bottom', 'right', 'left'];
                var data_overlap_attr = $(selector).attr('data-linked-overlaps');
                if (typeof data_overlap_attr !== typeof undefined && data_overlap_attr !== false) {
                  var linked_tooltips = data_overlap_attr.split(",");
                  for (var index in linked_tooltips) {
                    if (linked_tooltips[index]) {
                      $("." + linked_tooltips[index] + ":first").tooltip({
                        position: positions[display_count % positions.length],
                        hideEvent: 'mouseleave',
                        onShow: function() {
                          // Reset onShow to not have a custom function to
                          // prevent recursive calls to onShow.
                        },
                        onHide: function() {
                          var checked = $("#easyui_tree").tree('getChecked');
                          for (var j = 0; j < checked.length; j++) {
                            if (linked_tooltips[index].search(checked[j]['attributes']['annotationId']) != -1) {
                              // Rebuild tooltip to restore removed the onShow
                              // function and remove this onHide function.
                              show_entity_tooltip(checked[j]['attributes'], checked[j]['attributes']['annotationId']);
                              break;
                            }
                          }
                        }
                      });
                      $("." + linked_tooltips[index] + ":first").tooltip('show');
                      display_count++;
                    }
                  }
                }
              }
            }
          }).show();
        }
        // Reset to remove ":first" to keep the on click working correctly.
        if (data['anchorType'] == 'offset') {
          selector = 'span.overlap-spanning-annotation.' + ent_id;
          // To enable overlay hover all spans not just first.
          $(selector).hover(
            function() {
              $( selector + ':first' ).tooltip('show');
            }, function() {
              $( selector + ':first' ).tooltip('hide');
            }
          );
        }
        $(selector).click(function() {
          // Hide any active tooltips (sometimes they do not clear).
          $('.tooltip-f').tooltip('hide');
          var hasMarkup = (typeof data['dialogMarkup'] != 'undefined' && data['dialogMarkup'] !== null);
          if ($('#ent_dialog_' + ent_id).length == 0 && hasMarkup) {
            $('#content').append(data['dialogMarkup']);
          }
          if (hasMarkup) {
            $('#ent_dialog_' + ent_id).dialog({
              title: data['cwrcAttributes']['cwrcInfo']['name'],
              width: 400,
              height: 200,
              closed: false,
              cache: false,
              resizeable: true,
              collapsible: true,
              modal: false
            });
          }
        });
      }
      else {
        // In this case, we are dealing with a plain image annotation.
        // Wireframes dont well me what to show in this case.
      }
    }


    function build_dialog_content(data) {
      var content = "";
      for (var key in data["cwrcAttributes"]) {
        if (typeof data["cwrcAttributes"][key] !== "object") {
          content += key + ": " + data["cwrcAttributes"][key] + '&#13;&#10;';
        }
        else {
          var obj_key_string = "";
          for (var obj_key in data["cwrcAttributes"][key]) {
            obj_key_string += obj_key + ": " + data["cwrcAttributes"][key][obj_key] + '&#13;&#10;';
          }
          content += obj_key_string;
        }
      }
      return  '<textarea style="width:100%;height:100%;resize:none">' + content + '</textarea>';
    }

    function show_transcription(page) {
      var type = $('#wb_dt').hasClass('img_selected') ? 'dt' : 'rd';
      var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + page + '&type=' + type;
      if (type = 'dt') {
        add_tab("wb_dt_tab", url, 'diplomatic_tei');
      }
      else {
        add_tab("wb_reading_tab", url, 'reading_tei');
      }
      advance_shared_canvas_page(page);
    }

    function advance_shared_canvas_page(page) {
      $.ajax({
        url: Drupal.settings.basePath + 'islandora/anno/setup/'
            + Drupal.settings.versionable_object_viewer.pids[page - 1],
        async: false,
        success: function(data, status, xhr) {
          islandora_canvas_params = data;
          continueSetup();
        },
        error: function(data, status, xhd) {
          alert(Drupal.t("Please Login to site"));
        },
        dataType: 'json'
      });
    }

    $('#eui_window').layout('collapse', 'south');

    $('.work_action_img').click(function() {
      var is_selected = false;
      if ($(this).hasClass('img_selected')) {
        $(this).removeClass('img_selected');
        is_selected = false;
      }
      else {
        $(this).addClass('img_selected');
        is_selected = true;
      }

      var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
      switch ($(this).attr('id')) {
        case 'wb_meta':
          $('#wb_meta').removeClass('img_selected');

          $('#wb_meta').addClass('img_selected');
          toggle_layout(is_selected, 'south', 'wb_meta');
          // Change the window size based on if the easyui is fullscreen or not.
          if ($('#eui_window').hasClass('eui-window-fullscreen')) {
            // Use window height and subtract 50px for the menu bar display.
            var height = $(window).height() - 50;
            $('#eui_window').layout('panel', 'south').panel('resize', {height: height});
          } else {
            $('#eui_window').layout('panel', 'south').panel('resize', {height: '678'});
          }
          break;
        case 'wb_dt':
          $('#wb_reading').removeClass('img_selected');
          $('#wb_tei_markup').removeClass('img_selected');
          $('#wb_dt').removeClass('img_selected');
          $('#wb_tei_markup').removeClass('img_selected');

          $('#wb_dt').addClass('img_selected');
          var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=dt';
          add_tab("wb_dt_tab", url, 'diplomatic_tei');
          break;
        case 'wb_image':
//          $('#wb_image').removeClass('img_selected');
//          $('#wb_image').addClass('img_selected');
          toggle_layout(is_selected, 'east', 'wb_image');
          break;
        case 'wb_reading':
          $('#wb_dt').removeClass('img_selected');
          $('#wb_tei_markup').removeClass('img_selected');
          $('#wb_reading').removeClass('img_selected');

          $('#wb_reading').addClass('img_selected');
          var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=rd';
          add_tab("wb_reading_tab", url, 'reading_tei');
          break;
        case 'wb_tei_markup':
          $('#wb_reading').removeClass('img_selected');
          $('#wb_meta').removeClass('img_selected');
          $('#wb_dt').removeClass('img_selected');
          $('#wb_tei_markup').removeClass('img_selected');
          $('#wb_tei_markup').addClass('img_selected');
          var pid = Drupal.settings.versionable_object_viewer.tei_rdf_pids[pageNumber - 1];
          var url = Drupal.settings.basePath + 'islandora/version_viewer/tei_markup/page/' + pid;
          add_tab("wb_tei_markup_tab", url, "", "json", true);
          break;
        case 'wb_show_annos':
          var ddt = $("#easyui_tree").tree('find', 'tree_imageannotations');
          var dda = $("#easyui_tree").tree('find', 'tree_entities');

          if ($(this).hasClass('annos')) {
            $(this).removeClass('annos');
            $(this).removeClass('img_selected');
            if (ddt) {
              $('#' + ddt.domId).hide();
              if (ddt['children'].length > 0) {
                hide_tree_children(ddt['children']);
              }
            }
            if (dda) {
              $('#' + dda.domId).hide();
              if (dda['children'].length > 0) {
                hide_tree_children(dda['children']);
              }
            }
          }
          else {
            $(this).addClass('annos');
            $('#wb_show_annos').addClass('img_selected');
            if (ddt) {
              $('#' + ddt.domId).show();
              if (ddt['children'].length > 0) {
                show_tree_children(ddt['children']);
              }
            }
            if (dda) {
              $('#' + dda.domId).show();
              if (dda['children'].length > 0) {
                show_tree_children(dda['children']);
              }
            }
          }
          break;
        case 'wb_show_til':
          var ddt = $("#easyui_tree").tree('find', 'tree_textimagelinks');
          if ($(this).hasClass('annos')) {
            $(this).removeClass('annos');
            $(this).removeClass('img_selected');
            if (ddt) {
              $('#' + ddt.domId).hide();
              if (ddt['children'].length > 0) {
                hide_tree_children(ddt['children']);
              }
            }
          }
          else {
            $(this).addClass('annos');
            $('#wb_show_til').addClass('img_selected');
            if (ddt) {
              $('#' + ddt.domId).show();
              if (ddt['children'].length > 0) {
                show_tree_children(ddt['children']);
              }
            }
          }
          break;
      }
      // This crazy bit of logic opens/closes the annotations window, given the correct conditions.
      if ($('#wb_show_til').hasClass('annos') == false && $('#wb_show_annos').hasClass('annos') == false) {
        $('#eui_window').layout('collapse', 'west');
        is_toggled = true;
      }
      else {
        if (is_toggled == true) {
          is_toggled = false;
          $('#eui_window').layout('expand', 'west');
        }
      }
    });

    function hide_tree_children(children) {
      for (var i = 0; i < children.length; i++) {
        $("#" + children[i].domId).hide();
      }
      children = $("#easyui_tree").tree('getChecked');
      hide_annotations(children);
    }

    function show_tree_children(children) {
      for (var i = 0; i < children.length; i++) {
        $("#" + children[i].domId).show();
      }
      children = $("#easyui_tree").tree('getChecked');
      show_annotations(children);
    }

    function hide_all_imageannotations() {
      var node = $("#easyui_tree").tree('find', 'tree_imageannotations');
      hide_annotations($(node).children());
    }

    function add_tab(type, endpoint, add_class, data_type) {
      add_class = typeof add_class !== 'undefined' ? add_class : "";
      data_type = typeof data_type !== 'undefined' ? data_type : "json";
      $.ajax({
        type: 'GET',
        async: false,
        dataType: data_type,
        url: endpoint,
        success: function(data, status, xhr) {
          construct_tab(data, type);
          if (add_class != "") {
            $('#' + type).addClass(add_class);
          }
          // Resize with new content.
          $('#eui_window').layout('resize', {
            width: '100%'
          });
          try {
            Drupal.attachBehaviors();
          } catch(e) {
            console.log(e);
          }
        },
        error: function(xhRequest, ErrorText, thrownError) {
          console.log(ErrorText + ":" + thrownError);
        }
      });
    }

    function construct_tab(data, type) {
      $('#content_data').empty();
      $('#content_data').append(data['body']);
      prettyPrint();
    }

    function toggle_layout(selected, region, selector) {
      if (!selected) {
        $('#eui_window').layout('collapse', region);
        $('#' + selector).removeClass('img_selected');
        if (selector == "wb_show_annos") {
          var ddt = $("#easyui_tree").tree('find', 'tree_textimagelinks');
          $('#' + ddt.domId).show();
        }
      }
      else {
        $('#eui_window').layout('expand', region);
        $('#' + selector).addClass('img_selected');
        if (selector == "wb_show_annos") {
          var ddt = $("#easyui_tree").tree('find', 'tree_textimagelinks');
          $('#' + ddt.domId).hide();
        }
      }
    }

    function add_tooltip_imageannotations() {
      var node = $("#easyui_tree").tree('find', 'tree_imageannotations');
      if (node) {
        var children = node['children'];
        var data = "";
        for (var i = 0; i < children.length; i++) {
          data = children[i]['attributes'];
          var tool_tip_content = data['title'];
          if (data['cwrcInfo'].hasOwnProperty('description')) {
            tool_tip_content = data['cwrcInfo']['description'];
          }
          $("#" + children[i].domId).tooltip({
            position: 'right',
            content: '<div class="easyui-panel" style="width:250px;height:\'auto\';padding:10px;">' + tool_tip_content + '</div>'
          }).show();
        }
      }
    }
    add_tooltip_imageannotations();
    var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
    var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber);

    // Show our first tab.
    add_tab("wb_reading_tab", url, "reading_tei");

    // Callback to fix the drawing of SVG annotations upon resize.
    var cleanDrawSVGAnnotations = function() {
      var children = $("#easyui_tree").tree('getChecked');
      hide_annotations(children);
      show_annotations(children);
    };

    $('#easy-ui-east').panel({
      onResize: function(w, h) {
        var mode = Drupal.settings.versionable_object_viewer.mode;
        if (mode == "text" || mode == "image") {
          // @see emicdora/modules/version_viewer/js/islandora_image_annotation_init.js.
          resizeCanvas(cleanDrawSVGAnnotations);
        }
      }
    });

    $('#center_data').panel({
      onResize: function(w, h) {
        var mode = Drupal.settings.versionable_object_viewer.mode;
        if (mode == "text" || mode == "image") {
          // @see emicdora/modules/version_viewer/js/islandora_image_annotation_init.js.
          resizeCanvas(cleanDrawSVGAnnotations);
        }
      }
    });

    $('#eui_window').css('max-height', '729px');
    // The panels will automatically set to fit, but we
    // use these method to trigger the resize event.
    $('#eui_window').layout('resize', {
      width: '100%',
      height: '729px'
    });
    
//    $("span[data-annotationid]").bind('mouseenter mouseleave', function(e){
//    	console.log(e);
//      console.log("hit it")
//    });
  });
})(jQuery);
