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
CXXFLAGS+=-std=c++14
CXXFLAGS+=$(PKG_CFLAGS)
LDFLAGS+=$(PKG_LDFLAGS)

# Reduce executable size
CXXFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,-gc-sections

#CXXFLAGS+=-O3

#CXXFLAGS+=$(DBGFLAGS)
#LDFLAGS+=$(DBGFLAGS)

#------------------------------------------------------------------------------

SRCS_ENGINE:=\
	src/engine/app.cpp\
	src/engine/main.cpp\
	src/audio/audio.cpp\
	src/audio/sound_ogg.cpp\
	src/base/logger.cpp\
	src/misc/base64.cpp\
	src/misc/decompress.cpp\
	src/misc/file.cpp\
	src/misc/json.cpp\
	src/misc/math.cpp\
	src/misc/stats.cpp\
	src/misc/string.cpp\
	src/misc/time.cpp\
	src/render/matrix3.cpp\
	src/render/model.cpp\
	src/render/picture.cpp\
	src/render/png.cpp\
	src/render/renderer.cpp\

SRCS_ENGINE+=\
	src/platform/audio_sdl.cpp\
	src/platform/input_sdl.cpp\
	src/platform/display_ogl.cpp\
	src/platform/glad.cpp\

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
	src/entities/hero.cpp\
	src/entities/hopper.cpp\
	src/entities/ladder.cpp\
	src/entities/lift.cpp\
	src/entities/moving_wall.cpp\
	src/entities/savepoint.cpp\
	src/entities/spider.cpp\
	src/entities/spikes.cpp\
	src/entities/sweeper.cpp\
	src/entities/switch.cpp\
	src/entities/wheel.cpp\
	src/gameplay/draw_minimap.cpp\
	src/gameplay/entity_factory.cpp\
	src/gameplay/game.cpp\
	src/gameplay/physics.cpp\
	src/gameplay/presenter.cpp\
	src/gameplay/load_quest.cpp\
	src/gameplay/resources.cpp\
	src/gameplay/state_bootup.cpp\
	src/gameplay/state_ending.cpp\
	src/gameplay/state_playing.cpp\
	src/gameplay/state_paused.cpp\
	src/gameplay/state_splash.cpp\

#------------------------------------------------------------------------------

SRCS:=\
	$(SRCS_GAME)\
	$(SRCS_ENGINE)\

$(BIN)/rel/game$(EXT): $(SRCS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/rel/game$(EXT)

game: $(BIN)/rel/game$(EXT)

#------------------------------------------------------------------------------
include assets/project.mk

#------------------------------------------------------------------------------

SRCS_TESTS:=\
	$(SRCS_GAME)\
	src/gameplay/smarttiles.cpp\
	src/gameplay/preprocess_quest.cpp\
	$(filter-out src/engine/main.cpp, $(SRCS_ENGINE))\
	src/tests/tests.cpp\
	src/tests/tests_main.cpp\
	src/tests/audio.cpp\
	src/tests/base64.cpp\
	src/tests/decompress.cpp\
	src/tests/delegate.cpp\
	src/tests/json.cpp\
	src/tests/util.cpp\
	src/tests/png.cpp\
	src/tests/entities.cpp\
	src/tests/level_graph.cpp\
	src/tests/physics.cpp\

$(BIN)/tests$(EXT): $(SRCS_TESTS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/tests$(EXT)

#------------------------------------------------------------------------------
$(BIN_HOST):
	@mkdir -p "$@"

SRCS_PACKQUEST:=\
	src/misc/decompress.cpp\
	src/misc/base64.cpp\
	src/misc/json.cpp\
	src/misc/file.cpp\
	src/misc/string.cpp\
	src/gameplay/load_quest.cpp\
	src/gameplay/smarttiles.cpp\
	src/gameplay/preprocess_quest.cpp\
	src/gameplay/packquest.cpp\

$(BIN_HOST)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo [HOST] compile "$@"
	g++ -Isrc -c "$^" -o "$@"

$(BIN_HOST)/packquest.exe: $(SRCS_PACKQUEST:%=$(BIN_HOST)/%.o)
	@mkdir -p $(dir $@)
	g++ $^ -o '$@'

TARGETS+=$(BIN_HOST)/packquest.exe

include build/common.mak
