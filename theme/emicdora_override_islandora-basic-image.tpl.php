<?php

/**
 * @file
 * This is an override template file for the object page for basic image
 *
 * @changes Removed "In Collections" section.
 */
?>

<div class="islandora-basic-image-object islandora" vocab="http://schema.org/" prefix="dcterms: http://purl.org/dc/terms/" typeof="ImageObject">
  <div class="islandora-basic-image-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-basic-image-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
  </div>
  <div class="islandora-basic-image-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
</div>
