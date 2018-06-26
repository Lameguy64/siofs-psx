# SioFS-PSX
Client library for the original PlayStation for accessing files from a PC host via the serial port using the SioFS protocol. Supports most commonly used file read and write operations and listing directory contents.

## Compiling
This library and example program can only be compiled using the PsyQ or Programmer's Tool SDK. PSXSDK is not supported due to the lack of serial support in that toolchain.

Use mingw32-make from the Mingw32 toolchain to process the makefile. MSys' make has issues with psylib and Cygwin may work but its not tested.

## SioFS Host Programs
A SioFS host program is needed for this library to be able to access files from your PC. The only tool that supports the protocol is currently [mcomms](https://github.com/Lameguy64/mcomms).