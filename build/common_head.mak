#-------------------------------------------------------------------------------
# file Makefile
# author Sebastien Alaiwan
# date 2014-04-22
#-------------------------------------------------------------------------------

TARGETS:=
DEPS:=

BIN?=bin

all: true_all

define addTarget
  SRCS:=$2
  OBJS:=$$(SRCS:%.d=$$(BIN)/%_d.o)
  OBJS:=$$(OBJS:%.cpp=$$(BIN)/%_cpp.o)
  OBJS:=$$(OBJS:%.c=$$(BIN)/%_c.o)
  DEPS+=$$(OBJS:%.o=%.deps)
  $$(BIN)/$1: $$(OBJS)
  TARGETS+=$$(BIN)/$1
endef

GetRelDir=$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
GetScope=$(GetRelDir:/=-)

COMMON_HEAD_HEADER_VERSION:=1
