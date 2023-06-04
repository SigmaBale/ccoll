# Here should only be 2 dirs
# Integration directory for integration tests
# and unit dirctory for unit tests
INTEGRATION = integration
UNIT = unit

all: $(INTEGRATION) $(UNIT)
	@$(MAKE) -C $(INTEGRATION)
	@$(MAKE) -C $(UNIT)

$(UNIT) $(INTEGRATION):
	@$(MAKE) -C $@

clean: clean-unit clean-integration

clean-unit:
	@$(MAKE) -C $(UNIT) clean

clean-integration:
	@$(MAKE) -C $(INTEGRATION) clean

test unit-test:
	@$(MAKE) -C $(UNIT) test

integration-test:
	@$(MAKE) -C $(INTEGRATION) test

test-leak unit-test-leak:
	@$(MAKE) -C $(UNIT) test

integration-test-leak:
	@$(MAKE) -C $(INTEGRATION) test

.PHONY: all clean clean-unit clean-integration unit-test integration-test test unit-test-leak integration-test-leak
