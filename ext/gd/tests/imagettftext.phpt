--TEST--
imagettftext() function test
--SKIPIF--
<?php 
	if (!extension_loaded('gd')) {
		die("skip gd extension not avaliable.");
	}
	if (!function_exists("imagettftext")) {
		die("skip imagettftext() not available.");
	}
?>
--FILE--
<?php
	$cwd = dirname(__FILE__);
	$fontfile_8859 = "$cwd/test8859.ttf";

	function testrun($im, $fontfile) {
		$sx = imagesx($im);
		$sy = imagesy($im);

		$colour_w = imagecolorallocate($im, 255, 255, 255);
		$colour_b = imagecolorallocate($im, 0, 0, 0);

		imagefilledrectangle($im, 0, 0, $sx - 1, $sy - 1, $colour_b);
		imagettftext($im, $sy * 0.75, 0, 0, $sy - 1, $colour_w, $fontfile, "A");

		$cnt = 0;
		for ($y = 0; $y < $sy; ++$y) {
			for ($x = 0; $x < $sx; ++$x) {
				if (imagecolorat($im, $x, $y) == $colour_b) {
					++$cnt;
				}
			}
		}

		// imagecolordeallocate($im, $colour_w);
		// imagecolordeallocate($im, $colour_b);

		return ($cnt < ($sx * $sy));
	}

	$im = imagecreate(256, 256);
	var_dump(testrun($im, $fontfile_8859));
	imagedestroy($im);

/* considered to be disabled for now as discussed in the QA list.
	$im = imagecreatetruecolor(256, 256);
	var_dump(testrun($im, $fontfile_8859));
	imagedestroy($im);
*/
?>
--EXPECT--
bool(true)
