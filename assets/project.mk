#-----------------------------------
# Sound & Music

SOUNDS_SRC+=$(wildcard assets/sounds/*.ogg)
SOUNDS_SRC+=$(wildcard assets/music/*.ogg)
TARGETS+=$(SOUNDS_SRC:assets/%.ogg=res/%.ogg)

res/%.ogg: assets/%.ogg
	@mkdir -p $(dir $@)
	./scripts/cook-audio.sh "$@" "$<"

SPRITES_SRC+=$(wildcard assets/sprites/*.json)
TARGETS+=$(SPRITES_SRC:assets/%.json=res/%.model)
TARGETS+=$(SPRITES_SRC:assets/%.json=res/%.png)

#-----------------------------------
# ROOMS

ROOMS_SRC+=$(wildcard assets/rooms/*.json)

#-----------------------------------
# Fonts

TARGETS+=res/font.png
TARGETS+=res/font.model

TILES_SRC+=$(wildcard assets/tiles/*.xcf)
TARGETS+=$(TILES_SRC:assets/%.xcf=res/%.png)

$(BIN)/res/quest.json: assets/quest.world $(BIN_HOST)/packquest.exe $(ROOMS_SRC)
	@mkdir -p $(dir $@)
	$(BIN_HOST)/packquest.exe "$<" "$@"

TARGETS+=res/quest.gz
res/quest.gz: $(BIN)/res/quest.json
	gzip -n -c "$<" > "$@"

res/%.model: assets/%.json
	@mkdir -p $(dir $@)
	@cp "$<" "$@"

res/%.png: assets/%.xcf
	@mkdir -p $(dir $@)
	@echo "Render $<"
	@xcf2png "$<" -o "$@"

#-----------------------------------
# Shaders

SHADERS_SRC+=$(wildcard assets/shaders/*.vert)
SHADERS_SRC+=$(wildcard assets/shaders/*.frag)
TARGETS+=$(SHADERS_SRC:assets/%=res/%)

res/%.frag: assets/%.frag
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(BIN)/$*)
	glslangValidator -G -o "$(BIN)/$*.spv" "$<"
	@cp "$<" "$@"

res/%.vert: assets/%.vert
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(BIN)/$*)
	glslangValidator -G -o "$(BIN)/$*.spv" "$<"
	@cp "$<" "$@"

#-----------------------------------
# fallback copy
res/%: assets/%
	@mkdir -p $(dir $@)
	@cp "$<" "$@"

