#
# @file Common
# @brief
# @author Sebastien Alaiwan
# @date 2013-12-05
#

.PHONY: all true_all depend clean show_targets

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

LINK?=gdc

$(BIN)/%.exe:
	@echo "$(CLR_INFO)Linking $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(LINK) $^ -o $@ -g $(DLIBS) $(LDFLAGS)

$(BIN)/%.dll:
	@echo "$(CLR_INFO)Linking $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(LINK) $^ -shared -o $@ -g $(DLIBS) $(LDFLAGS)

$(BIN)/%.a:
	@echo "$(CLR_INFO)Archiving $@$(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	ar cru "$@" $^

# Compiling
#
$(BIN)/%_d.o: %.d
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^) $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	@gdc -fdeps="$(BIN)/$*_d.fdeps" -c "$<" -o "$@" $(DINCS) $(DFLAGS)
	@$(THIS)/convert_deps "$(BIN)/$*_d.fdeps" "$(BIN)/$*_d.deps" "$@"

$(BIN)/%_cpp.o: %.cpp
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^) $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(CXX) -c "$<" -o "$@" $(DINCS) $(CXXFLAGS)
	@$(CXX) -MM "$<" -MT "$@" -o "$(BIN)/$*_cpp.deps" $(DINCS) $(CXXFLAGS)

$(BIN)/%_c.o: %.c
	@echo "$(CLR_INFO)Compiling $@ $(CLR_DBG)(depends on $^)  $(CLR_OFF)"
	@mkdir -p "$(dir $@)"
	$(CC) -c  "$<" -o  "$@" $(DINCS) $(CXXFLAGS)
	@$(CC) -MM "$<" -MT "$@" -o "$(BIN)/$*_c.deps" $(DINCS) $(CXXFLAGS)

# Inclusion

$(BIN)/%.mk: %.mk
	@mkdir -p $(dir $@)
	@$(THIS)/include "$<" > "$@"

# Common

clean:
	rm -rf $(BIN) $(TARGETS)
	mkdir $(BIN)
	@echo "exclude /*" > $(BIN)/.rsync-filter

REPO_URL?=http://bazaar.mooo.com/~alaiwans/bzr

-include $(shell test -d $(BIN) && find $(BIN) -name "*.deps")

true_all: $(TARGETS)
