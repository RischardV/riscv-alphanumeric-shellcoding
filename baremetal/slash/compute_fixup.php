#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

$args = getopt('i:o:');

$f = file_get_contents($args['i']);

$step6_offset = intval($f, 16);
$xp_init_value = 8; /* Sync with 'jal step1' */
if($step6_offset < $xp_init_value) {
	die("Oops. Please improve the algorithm (1).");
}
$xp_offset = $step6_offset - $xp_init_value;
$addi_cost = 2;
$sp_ops = [112,-112,144,-144,16,-160,176,-176,-192,208,-208,240,-240,
           272,-272,304,-304,320,336,-336,352,368,-368,400,-400,-432,
		   448,464,-464,48,-48,-496,-64,80];

foreach($sp_ops as &$e) {
/* Negative weights are difficult to handle.
   Here, they would not make the optimal better than more than a single instruction.
   (because [1-29] can be made by at most the sum of 2 values)
   Let us drop them. */
	if($e < 0) {
		unset($sp_ops[$e]);
	} else {
		$e -= 2; /* Remove the cost from the instruction itself */
	}
}
sort($sp_ops);
/* We now have 1,3,5,7,9,11,13,15,17,19,20,21,22,23,25,28,29 */

/* We need quite a lot of 464, and we have 448 available.
 * Thus we simply need to compute an upper bound with 464,
 * then switch some of the 464 to 448 to get down at close
 * as possible to the target.
 */

$v464 = 464-$addi_cost;
$v448 = 448-$addi_cost;

if($xp_offset == 0) {
	die("Oops. Please improve the algorithm (2).");
}

$n464 = ceil($xp_offset / $v464); /* quotient */
$r464 = $v464 - ($xp_offset % $v464); /* remainder */
$n448 = floor($r464 / ($v464-$v448));

$n464 -= $n448;
if($n464 < 0) {
	die("Oops. Please improve the algorithm (3).");
}

$r448 = $n464*$v464 + $n448*$v448 - $xp_offset;
if(($r448%2) !== 0) {
	die("Oops. Please improve the algorithm (4).");
}
$end_nopsled = $r448 / 2;

$s = ".macro xp_fixup\n".
	 sprintf("    # n464:%d n448:%d\n", $n464, $n448).
	 str_repeat("    addi XP, XP, 464\n", $n464).
	 str_repeat("    addi XP, XP, 448\n", $n448).
	 ".endm\n".
	 ".macro end_nopsled\n".
	 sprintf("    # len:%d\n", $end_nopsled).
	 str_repeat("    dangerous_nop2\n", $end_nopsled).
	 ".endm\n";
file_put_contents($args['o'], $s);

?>