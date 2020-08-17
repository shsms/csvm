CC=gcc
CXX=g++
CPPFLAGS = -std=c++17 -O3 -Ivendor/PEGTL/include
LDFLAGS =

#RUN_ARGS = "select(E >= '303' || B == D); !cols(D)"
#RUN_ARGS="select(a == z || (b == c && d==w))"
#RUN_ARGS = "to_num(trdSz); select(type=='t' && arrTm >= '150000' && trdSz >= 400 && trdSz < 1500); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);to_str(trdSz)"
RUN_ARGS = "select(type=='q');"
#RUN_ARGS = "select(type=='t' && arrTm >= '150000'); cols(date,arrTm,ticker,type,trdPx,trdSz,trdTm);"

SRCS = $(shell cd src && find * -type f -name '*.cc')

OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvq
LIBS =

.PHONY: run clean cleanAll

build: bin $(TARGET_BIN)

clean:
	rm -rf $(OBJS)

cleanAll: clean
	rm -rf build bin

run: build
	@$(TARGET_BIN) $(RUN_ARGS)

$(TARGET_BIN): $(OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $^

bin:
	mkdir bin

build/.objs/%.mkdir: src/%.cc
	@mkdir -p $(shell dirname $@)

build/.objs/%.o: src/%.cc src/%.hh build/.objs/%.mkdir vendor/PEGTL/include
	$(CXX) $(CPPFLAGS) -c -o $@ $<

.PRECIOUS: vendor/%/include
vendor/%/include:
	git submodule update --init --recursive $(dir $@)
