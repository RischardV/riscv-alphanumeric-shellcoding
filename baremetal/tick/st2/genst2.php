#!/usr/bin/php
<?php
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

$reg_list_C  = ["s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5"];
$reg_list_XS  = ["ra", "sp", "gp", "tp", "t0", "t1", "t2",
                 "a6", "a7", "s2", "s3", "s4", "s5", "s6",
                 "s7", "s8", "s9", "s10", "s11",
                 "t3", "t4", "t5", "t6"
                ];

function pc_next_permutation($p, $size) {
    for ($i = $size - 1; @$p[$i] >= $p[$i+1]; --$i) { }
    if ($i == -1) { return false; }
    for ($j = $size; $p[$j] <= $p[$i]; --$j) { }
    $tmp = $p[$i]; $p[$i] = $p[$j]; $p[$j] = $tmp;
    for (++$i, $j = $size; $i < $j; ++$i, --$j) {
         $tmp = $p[$i]; $p[$i] = $p[$j]; $p[$j] = $tmp;
    }
    return $p;
}

function add_with_reg($nb, $XA, $XB, $XP, $XQ, $XS, $XJ) {
    $s = sprintf("#define XA %s\n".
                 "#define XB %s\n".
                 "#define XP %s\n".
                 "#define XQ %s\n".
                 "#define XS %s\n".
                 "#define XJ %s\n".
                 "#include \"../st2_core.S\"\n",
                 $XA, $XB, $XP, $XQ, $XS, $XJ);

    file_put_contents("build/".$nb.".S", $s);
}

@mkdir("build");
@mkdir("build/x");

$perm_size = count($reg_list_C) - 1;
$perm = range(0, $perm_size);
$j = 0;
do {
    if(($perm[5] > $perm[6]) || ($perm[6] > $perm[7])) {
        continue; /* we only use up to 4 */
    }
    foreach($reg_list_XS as $XS) {
        add_with_reg($j,
                     $reg_list_C[$perm[0]], $reg_list_C[$perm[1]], $reg_list_C[$perm[2]],
                     $reg_list_C[$perm[3]], $XS, $reg_list_C[$perm[4]]);
        ++$j;
    }
} while ($perm = pc_next_permutation($perm, $perm_size));
printf("Done:%d.\n", $j);

$mk = '
RV_ABI    := rv64gc
include ../../scripts/common.mk

all: try

# $1: number
define build_block =
build/$1.x: build/$1.S
	@tmpfile=`mktemp build/tmp.XXXX` && \
	 (riscv64-unknown-elf-gcc -P -E $$^ | riscv64-unknown-elf-as -march=$(RV_ABI) - -o $$$$tmpfile) && \
	 riscv64-unknown-elf-objcopy -O binary $$$$tmpfile $$@ && \
	 rm $$$$tmpfile

all: build/$1.x
endef

#NB := $(shell find build/ -name "*.S" | wc -l)
#$(eval NB=$(shell echo $$(($(NB)-1))))
#$(foreach I, $(shell echo  {0..$(NB)}), $(eval $(call build_block,$(I))))
';
$mk2 = 'all: ';
for($i=0; $i<$j; $i++) {
    $mk .=
"build/".$i.".x: build/".$i.'.S
	@tmpfile=`mktemp build/tmp.XXXX` && \
	 (riscv64-unknown-elf-gcc -P -E $^ | riscv64-unknown-elf-as -march=$(RV_ABI) - -o $$tmpfile) && \
	 riscv64-unknown-elf-objcopy -O binary $$tmpfile $@ && \
	 rm $$tmpfile
';
    $mk2 .= 'build/'.$i.'.x ';
}
$mk .= $mk2 ."
try: try.cpp
	g++ -O3 -std=gnu++11 -march=native -ffp-contract=on -Wall -Wextra -Wconversion $< -o $@
";
file_put_contents("Makefile", $mk);

?>