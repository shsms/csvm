CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -g -Ivendor/PEGTL/include -Ivendor/fmt/include
LDFLAGS =

RUN_ARGS = "cols(A,B); cols!(C,D)"

SRCS += main.cc parser.cc

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = build/csvq
LIBFMT_TGT = build/.libs/fmt/libfmt.a
LIBS = $(LIBFMT_TGT)

.PHONY: run clean cleanAll build_init

build: build_init $(TARGET_BIN)

clean:
	rm -rf $(OBJS)

cleanAll: clean
	rm -rf build

run: $(TARGET_BIN)
	$(TARGET_BIN) $(RUN_ARGS)

$(TARGET_BIN): $(OBJS) $(LIBS)
	mkdir -p build
	$(CXX) $(LDFLAGS) -o $@ $^

build_init: build/.objs

build/.objs:
	mkdir -p $@

build/.objs/%.o: src/%.cc vendor/PEGTL/include vendor/fmt/include
	$(CXX) $(CPPFLAGS) -c -o $@ $<

.PRECIOUS: vendor/%/include
vendor/%/include:
	git submodule update --init --recursive $(dir $@)

$(LIBFMT_TGT): vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) fmt
