<?php
/**
 * @file
 * This is the template file for the Critical Edition detail viewer
 * viewer.
 */
?>
 <div style="margin:20px 0;"></div>
    <div class="easyui-layout" style="width:700px;height:550px;">
        <div data-options="region:'north'" style="height:50px">
            <div id="critical-edition-viewer">
                <div style="float:left;">
                  <ul class="workbench_toolbar_lists action_img">
  					<li title="Diplomatic Transcriptions" class="work_action_img diplomatic-transcriptions img_selected"></li>
  					<li title="Transcription" class="work_action_img transcription"></li>
  					<li title="Image" class="work_action_img image"></li>
  					<li title="TEI Markup" class="work_action_img tei-markup"></li>
  					<li title="Detail Metadata" class="work_action_img detail-meta data_anchor"></li>
  					<li title="Show/Hide annotations" data-value="0" class="work_action_img anno-entity-switch switch"></li>
  					<li title="Show/Hide Text Image Links" data-value="0" class="work_action_img til-switch switch"></li>
  				  </ul>
                </div>
                  <div class="easyui-pagination"
                    data-options="
                      total: '<?php print sizeof($version_data); ?>',
                      pageSize:1,
                      showPageList: false,
                      showRefresh: false,
                      displayMsg: '',
                      onSelectPage: function(pageNumber, pageSize){
                          $('#center_data').empty();
                          console.log('Now that is cool');
                      }"
                    style="float:right;">
                  </div>

			</div>
        </div>
        <!-- div data-options="region:'south',split:true" style="height:50px;"></div-->
        <div data-options="region:'east',split:true" title="Source" style="width:200px;">
          <?php print $anno_img_pane; ?>
        </div>
        <div data-options="region:'west',split:true" title="Annotations" style="width:150px;height:500px;">
          <div id="uitree-wrapper" class="easyui-panel" style="width:auto;height:auto;">
              <ul class="easyui-tree" data-options="animate:true,lines:true,checkbox:true">
                  <li>
                      <span>My Documents</span>
                      <ul>
                          <li data-options="state:'closed'">
                              <span>Photos</span>
                              <ul>
                                  <li>
                                      <span>Friend</span>
                                  </li>
                                  <li>
                                      <span>Wife</span>
                                  </li>
                                  <li>
                                      <span>Company</span>
                                  </li>
                              </ul>
                          </li>
                          <li>
                              <span>Program Files</span>
                              <ul>
                                  <li>Intel</li>
                                  <li>Java</li>
                                  <li>Microsoft Office</li>
                                  <li>Games</li>
                              </ul>
                          </li>
                          <li>index.html</li>
                          <li>about.html</li>
                          <li>welcome.html</li>
                      </ul>
                  </li>
              </ul>
          </div>
        </div>
        <div id="center_data" data-options="region:'center',title:'<?php print $version_data[0]['title']; ?>',iconCls:'icon-ok'">
          <pre><?php print $version_data[0]['text']; ?></pre>
        </div>
    </div>