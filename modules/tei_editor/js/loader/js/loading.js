$(document).ready(function() {
  var img_loader = Drupal.settings.islandora_markup_editor.image_loader;
  var loading = 
  '<div id="loading" style="width:100%;height:100%background-color:white;top:0;left:0;position:absolute;z-index:1999">' +
    '<img id="loading-image" src="' + img_loader + '" alt="Loading..." />' +
    '<h1 id="loading_text"></h1>' +
  '</div>';
  $("#iframe_replacement").append(loading);
  
  $("#cwrc_wrapper").css({ opacity: 0.0 });
});

function update_loading_text(new_text) {
  $('#loading_text').text(new_text);
}

function hide_loading_bar() {
  $("#cwrc_wrapper").css({ opacity: 1 });
  $('#loading').hide();
}