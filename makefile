# I still don't know how to make a good makefile...
# https://stackoverflow.com/questions/10276202/exclude-source-file-in-compilation-using-makefile
# https://www.gnu.org/software/make/manual/make.html#Foreach-Function
# https://stackoverflow.com/questions/63305865/makefile-create-a-build-directory
# ^ all of my environments expect unix commands so this is fine
# https://stackoverflow.com/questions/44754190/how-to-use-gccs-i-flag-to-include-multiple-folders

# Source, build settings
SOURCE = source source/graphics/pc source/graphics source/interpreter source/subsystem source/extension
TESTS = tests
BUILD = build/
# Installed as git submodule
CSFML = util/external/CSFML/
CSFML_INCLUDE = $(CSFML)include/
CSFML_LIB = $(CSFML)lib/

# Compiler settings
CC = gcc
# https://stackoverflow.com/questions/1867065/how-to-suppress-gcc-warnings-from-library-headers
# -isystem needed for SFML, which throws a deprecation warning (error) otherwise
# The -MMD is important. It generates the actual dependencies...
CFLAGS = -std=c11 -Wall -Werror -Wextra -Wpedantic -isystem$(CSFML_INCLUDE) $(foreach srcdir,$(SOURCE) $(TESTS),-I$(srcdir)) -DPC -MMD
# https://stackoverflow.com/a/10168396
# indicates location of CSFML shared object files
LFLAGS += -Wl,-rpath,$(CSFML_LIB) -L$(CSFML_LIB)
# All the libraries that need to be linked
LIBFLAGS = -lm -lcsfml-audio -lcsfml-graphics -lcsfml-window -lcsfml-system

# Some magic makefile nonsense
# get source files list 
srcfiles = $(foreach srcdir,$(SOURCE),$(wildcard $(srcdir)/*.c))
testfiles = $(foreach srcdir,$(TESTS),$(wildcard $(srcdir)/*.c))
# convert to object files list (remove 'main' files)
src_objects = $(filter-out source/main.o source/graphics/pc/opengl_test.o, $(srcfiles:%.c=%.o))
test_objects = $(filter-out tests/test_main.o, $(testfiles:%.c=%.o))
# convert to build location list
build = $(src_objects:%=$(BUILD)%)
test_build = $(test_objects:%=$(BUILD)%)

main_objs = $(build) $(BUILD)source/main.o
test_objs = $(build) $(test_build) $(BUILD)tests/test_main.o

$(BUILD)%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.phony: debug release test profile

main: debug
debug: CFLAGS+=-g
debug: CFLAGS+=-pg
release: CFLAGS+=-DNDEBUG
release: CFLAGS+=-O2
debug release: main_build
profile: CFLAGS+=--coverage
profile: LFLAGS+=-lgcov
profile: main_build
test: CFLAGS+=-g
test: CFLAGS+=-pg
#test: LFLAGS+=-lgcov
compsize: CFLAGS+=-DNDEBUG


main_build: $(main_objs) $(BUILD)csfml
	$(CC) $(CFLAGS) $(LFLAGS) $(main_objs) -o sbc $(LIBFLAGS)

# gl:
#	$(CC) $(CFLAGS) $(LFLAGS) source/graphics/pc/opengl_test.c -o test $(LIBFLAGS) -lGL

# compsize: $(main_objs)
#	$(CC) $(CFLAGS) $(LFLAGS) $(build) compiled_size.c -o test $(LIBFLAGS)

# TODO:CODE:NONE csfml probably shouldn't be a dependency here
test: $(test_objs) csfml
	$(CC) $(CFLAGS) $(LFLAGS) $(test_objs) -o test $(LIBFLAGS)

ntr_to_ptc:
	$(CC) util/ntr_to_ptc.c -o util/ntr_to_ptc

# builds ndstool only if it wasn't built already
ndstool: util/external/ndstool/ndstool
util/external/ndstool/ndstool:
	cd util/external/ndstool/ && ./autogen.sh && ./configure && make

# just check for one of the built libraries
csfml: util/external/CSFML/lib/libcsfml-system.so
util/external/CSFML/lib/libcsfml-system.so:
	cd util/external/CSFML/ && cmake . && make

resource: ntr_to_ptc $(BUILD)ndstool
	./util/prepare_resource "$(NDS_FILE)" "extract/"

clean:
	rm -f $(main_objs) $(test_objs)
	rm -f test sbc
	rm -f $(build:%.o=%.d)
	rm -f util/ntr_to_ptc
	
# https://stackoverflow.com/questions/313778/generate-dependencies-for-a-makefile-for-a-project-in-c-c	
# https://stackoverflow.com/a/10168396
# this seems to be the simplest solution
-include $(build:%.o=%.d)
