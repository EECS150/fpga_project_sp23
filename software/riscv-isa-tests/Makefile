SHELL := $(shell which bash) -o pipefail
TESTS := $(filter-out fence_i.S ma_data.S,$(notdir $(wildcard riscv-tests/isa/rv32ui/*.S)))
TESTS_HEX := $(subst .S,.hex,$(TESTS))
GCC_OPTS := -march=rv32i -mabi=ilp32 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -T env_151/link.ld -Wl,--build-id=none

RISCV := riscv64-unknown-elf

all: riscv-tests/isa $(TESTS_HEX)

riscv-tests/isa:
	git submodule update --init --recursive
	$(MAKE)

%.hex: riscv-tests/isa/rv32ui/%.S
	$(RISCV)-gcc $(GCC_OPTS) -Ienv_151 -Iriscv-tests/env -Iriscv-tests/isa/macros/scalar $^ -o $(basename $(notdir $^)).elf
	$(RISCV)-objdump -D -Mnumeric $(basename $(notdir $^)).elf > $(basename $@).dump
	$(RISCV)-objcopy $(basename $@).elf -O binary $(basename $@).bin
	$(RISCV)-bin2hex -w 32 $(basename $@).bin $(basename $@).hex

clean:
	rm -f *.elf *.bin *.hex *.dump
