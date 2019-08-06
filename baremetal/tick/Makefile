#
# (c) 2018-2019 Hadrien Barral
# SPDX-License-Identifier: Apache-2.0
#

RV_ABI    := rv64gc
include ../scripts/common.mk
NAME      := shellcode
PAYD      := payload_serial

all: $(NAME).bin

$(BUILD):
	$(Q)mkdir -p $@

$(BUILD)/stage1_nofixup.o: stage1.S | $(BUILD)
	$(Q)m4 -I $(BUILD) $< > $(BUILD)/$<.m4.S
	$(Q)$(GCC) -DCOMPUTE_XP_FIXUP -o $(BUILD)/$<.i -P -E $(BUILD)/$<.m4.S
	$(Q)$(AS) $(ASFLAGS)   -o $@ -c $(BUILD)/$<.i

$(BUILD)/xp_fixup.S: $(BUILD)/stage1_nofixup.o compute_fixup.php
	$(Q)$(OBJDUMP) -x $< | grep step6 | awk '{print $$1}' > $@.step6_value
	$(Q)php $(word 2,$^) -i $@.step6_value -o $@

$(BUILD)/stage1.o: stage1.S $(BUILD)/xp_fixup.S
	$(Q)m4 -I $(BUILD) $< > $(BUILD)/$<.m4.S
	$(Q)$(GCC) -o $(BUILD)/$<.i -P -E $(BUILD)/$<.m4.S
	$(Q)$(AS) $(ASFLAGS)   -o $@ -c $(BUILD)/$<.i

$(BUILD)/stage1.elf: $(BUILD)/stage1.o stage1.ld
	$(Q)$(LD) $(LDFLAGS) -T $(word 2,$^) -o $@ $<

$(BUILD)/stage1.bin: $(BUILD)/stage1.elf
	$(Q)$(OBJCOPY) -O binary $< $@

$(BUILD)/$(NAME).bin.nocheck: $(BUILD)/stage1.bin $(BUILD)/$(PAYD).bin \
                              $(BUILD)/stage1.o insert.php stage1.S
	$(Q)$(OBJDUMP) -x $(word 3,$^) | grep step6 | awk '{print $$1}' > $@.step6_value
	$(Q)grep "^\s*next_block$$" $(word 5,$^) | wc -l > $@.stage2_value
	$(Q)php $(word 4,$^) -i $< -p $(word 2,$^) -v $@.step6_value -t $@.stage2_value -o $@

$(NAME).bin: $(BUILD)/$(NAME).bin.nocheck check.php
	$(Q)php $(word 2,$^) -i $< -c "A-Za-z0-9\'"
	$(Q)cp $< $@

$(BUILD)/stage2.o: stage2.S | $(BUILD)
	$(Q)$(GCC) -o $(BUILD)/$<.i -P -E $<
	$(Q)$(AS) $(ASFLAGS)   -o $@ $(BUILD)/$<.i

$(BUILD)/stage2.bin: $(BUILD)/stage2.o
	$(Q)$(OBJCOPY) -O binary $< $@

$(BUILD)/loadgen: loadgen.cpp
	$(Q)g++ -O3 -std=gnu++17 -march=native -ffp-contract=on -Wall -Wextra -Wconversion $< -o $@

include ../payload/payload.mk
$(eval $(call build_payload,$(PAYD),0x8000800580,$(BUILD)/$(PAYD).bin))

clean:
	$(Q)rm -rf $(BUILD) $(NAME).bin
