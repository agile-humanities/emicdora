<?php

/**
 * @file
 * This is an override template file for the object page for audio file
 *
 * @changes Removed "In Collections" section.
 */
?>

<div class="islandora-audio-object islandora" vocab="http://schema.org/" prefix="dcterms: http://purl.org/dc/terms/" typeof="AudioObject">
  <div class="islandora-audio-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="islandora-audio-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
  </div>
  <div class="islandora-audio-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
</div>
