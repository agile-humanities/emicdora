(function ($) {
  $(document).ready(function () {
    // jQuery EasyUI tree controller.
    // Use this to control image anotations.
    $("#easyui_tree").tree({
      onClick: function(node) {
        //console.log(node);
      },
      onCheck: function(node, checked) {
        if (node['attributes']['urn']) {
	      if (checked) {
	        show_annotations([node]);
	      }
	      else {
	        hide_annotations([node]);
	      }
        }
        else {
          if (checked) {
              show_annotations(node.children);
          }
          else {
            hide_annotations(node.children);
          }
        }
      }
    });
    
    $('#ui-easy-paginator').pagination({
    	onSelectPage:function(pageNumber, pageSize){
    		show_transcription(pageNumber);
    	}
    });
    
    function show_annotations(anno_array) {
      for (var i = 0; i < anno_array.length; i++) {
        console.log(anno_array[i]);
        var anno_id = anno_array[i]['attributes']['urn'].replace("urn:uuid:", "");
        paint_commentAnnoTargets(null, 'canvas_0', anno_id, anno_array[i]['attributes']['type']);
      }
    }
    
    function hide_annotations(anno_array) {
      for (var i = 0; i < anno_array.length; i++) {
        console.log(anno_array[i]);
        var anno_id = anno_array[i]['attributes']['urn'].replace("urn:uuid:", "");
        $('.svg_' + anno_id).remove();
      }
    }
    
    function show_transcription(page) {
      var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + page + '&type=rd';
      add_tab("wb_reading_tab", url, 'reading_tei');
      advance_shared_canvas_page(page);
    }
    
    function advance_shared_canvas_page(page) {
    	console.log(Drupal.settings.versionable_object_viewer.pids);
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
	        	console.log("dip tei");
	        	//$('#wb_dt_tab').addClass('diplomatic_tei');
	            break;
	        case 'wb_image':
	        	toggle_layout(is_selected, 'east');
	        	break;
	        case 'wb_reading':
	        	var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber) + '&type=rd';
	        	add_tab("wb_reading_tab", url, 'reading_tei');
	        	//$('#wb_dt_tab').addClass('reading_tei');
	            break;
	        case 'wb_tei_markup':
	        	var pid = Drupal.settings.versionable_object_viewer.tei_rdf_pids[pageNumber - 1];//Drupal.settings.versionable_object_viewer.pids[pageNumber - 1];
	        	console.log(pid);
	        	var url = Drupal.settings.basePath + 'islandora/version_viewer/tei_markup/page/' + pid;
	        	add_tab("wb_tei_markup_tab", url, "", "json", true);
	            break;
	        case 'wb_show_annos':
	        	toggle_layout(!is_selected, 'west');
	            break;
	        case 'wb_show_til':
	        	toggle_layout(!is_selected, 'west');
	            break;
	    }
    });
    
    function add_tab(type, endpoint, add_class = "", data_type = "json") {
    	console.log(endpoint);
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
    	var nfo = decodeURIComponent(data['body']);
    	var ddt = decodeURIComponent(nfo);
    	console.log(ddt);
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
            resizeCanvas();
        }
    });
    
    function toggle_layout(selected, region) {
      if (!selected) {
        $('#eui_window').layout('collapse', region);
        console.log('show');
      }
      else {
        $('#eui_window').layout('expand', region);
        console.log('hide');
      }
    }
    
    var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
	var url = Drupal.settings.versionable_object_viewer.trans_url + '?page=' + (pageNumber);
	
	// Show our first tab.
	add_tab("wb_reading_tab", url, "reading_tei");
	toggle_layout(false, 'west');
  });
})(jQuery);