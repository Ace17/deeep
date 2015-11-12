include build/common_head.mak

ARCH:=$(shell $(CXX) -dumpmachine)

EXT:=exe
ifeq ($(ARCH),asmjs-unknown-emscripten)
	EXT:=html
endif

all: true_all

SDL_CFLAGS:=$(shell sdl-config --cflags)
SDL_LDFLAGS:=$(shell sdl-config --libs)

CXXFLAGS+=-Wall -Wextra
CXXFLAGS+=-Isrc
CXXFLAGS+=-std=c++11
CXXFLAGS+=$(SDL_CFLAGS)
LDFLAGS+=$(SDL_LDFLAGS) -lSDL_image -lSDL_mixer
LDFLAGS+=-lGL -lGLU

CXXFLAGS+=-O3

SRCS:=\
	src/game/game.cpp\
	src/game/smarttiles.cpp\
	src/game/level1.cpp\
	src/engine/app.cpp\
	src/engine/main.cpp\
	src/engine/display.cpp\
	src/engine/sound.cpp\
	$(BIN)/vertex.glsl.cpp\
	$(BIN)/fragment.glsl.cpp\

OBJS:=$(SRCS:%.cpp=$(BIN)/%_cpp.o)

$(BIN)/deeep.$(EXT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++11 $^ -o '$@' $(CXXFLAGS) $(LDFLAGS)

TARGETS+=$(BIN)/deeep.$(EXT)

$(BIN)/vertex.glsl.cpp: src/engine/vertex.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "VertexShaderCode"

$(BIN)/fragment.glsl.cpp: src/engine/fragment.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "FragmentShaderCode"

include build/common.mak
