include build/common_head.mak

CROSS_COMPILE?=
ifneq (,$(CROSS_COMPILE))
CXX:=$(CROSS_COMPILE)g++
endif

HOST_CXX?=g++

EXT?=.exe

all: true_all

PKGS:=\
	sdl2\
	ogg \
	vorbisfile \

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
	src/entities/blocks.cpp\
	src/entities/bonus.cpp\
	src/entities/conveyor.cpp\
	src/entities/detector.cpp\
	src/entities/door.cpp\
	src/entities/exitpoint.cpp\
	src/entities/explosion.cpp\
	src/entities/hatch.cpp\
	src/entities/hopper.cpp\
	src/entities/ladder.cpp\
	src/entities/lift.cpp\
	src/entities/moving_platform.cpp\
	src/entities/rockman.cpp\
	src/entities/savepoint.cpp\
	src/entities/spider.cpp\
	src/entities/spikes.cpp\
	src/entities/sweeper.cpp\
	src/entities/switch.cpp\
	src/entities/wheel.cpp\
	src/entity_factory.cpp\
	src/game.cpp\
	src/physics.cpp\
	src/preprocess_quest.cpp\
	src/load_quest.cpp\
	src/resources.cpp\
	src/smarttiles.cpp\
	src/state_ending.cpp\
	src/state_playing.cpp\
	src/state_paused.cpp\
	src/state_splash.cpp\

#------------------------------------------------------------------------------

SRCS:=\
	$(SRCS_GAME)\
	$(SRCS_ENGINE)\

$(BIN)/rel/game$(EXT): $(SRCS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/rel/game$(EXT)

#------------------------------------------------------------------------------
include res-src/project.mk

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

SRCS_PACKQUEST:=\
	$(SRCS_GAME)\
	$(filter-out $(ENGINE_ROOT)/src/main.cpp, $(SRCS_ENGINE))\
	src/packquest.cpp\


$(BIN)/packquest.exe: $(SRCS_PACKQUEST:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/packquest.exe

include build/common.mak
