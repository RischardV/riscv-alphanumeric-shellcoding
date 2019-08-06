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

	$args = getopt('i:o:p:v:');

	$p = file_get_contents($args['i']); /* Input file */
	$pool_size = intval($args['p']); /* Pool size (in bytes) */
	$payload_offset = intval($args['v'], 0); /* Payload offset (in bytes) */
	$maxpayload = 0x400; /* It is possible to increase it with minor work */

	$payload_size = strlen($p);
	if($payload_size > $maxpayload) {
		printf("Payload too large: %d/%d\n", strlen($p), $maxpayload);
		exit(-1);
	}

	$s = ".fill ". $payload_offset .", 1, 0x42\n";
	printf("insert: %d bytes from offset:0x%x\n", 2*$payload_size, $payload_offset);
	for($i = 0 ; $i<min($maxpayload, $payload_size); $i++)
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

		$s .= sprintf(".short 0x%02x%02x\n", $high, $low);
	}

	$pool_size -= $payload_offset + 2*$payload_size;
	if($pool_size > 0) {
		$s .= ".fill ". $pool_size .", 1, 0x42\n";
	}
	file_put_contents($args['o'], $s);
?>
