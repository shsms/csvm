CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -g -Ivendor/PEGTL/include -Ivendor/fmt/include
LDFLAGS =

# RUN_ARGS = "cols(A,B); !cols(C,D); select(A != B || z==54)"
RUN_ARGS = "cols(D,B)"

SRCS = $(shell cd src && find * -type f -name '*.cc')

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvq
LIBFMT_TGT = build/.libs/fmt/libfmt.a
LIBS = $(LIBFMT_TGT)

.PHONY: run clean cleanAll

build: $(TARGET_BIN)

clean:
	rm -rf $(OBJS)

cleanAll: clean
	rm -rf build bin

run: build
	$(TARGET_BIN) $(RUN_ARGS)

$(TARGET_BIN): $(OBJS) $(LIBS)
	mkdir -p $(shell dirname $@)
	$(CXX) $(LDFLAGS) -o $@ $^

build/.objs/%.mkdir: src/%.cc
	mkdir -p $(shell dirname $@)

build/.objs/%.o: src/%.cc build/.objs/%.mkdir vendor/PEGTL/include vendor/fmt/include
	$(CXX) $(CPPFLAGS) -c -o $@ $<

.PRECIOUS: vendor/%/include
vendor/%/include:
	git submodule update --init --recursive $(dir $@)

$(LIBFMT_TGT): vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) fmt
