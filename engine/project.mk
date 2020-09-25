SRCS_ENGINE:=\
	$(BIN)/$(ENGINE_ROOT)/src/render/fragment.glsl.cpp\
	$(BIN)/$(ENGINE_ROOT)/src/render/vertex.glsl.cpp\
	$(ENGINE_ROOT)/src/app.cpp\
	$(ENGINE_ROOT)/src/main.cpp\
	$(ENGINE_ROOT)/src/audio/audio.cpp\
	$(ENGINE_ROOT)/src/audio/audio_sdl.cpp\
	$(ENGINE_ROOT)/src/audio/sound_ogg.cpp\
	$(ENGINE_ROOT)/src/misc/base64.cpp\
	$(ENGINE_ROOT)/src/misc/decompress.cpp\
	$(ENGINE_ROOT)/src/misc/file.cpp\
	$(ENGINE_ROOT)/src/misc/json.cpp\
	$(ENGINE_ROOT)/src/render/display_ogl.cpp\
	$(ENGINE_ROOT)/src/render/glad.cpp\
	$(ENGINE_ROOT)/src/render/model.cpp\
	$(ENGINE_ROOT)/src/render/picture.cpp\
	$(ENGINE_ROOT)/src/render/png.cpp\

SRCS_ENGINE+=\
	$(ENGINE_ROOT)/src/platform/input_sdl.cpp\

$(BIN)/$(ENGINE_ROOT)/src/render/vertex.glsl.cpp: NAME=VertexShaderCode
$(BIN)/$(ENGINE_ROOT)/src/render/fragment.glsl.cpp: NAME=FragmentShaderCode

$(BIN)/%.glsl.cpp: %.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "$(NAME)"

$(BIN)/$(ENGINE_ROOT)/src/%: CXXFLAGS+=-I$(ENGINE_ROOT)/src
