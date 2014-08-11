<?php
/**
 * @file
 * islandora-basic-collection.tpl.php
 */
?>

<div>
	<div class="coop_desc_feat" style="width: 100%;">
		<div class="coop_description" style="width: 49%;float: left;">
			<p>Welcome to the COOP. This is where all the source material for the Critical </br> Editions resides. We have grouped the content by various categories.</p>
			<div class="coop_browse_all">
			<a href="<?php print $source_collection; ?>">Browse All</a>
		</div>
		</div>
		<div class="featured-links" style="width: 49%;float: left;">
			<div class="coop_add_to" style="padding-bottom:2em;position: absolute;">
				<div class="coop_select_add">
					<label for="coop_add_content">Add to CO-OP</label>
					<select id="coop_add_content" onchange="this.options[this.selectedIndex].value && (window.location = this.options[this.selectedIndex].value);">
						<option value="none">Select...</option>
						<?php foreach ($add_list as $item): ?>
							<option value="<?php print $item['href']; ?>"><?php print $item['title']; ?></option>
						<?php endforeach; ?>
					</select>
				</div>
			</div>
			<div class="coop_icon_ul" style="float: right;">
				<ul>
					<li class="coop">
						<a href="#"></a>
					</li>
				</ul>
			</div>
		</div>
	</div>
</div>
<div style="width: 100%; height:700px;border-style: solid;border-color: #0000ff;float: left;display: inline-block;vertical-align: top"></div>
