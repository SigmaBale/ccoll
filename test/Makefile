TEST_DIRS := $(wildcard */.)

all: $(TEST_DIRS)
	$(eval TEST_EXECUTABLES := $(foreach Dir,$(TEST_DIRS),$(wildcard $(Dir)/*_test)))
	$(foreach test,$(TEST_EXECUTABLES),./$(test))

$(TEST_DIRS):
	$(MAKE) -C $@

clean:
	$(foreach dir,$(TEST_DIRS),$(MAKE) -C $(dir) clean)

.PHONY: all $(TEST_DIRS) clean
