# Small Basic Computer

# What is this?

This is a re-implementation of Petit Computer (again) but with some different
goals from PTC-EmkII.

The goals of this program are roughly the following (in priority order)
* Lower memory usage compared to PTC
* Performance comparable to PTC
* Compatibility with PTC

The goal of this is to write a cross-platform "compatible with PTC" interpreter
that can work on both computer and on a DS(i).

This BASIC implementation also includes some extra functions for memory access.

# How do I build it?

See build.md

# Usage

NDS: Just launch the program for your flashcart kernel or TWiLight Menu.
PC: Execute `./sbc` to launch the program. SBC will start with an interpreter prompt, if everything worked correctly. From here, you can use LOAD/RUN or EXEC to launch programs.

To autostart a program, you can optionally specify a PTC file, using `./sbc your_ptc_file.PTC`. This skips the interpreter prompt and executes the program immediately.

# Licenses

On NDS, this program relies on devkitARM and libfat, provided by devkitPro: https://devkitpro.org/
The license is listed here: https://github.com/devkitPro/libnds/blob/master/libnds_license.txt
Libfat individually can be found here, and has the license at the top of various source files: https://github.com/devkitPro/libfat

On PC, this program uses SFML with the CSFML binding, licensed under the zlib license: https://www.sfml-dev.org/

For extracting resources, this program uses ndstool under GPLv3. See the submodule for source + info.
