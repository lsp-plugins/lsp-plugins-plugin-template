<?php
	plugin_header();
	$m      =   ($PAGE == 'plugin_template_mono') ? 'm' : 's';
?>

<p>
	This is a template plugin (<?= ($m == 'm') ? 'mono' : 'stereo' ?> version);
</p>

<p><b>Controls:</b></p>
<ul>
	<li>
		<b>Bypass</b> - bypass switch, when turned on (led indicator is shining), the output signal is similar to input signal. That does not mean
		that the plugin is not working.
	</li>
	<li><b>Samples</b> - sets the delay in samples.</li>
	<li><b>Dry amount</b> - the amount of the unprocessed (dry) signal in the output signal<?= $cc ?>.</li>
	<li><b>Wet amount</b> - the amount of the processed (wet) signal in the output signal<?= $cc ?>.</li>
    <li><b>Output</b> - the loudness of the processed output signal<?= $cc ?>.</li>
    <li><b>Delay (ms)</b> - indicator that displays delay in milliseconds.</li>
</ul>
