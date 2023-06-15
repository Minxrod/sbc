# https://stackoverflow.com/questions/10024279/how-to-use-shell-commands-in-makefile
# https://stackoverflow.com/questions/10276202/exclude-source-file-in-compilation-using-makefile
objects = $(shell ls source/*.c)
objs = $(filter-out source/main.c, $(objects))

test: source/*.c source/*.h
	echo $(objs)
	gcc -g -Wall -Werror -Wextra -Wpedantic -Isource/ $(objs) -o test

.phony clean:
	rm test
