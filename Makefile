LIB = build/release/libcoll.so
INC_DIRS = src include
TEST_DIR = test
SRC_DIR = src
OBJ_DIR = build/objects

HEADER_INSTALL_DIR = /usr/local/include
LIB_INSTALL_DIR = /usr/local/lib

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
DEP_FILES := $(patsubst %.o,%.d,$(OBJ_FILES))

CC = gcc
INCLUDES := $(foreach dir,$(INC_DIRS),-I$(dir))
DEPFLAGS = -MD -MP
OPT_BUILD = -Os
CFLAGS := -Wall -Werror -Wextra $(OPT_BUILD) $(INCLUDES) $(DEPFLAGS)

all: $(LIB)

$(LIB): $(OBJ_FILES)
	@$(CC) -shared -o $@ $^

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c -fPIC $< -o $@ 

clean:
	rm -rf $(LIB) $(OBJ_FILES) $(DEP_FILES)
	@$(MAKE) -C $(TEST_DIR) clean

unit-test test: $(LIB)
	@$(MAKE) -C $(TEST_DIR) test

integration-test: $(LIB)
	@$(MAKE) -C $(TEST_DIR) integration-test

test-leak unit-test-leak: $(LIB)
	@$(MAKE) -C $(TEST_DIR) unit-test-leak

integration-test-leak: $(LIB)
	@$(MAKE) -C $(TEST_DIR) integration-test-leak

install: $(LIB)
	@sudo cp -i $(LIB) $(LIB_INSTALL_DIR)
	@sudo cp -i include/collib.h $(HEADER_INSTALL_DIR)

-include $(DEP_FILES)

.PHONY: all clean test unit-test integration-test unit-test-leak integration-test-leak test-leak install
