SUBDIRS := asm  bios  c_tests  echo  benchmark  riscv-isa-tests  uart_parse  small

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIRS)

.PHONY: all $(SUBDIRS) clean
