include build/common_head.mak

CROSS_COMPILE?=
ifneq (,$(CROSS_COMPILE))
CXX:=$(CROSS_COMPILE)g++
endif

ARCH:=$(shell $(CXX) -dumpmachine)

EXT:=exe
ifeq ($(ARCH),asmjs-unknown-emscripten)
	EXT:=html
endif

all: true_all

SDL_CFLAGS:=$(shell pkg-config sdl --cflags)
SDL_LDFLAGS:=$(shell pkg-config sdl SDL_image SDL_mixer --libs --static || echo "ERROR")

ifeq (ERROR,$(SDL_LDFLAGS))
  $(error At least one library was not found in the build environment)
endif

CXXFLAGS+=-Wall -Wextra
CXXFLAGS+=-Isrc
CXXFLAGS+=-I.
CXXFLAGS+=-std=c++14
CXXFLAGS+=$(SDL_CFLAGS)
LDFLAGS+=$(SDL_LDFLAGS)
LDFLAGS+=-lGL -lGLU

CXXFLAGS+=-O3

#------------------------------------------------------------------------------

SRCS:=\
	src/game/game.cpp\
	src/game/smarttiles.cpp\
	src/game/graph_tools.cpp\
	src/game/level1.cpp\
	src/game/level2.cpp\
	src/game/level3.cpp\
	src/game/entities/rockman.cpp\
	src/engine/app.cpp\
	src/engine/json.cpp\
	src/engine/main.cpp\
	src/engine/model.cpp\
	src/engine/display.cpp\
	src/engine/sound.cpp\
	src/engine/base64.cpp\
	$(BIN)/vertex.glsl.cpp\
	$(BIN)/fragment.glsl.cpp\

OBJS:=$(SRCS:%.cpp=$(BIN)/%_cpp.o)

$(BIN)/deeep.$(EXT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/deeep.$(EXT)

#------------------------------------------------------------------------------

SRCS_TESTS:=\
	src/engine/base64.cpp\
	src/engine/json.cpp\
	tests/tests.cpp\
	tests/tests_main.cpp\
	tests/base64.cpp\
	tests/json.cpp\
	tests/tokenizer.cpp\

OBJS_TESTS:=$(SRCS_TESTS:%.cpp=$(BIN)/%_cpp.o)
$(BIN)/tests.$(EXT): $(OBJS_TESTS)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o '$@' $(LDFLAGS)

TARGETS+=$(BIN)/tests.$(EXT)

$(BIN)/vertex.glsl.cpp: src/engine/vertex.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "VertexShaderCode"

$(BIN)/fragment.glsl.cpp: src/engine/fragment.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "FragmentShaderCode"

include build/common.mak
