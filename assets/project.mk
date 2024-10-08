#-----------------------------------
# Sound & Music

SOUNDS_SRC+=$(wildcard assets/sounds/*.ogg)
SOUNDS_SRC+=$(wildcard assets/music/*.ogg)
RESOURCES+=$(SOUNDS_SRC:assets/%.ogg=res/%.ogg)

res/%.ogg: assets/%.ogg
	@mkdir -p $(dir $@)
	./scripts/cook-audio.sh "$@" "$<"

SPRITES_SRC+=$(wildcard assets/sprites/*.json)
RESOURCES+=$(SPRITES_SRC:assets/%.json=res/%.model)
RESOURCES+=$(SPRITES_SRC:assets/%.json=res/%.png)

BACKGROUNDS_SRC+=$(wildcard assets/backgrounds/*.json)
RESOURCES+=$(BACKGROUNDS_SRC:assets/%.json=res/%.model)
RESOURCES+=$(BACKGROUNDS_SRC:assets/%.json=res/%.jpg)

#-----------------------------------
# ROOMS

ROOMS_SRC+=$(wildcard assets/rooms/*.json)

#-----------------------------------
# Fonts

RESOURCES+=res/font.png
RESOURCES+=res/font.model

TILES_SRC+=$(wildcard assets/tiles/*.xcf)
RESOURCES+=$(TILES_SRC:assets/%.xcf=res/%.png)

$(BIN)/res/quest.json: assets/quest.ldtk $(BIN_HOST)/packquest.exe $(ROOMS_SRC)
	@mkdir -p $(dir $@)
	$(BIN_HOST)/packquest.exe "$<" "$@"

TARGETS+=res/quest.gz
res/quest.gz: $(BIN)/res/quest.json
	gzip -n -c "$<" > "$@"

res/%.model: assets/%.json
	@mkdir -p $(dir $@)
	@cp "$<" "$@"

res/%.jpg: assets/%.xcf
	@mkdir -p $(dir $@)
	@echo "Render $<"
	@xcf2png "$<" -o "$@.png"
	@ffmpeg -loglevel 1 -i "$@.png" -pix_fmt yuvj420p -q:v 1 -y "$@" </dev/null
	@rm -f "$@.png"

res/%.png: assets/%.xcf
	@mkdir -p $(dir $@)
	@echo "Render $<"
	@xcf2png "$<" -o "$@"

#-----------------------------------
# Shaders

SHADERS_SRC+=$(wildcard assets/shaders/*.vert)
SHADERS_SRC+=$(wildcard assets/shaders/*.frag)
RESOURCES+=$(SHADERS_SRC:assets/%=res/%)

res/%.frag: assets/%.frag
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(BIN)/$*)
	@#glslangValidator -G -o "$(BIN)/$*.spv" "$<"
	glslangValidator "$<"
	@cp "$<" "$@"

res/%.vert: assets/%.vert
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(BIN)/$*)
	@#glslangValidator -G -o "$(BIN)/$*.spv" "$<"
	glslangValidator "$<"
	@cp "$<" "$@"

#-----------------------------------
# fallback copy
res/%: assets/%
	@mkdir -p $(dir $@)
	@cp "$<" "$@"

resources: $(RESOURCES)

TARGETS+=$(RESOURCES)

