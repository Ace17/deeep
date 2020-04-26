SOUNDS_SRC+=$(wildcard assets/sounds/*.ogg)
SOUNDS_SRC+=$(wildcard assets/music/*.ogg)
TARGETS+=$(SOUNDS_SRC:assets/%.ogg=res/%.ogg)

res/%.ogg: assets/%.ogg
	@mkdir -p $(dir $@)
	./scripts/cook-audio.sh "$@" "$<"

SPRITES_SRC+=$(wildcard assets/sprites/*.json)
TARGETS+=$(SPRITES_SRC:assets/%.json=res/%.model)
TARGETS+=$(SPRITES_SRC:assets/%.json=res/%.png)

ROOMS_SRC+=$(wildcard assets/rooms/*.json)

TARGETS+=res/font.png
TARGETS+=res/font.model

TILES_SRC+=$(wildcard assets/tiles/*.xcf)
TARGETS+=$(TILES_SRC:assets/%.xcf=res/%.png)

$(BIN)/res/quest.json: assets/quest.json $(BIN_HOST)/packquest.exe $(ROOMS_SRC)
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

# fallback copy
res/%: assets/%
	@mkdir -p $(dir $@)
#@echo "Copy $<"
	@cp "$<" "$@"

