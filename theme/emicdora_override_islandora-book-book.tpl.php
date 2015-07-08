<?php
/**
 * @file
 * This is an override template file for the object page for islandora books.
 *
 * @changes Removed "In Collections" section.
 */
?>
<?php if(isset($viewer)): ?>
  <div id="book-viewer">
    <?php print $viewer; ?>
  </div>
<?php endif; ?>

<?php if($display_metadata === 1): ?>
  <div class="islandora-book-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
<?php endif; ?>
