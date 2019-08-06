#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

$args = getopt('i:o:l:');

$t = json_decode(file_get_contents($args['l']), true);

if(gettype($t) !== 'array') {
	die("loadgen.json load failed\n");
}

$f = file_get_contents($args['i']);

$s = '';

$l = strlen($f);

$store_offset = 1920;

for($i=0; $i<$l; $i+=2) {
	if($i+1>=$l) {
		die("Invalid binary size.");
	}

	$value = ord($f[$i]) + ((ord($f[$i+1])) << 8);
	if(isset($t[$value])) {
		$s .= sprintf("# Loading 0x%x in x%d\n%s\n", $value, $t[$value]['dst'], $t[$value]['txt']);

		$dst = $t[$value]['dst'];
		switch($dst) {
			case 20:
				$dst = "s4";
				break;
			case 6:
				$dst = "t1";
				break;
			default:
				printf("Unknown store register '".$dst."'\n");
				exit(1);
		}

		$s .= sprintf("sd %s, %d(XP)\n", $dst, $store_offset);
	} else {
		printf("Nomatch [i:0x%x]: 0x%x\n", $i, $value);
		exit(1);
	}

	$store_offset += 2;
	if($store_offset > 1938) {
		$s .= "next_block\n";
		$store_offset -= 16;
	}
}

file_put_contents($args['o'], $s);
?>