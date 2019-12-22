CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -O2 -Ivendor/PEGTL/include -Ivendor/fmt/include
LDFLAGS =

SRCS += main.cc parser.cc

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = build/csvq
LIBFMT_TGT = build/.libs/fmt/libfmt.a
LIBS = $(LIBFMT_TGT)

.PHONY: run clean cleanAll

build: $(TARGET_BIN)

clean:
	rm -rf $(OBJS)

cleanAll: clean
	rm -rf build

run: $(TARGET_BIN)
	$(TARGET_BIN)

$(TARGET_BIN): $(OBJS) $(LIBS)
	mkdir -p build
	$(CXX) $(LDFLAGS) -o $@ $^

build/.objs/%.o: src/%.cc src/%.hh vendor/PEGTL/include vendor/fmt/include build/.objs
	$(CXX) $(CPPFLAGS) -c -o $@ $<

build/.objs:
	mkdir -p $@

vendor/%/include:
	git submodule update --init --recursive

$(LIBFMT_TGT): vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) -j6 fmt/fast

