<?php
/**
 * @file
 * This is the template file for the Critical Edition detail viewer
 * viewer.<pre><?php print $version_data[0]['text']; ?></pre>
 */
?>
<div id="eui_window" class="easyui-layout" style="width:100%;height:550px;" data-options="fit:true">
        <div id="easy-ui-north" data-options="region:'north',onBeforeExpand:function(){return false}" style="height:50px;border-style:solid #191972">
           <div id="critical-edition-viewer">
                <div style="float:left;">
                  <ul class="workbench_toolbar_lists action_img">
                    <li id="wb_reading" title="Reading Transcription" class="work_action_img transcription img_selected"></li>
  					<li id="wb_dt" title="Diplomatic Transcriptions" class="work_action_img diplomatic-transcriptions"></li>
  					<li id="wb_image" title="Image" class="work_action_img image"></li>
  					<li id="wb_tei_markup" title="TEI Markup" class="work_action_img tei-markup"></li>
  					<li id="wb_meta" title="Detail Metadata" class="work_action_img detail-meta data_anchor"></li>
  					<li id="wb_show_annos" title="Show/Hide annotations" data-value="0" class="work_action_img anno-entity-switch switch"></li>
  					<li id="wb_show_til" title="Show/Hide Text Image Links" data-value="0" class="work_action_img til-switch switch"></li>
  					<li id="wb_max_min" title="Maximize Viewer" data-value="0" class="work_action_img win-switch-full switch BRicon full"></li>
  				  </ul>
                </div>
                  <div id="ui-easy-paginator" class="easyui-pagination"
                    data-options="
                      total: '<?php print sizeof($pager_data); ?>',
                      pageSize:1,
                      showPageList: false,
                      showRefresh: false,
                      displayMsg: '',"
                    style="float:right;" data-pid="<?php print $islandora_object->id; ?>">
                  </div>
          </div>
        </div>
        <div id="easy-ui-south" data-options="region:'south',split:true,collapsible:false,minimized:true,border:true" style="height:678px;">
          <div id="uimeta-wrapper" class="easyui-panel" style="width:auto;height:auto;border-style:solid #191972" data-options="fit:true,split:true,border:true,href:'<?php print $meta_source; ?>?pid=<?php print $islandora_object->id; ?>'">
          </div>
        </div>
        <div id="easy-ui-east" class="easyui-panel" data-options="region:'east',split:true,border:true,collapsible:false"  style="width:200px;border-style:solid #191972">
          <?php print $islandora_content; ?>
        </div>
        <div id="easy-ui-west" data-options="region:'west',split:true,border:true,maximizable:false,closable:false,minimized:<?php print $hide_components['w']; ?>,border:true,collapsible:false" style="width:150px;border-style:solid #191972">
          <div id="uitree-wrapper" class="easyui-panel" data-options="fit:true" style="width:auto;height:auto;border-style:solid #191972">
              <ul id="easyui_tree" class="easyui-tree" data-options="animate:true,lines:true,checkbox:true,fit:true">
              </ul>
          </div>
        </div>
        <div id="center_data" class="easyui-panel" data-options="region:'center'" style="width:100%;height:auto;border-style:solid #191972">
              <div id="content_data" class="easyui-panel" style="width:100%;height:auto;background:#fafafa;border-style:solid #191972" data-options="fit:true">
		     </div>
      </div>
</div>
<div id="ui-tool-tips" style="display:none">
  <div id="ui_p" class="easyui-panel" style="width:100px;height:75px;padding:10px;"></div>
</div>