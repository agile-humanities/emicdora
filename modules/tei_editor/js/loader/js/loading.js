$(window).load(function() {
  $("#iframe_replacement").css({ opacity: 1 });
    $('#loading').hide();
  console.log(window.document);
});

$(document).ready(function() {
  var img_loader = Drupal.settings.islandora_markup_editor.image_loader;
  var loading = 
  '<div id="loading">' +
    '<img id="loading-image" src="' + img_loader + '" alt="Loading..." />' +
    '<h1 id="loading_text"></h1>' +
  '</div>';
  $("#islandora-crited-wrapper-id").append(loading);
  $("#iframe_replacement").css({ opacity: 0.0 });
});

function update_loading_text(new_text) {
  $('#loading_text').text(new_text);
}