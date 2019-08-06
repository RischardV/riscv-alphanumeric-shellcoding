#
# (c) 2018-2019 Hadrien Barral
# SPDX-License-Identifier: Apache-2.0
#

# $1: payload name
# $2: stack address
# $3: out file
define build_payload =
payload_BASE:=$(shell dirname $(lastword $(MAKEFILE_LIST)))
payload_PAYD := $1
payload_BUILD := $(BUILD)/$$(payload_PAYD)
payload_ABI := rv64ic
payload_GCCFLAGS := -march=$$(payload_ABI) -mabi=lp64 -Os -Wall -Wextra -Werror -pipe -nostdlib -nostartfiles -ffreestanding -fPIC -std=c11
payload_ASFLAGS := -march=$$(payload_ABI) --warn

$$(payload_BUILD):
	$(Q)mkdir -p $$@

$$(payload_BUILD)/$$(payload_PAYD).c.o: $$(payload_BASE)/$$(payload_PAYD).c | $$(payload_BUILD)
	$(Q)$(GCC) $$(payload_GCCFLAGS) -o $$@ -c $$<

$$(payload_BUILD)/$$(payload_PAYD).S.i: $$(payload_BASE)/$$(payload_PAYD).S | $$(payload_BUILD)
	$(Q)$(GCC) -DPAYLOAD_STACK=$2 -o $$@ -P -E $$<

$$(payload_BUILD)/$$(payload_PAYD).S.o: $$(payload_BUILD)/$$(payload_PAYD).S.i
	$(Q)$(AS)  $$(payload_ASFLAGS) -o $$@ $$<

$$(payload_BUILD)/$$(payload_PAYD).elf: $$(payload_BASE)/$$(payload_PAYD).ld $$(payload_BUILD)/$$(payload_PAYD).c.o $$(payload_BUILD)/$$(payload_PAYD).S.o
	$(Q)$(LD)  $(LDFLAGS) -T $$< -o $$@ $(filter-out $$<,$$^)

$$(payload_BUILD)/$$(payload_PAYD).bin: $$(payload_BUILD)/$$(payload_PAYD).elf
	$(Q)$(OBJCOPY) -O binary $$< $$@

$3: $$(payload_BUILD)/$$(payload_PAYD).bin
	$(Q)cp $$< $$@
endef