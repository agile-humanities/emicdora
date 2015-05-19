<?php
/**
 * @file
 * Display the Collation Compare View.
 */
?>
<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all.css">
<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all-gray.css">
<div id ="emicdora_collation_container">
  <div id="metadata"></div>
  <div id="uiplaceholder"></div>
  <?php if (user_access(COLLATION_EDIT_COLLATION) && $variables['display_type'] == 'compare'): ?>
    <h3 id="merge_label" class="ui-widget-header field_label">Associate text</h3>
    <div id="merge_container" class="emicdora_edit_container">
      <h3 id="top-label" class="ui-widget-header">Version 1</h3>
      <div id="diff_l" class="ui-widget-content collation_resize emcidora_input" ></div>
      <h3 id="bottom-label" class="ui-widget-header">Version 2</h3>
      <div id="diff_r" class="ui-widget-content collation_resize emcidora_input" ></div>
      <button class = "emicdora_button" id="collation_link">Merge</button>
      <button class = "emicdora_button" id="collation_variant" class="form-submit" type="submit" value="Variant">Variant</button>
      <br/>
    </div>
    <h3 id="unmerge_label" class="ui-widget-header field_label">Disassociate text</h3>
    <div id="unmerge_container" class="emicdora_edit_container">
      <h3 id="merged" class="ui-widget-header">Merged text</h3>
      <div id="merged_text" class="ui-widget-content merged_resize emcidora_input"></div>
      <button class = "emicdora_button" id="collation_unlink">Unmerge</button><br/>
    </div>
    <button class = "emicdora_button" id="save_changes">Save Changes</button>
  <?php endif; ?>
  <br/>
  <hr />
  <div id="collatex_iframe">
    <div id="collatex_toolbar_header">
      <input id="collatex_use_button" title="To use CollateX (http://collatex.net/), copy text from viewer panes above and paste into the witness boxes below. Click 'Add' to open a new witness box to collate additional material. Click 'Collate' to generate results." class="emicdora_button use-collate-x-btn" type="submit" value="<?php print t("Use CollateX"); ?>" />
      <input id="full-window-button" class="emicdora_button collate-x-full-window fullscreenIcon" type="button" value="Full Window">
    </div>
    <div id ="emicdora_collatex_info_window" title = 'Copyright © 2010-2013 The Interedition Development Group. All rights reserved.'>
    </div>
    <iframe id ="emicdora_collatex_iframe" src ="/collatex/" ></iframe>
  </div>
</div>
