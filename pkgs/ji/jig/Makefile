# Local development build of jig and its unit tests, outside the bootstrap (`nix-shell` here, then
# `make test`). The derivation is bootstrap.nu. This only exists for a fast edit-compile-test loop.
CXX      ?= clang++
CXXFLAGS ?= -std=c++26 -O0 -g -Wall -Wextra -Werror -Wno-unused-command-line-argument \
            -U_LIBCPP_HARDENING_MODE -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE -fno-exceptions \
            -DJIG_STORE_DIR='"$(NIX_STORE_DIR)"'
NIX_STORE_DIR ?= /nix/store
# `make test` also runs the suite for a store unlike Nix's: under /opt, its hashes 64 hex digits
OTHER_FLAGS ?= -UJIG_STORE_DIR -DJIG_STORE_DIR='"/opt/store"' -DJIG_STORE_HASH_LENGTH=64 \
               -DJIG_STORE_HASH_ALPHABET='"0123456789abcdef"' -DJIG_STORE_HASH_PLACEHOLDER="'x'"
LDLIBS   ?= -lblake3 -lzstd
BUILD    ?= build
OTHER    ?= $(BUILD)-other

srcs   := $(wildcard src/*.cc)
objs   := $(patsubst src/%.cc,$(BUILD)/%.o,$(srcs))
common := $(filter-out $(BUILD)/main.o $(BUILD)/jig_test.o,$(objs))

.PHONY: all test clean
all: $(BUILD)/jig $(BUILD)/jig_test
test: $(BUILD)/jig_test $(OTHER)/jig_test
	$(BUILD)/jig_test
	$(OTHER)/jig_test

$(BUILD)/jig: $(BUILD)/main.o $(common)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
$(BUILD)/jig_test: $(BUILD)/jig_test.o $(common)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
$(BUILD)/%.o: src/%.cc $(wildcard src/*.h) | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(BUILD):
	mkdir -p $@

other_objs := $(patsubst src/%.cc,$(OTHER)/%.o,$(srcs))
$(OTHER)/jig_test: $(OTHER)/jig_test.o $(filter-out $(OTHER)/main.o $(OTHER)/jig_test.o,$(other_objs))
	$(CXX) $(CXXFLAGS) $(OTHER_FLAGS) -o $@ $^ $(LDLIBS)
$(OTHER)/%.o: src/%.cc $(wildcard src/*.h) | $(OTHER)
	$(CXX) $(CXXFLAGS) $(OTHER_FLAGS) -c -o $@ $<
$(OTHER):
	mkdir -p $@
clean:
	rm -rf $(BUILD) $(OTHER)
