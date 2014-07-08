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
              data: data,
            });
          },
          error: function(data, status, xhd) {
        	  console.log(data);
        	  console.log(status);
        	  console.log(xhd);
              //alert("Please Login to site");
          },
          //dataType: 'json'
      });
    }
    
    $('#ui-easy-paginator').pagination({
      onSelectPage:function(pageNumber, pageSize){
        show_transcription(pageNumber);
        update_tree_data();
      }
    });
    
    function show_annotations(nodes) {
      if (nodes[0]['attributes']['urn']) {
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
        }
      }
    }
    function hide_annotations(nodes) {
      if (nodes[0]['attributes']['urn']) {
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
        }
      }
    }
    
    function show_entity_tooltip(data, ent_id) {
    	console.log(data);
      $("span[data-annotationid='" + ent_id + "']").css('background-color', 'red');
      $("span[data-annotationid='" + ent_id + "']").tooltip({
        position: 'top',
        hideEvent: 'none',
        content: function(){
          // Hidden in the dom.
          return $("#ui_p").panel({
            width:150,
            height:100,
            content: "<div>" + data['cwrcAttributes']['cwrcInfo']['name'] +"</div>",
          });
        },
        onShow: function(){
          var t = $(this);
          t.tooltip('tip').focus().unbind().bind('blur',function(){
          t.tooltip('hide');
          });
        }
      }).show();
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
          $('#center_data').empty();
          $('#center_data').append('<pre>' + data + '</pre>');
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
        $(this).removeClass('img_selected')
      }
      else {
        $(this).addClass('img_selected');
        is_selected = true;
      }
      var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
      switch($(this).attr('id')) {
          case 'wb_meta':
            toggle_layout(is_selected, 'south');
            break;
          case 'wb_dt':
            var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=dt';
            add_tab("wb_dt_tab", url, 'diplomatic_tei');
            break;
          case 'wb_image':
            toggle_layout(!is_selected, 'east');
            break;
          case 'wb_reading':
            var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=rd';
            add_tab("wb_reading_tab", url, 'reading_tei');
            break;
          case 'wb_tei_markup':
            var pid = Drupal.settings.versionable_object_viewer.tei_rdf_pids[pageNumber - 1];
            var url = Drupal.settings.basePath + 'islandora/version_viewer/tei_markup/page/' + pid;
            add_tab("wb_tei_markup_tab", url, "", "json", true);
            break;
          case 'wb_show_annos':
            toggle_layout(is_selected, 'west');
            break;
          case 'wb_show_til':
            toggle_layout(is_selected, 'west');
            break;
      }
    });
    
    function add_tab(type, endpoint, add_class = "", data_type = "json") {
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
        },
      });
    }
    
    function construct_tab(data, type) {
      $('#versions_tabs').tabs('add',{
      id: type,
      title: data['title'],
      content:data['body'],
      closable:true,
      });
      prettyPrint();
    }
    
    $('#easy-ui-east').panel({
        onResize:function(w,h){
          var mode = Drupal.settings.versionable_object_viewer.mode;
       if ( mode == "text" || mode == "image") {
         resizeCanvas();
       }
        }
    });
    
    function toggle_layout(selected, region) {
      if (!selected) {
        $('#eui_window').layout('collapse', region);
      }
      else {
        $('#eui_window').layout('expand', region);
      }
    }
    
    var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
  var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber);
  
  // Show our first tab.
  add_tab("wb_reading_tab", url, "reading_tei");
  
  toggle_layout(false, 'west');
  });
})(jQuery);