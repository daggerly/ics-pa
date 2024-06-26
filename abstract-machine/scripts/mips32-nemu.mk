AM_HOME := $(shell echo "/home/liuyang/Downloads/ics2022/abstract-machine")
include $(AM_HOME)/scripts/isa/mips32.mk
include $(AM_HOME)/scripts/platform/nemu.mk
CFLAGS  += -DISA_H=\"mips/mips32.h\"

AM_SRCS += mips/nemu/start.S \
           mips/nemu/cte.c \
           mips/nemu/trap.S \
           mips/nemu/vme.c
