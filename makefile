# I still don't know how to make a good makefile...
# https://stackoverflow.com/questions/10276202/exclude-source-file-in-compilation-using-makefile
# https://www.gnu.org/software/make/manual/make.html#Foreach-Function
# https://stackoverflow.com/questions/63305865/makefile-create-a-build-directory
# ^ all of my environments expect unix commands so this is fine
# https://stackoverflow.com/questions/44754190/how-to-use-gccs-i-flag-to-include-multiple-folders

# Source, build settings
SOURCE = source source/graphics/pc source/graphics source/interpreter source/subsystem tests
BUILD = build/
CSFML = /home/minxrod/Documents/source/notmine/CSFML/
CSFML_INCLUDE = $(CSFML)include/
CSFML_LIB = $(CSFML)lib/

# Compiler settings
CC = gcc
# https://stackoverflow.com/questions/1867065/how-to-suppress-gcc-warnings-from-library-headers
# -isystem needed for SFML, which throws a deprecation warning (error) otherwise
# The -MMD is important. It generates the actual dependencies...
CFLAGS = -g -std=c11 -Wall -Werror -Wextra -Wpedantic -isystem$(CSFML_INCLUDE) $(foreach srcdir,$(SOURCE),-I$(srcdir)) -DPC -MMD
# https://stackoverflow.com/a/10168396
LFLAGS += -Wl,-rpath,$(CSFML_LIB) -L$(CSFML_LIB)
# All the libraries that need to be linked
LIBFLAGS = -lm -lcsfml-graphics -lcsfml-window -lcsfml-system

# Some magic makefile nonsense
# get source files list 
srcfiles = $(foreach srcdir,$(SOURCE),$(wildcard $(srcdir)/*.c))
# convert to object files list (remove 'main' files)
objects = $(filter-out source/main.o tests/test_main.o source/graphics/pc/opengl_test.o, $(srcfiles:%.c=%.o))
# convert to build location list
build = $(objects:%=$(BUILD)%)

main_objs = $(build) $(BUILD)source/main.o
test_objs = $(build) $(BUILD)tests/test_main.o

$(BUILD)%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(main_objs)
#	echo $(objects)
#	echo $(build)
	# TODO: Better CSFML library location? (How do you install it?)
	$(CC) $(CFLAGS) $(LFLAGS) $(main_objs) -o test $(LIBFLAGS)
#	gcc $(CFLAGS) -Isource/ $(objs) source/main.c -o test

#gl:
#	$(CC) $(CFLAGS) $(LFLAGS) source/opengl_test.c -o test $(LIBFLAGS) -lGL

test: $(test_objs)
	$(CC) $(CFLAGS) $(LFLAGS) $(test_objs) -o test $(LIBFLAGS)
#	gcc $(CFLAGS) -I$(SOURCE) $(BUILD) tests/test_main.c -o test

.phony clean:
	rm -f $(main_objs) $(test_objs)
	rm -f test
	rm -f $(build:%.o=%.d)
	
# https://stackoverflow.com/questions/313778/generate-dependencies-for-a-makefile-for-a-project-in-c-c	
# https://stackoverflow.com/a/10168396
# this seems to be the simplest solution
-include $(build:%.o=%.d)
