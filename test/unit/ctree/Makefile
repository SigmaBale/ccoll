EXECUTABLE_TEST_DIRS = insert remove

all: $(EXECUTABLE_TEST_DIRS)

$(EXECUTABLE_TEST_DIRS):
	@$(MAKE) -C $@

clean:
	@$(foreach dir,$(EXECUTABLE_TEST_DIRS),$(MAKE) -C $(dir) clean;)

test:
	@$(foreach dir,$(EXECUTABLE_TEST_DIRS),$(MAKE) -C $(dir) test;)

.PHONY: all clean test $(EXECUTABLE_TEST_DIRS)
