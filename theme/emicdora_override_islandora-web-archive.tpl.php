<?php
/**
 * @file
 * This is an override template file for the object page for web archives
 *
 * @changes Removed "In Collections" section.
 */
?>

<div class="islandora-web-archive-object islandora">
  <div class="islandora-web-archive-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-web-archive-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
  </div>
  <div class="islandora-web-archive-metadata">
    <?php print $description; ?>
    <div>
      <h2><?php print t('Download'); ?></h2>
      <ul>
        <?php if (isset($islandora_warc)): ?>
        <li>Warc: <?php print $islandora_warc; ?>
          <?php endif; ?>
          <?php if (isset($islandora_pdf)): ?>
        <li>PDF: <?php print $islandora_pdf; ?>
          <?php endif; ?>
          <?php if (isset($islandora_png)): ?>
        <li>Screenshot: <?php print $islandora_png; ?>
          <?php endif; ?>
          <?php if (isset($islandora_csv)): ?>
        <li>CSV: <?php print $islandora_csv; ?>
          <?php endif; ?>
      </ul>
    </div>
    <?php print $metadata; ?>
  </div>
</div>
