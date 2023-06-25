# https://stackoverflow.com/questions/10024279/how-to-use-shell-commands-in-makefile
# https://stackoverflow.com/questions/10276202/exclude-source-file-in-compilation-using-makefile
objects = $(shell ls source/*.c)
objs = $(filter-out source/main.c source/test.c, $(objects))

main: source/*.c source/*.h
	echo $(objs)
	gcc -g -Wall -Werror -Wextra -Wpedantic -Isource/ $(objs) -o test

test: source/*.c source/*.h tests/*.c
	echo $(objs)
	gcc -g -Wall -Werror -Wextra -Wpedantic -Isource/ $(objs) tests/test_main.c -o test

.phony clean:
	rm test
