CC = /home/kamel/old/kamel/cybersecurity-project/llvm-project-base/build/bin/clang++ --target=aarch64-linux-gnu -fuse-ld=lld --sysroot=$(AARCH64_SYSROOT)
AR = /home/kamel/old/kamel/cybersecurity-project/llvm-project-base/build/bin/llvm-ar

CFLAGS     = -g --target=aarch64-linux-gnu --sysroot=/home/kamel/old/kamel/cybersecurity-project/qemu-arm64-machine/debian/aarch64-sysroot/
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