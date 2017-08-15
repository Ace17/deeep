include build/common_head.mak

CROSS_COMPILE?=
ifneq (,$(CROSS_COMPILE))
CXX:=$(CROSS_COMPILE)g++
endif

EXT?=.exe

all: true_all

PKGS:=\
	gl\
	sdl2\
	SDL2_image\
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
CXXFLAGS+=-std=c++14
CXXFLAGS+=$(PKG_CFLAGS)
LDFLAGS+=$(PKG_LDFLAGS)

CXXFLAGS+=-O3

CXXFLAGS+=$(DBGFLAGS)
LDFLAGS+=$(DBGFLAGS)

#------------------------------------------------------------------------------

SRCS_GAME:=\
	src/game/entities/all.cpp\
	src/game/entities/bonus.cpp\
	src/game/entities/explosion.cpp\
	src/game/entities/rockman.cpp\
	src/game/entities/spider.cpp\
	src/game/entities/switch.cpp\
	src/game/entity_factory.cpp\
	src/game/game.cpp\
	src/game/level_graph.cpp\
	src/game/level_tiled.cpp\
	src/game/physics.cpp\
	src/game/resources.cpp\
	src/game/smarttiles.cpp\

#------------------------------------------------------------------------------

SRCS:=\
	$(SRCS_GAME)\
	extra/miniz.c\
	$(BIN)/fragment.glsl.cpp\
	$(BIN)/vertex.glsl.cpp\
	src/engine/app.cpp\
	src/engine/base64.cpp\
	src/engine/decompress.cpp\
	src/engine/display.cpp\
	src/engine/json.cpp\
	src/engine/main.cpp\
	src/engine/model.cpp\
	src/engine/sound.cpp\

$(BIN)/deeep$(EXT): $(SRCS:%.cpp=$(BIN)/%_cpp.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/deeep$(EXT)

#------------------------------------------------------------------------------
include res-src/project.mk

#------------------------------------------------------------------------------

SRCS_TESTS:=\
	$(SRCS_GAME)\
	extra/miniz.c\
	src/engine/base64.cpp\
	src/engine/decompress.cpp\
	src/engine/json.cpp\
	tests/base64.cpp\
	tests/decompress.cpp\
	tests/game/entities.cpp\
	tests/game/level_graph.cpp\
	tests/game/physics.cpp\
	tests/json.cpp\
	tests/tests.cpp\
	tests/tests_main.cpp\
	tests/tokenizer.cpp\
	tests/util.cpp\

# get rid of those
SRCS_TESTS+=\
	src/game/level_tiled.cpp\

$(BIN)/tests$(EXT): $(SRCS_TESTS:%.cpp=$(BIN)/%_cpp.o)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/tests$(EXT)

$(BIN)/vertex.glsl.cpp: src/engine/vertex.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "VertexShaderCode"

$(BIN)/fragment.glsl.cpp: src/engine/fragment.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "FragmentShaderCode"

include build/common.mak
