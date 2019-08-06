divert(-1)
/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */
changequote({,})
define({LQ},{changequote(`,'){dnl}
changequote({,})})
define({RQ},{changequote(`,')dnl{
}changequote({,})})
changecom({;})

define({repeat}, {ifelse($1, 0, {}, $1, 1, {$2}, {$2
        repeat(eval($1-1), {$2})})})
divert(0)dnl