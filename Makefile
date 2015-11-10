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

SRCS:=\
	src/app.cpp\
	src/main.cpp\
	src/game.cpp\
	src/display.cpp\
	src/sound.cpp\
	$(BIN)/vertex.glsl.cpp\
	$(BIN)/fragment.glsl.cpp\

OBJS:=$(SRCS:%.cpp=$(BIN)/%_cpp.o)

$(BIN)/deeep.$(EXT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++11 $^ -o '$@' $(CXXFLAGS) $(LDFLAGS)

TARGETS+=$(BIN)/deeep.$(EXT)

$(BIN)/vertex.glsl.cpp: src/vertex.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "VertexShaderCode"

$(BIN)/fragment.glsl.cpp: src/fragment.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "FragmentShaderCode"

include build/common.mak
