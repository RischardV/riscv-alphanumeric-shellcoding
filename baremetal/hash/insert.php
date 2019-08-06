#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

	function error($errno, $errstr, $errfile, $errline) {
		throw new ErrorException($errstr, $errno, 0, $errfile, $errline);
	}
	set_error_handler('error');

	$args = getopt('i:p:v:t:o:');

	$s = file_get_contents($args['i']); /* Input file */
	$p = file_get_contents($args['p']); /* Payload */
	$v = file_get_contents($args['v']); /* step6 offset to compute pool offset */
	$v = intval($v, 16);
	$t = file_get_contents($args['t']); /* stage2 sp shift */
	$t = intval($t, 10);
	$b = $v - 0x1000 + (16*$t) - 1920;  /* Sync with stage2 start load address and store offset */
	$maxpayload = 0x400; /* It is possible to increase it with minor work */

	if(strlen($p) > $maxpayload) {
		printf("Payload too large: %d/%d\n", strlen($p), $maxpayload);
		exit(-1);
	}
	printf("insert: %d bytes from offset:0x%x\n", 2*strlen($p), $b);
	for($i=0; $i<min($maxpayload, strlen($p)); $i++)
	{
		/* [Notation: capital letters are hex digits]
		 * We want to encode byte 'XY' in alnum bytes 'KA' 'LB'.
		 * The equation to solve (see stage2.S) is:
		 * XY == KA ^ BK, ('XY'€[0;255], 'KA' and 'LB' alnum]
		 */
		$q = ord($p[$i]);
		$wanted_bottom  = $q & 0xF;
		$wanted_top = ($q >> 4) & 0xF;

		$low_top = 0x4;
		if($wanted_bottom == 0x4) {
			$low_top = 0x6;
		}
		$low_bottom = $wanted_bottom ^ $low_top;

		$high_bottom = $wanted_top ^ $low_top;
		$high_top = ($high_bottom == 0) ? 0x5 : 0x4;

		$low  = ($low_top << 4) + $low_bottom;
		$high = ($high_top << 4) + $high_bottom;

		$s[$b+2*$i  ] = chr($low);
		$s[$b+2*$i+1] = chr($high);

		//printf("INS: %X -> %X %X\n", $q, $low, $high);
	}
	file_put_contents($args['o'], $s);
?>
