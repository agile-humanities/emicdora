<?php
/**
 * @file
 * islandora-basic-collection.tpl.php
 */
$transcription = $variables['transcription'];
?>

<?php if (!$variables['multiple']): ?>
  <div class="islandora_transcription_object">
    <?php
    print $transcription;
    ?>
  </div>
<?php else:; ?>
  <div id="tabs">
    <ul>
      <?php foreach ($objects as $flat_pid => $object): ?>
        <li><a href="#<?php print $flat_pid; ?>"><?php print $object->label; ?></a></li>
      <?php endforeach; ?>
    </ul>
    <?php foreach ($objects as $flat_pid => $object): ?>
      <div id="<?php print $flat_pid; ?>">
        <p><?php print str_replace("\n", "<br />", $object['TRANSCRIPTION']->content); ?></p>
      </div>
    <?php endforeach; ?>
  </div>
<?php endif; ?>
