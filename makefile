#---------------------------------------------------------------------------------
# General-purpose makefile for PlayStation Homebrew projects by Lameguy64
# 2016 Meido-Tek Productions
#
# NOTE: This makefile is for use only with the official Sony Programmer's Tool
# PlayStation SDK and will not work with PSXSDK.
#
# Supports compiling C and assembly source files as well as disc image builds
# for convenience (requires mkpsxiso).
#
# NOTE: Do not use PSYMAKE to parse this makefile! Instead, setup msys for the
# *nix style commands:
# http://www.mingw.org/wiki/msys
#
# Then use mingw32-make included with the MinGW GCC Compiler.
#
#---------------------------------------------------------------------------------

TARGET		= siofs
PROGADDR	= 0x80010000

SOURCES		=

INCLUDES	= lib

LIBDIRS		= lib

LIBS		= libsiofs

CFLAGS		= -Xm -Wall -O2

CC			= ccpsx
ASM			= asmpsx

CFILES		= $(notdir $(wildcard ./*.c)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES	= $(notdir $(wildcard ./*.cpp)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
AFILES		= $(notdir $(wildcard ./*.s)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))

OFILES		= $(CFILES:.c=.obj) $(CPPFILES:.cpp=.obj) $(AFILES:.s=.obj)

all: $(OFILES)
	$(MAKE) -C lib
	$(CC) -Xo$(PROGADDR) $(CFLAGS) $(addprefix -L,$(LIBDIRS)) $(addprefix -l,$(LIBS)) $(OFILES) -o $(TARGET).cpe,,$(TARGET).map
	cpe2x $(TARGET).cpe
	
%.obj: %.c
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

%.obj: %.cpp
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

clean:
	$(MAKE) -C lib clean
	rm -f $(OFILES) $(TARGET).exe $(TARGET).cpe $(TARGET).map

cleanall: clean
