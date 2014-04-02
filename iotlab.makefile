.PHONY: tests compile_tests clean

tests: compile_tests

compile_tests:
	bash test_all_platforms.sh

clean:
	rm -rf build.*
