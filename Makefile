CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -O3 -Ivendor/PEGTL/include -Ivendor/fmt/include
LDFLAGS = -lpthread

#RUN_ARGS = "to_num(trdSz); select(type=='t' && arrTm >= '150000' && trdSz >= 400 && trdSz < 1500); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);to_str(trdSz)"
RUN_ARGS = "select(type=='q');"
#RUN_ARGS = "select(type=='t' && arrTm >= '150000'); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);"
#RUN_ARGS = ""
SRCS = $(shell cd src && find * -type f -name '*.cc')

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvq
LIBFMT_TGT = build/.libs/fmt/libfmt.a
LIBS = $(LIBFMT_TGT)

.PHONY: run clean cleanAll

build: bin $(TARGET_BIN)

cleanAll: clean
	rm -rf build bin

run: build
	@$(TARGET_BIN) $(RUN_ARGS)

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
	clang-format -i $(ABS_SRCS) $(shell find src -name '*.hh')

tidy: format
	clang-tidy --checks=* $(ABS_SRCS) -- $(CPPFLAGS)

.PRECIOUS: vendor/%/include
vendor/%/include:
	git submodule update --init --recursive $(dir $@)

$(LIBFMT_TGT): vendor/fmt/include
	mkdir -p $(dir $@)
	cd $(dir $@) && cmake $(PROJECT_ROOT)/$(dir $<)
	make -C $(dir $@) fmt
