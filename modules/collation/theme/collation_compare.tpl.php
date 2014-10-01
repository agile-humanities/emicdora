<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all.css">
<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all-gray.css">
<?php
$full_window = t("Full Window");
print '<div id ="emicdora_collation_container">';
print '<div id="metadata"></div>
    <div id="uiplaceholder"></div>';
if (user_access(EMICDORA_EDIT_COLLATION) && $variables['display_type'] == 'compare') {
  print '
    <h3 id="top-label" class="ui-widget-header">Version 1</h3>
    <div id="diff_l" class="ui-widget-content collation_resize emcidora_input">
    </div>
    <h3 id="bottom-label" class="ui-widget-header">Version 2</h3>
    <div id="diff_r" class="ui-widget-content collation_resize emcidora_input">
    </div>
  <button class = "emicdora_button" id="collation_link">Merge</button>
  <button class = "emicdora_button" id="collation_unlink" class="form-submit" type="submit" value="Variant">Variant</button><br/>

  <button class = "emicdora_button" id="save_changes">Save Changes</button><br />
    <div id="collatex_iframe">
    <input id="full-window-button" class="emicdora_button" type="button" value="' . $full_window . '" />
      <p>To use CollateX simply copy and paste desired text from windows above into each of the Witness boxes below.<br />
      Press \'Collate\' to generate results.<br />
      Press \'Add\' to open a fresh witness box to collate additional material.
      </p>
    <iframe id ="emicdora_collatex_iframe" src ="/collatex/" ></iframe>
  </div>

  ';
}
print '</div>';
?>

