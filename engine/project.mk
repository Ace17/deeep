SRCS_ENGINE:=\
	$(BIN)/$(ENGINE_ROOT)/src/fragment.glsl.cpp\
	$(BIN)/$(ENGINE_ROOT)/src/vertex.glsl.cpp\
	$(ENGINE_ROOT)/src/app.cpp\
	$(ENGINE_ROOT)/src/base64.cpp\
	$(ENGINE_ROOT)/src/decompress.cpp\
	$(ENGINE_ROOT)/src/file.cpp\
	$(ENGINE_ROOT)/src/json.cpp\
	$(ENGINE_ROOT)/src/png.cpp\
	$(ENGINE_ROOT)/src/model.cpp\
	$(ENGINE_ROOT)/src/display_ogl.cpp\
	$(ENGINE_ROOT)/src/audio_sdl.cpp\
	$(ENGINE_ROOT)/src/sound_ogg.cpp\
	$(ENGINE_ROOT)/src/main.cpp\

$(BIN)/$(ENGINE_ROOT)/src/vertex.glsl.cpp: NAME=VertexShaderCode
$(BIN)/$(ENGINE_ROOT)/src/fragment.glsl.cpp: NAME=FragmentShaderCode

$(BIN)/%.glsl.cpp: %.glsl
	@mkdir -p $(dir $@)
	scripts/embed.sh "$<" "$@" "$(NAME)"

