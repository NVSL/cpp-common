all: tests

.PHONY: tests
tests:
	$(MAKE) -C tests
	tests/test.bin  --gmock_verbose=info --gtest_stack_trace_depth=10

clean:
	$(MAKE) -C tests clean
