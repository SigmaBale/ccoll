UNIT_TEST_DIRS = ctree cvec clist

all: $(UNIT_TEST_DIRS)

$(UNIT_TEST_DIRS):
	@$(MAKE) -C $@

clean:
	@$(foreach dir,$(UNIT_TEST_DIRS),$(MAKE) -C $(dir) clean;)

test:
	@$(foreach dir,$(UNIT_TEST_DIRS),$(MAKE) -C $(dir) test;)

test-leak:
	@$(foreach dir,$(UNIT_TEST_DIRS),$(MAKE) -C $(dir) test-leak;)


.PHONY: all clean test $(UNIT_TEST_DIRS)
