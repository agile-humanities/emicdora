<?php

/**
 * @file
 * This is an override template file for the document object
 *
 * @changes Removed "In Collections" section.
 */
?>

<div class="islandora-pdf-object islandora" vocab="http://schema.org/" prefix="dcterms: http://purl.org/dc/terms/" typeof="Article">
  <div class="islandora-pdf-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-pdf-content">
        <?php print $islandora_content; ?>
      </div>
      <?php if (isset($islandora_download_link)): ?>
        <?php print $islandora_download_link; ?>
      <?php endif; ?>
    <?php endif; ?>
  </div>
  <div class="islandora-pdf-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
</div>
