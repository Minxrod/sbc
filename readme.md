# Small Basic Computer

# What is this?

This is a re-implementation of Petit Computer (again) but with some different
goals from PTC-EmkII.

The goals of this program are roughly the following (in priority order)
* Low memory usage
* Performance
* Compatibility

The goal of this is to write a cross-platform "compatible with PTC" interpreter
that can work on both computer and on a DS(i).

# How do I run it?

There are two targets currently.
`make test` - build tests
`make` - build regular program

## NDS/DSi

Install devkitARM, run `make -f Makefile`, and see if the output .nds works
on your device.

Untested for several versions currently.
* TODO: Document test devices!

## Computers

There is also a makefile for regular computer platforms. (i.e. Windows, Linux)
This targets `gcc -std=c11`, but may work with other compilers as well. If it
doesn't, create an issue and I'll see if I can fix it.

To build the main program for the computer target, use `make -f makefile`.
To build the test cases, use `make -f makefile test`.
You will need to specify the SFML install path currently, and build CSFML
(located here: https://github.com/SFML/CSFML)

