(function ($) {
  $(document).ready(function () {
	  
	  
  // Initilize our layout per versionable obj type.
  switch(Drupal.settings.versionable_object_viewer.mode) {
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
  var btn_background_color = 'red';
  $('#wb_show_til').css('background-color', btn_background_color);
  $('#wb_show_til').addClass('annos');
  $('#wb_show_annos').css('background-color', btn_background_color);
  $('#wb_show_annos').addClass('annos');
  
  $('#wb_image').css('background-color', btn_background_color);
  $('#wb_reading').css('background-color', btn_background_color);
  
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
      $.ajax({
          url: Drupal.settings.basePath + 'islandora/object/' + pid + '/get_tree_data/' + dpid,
          async:false,
          success: function(data, status, xhr) {
            $('#easyui_tree').tree({
              data: data
            });
          },
          error: function(data, status, xhd) {
          }
      });
    }
    
    $('#ui-easy-paginator').pagination({
      onSelectPage:function(pageNumber, pageSize){
        show_transcription(pageNumber);
        update_tree_data();
      }
    });
    
    function show_annotations(nodes) {
      if (nodes.length > 0 && nodes[0]['attributes']['urn']) {
        for (var i = 0; i < nodes.length; i++) {
          var anno_id = nodes[i]['attributes']['urn'].replace("urn:uuid:", "");
          paint_commentAnnoTargets(null, 'canvas_0', anno_id, nodes[i]['attributes']['type']);
        }
      }
      else {
        for (var i = 0; i < nodes.length; i++) {
          var ent_id = nodes[i]['attributes']['annotationId'];
          $("span[data-annotationid='" + ent_id + "']").css('background-color', 'red');
          show_entity_tooltip(nodes[i]['attributes'], ent_id);
          if (nodes[i]['attributes']['cwrcType'] == 'textimagelink') {
            var anno_id = nodes[i]['attributes']['cwrcAttributes']['attributes']['uuid'].replace("urn:uuid:", "");
            paint_commentAnnoTargets(null, 'canvas_0', anno_id, "comment");
          }
          if (nodes[i]['attributes']['cwrcType'] == 'imageannotation') {
        	  var anno_id = nodes[i]['attributes']['uuid'];
        	  paint_commentAnnoTargets(null, 'canvas_0', anno_id, "comment");
          }
        }
      }
    }
    function hide_annotations(nodes) {
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
          $("span[data-annotationid='" + ent_id + "']").css('background-color', 'initial');
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
    
    function show_entity_tooltip(data, ent_id) {
      if (data.hasOwnProperty('cwrcAttributes')) {
	   var colour = "red";
	    if (data['cwrcAttributes']['attributes']['Colour']) {
	      colour = data['cwrcAttributes']['attributes']['Colour'];
	    }
	    $("span[data-annotationid='" + ent_id + "']").css('background-color', colour);
	    
	    $("span[data-annotationid='" + ent_id + "']").tooltip({
	      position: 'top',
	      width: 100,
	      height: 100,
	      hideEvent: 'none',
	      content: function(){
	        var tool_tip_content = data['cwrcAttributes']['cwrcInfo']['name'];
	        if (data['cwrcAttributes']['cwrcInfo'].hasOwnProperty('description') ) {
	          tool_tip_content = data['cwrcAttributes']['cwrcInfo']['description'];
	        }
	        return '<div class="easyui-panel" style="width:100px;height:100px;padding:10px;">' + 
	        tool_tip_content +
	        '</div>';
	      },
	      onShow: function(){
	        var t = $(this);
	        t.tooltip('tip').focus().unbind().bind('blur',function(){
	        t.tooltip('hide');
	        });
	      }
	    }).show();
	    $("span[data-annotationid='" + ent_id + "']").click(function(){
	      if ($('#ent_dialog_' + ent_id).length == 0) {
	        $('#content').append('<div id="' + 'ent_dialog_' + ent_id + '">' + build_dialog_content(data) + '</div>');
	      }
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
      var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + page + '&type=rd';
      add_tab("wb_reading_tab", url, 'reading_tei');
      advance_shared_canvas_page(page);
    }
    
    function advance_shared_canvas_page(page) {
      $.ajax({
          url: Drupal.settings.basePath + 'islandora/anno/setup/'
            + Drupal.settings.versionable_object_viewer.pids[page - 1],
          async:false,
          success: function(data, status, xhr) {
            islandora_canvas_params = data;
            continueSetup();
          },
          error: function(data, status, xhd) {
              alert("Please Login to site");
          },
          dataType: 'json'
      });
    }
    
    function show_tei() {
      $.ajax({
        url: Drupal.settings.versionable_object_viewer.trans_url + '?page=' + page,
        async: false,
        timeout: 3000,
        success: function(data, status, xhr) {
          $('#content_data').empty();
          $('#content_data').append('<pre>' + data + '</pre>');
        },
        error: function() {
          console.log("failure");
        }
        });
    }
    
    $('#eui_window').layout('collapse','south');

    $('.work_action_img').click(function() {
      var is_selected = false;
      if ($(this).hasClass('img_selected')) {
        $(this).removeClass('img_selected');
      }
      else {
        $(this).addClass('img_selected');
        is_selected = true;
      }
      
      var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
      switch($(this).attr('id')) {
          case 'wb_meta':
            $('#wb_meta').css('background-color', 'initial');
            
            $('#wb_meta').css('background-color', btn_background_color);
            $('#easy-ui-south').css('height', '500px');
            toggle_layout(is_selected, 'south', 'wb_meta');
            break;
          case 'wb_dt':
        	  $('#wb_reading').css('background-color', 'initial');
              $('#wb_tei_markup').css('background-color', 'initial');
        	  $('#wb_dt').css('background-color', 'initial');
        	  $('#wb_tei_markup').css('background-color', 'initial');
        	  
        	  $('#wb_dt').css('background-color', btn_background_color);
            var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=dt';
            add_tab("wb_dt_tab", url, 'diplomatic_tei');
            break;
          case 'wb_image':
        	  $('#wb_image').css('background-color', 'initial');
        	  $('#wb_image').css('background-color', btn_background_color);
            toggle_layout(!is_selected, 'east', 'wb_image');
            break;
          case 'wb_reading':
        	  $('#wb_dt').css('background-color', 'initial');
              $('#wb_tei_markup').css('background-color', 'initial');
        	  $('#wb_reading').css('background-color', 'initial');
        	  
        	  $('#wb_reading').css('background-color', btn_background_color);
            var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=rd';
            add_tab("wb_reading_tab", url, 'reading_tei');
            break;
          case 'wb_tei_markup':
        	  $('#wb_reading').css('background-color', 'initial');
        	  $('#wb_meta').css('background-color', 'initial');
        	  $('#wb_dt').css('background-color', 'initial');
        	  $('#wb_tei_markup').css('background-color', 'initial');
        	  $('#wb_tei_markup').css('background-color', btn_background_color);
            var pid = Drupal.settings.versionable_object_viewer.tei_rdf_pids[pageNumber - 1];
            var url = Drupal.settings.basePath + 'islandora/version_viewer/tei_markup/page/' + pid;
            add_tab("wb_tei_markup_tab", url, "", "json", true);
            break;
          case 'wb_show_annos':
            var ddt = $("#easyui_tree").tree('find', 'tree_imageannotations');
            var dda = $("#easyui_tree").tree('find', 'tree_entities');
            
        	  if ($(this).hasClass('annos')) {
    	        $(this).removeClass('annos');
    	        $(this).css('background-color', 'initial');
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
    	        $('#wb_show_annos').css('background-color', btn_background_color);
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
      	        $(this).css('background-color', 'initial');
      	        if (ddt) {
      	          $('#' + ddt.domId).hide();
      	          if (ddt['children'].length > 0) {
      	            hide_tree_children(ddt['children']);
      	          }
      	        }
      	      }
      	      else {
      	        $(this).addClass('annos');
      	        $('#wb_show_til').css('background-color', btn_background_color);
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
      for (var i = 0; i< children.length; i++) {
        $("#" + children[i].domId).hide();
      }
      children = $("#easyui_tree").tree('getChecked');
      hide_annotations(children);
    }
    
    function show_tree_children(children) {
      for (var i = 0; i< children.length; i++) {
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
        $('#' + selector).css('background-color', 'initial');
        if (selector == "wb_show_annos") {
          var ddt = $("#easyui_tree").tree('find', 'tree_textimagelinks');
          $('#' + ddt.domId).show();
        }
      }
      else {
        $('#eui_window').layout('expand', region);
        $('#' + selector).css('background-color', btn_background_color);
        if (selector == "wb_show_annos") {
          var ddt = $("#easyui_tree").tree('find', 'tree_textimagelinks');
          $('#' + ddt.domId).hide();
        }
      }
    }
    
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
      onResize:function(w,h){
        var mode = Drupal.settings.versionable_object_viewer.mode;
        if ( mode == "text" || mode == "image") {
          // @see emicdora/modules/version_viewer/js/islandora_image_annotation_init.js.
          resizeCanvas(cleanDrawSVGAnnotations);
        }
      }
  });
  });
})(jQuery);