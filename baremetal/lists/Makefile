#
# (c) 2018-2019 Hadrien Barral
# SPDX-License-Identifier: Apache-2.0
#

MAKEFLAGS += --no-builtin-rules
SHELL    := /bin/bash -o pipefail
PREFIX   := riscv64-unknown-elf-
OBJDUMP  := $(PREFIX)objdump
BUILD    := build
Q        :=


FLAVOURS := base hash tick slash
ILEN     := 2 4
ISA := 32 64

all: $(FLAVOURS)

$(BUILD):
	$(Q)mkdir -p $@

# $1:flavour $2:isa $3:ilen
define template3 =
$(BUILD)/$(1)_$(2)_$(3).diss: $(BUILD)/$(1)_$(3).raw
	$(Q)$(OBJDUMP) -D -b binary -m riscv:rv$(2) $$< > $$@

$(BUILD)/$(1)_$(2)_$(3).txt: $(BUILD)/$(1)_$(2)_$(3).diss
	$(Q)tail --lines=+8 $$< | awk '{print substr($$$$0, index($$$$0, $$$$3))}' | grep -v "^0x" > $$@.tmp1
	$(Q)sort -n $$@.tmp1 > $$@.tmp2
	$(Q)rm $$@.tmp1
	$(Q)mv $$@.tmp2 $$@
endef

# $1:flavour $2:isa
define template2 =
$(1): $(1)_$(2).txt
$(foreach ilen,$(ILEN),$(eval $(call template3,$(1),$(2),$(ilen))))

$(BUILD)/$(1)_$(2).txt: $(BUILD)/$(1)_$(2)_2.txt $(BUILD)/$(1)_$(2)_4.txt
	$(Q)cp $(BUILD)/$(1)_$(2)_2.txt $$@.tmp
	$(Q)echo "================" >> $$@.tmp
	$(Q)cat $(BUILD)/$(1)_$(2)_4.txt >> $$@.tmp
	$(Q)mv $$@.tmp $$@

$(1)_$(2).txt: $(BUILD)/$(1)_$(2).txt
	$(Q)cp $$< $$@
endef

# $1:flavour $2:ilen
define template_geninst =
$(BUILD)/$(1)_$(2).raw: geninst.php | $(BUILD)
	$(Q)php $$< $(1) $(2) $$@.tmp
	$(Q)mv $$@.tmp $$@
endef

# $1:flavour
define template1 =
.PHONY: $(1)
$(foreach isa,$(ISA),$(eval $(call template2,$(1),$(isa))))
$(foreach ilen,$(ILEN),$(eval $(call template_geninst,$(1),$(ilen))))
endef

$(foreach flavour,$(FLAVOURS),$(eval $(call template1,$(flavour))))

clean:
	$(Q)rm -rf build *.txt