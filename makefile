LLVM_BIN ?= /change/to/llvm/bin/path
SYSROOT_DIR ?= /change/to/aarch64-linux-gnu/sysroot/directory

CC = $(LLVM_BIN)/clang++ --target=aarch64-linux-gnu -fuse-ld=lld --sysroot=$(AARCH64_SYSROOT)
AR = $(LLVM_BIN)/llvm-ar

CFLAGS     = -O3 --target=aarch64-linux-gnu --sysroot=$(SYSROOT_DIR) 
INC_FLAGS  = -I./include

SRC_DIR   = ./code
BUILD_DIR = ./build
OBJS_DIR  = $(BUILD_DIR)/objs
LIB_DIR   = $(BUILD_DIR)/lib

TARGET    = libQarma64.a 

SRC_FILES     = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES     = $(patsubst $(SRC_DIR)/%.cpp, $(OBJS_DIR)/%.o, $(SRC_FILES))
DEPENDENCIES  = $(OBJ_FILES:.o=.d)

# Build rules
$(LIB_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $(OBJ_FILES)


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC_FLAGS) -MMD -MP -c $< -o $@

# Phony targets
.PHONY: info clean

info: 
	$(info $(SRC_FILES))
	$(info $(OBJ_FILES))
	$(info $(DEPENDENCIES))

# Clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPENDENCIES)