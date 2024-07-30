# Based on https://matansilver.com/2017/08/29/universal-makefile/
# Modified by Stormy

# Settings
# Set to 0 to enable C mode
CPP_MODE := 1
ifeq ($(CPP_MODE), 0)
FB_EXT := .c
else
FB_EXT := .cpp
endif

# Flags
MY_CC := $(BUILD_DIR)/compiler
CFLAGS := -Wall -std=c11
CXXFLAGS := -Wall -Wno-register -std=c++17
FFLAGS :=
BFLAGS := -d
LDFLAGS :=

# Debug flags
DEBUG ?= 1
ifeq ($(DEBUG), 0)
CFLAGS += -O2
CXXFLAGS += -O2
else
CFLAGS += -g -O0
CXXFLAGS += -g -O0
endif

# Compilers
CC := clang
CXX := clang++
FLEX := flex
BISON := bison

# Directories
TOP_DIR := .
TARGET_EXEC := compiler
SRC_DIR := $(TOP_DIR)/src
BUILD_DIR ?= $(TOP_DIR)/build
LIB_DIR ?= $(CDE_LIBRARY_PATH)/native
INC_DIR ?= $(CDE_INCLUDE_PATH)
CFLAGS += -I$(INC_DIR)
CXXFLAGS += -I$(INC_DIR)
LDFLAGS += -L$(LIB_DIR) -lkoopa

# Tests
FUNCTION := $(TOP_DIR)/functional
HIDDEN_FUNCTION := $(TOP_DIR)/hidden_functional
PERFORMANCE := $(TOP_DIR)/performance
FINAL_PERFORMANCE := $(TOP_DIR)/final_performance

# Source files & target files
FB_SRCS := $(patsubst $(SRC_DIR)/%.l, $(BUILD_DIR)/%.lex$(FB_EXT), $(shell find $(SRC_DIR) -name "*.l"))
FB_SRCS += $(patsubst $(SRC_DIR)/%.y, $(BUILD_DIR)/%.tab$(FB_EXT), $(shell find $(SRC_DIR) -name "*.y"))
SRCS := $(FB_SRCS) $(shell find $(SRC_DIR) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS))
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(OBJS))
OBJS := $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.cc.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.cc, $(BUILD_DIR)/%.cc.o, $(OBJS))

# Header directories & dependencies
INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_DIRS += $(INC_DIRS:$(SRC_DIR)%=$(BUILD_DIR)%)
INC_FLAGS := $(addprefix -I, $(INC_DIRS))
DEPS := $(OBJS:.o=.d)
CPPFLAGS = $(INC_FLAGS) -MMD -MP


# Main target
$(BUILD_DIR)/$(TARGET_EXEC): $(FB_SRCS) $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -lpthread -ldl -o $@

# C source
define c_recipe
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endef
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c; $(c_recipe)
$(BUILD_DIR)/%.c.o: $(BUILD_DIR)/%.c; $(c_recipe)

# C++ source
define cxx_recipe
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
endef
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp; $(cxx_recipe)
$(BUILD_DIR)/%.cpp.o: $(BUILD_DIR)/%.cpp; $(cxx_recipe)
$(BUILD_DIR)/%.cc.o: $(SRC_DIR)/%.cc; $(cxx_recipe)

# Flex
$(BUILD_DIR)/%.lex$(FB_EXT): $(SRC_DIR)/%.l
	mkdir -p $(dir $@)
	$(FLEX) $(FFLAGS) -o $@ $<

# Bison
$(BUILD_DIR)/%.tab$(FB_EXT): $(SRC_DIR)/%.y
	mkdir -p $(dir $@)
	$(BISON) $(BFLAGS) -o $@ $<


.PHONY: clean floatTest docker docker-func

clean:
	-rm -rf $(BUILD_DIR)

docker:
	docker run -it \
	-v $(TOP_DIR):/root/compiler \
	stormy/compiler:autotest bash

docker-func:
	docker run -it \
	-v $(TOP_DIR):/root/compiler \
	-v $(FUNCTION):/opt/bin/testcases/functional \
	-v $(HIDDEN_FUNCTION):/opt/bin/testcases/hidden_functional \
	stormy/compiler:autotest bash

docker-perf:
	docker run -it \
	-v $(TOP_DIR):/root/compiler \
	-v $(PERFORMANCE):/opt/bin/testcases/performance \
	-v $(FINAL_PERFORMANCE):/opt/bin/testcases/final_performance \
	stormy/compiler:autotest bash

debug:
	docker run -it -v $(TOP_DIR):/root/compiler \
  	--cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
  	stormy/compiler:autotest bash

test:
	make clean
	make
	build/compiler $(TARGET) hello.c -o hello.koopa

autotest:
	autotest -koopa -s lv8 /root/compiler

koopa-test:
	./build/compiler -koopa -o hello.koopa  hello.c
	koopac hello.koopa | llc --filetype=obj -o hello.o
	clang hello.o -L $$CDE_LIBRARY_PATH/native -lsysy -o hello
	./hello

riscv-test:
	./build/compiler -S -o hello.S hello.c 
	clang hello.S -c -o hello.o -target riscv64-unknown-linux-elf -march=rv64imafdc -mabi=lp64d -mno-relax
	ld.lld hello.o -L . -lsysy -o hello
	riscv64-linux-gnu-objdump -d hello > hello.obj
	qemu-riscv64-static hello < hello.in

gdb:
	gdb --args ./build/compiler -riscv -o hello.S  hello.c
	# gdb --args ./build/compiler -astT -o test.ast test.c

all:
	sudo -S make

riscv-debug:
	qemu-riscv64-static -g 1234 hello < hello.in & 
	gdb-multiarch hello 

-include $(DEPS)


