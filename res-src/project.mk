SOUNDS_SRC+=$(wildcard res-src/sounds/*.ogg)
SOUNDS_SRC+=$(wildcard res-src/music/*.ogg)
TARGETS+=$(SOUNDS_SRC:res-src/%.ogg=res/%.ogg)

res/%.ogg: res-src/%.ogg
	@mkdir -p $(dir $@)
	./scripts/cook-audio.sh "$@" "$<"

SPRITES_SRC+=$(wildcard res-src/sprites/*.json)
TARGETS+=$(SPRITES_SRC:res-src/%.json=res/%.model)
TARGETS+=$(SPRITES_SRC:res-src/%.json=res/%.png)

ROOMS_SRC+=$(wildcard res-src/rooms/*.json)
TARGETS+=$(ROOMS_SRC:res-src/%=res/%)

TARGETS+=res/quest.json

TARGETS+=res/font.png
TARGETS+=res/font.model

TILES_SRC+=$(wildcard res-src/tiles/*.xcf)
TARGETS+=$(TILES_SRC:res-src/%.xcf=res/%.png)

res/quest.json: res-src/quest.json $(BIN)/packquest.exe
	@mkdir -p $(dir $@)
	$(BIN)/packquest.exe "$<" "$@"

res/%.json: res-src/%.json
	@mkdir -p $(dir $@)
	@cat "$<" > "$@"

res/%.model: res-src/%.json
	@mkdir -p $(dir $@)
	@cp "$<" "$@"

res/%.png: res-src/%.xcf
	@mkdir -p $(dir $@)
	@echo "Render $<"
	@xcf2png "$<" -o "$@"

# fallback copy
res/%: res-src/%
	@mkdir -p $(dir $@)
#@echo "Copy $<"
	@cp "$<" "$@"

