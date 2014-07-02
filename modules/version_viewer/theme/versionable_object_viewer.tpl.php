<?php
/**
 * @file
 * This is the template file for the Critical Edition detail viewer
 * viewer.<pre><?php print $version_data[0]['text']; ?></pre>
 */
?>
<div style="margin:20px 0;"></div>
    <div id="eui_window" class="easyui-layout" style="width:700px;height:550px;">
        <div data-options="region:'north'" style="height:50px">
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
  				  </ul>
                </div>
                  <div id="ui-easy-paginator" class="easyui-pagination"
                    data-options="
                      total: '<?php print sizeof($page_pids); ?>',
                      pageSize:1,
                      showPageList: false,
                      showRefresh: false,
                      displayMsg: '',"
                    style="float:right;" data-pid="<?php print $islandora_object->id; ?>">
                  </div>
			</div>
        </div>
        <div id="easy-ui-south" data-options="region:'south',split:true,collapsible:true,minimized:true" style="height:500px;">
          <div id="uimeta-wrapper" class="easyui-panel" style="width:auto;height:auto;" data-options="fit:true,href:'<?php print $meta_source; ?>?pid=<?php print $islandora_object->id; ?>'">
          </div>
        </div>
        <div id="easy-ui-east" class="easyui-panel" data-options="region:'east',split:true" title="Source" style="width:200px;maxWidth:700px">
          <?php print $islandora_content; ?>
        </div>
        <div data-options="region:'west',split:true,minimized:<?php print $hide_components['w']; ?>" title="Annotations" style="width:150px;height:500px;">
          <div id="uitree-wrapper" class="easyui-panel" style="width:auto;height:auto;">
              <ul id="easyui_tree" class="easyui-tree" data-options="animate:true,lines:true,checkbox:true">
              </ul>
          </div>
        </div>
        <div id="center_data" class="easyui-panel" data-options="region:'center',iconCls:'icon-ok'">
          <div id="versions_tabs" data-options="fit:true" class="easyui-tabs" style="width:auto;height:auto">
          </div>
        </div>
    </div>
    <div style="display:none">
      <div id="toolbar">
        <a href="#" class="easyui-linkbutton easyui-tooltip" title="Add" data-options="iconCls:'icon-add',plain:true"></a>
        <a href="#" class="easyui-linkbutton easyui-tooltip" title="Cut" data-options="iconCls:'icon-cut',plain:true"></a>
        <a href="#" class="easyui-linkbutton easyui-tooltip" title="Remove" data-options="iconCls:'icon-remove',plain:true"></a>
        <a href="#" class="easyui-linkbutton easyui-tooltip" title="Undo" data-options="iconCls:'icon-undo',plain:true"></a>
        <a href="#" class="easyui-linkbutton easyui-tooltip" title="Redo" data-options="iconCls:'icon-redo',plain:true"></a>
      </div>
    </div>
