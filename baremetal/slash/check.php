#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

	$args = getopt('i:c:');

	$f = file_get_contents($args['i']);

	$regex = "/[".$args['c']."]/";

	/* For debug */
	function not_alnum_verbose(&$f, $regex) {
		$len = strlen($f);
		for ($i=0; $i<$len; $i++) {
			$c = $f[$i];
			if (preg_replace($regex, '', $c) !== '') {
				printf("Not alnum (0x%x)!\n", $i);
				return;
			}
		}
	}

	if (preg_replace($regex, '', $f) !== '') {
		not_alnum_verbose($f, $regex);
		//exit(0); /* For dev */
		exit(1);
	}
?>