CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -O3 -Ivendor/PEGTL/include -Ivendor/fmt/include
LDFLAGS = -lpthread

RUN_ARGS = -n 4 -f tq.csv
#SCRIPT = "to_num(trdSz); select(type=='t' && arrTm >= '150000' && trdSz >= 400 && trdSz < 1500); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);to_str(trdSz)"
SCRIPT = "select(type=='q');"
#SCRIPT = "select(type=='t' && arrTm >= '150000'); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);"
#SCRIPT = ""
SRCS = $(shell cd src && find * -type f -name '*.cc')

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
ABS_HEADERS = $(shell find src -type f -name '*.hh')
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvq
LIBFMT_TGT = build/.libs/fmt/libfmt.a
LIBS = $(LIBFMT_TGT)

.PHONY: run clean cleanAll

build: bin $(TARGET_BIN)

cleanAll: clean
	rm -rf build bin

run: build
	@$(TARGET_BIN) ${RUN_ARGS} $(SCRIPT)

valgrind: build
	valgrind  --tool=callgrind $(TARGET_BIN) $(RUN_ARGS)

$(TARGET_BIN): $(OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $^

bin:
	mkdir bin

DEP = $(OBJS:%.o=%.d)
-include $(DEP)

build/.objs/%.o: src/%.cc
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CPPFLAGS) -MMD -c -o $@ $<

clean:
	rm -rf $(OBJS) $(DEP)

format:
	clang-format -i $(ABS_SRCS) $(ABS_HEADERS)

tidy: format
	clang-tidy --checks=readability-*,performance-*,cppcoreguidelines-*,bugprone-*,misc-* $(ABS_HEADERS) $(ABS_SRCS) -- $(CPPFLAGS)
	make format

check:
	clang-check -analyze $(ABS_HEADERS) $(ABS_SRCS) -- $(CPPFLAGS)

tidy-fix: format
	clang-tidy --checks=readability-*,performance-*,cppcoreguidelines-*,bugprone-*,misc-* --fix $(ABS_HEADERS) $(ABS_SRCS) -- $(CPPFLAGS)
	make format

.PRECIOUS: vendor/%/include
vendor/%/include:
	git submodule update --init --recursive $(dir $@)

$(LIBFMT_TGT): vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) fmt
