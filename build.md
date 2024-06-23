Note: PC build processes only tested on Debian 12. While this may work on Windows, it might require some extra work. Some testing is needed here.

# Source code
Download the source code. If you have `git` installed, the simplest way to do this is
```
git clone https://github.com/Minxrod/sbc --recurse-submodules
```

## Dependences
You will also need the dependencies. Those listed are intended for Debian, but the process should be basically the same for other distro's package managers.

PC specific dependencies:
```sh
sudo apt install libsfml-dev
```

Resources to build dependences:
```sh
sudo apt install autoconf cmake
```

# Extracting resources

To automatically extract the files needed from your copy of PTC, execute
```sh
make resource NDS_FILE='path/to/ptc/file'
```
from the project root. This only needs to be executed once, as these files never change.
`make resource` also extracts the sample programs to `programs/`.

If you have previously used PTC-EmkII, you can copy the files from `resources/graphics/` and `resources/ui/` to SBC's `resources/` folder, which also works. This does not include the sample programs.

# Building for NDS/DSi

Requires devkitARM to be installed first. You can find instructions for setting up here: https://devkitpro.org/wiki/Getting_Started

Once installed, from the project root run `make -f Makefile`, and copy both the output .nds and the resources/ directory to the same location. Example:
```sh
# cd sbc/
make -f Makefile
cp *.nds 'path/to/your/sd/'
cp -r resources 'path/to/your/sd/.'
```

To add programs and resources to run/load, place your .PTC files in the programs/ directory. The file format can be PTC SD file format or PTC's internal format.

This program has been tested with melonDS and on an R4.

# Building for PC

While this program may compile with other compilers, it was only tested with `gcc` on Debian 12. If it does not work with your compiler, please create an issue and provide OS/compiler version information.

The build process builds CSFML from source as a submodule.
For the PC version, first install SFML for your platform as described in the dependencies section. Note that this program was built for SFML 2.5.1, and so the version you need will be found here, if on Windows: https://www.sfml-dev.org/download/sfml/2.5.1/.
For CSFML, the 2.5.1 bindings can also be downloaded here: https://www.sfml-dev.org/download/csfml/.

Once everything is installed, you can build with
```sh
# cd sbc/
make -f makefile
```
If it works, the output executable file will be called 'sbc'.

To add programs and resources to run/load, place your .PTC files in the programs/ directory.
