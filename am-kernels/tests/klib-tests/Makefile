.PHONY: all run gdb clean latest $(ALL)

RESULT = .result
$(shell > $(RESULT))
AM_HOME := $(shell echo "/home/liuyang/Downloads/ics2022/abstract-machine")

COLOR_RED   = \033[1;31m
COLOR_GREEN = \033[1;32m
COLOR_NONE  = \033[0m

ALL = $(basename $(notdir $(shell find src/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))
	@echo am-kernels/tests/klib-tests/Makefile:14 "" $(ALL)

$(ALL): %: Makefile.%

Makefile.%: src/%.c latest
	@/bin/echo -e "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude /home/liuyang/Downloads/ics2022/abstract-machine/Makefile" > $@
	@if make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS); then \
		printf "[%14s] $(COLOR_GREEN)PASS!$(COLOR_NONE)\n" $* >> $(RESULT); \
	else \
		printf "[%14s] $(COLOR_RED)FAIL!$(COLOR_NONE)\n" $* >> $(RESULT); \
	fi
	-@rm -f Makefile.$*

run: all
	@cat $(RESULT)
	@rm $(RESULT)

gdb: all

clean:
	rm -rf Makefile.* build/

latest:
