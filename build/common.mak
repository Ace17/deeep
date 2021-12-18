.PHONY: all true_all depend clean show_targets
.DELETE_ON_ERROR:

V?=0
ifeq ($(V),1)
Q:=
else
Q:=@
endif

ifneq ($(COMMON_HEAD_HEADER_VERSION),1)
  $(error common makefile header version mismatch)
endif

ifeq ($(BIN),)
  $(error BIN must be defined)
endif

CLR_INFO:=
CLR_DBG:=
CLR_OFF:=

ENABLE_COLORS:=0

ifeq ($(TERM), xterm)
  ENABLE_COLORS:=1
endif

ifeq ($(TERM), xterm-256color)
  ENABLE_COLORS:=1
endif

ifeq ($(TERM), rxvt-unicode)
  ENABLE_COLORS:=1
endif

ifeq ($(ENABLE_COLORS),1)
  CLR_INFO:=[32m
  CLR_DBG:=[34m
  CLR_OFF:=[0m
endif

show_targets:
	echo $(TARGETS)

DC?=gdc

CROSS_COMPILE?=
ifneq (,$(CROSS_COMPILE))
CXX:=$(CROSS_COMPILE)g++
CC:=$(CROSS_COMPILE)gcc
DC:=$(CROSS_COMPILE)gdc
endif

LINK?=$(DC)

$(BIN)/%.exe:
	@echo "$(CLR_INFO)Linking $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)$(LINK) $^ -o $@ -g $(DLIBS) $(LDFLAGS)

$(BIN)/%.dll:
	@echo "$(CLR_INFO)Linking $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)$(LINK) $^ -shared -o $@ -g $(DLIBS) $(LDFLAGS)

$(BIN)/%.a:
	@echo "$(CLR_INFO)Archiving $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)ar cr "$@" $^

# Compiling
#
$(BIN)/%.d.o: %.d
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^) $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)$(DC) -c "$<" -o "$@" $(DINCS) $(DFLAGS)
	@$(DC) -MP -MM "$<" -MT "$@" -o "$@" $(DINCS) $(DFLAGS) -w -O0 -MF "$@.deps"

$(BIN)/%.cpp.o: %.cpp
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^) $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)$(CXX) -MMD -MT "$@" -MF "$@.deps" -c $(CXXFLAGS) -o "$@" "$<"

$(BIN)/%.c.o: %.c
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^)  $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(Q)$(CXX) -MMD -MT "$@" -MF "$@.deps" -c $(CXXFLAGS) -o "$@" "$<"

# Dependency generation

include $(shell test -d $(BIN) && find $(BIN) -name "*.deps")

# Inclusion

$(BIN)/%.mk: %.mk
	@mkdir -p $(dir $@)
	@$(THIS)/include "$<" > "$@"

# Common

clean:
	rm -rf $(BIN) $(TARGETS)
	mkdir $(BIN)
	@echo "exclude /*" > $(BIN)/.rsync-filter

REPO_URL?=http://code.alaiwan.org/bzr

true_all: $(TARGETS)
