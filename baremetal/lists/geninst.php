#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

function acceptable_base($s)
{
	return ctype_alnum($s); /* No store */
}

function acceptable_hash($s)
{
	return(ctype_alnum($s) || ($s === '#'));
}

function acceptable_tick($s)
{
	return(ctype_alnum($s) || ($s === '\''));
}

function acceptable_slash($s)
{
	return(ctype_alnum($s) || ($s === '/'));
}

function instr4($fn, $acceptable)
{
	$f = fopen($fn, 'wb');

	for($aa = 0; $aa<256; $aa++) {
		if(($aa & 0b11) != 0b11) { continue; }
		if(($aa & 0b11100) == 0b11100) { continue; }
		$a = chr($aa);
		if(!$acceptable($a)) { continue; }
		printf("%s", $a);
	for($bb = 0; $bb<256; $bb++) {
		$b = chr($bb);
		if(!$acceptable($b)) { continue; }
	for($cc = 0; $cc<256; $cc++) {
		$c = chr($cc);
		if(!$acceptable($c)) { continue; }
	for($dd = 0; $dd<256; $dd++) {
		$d = chr($dd);
		if(!$acceptable($d)) { continue; }

		$s = $a.$b.$c.$d;
		fwrite($f, $s);
	}}}}

	print("\n");
	fclose($f);
}

function instr2($fn, $acceptable)
{
	$f = fopen($fn, 'wb');

	for($aa = 0; $aa<256; $aa++) {
		$a = chr($aa);
		if(!$acceptable($a)) { continue; }
		printf("%s", $a);
	for($bb = 0; $bb<256; $bb++) {
		$b = chr($bb);
		if(($bb & 0b11) === 0b11) { continue; }
		if(!$acceptable($b)) { continue; }

		$s = $b.$a;
		fwrite($f, $s);
	}}

	printf("\n");
	fclose($f);
}

function instr($name, $ilen, $outfile) {
	$acceptable = "acceptable_".$name;
	if($ilen == 2) {
		instr2($outfile, $acceptable);
	} else if($ilen == 4) {
		instr4($outfile, $acceptable);
	} else {
		die("Invalid ilen.\n");
	}
}

printf("Generating instructions for flavor:'%s' ilen:'%s' to file '%s'\n", $argv[1], $argv[2], $argv[3]);
instr($argv[1], $argv[2], $argv[3]);

?>
