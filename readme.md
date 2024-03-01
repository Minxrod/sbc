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

## NDS/DSi

Requires devkitARM to be installed first. You can find instructions for setting up here: https://devkitpro.org/wiki/Getting_Started

Once installed, from the project root run `make -f Makefile`, and copy both the output .nds and the resources/ directory to the same location. Example:
```sh
cd sbc/
make -f Makefile
cp *.nds 'path/to/your/sd/'
cp -r resources 'path/to/your/sd/.'
```
You will additionally need the various graphical resources not provided. To dump these from a copy of PTC, see instructions here: https://github.com/Minxrod/PTC-EmkII?tab=readme-ov-file#first-time-setup
The expected file structure is flattened here: simply move all graphical files to the resources/ folder.

This program has been tested with melonDS and on an R4.

## Computers

While this program may compile with other compilers, it was only tested with gcc on Debian 12. If it does not work with your compiler, please create an issue and provide OS/compiler version information.

For the PC version, first install SFML and CSFML for your platform. Note that this program was built for SFML 2.5.1, and so the version you need will be found here: https://www.sfml-dev.org/download/sfml/2.5.1/.
For CSFML, the 2.5.1 bindings can be downloaded here: https://www.sfml-dev.org/download/csfml/, or you can build from source (located at https://github.com/SFML/CSFML).

Once SFML and CSFML are installed, you will need to edit the makefile (with a lowercase 'm') and modify the CSFML path to point to your installation. Once this is done, you should be able to compile with
```sh
cd sbc/
make -f makefile
```
If it works, the output executable file will be called 'test'. You can provide the necessary resource files in the resources/ directory.

# Licenses

On NDS, this program relies on devkitARM and libfat, provided by devkitPro: https://devkitpro.org/
The license is listed here: https://github.com/devkitPro/libnds/blob/master/libnds_license.txt
Libfat individually can be found here, and has the license at the top of various source files: https://github.com/devkitPro/libfat

On PC, this program uses SFML with the CSFML binding, licensed under the zlib license: https://www.sfml-dev.org/
