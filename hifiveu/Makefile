#
# (c) 2018-2019 Hadrien Barral
# SPDX-License-Identifier: Apache-2.0
#

RV_ABI    := rv64gc
include ../scripts/common.mk
OUT := out

all: $(BUILD)/vuln.bin \
     $(OUT)/hello_hash.txt  $(OUT)/hello_slash.txt  $(OUT)/hello_tick.txt \
     $(OUT)/shell_hash.txt  $(OUT)/shell_slash.txt  $(OUT)/shell_tick.txt \
     $(OUT)/shadow_hash.txt $(OUT)/shadow_slash.txt $(OUT)/shadow_tick.txt

$(BUILD):
	$(Q)mkdir -p $@

$(OUT):
	$(Q)mkdir -p $@

$(BUILD)/vuln.bin: $(BUILD)/main.o
	$(Q)$(GCC) -Wl,-N -z execstack -o $@.nostrip $^
	@#riscv64-unknown-elf-objcopy --set-section-flags .text=contents,alloc,load,code $< $@
	@#riscv64-unknown-elf-strip $@.nostrip -o $@
	@cp $@.nostrip $@

$(BUILD)/main.o: main.c | $(BUILD)
	$(Q)$(GCC) -o $@ -c $< -Wall -Wextra -O2

include ../payload/payload.mk

# $1: user-friendly name
# $2: check regex
# $3: encode p arg
# $4: encode v arg
# $5: payload
# $6: shellcode-flavour
define build_shellcode =
$(BUILD)/shellcode_$1.o: $6.S $(BUILD)/$1.encoded
	$(Q)$(GCC) $(ASFLAGS) -o $$@ -c $$< -DPAYLOAD_FILE="\"$$(word 2,$$^)\""

$(BUILD)/$1.encoded: $(BUILD)/$5.bin encode.php
	$(Q)php $$(word 2,$$^) -i $$< -o $$@ -p $3 -v $4

$(BUILD)/shellcode_$1.txt: $(BUILD)/shellcode_$1.o
	$(Q)$(OBJCOPY) -O binary $$< $$@

$(OUT)/$1.txt: $(BUILD)/shellcode_$1.txt check.php | $(OUT)
	$(Q)php $$(word 2,$$^) -i $$< -c $2
	$(Q)cp $$< $$@
endef

# $1: payload
# $2: user-friendly name
define build_shellcode_family =
$(eval $(call build_payload,$1,0x40000,$(BUILD)/$1.bin))

$(eval $(call build_shellcode,$2_hash, "A-Za-z0-9#", 10798,0x1414,$1,hash))
$(eval $(call build_shellcode,$2_slash,"A-Za-z0-9\/",10966,0x1E8C,$1,slash))
$(eval $(call build_shellcode,$2_tick, "A-Za-z0-9\'",10606,0x1264,$1,tick))
endef

$(eval $(call build_shellcode_family,payload_linux_hello,hello))
$(eval $(call build_shellcode_family,payload_linux_shell,shell))
$(eval $(call build_shellcode_family,payload_linux_etc_shadow,shadow))

.PHONY: clean
clean:
	rm -rf $(BUILD) $(OUT)
