CXX=g++

INCLUDES += -Ivendor/PEGTL/include
INCLUDES += -Ivendor/CLI11/include

CPPFLAGS = -std=c++17 ${INCLUDES} -O3

LDFLAGS = -lpthread

SRCS = $(shell cd src && find * -type f -name '*.cc')
OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
ABS_SRCS = $(addprefix src/,$(SRCS))
ABS_HEADERS = $(shell find src -type f -name '*.hh')
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvm

INSTALL_PATH = $(shell systemd-path user-binaries)

.PHONY: run clean cleanAll

build: bin $(TARGET_BIN)

install: build
	@test "${INSTALL_PATH}" == "" && echo -e "\nunable to get 'user-binaries' path from 'systemd-path' command\n" || cp $(TARGET_BIN) ${INSTALL_PATH}/

cleanAll: clean
	rm -rf build bin

valgrind: clean build
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

init:
	git submodule update --init --recursive vendor/*
