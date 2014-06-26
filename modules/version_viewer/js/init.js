(function ($) {
  $(document).ready(function () {
    // Manually set up the draggable.
  //  $('.ui-resizable').draggable().resizable();

    // TODO: jQuery resizeable handles need to be set up manually..
    // @ see: http://api.jqueryui.com/resizable/#option-handles
   // $('#easy-ui-east').resizable( "option", "handles", "n, e, s, w" );
    
    $('#easy-ui-east').resizable({
        maxWidth:800,
        maxHeight:600
    });
    

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
      //var pid = $('#ui-easy-paginator').attr('data-pid');
	  $.ajax({
		url: Drupal.settings.versionable_object_viewer.trans_url + '?page=' + page,
		async: false,
		timeout: 3000,
		success: function(data, status, xhr) {
		        $('#transcription_panel').empty();
		        $('#transcription_panel').append(data);
					
		},
		error: function() {
			console.log("failure");
		}
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
    	console.log($(this).attr('id'));
    	switch($(this).attr('id')) {
	        case 'wb_meta':
	        	console.log("a");
	        	toggle_layout(is_selected, 'south');
	            break;
	        case 'wb_dt':
	        	console.log("b");
	        	add_tab(wb_dt);
	            break;
	        case 'wb_image':
	        	console.log("c");
	        	//console.log("image");
	        	toggle_layout(!is_selected, 'east');
	        	break;
	        case 'wb_reading':
	        	console.log("d");
	        	add_tab(wb_reading);
	            break;
	        case 'wb_tei_markup':
	        	console.log("e");
	        	add_tab(wb_tei_markup);
	            break;
	        case 'wb_show_annos':
	        	console.log("f");
	        	toggle_layout(!is_selected, 'west');
	            break;
	        case 'wb_show_til':
	        	console.log("g");
	        	toggle_layout(!is_selected, 'west');
	            break;
	    }
    });
    
    function add_tab(type) {
    	var pageNumber = $('#ui-easy-paginator').pagination('options').pageNumber;
    	console.log(pageNumber);
    	var pid = $('#ui-easy-paginator').attr('data-pid');
    	$('#center_data').append('<div id="d_trans" style="padding:10px;"></div>');
    	pid = "islandora:14";
    	//construct_easy_ui_panel();
		console.log(Drupal.settings.basePath + 'islandora/version_viewer/transformed_page/' + pid);
    	$.ajax({
    		type: 'GET',
    		async: false,
    		url: Drupal.settings.basePath + 'islandora/version_viewer/transformed_page/' + pid,
    		success: function(data, status, xhr) {
    			//$('#versions_tabs').append('<div id="d_trans" style="padding:10px;"></div>');
    			console.log("construct panel");
    			$('#versions_tabs').tabs('add',{
    				id: type,
    				title: data['title'],
    				content:data['body'],
    				closable:true,
    			});
    			console.log(data['title']);
    		},
    		error: function(xhRequest, ErrorText, thrownError) {
    			console.log(ErrorText + ":" + thrownError);
    		},
    	});
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
  });
})(jQuery);