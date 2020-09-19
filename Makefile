include build/common_head.mak

CROSS_COMPILE?=
ifneq (,$(CROSS_COMPILE))
CXX:=$(CROSS_COMPILE)g++
endif

HOST_CXX?=g++

EXT?=.exe
BIN_HOST?=bin_host
TARGETS+=$(BIN_HOST)

all: true_all

PKGS:=\
	sdl2\

PKG_CFLAGS:=$(shell pkg-config $(PKGS) --cflags)
PKG_LDFLAGS:=$(shell pkg-config $(PKGS) --libs || echo "ERROR")

ifeq (ERROR,$(PKG_LDFLAGS))
  $(error At least one library was not found in the build environment)
endif

DBGFLAGS?=-g

CXXFLAGS+=-Wall -Wextra
CXXFLAGS+=-Isrc
CXXFLAGS+=-I.
CXXFLAGS+=-Iengine
CXXFLAGS+=-Iengine/include
CXXFLAGS+=-std=c++14
CXXFLAGS+=$(PKG_CFLAGS)
LDFLAGS+=$(PKG_LDFLAGS)

#CXXFLAGS+=-O3

CXXFLAGS+=$(DBGFLAGS)
LDFLAGS+=$(DBGFLAGS)

#------------------------------------------------------------------------------

ENGINE_ROOT:=engine
include $(ENGINE_ROOT)/project.mk

#------------------------------------------------------------------------------

SRCS_GAME:=\
	src/gameplay/entity_factory.cpp\
	src/gameplay/game.cpp\
	src/gameplay/physics.cpp\
	src/gameplay/preprocess_quest.cpp\
	src/gameplay/load_quest.cpp\
	src/gameplay/resources.cpp\
	src/gameplay/rockman.cpp\
	src/gameplay/smarttiles.cpp\
	src/gameplay/state_ending.cpp\
	src/gameplay/state_playing.cpp\
	src/gameplay/state_paused.cpp\
	src/gameplay/state_splash.cpp\
	src/entities/blocks.cpp\
	src/entities/bonus.cpp\
	src/entities/conveyor.cpp\
	src/entities/detector.cpp\
	src/entities/door.cpp\
	src/entities/elevator.cpp\
	src/entities/exitpoint.cpp\
	src/entities/explosion.cpp\
	src/entities/hatch.cpp\
	src/entities/hopper.cpp\
	src/entities/ladder.cpp\
	src/entities/lift.cpp\
	src/entities/savepoint.cpp\
	src/entities/spider.cpp\
	src/entities/spikes.cpp\
	src/entities/sweeper.cpp\
	src/entities/switch.cpp\
	src/entities/wheel.cpp\

#------------------------------------------------------------------------------

SRCS:=\
	$(SRCS_GAME)\
	$(SRCS_ENGINE)\

$(BIN)/rel/game$(EXT): $(SRCS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/rel/game$(EXT)

#------------------------------------------------------------------------------
include assets/project.mk

#------------------------------------------------------------------------------

SRCS_TESTS:=\
	$(SRCS_GAME)\
	$(filter-out $(ENGINE_ROOT)/src/main.cpp, $(SRCS_ENGINE))\
	engine/tests/tests.cpp\
	engine/tests/tests_main.cpp\
	engine/tests/audio.cpp\
	engine/tests/base64.cpp\
	engine/tests/decompress.cpp\
	engine/tests/json.cpp\
	engine/tests/util.cpp\
	engine/tests/png.cpp\
	tests/entities.cpp\
	tests/level_graph.cpp\
	tests/physics.cpp\

$(BIN)/tests$(EXT): $(SRCS_TESTS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/tests$(EXT)

#------------------------------------------------------------------------------
$(BIN_HOST):
	@mkdir -p "$@"

SRCS_PACKQUEST:=\
	engine/src/misc/decompress.cpp\
	engine/src/misc/base64.cpp\
	engine/src/misc/json.cpp\
	engine/src/misc/file.cpp\
	src/gameplay/load_quest.cpp\
	src/gameplay/smarttiles.cpp\
	src/gameplay/preprocess_quest.cpp\
	src/gameplay/packquest.cpp\

$(BIN_HOST)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo [HOST] compile "$@"
	g++ -Iengine/include -Iengine/src -I. -c "$^" -o "$@"

$(BIN_HOST)/packquest.exe: $(SRCS_PACKQUEST:%=$(BIN_HOST)/%.o)
	@mkdir -p $(dir $@)
	g++ $^ -o '$@'

TARGETS+=$(BIN_HOST)/packquest.exe

include build/common.mak
