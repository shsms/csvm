CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -Ivendor/PEGTL/include -Ivendor/fmt/include

SRCS += main.cc parser.cc

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)

.PHONY: build run clean

build: build/csvq

clean:
	rm -rf build $(OBJS)

run: build
	build/csvq

build/csvq: $(OBJS) build/.libs/fmt/libfmt.a
	mkdir -p build
	$(CXX) $(LDFLAGS) -o $@ $^

build/.objs/%.o: src/%.cc vendor/PEGTL/include vendor/fmt/include build/.objs
	$(CXX) $(CPPFLAGS) -o $@ -c $<

build/.objs:
	mkdir -p $@

vendor/%/include:
	git submodule update --init --recursive

build/.libs/fmt/libfmt.a: vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) -j6 fmt/fast

