SUBDIRS := asm  bios  c_tests  echo  mmult  riscv-isa-tests  uart_parse  small  bdd

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIRS)

.PHONY: all $(SUBDIRS) clean
