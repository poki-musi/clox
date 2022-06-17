# ---
TARGET_EXEC := clox
BUILD_DIR := bin
SRC_DIR := src
TEST_DIR := tests

SRCS := $(shell find $(SRC_DIR) -name *.c)
# TESTS := $(shell find $(TEST_DIR) -name *.c)

RM := rm -f
CFLAGS := -g -Iinc/ -Wall
# ---

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
MAINS := $(BUILD_DIR)/$(SRC_DIR)/main.c.o
# EXEC_TESTS := $(TESTS:%=$(BUILD_DIR)/%.test.out)

MKDIR_P := mkdir -p

default: build

#
# --- Build Rule ---
#

build: $(BUILD_DIR)/$(TARGET_EXEC)

# Executable
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

#
# --- Test Rule ---
#

# test: $(EXEC_TESTS)

# Executable
# $(BUILD_DIR)/%.c.test.out: $(BUILD_DIR)/%.c.o $(filter-out $(MAINS),$(OBJS))
# 	@echo
# 	@echo ":: $(strip $(subst $(BUILD_DIR)/$(TEST_DIR)/, , $@)) ::"
# 	@$(MKDIR_P) $(dir $@)
# 	@$(CC) $(LDFLAGS) -o $@ $^
# 	@$@ && echo ":: PASS ::" || echo ":: FAIL ::"

#
# --- Other ---
#

# C source
$(BUILD_DIR)/%.c.o: %.c
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@$(RM) -r bin
