<?php
/**
 * @file
 * This is the template file for the Critical Edition detail viewer
 * viewer.
 */
?>
<div>
  <div id="critical-edition-viewer">
  	<ul class="workbench_toolbar_lists action_img">
  		<li title="Diplomatic Transcriptions" class="work_action_img diplomatic-transcriptions img_selected"></li>
  		<li title="Transcription" class="work_action_img transcription"></li>
  		<li title="Image" class="work_action_img image"></li>
  		<li title="TEI Markup" class="work_action_img tei-markup"></li>
  		<li title="Detail Metadata" class="work_action_img detail-meta data_anchor"></li>
  		<li title="Show/Hide annotations" data-value="0" class="work_action_img anno-entity-switch switch"></li>
  		<li title="Show/Hide Text Image Links" data-value="0" class="work_action_img til-switch switch"></li>

  		<div id="jqpagination" class="pagination img_pager">
  		    <a href="#" class="first" data-action="first">&laquo;</a>
  		    <a href="#" class="previous" data-action="previous">&lsaquo;</a>
  		    <input id="jqpagination_input" type="text" readonly="readonly" data-max-page="0" />
  		    <a href="#" class="next" data-action="next">&rsaquo;</a>
  		    <a href="#" class="last" data-action="last">&raquo;</a>
  	    </div>
  	</ul>
  </div>
  <!-- id prev AudioLayout -->
  <div id="ContentLayout" style="width:100%;height:500px;">
    <div id="view_box" style="width:100%;height:100%;overflow:auto;position: relative;">
      <div class="ui-layout-center">Center</div>
      <div class="ui-layout-north">North</div>
      <div class="ui-layout-south">South</div>
      <div class="ui-layout-east">East</div>
      <div class="ui-layout-west">West</div>
    </div>
  <div id="append_data"></div>
  </div>
</div>