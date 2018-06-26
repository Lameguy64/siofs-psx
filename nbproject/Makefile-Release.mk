#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=ccpsx
CCC=ccpsx
CXX=ccpsx
FC=gfortran
AS=asmpsx

# Macros
CND_PLATFORM=SDevTC_PSX_Compiler-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/6ba9c16/main.o \
	${OBJECTDIR}/graphics.o \
	${OBJECTDIR}/lib/comm.o \
	${OBJECTDIR}/lib/siofs.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/siofs.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/siofs.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/siofs ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/6ba9c16/main.o: /C/Users/Lameguy64/Desktop/Projects/siofs/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/6ba9c16
	$(COMPILE.cc) -O2 -o ${OBJECTDIR}/_ext/6ba9c16/main.o /C/Users/Lameguy64/Desktop/Projects/siofs/main.cpp

${OBJECTDIR}/graphics.o: graphics.cpp 
	${MKDIR} -p ${OBJECTDIR}
	$(COMPILE.cc) -O2 -o ${OBJECTDIR}/graphics.o graphics.cpp

${OBJECTDIR}/lib/comm.o: lib/comm.c 
	${MKDIR} -p ${OBJECTDIR}/lib
	$(COMPILE.c) -O2 -o ${OBJECTDIR}/lib/comm.o lib/comm.c

${OBJECTDIR}/lib/siofs.o: lib/siofs.c 
	${MKDIR} -p ${OBJECTDIR}/lib
	$(COMPILE.c) -O2 -o ${OBJECTDIR}/lib/siofs.o lib/siofs.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/siofs.exe

# Subprojects
.clean-subprojects:
