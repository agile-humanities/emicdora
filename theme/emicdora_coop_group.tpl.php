<?php
/**
 * @file
 * emicdora_coop_group.tpl.php
 */

?>

<ul class="ul_list_group" style="list-style:none outside none">
  <?php foreach ($variables['group_list'] as $list_entry): ?>
  <li>
    <div >
      <label for="list_item_title"><?php print $list_entry['label']; ?></label>
      <div id="list_item_title" class="list_group coop_list_item">
        <?php foreach ($list_entry['data'] as $item): ?>
          <div class="coop_list_item_div"></div>
        <?php endforeach; ?>
      </div>
      <div style="float: right;">
        <a style="right:0px;bottom:0px;" href="http://www.w3schools.com/">View more..</a>
      </div>
    </div>
  </li>
  <?php endforeach; ?>
</ul>
