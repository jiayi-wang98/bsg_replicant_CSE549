HB_HAMMERBENCH_PATH = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = tile-x_$(1)__tile-y_$(2)__image-size-x_$(3)__image-size-y_$(4)__warm-cache_$(5)

# call to get parameter from test name
get-tile-x = $(lastword $(subst _, ,$(filter tile-x_%,$(subst __, ,$(1)))))
get-tile-y = $(lastword $(subst _, ,$(filter tile-y_%,$(subst __, ,$(1)))))
get-image-size-x = $(lastword $(subst _, ,$(filter image-size-x_%,$(subst __, ,$(1)))))
get-image-size-y = $(lastword $(subst _, ,$(filter image-size-y_%,$(subst __, ,$(1)))))
get-warm-cache = $(lastword $(subst _, ,$(filter warm-cache_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo tile-x = $(call get-tile-x,$*) >> $@
	@echo tile-y = $(call get-tile-y,$*) >> $@
	@echo image-size-x = $(call get-image-size-x,$*) >> $@
	@echo image-size-y = $(call get-image-size-y,$*) >> $@
	@echo warm-cache = $(call get-warm-cache,$*) >> $@

include $(HB_HAMMERBENCH_PATH)/mk/testbench_common.mk
