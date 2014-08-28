<?php
/**
 * @file
 * islandora-basic-collection.tpl.php
 */
?>
<div class="emicdora-coop">
  <div style="float:right;">
    <div class="add-to-coop" style="float:left;">
      <label for="add-to-coop">Add to CO-OP</label>
      <select id="add-to-coop" onchange="this.options[this.selectedIndex].value && (window.location = this.options[this.selectedIndex].value);">
        <option value="none">Select...</option>
        <?php foreach ($add_source_links as $item): ?>
          <option value="<?php print $item['href']; ?>"><?php print $item['title']; ?></option>
        <?php endforeach; ?>
      </select>
    </div>
    <?php print theme_image(array(
      'path' => drupal_get_path('module', 'emicdora') . '/images/coop.png',
      'attributes' => array('style' => 'float:right;'),
    )); ?>
  </div>
  <p>Welcome to the COOP. This is where all the source material for the Critical </br> Editions resides. We have grouped the content by various categories.</p>
  <a href='islandora/search/NOT RELS_EXT_hasModel_uri_mt:(criticalEditionContainerCModel OR versionCModel)'>Browse All</a>
  <div class="clearfix"></div>
</div>
