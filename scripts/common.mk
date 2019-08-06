#
# (c) 2018-2019 Hadrien Barral
# SPDX-License-Identifier: Apache-2.0
#

MAKEFLAGS += --no-builtin-rules
SHELL     := /bin/bash -o pipefail
Q         := 
PREFIX    := riscv64-unknown-elf-
AS        := $(PREFIX)as
GCC       := $(PREFIX)gcc
LD        := $(PREFIX)ld
OBJCOPY   := $(PREFIX)objcopy
OBJDUMP   := $(PREFIX)objdump
ASFLAGS   := -march=$(RV_ABI)
LDFLAGS   := -nostdlib -nostartfiles -static
BUILD     := build
