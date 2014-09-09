<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all.css">
<link rel="stylesheet" href="/sites/all/libraries/ext-4.1.1a/resources/css/ext-all-gray.css">
<?php
print '<div id ="emicdora_collation_container">';
print '<div id="metadata"></div>
    <div id="uiplaceholder"></div>';
if (user_access(COLLATION_EDIT_COLLATION)) {
  print '
  <input id="diff_l"  type="text" style="width: 40%; -moz-user-select: text;" readonly="readonly" class="emicdora_input">
  <input id="diff_r"  type="text" style="width: 40%; -moz-user-select: text;" readonly="readonly" class="emicdora_input">
  <button class = "emicdora_button" id="collation_link">Merge Text</button>
  <br />
  <input id="merged_text"  type="text" style="width: 80%; -moz-user-select: text;" readonly="readonly" class="emicdora_input">
  <button class = "emicdora_button" id="collation_unlink" class="form-submit" type="submit" value="Unlink">Unlink</button><br/>
  <button class = "emicdora_button" id="save_changes">Save Changes</button><br />
  <p id ="emicdora_status"></p>
    <div id="collatex_iframe">
    <iframe id ="emicdora_collatex_iframe" src ="/collatex/" ></iframe>
  </div>
  ';
}
print '</div>';
?>




