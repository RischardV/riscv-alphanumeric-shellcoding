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

$fenci_str = "
lui     a0,0x412f3
li      s4,18
sra     t1,a0,s4
addiw   t1,t1,-20
addiw   t1,t1,-20
addiw   t1,t1,-20
amoor.w.aq t5,t1,(sp)
";

for($i=0; $i<$l; $i+=16) {
	if($i+15>=$l) {
		die("Invalid binary size. Missing filler?");
	}
	$value = ord($f[$i]) + ((ord($f[$i+1])) << 8);
	$next = ord($f[$i+2]) + ((ord($f[$i+3])) << 8);
	if($value == 0xFFFF) {
		die("Found 0xFFFF. Missing filler?\n");
	}
	$s .= "init_block\n";
	if(($value === 0x100F) && ($next === 0x0000)) {
		$s .= sprintf("# Loading fenci\n", $fenci_str);
		$s .= $fenci_str;
	} else if(isset($t[$value])) {
		$s .= sprintf("# Loading 0x%x\n%s\n", $value, $t[$value]['txt']);
	} else {
		printf("Nomatch [i:0x%x]: 0x%x\n", $i, $value);
		exit(1);
	}
	$s .= "next_block\n";
}

file_put_contents($args['o'], $s);
?>