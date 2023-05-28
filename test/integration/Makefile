INTEGRATION_TEST_DIRS =

all: $(INTEGRATION_TEST_DIRS)

$(INTEGRATION_TEST_DIRS):
	@$(MAKE) -C $@

clean:
	@$(foreach dir,$(INTEGRATION_TEST_DIRS),$(MAKE) -C $(dir) clean;)

test:
	@$(foreach dir,$(INTEGRATION_TEST_DIRS),$(MAKE) -C $(dir) test;)

.PHONY: all clean test $(INTEGRATION_TEST_DIRS)
