CXX=g++

INCLUDES += -Ivendor/PEGTL/include
INCLUDES += -Ivendor/CLI11/include
INCLUDES += -Ivendor/Catch2/single_include

CPPFLAGS = -std=c++17 ${INCLUDES} -O3
TEST_CPPFLAGS = -std=c++17 ${INCLUDES} -O0 -g --coverage

LDFLAGS = -lpthread
TEST_LDFLAGS = -lpthread --coverage

SRCS = $(shell cd src && find * -type f -not -name '*_test.cc' -name '*.cc')
OBJS = $(addprefix build/.objs/,$(subst .cc,.o,$(SRCS)))
TEST_SRCS = $(shell cd src && find * -type f -not -name 'main.cc' -name '*.cc')
TEST_OBJS = $(addprefix build/test/.objs/,$(subst .cc,.o,$(TEST_SRCS)))

ABS_SRCS = $(addprefix src/,$(SRCS))
ABS_HEADERS = $(shell find src -type f -name '*.hh')
PROJECT_ROOT = $(shell pwd)
TARGET_BIN = bin/csvm
TEST_BIN = bin/test

INSTALL_PATH = $(shell systemd-path user-binaries)

.PHONY: run clean cleanAll

build: bin $(TARGET_BIN)

test: bin $(TEST_BIN)
	$(TEST_BIN)  --reporter compact --success

testCover: test
	gcovr -r . -f src -e '.+_test\.cc$$' --html --html-details -o build/coverage.html

install: build
	@test "${INSTALL_PATH}" == "" && echo -e "\nunable to get 'user-binaries' path from 'systemd-path' command\n" || cp $(TARGET_BIN) ${INSTALL_PATH}/

cleanAll: clean
	rm -rf build bin

valgrind: clean build
	valgrind  --tool=callgrind $(TARGET_BIN) $(RUN_ARGS)

memusage: build
	valgrind --tool=massif $(TARGET_BIN) -o outq.csv -f tq-01.csv "select(type=='q'); sort(askSz)"

$(TARGET_BIN): $(OBJS) $(LIBS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(TEST_BIN): $(TEST_OBJS) $(LIBS)
	$(CXX) $(TEST_LDFLAGS) -o $@ $^

bin:
	mkdir bin

DEP = $(OBJS:%.o=%.d)
-include $(DEP)

build/.objs/%.o: src/%.cc
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CPPFLAGS) -MMD -c -o $@ $<

build/test/.objs/%.o: src/%.cc
	@mkdir -p $(shell dirname $@)
	$(CXX) $(TEST_CPPFLAGS) -MMD -c -o $@ $<

clean:
	rm -rf $(OBJS) $(TEST_OBJS) $(DEP)

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
