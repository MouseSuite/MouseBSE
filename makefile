# Copyright (C) 2025 The Regents of the University of California
# Authored by David W. Shattuck, Ph.D.
SHELL := /bin/bash
ifndef MACHTYPE
MACHTYPE:=$(shell set | grep ^MACHTYPE= | sed s-.*=--)
endif
VersionNum = 25a
Name=mousebse
InstName ?= mousebse

BUILDVERSION := -D'BUILDVERSION="$(shell git rev-parse --short HEAD)"' # delete this line and remove $(BUILDVERSION) for release

CC := clang++ -O3 $(DEBUG) $(BUILDVERSION) -std=c++20
InstallCmd = install -d
BaseDir := ./
Includes = $(LocalIncludes) -I. -Ivol3d/include 

BinDir = ${BaseDir}bin/$(MACHTYPE)
ObjDir = $(BaseDir)obj/$(MACHTYPE)/$(Name)/

LongName = $(Name)$(VersionNum)_$(MACHTYPE)
Target = $(BinDir)/$(LongName)

LocalLibDirs = $(LibDirs) -Lvol3d/lib/$(MACHTYPE) 

InstallDir ?= $(HOME)/$(InstName)$(VersionNum)/bin/$(MACHTYPE)

CCExtension = .cpp
SrcFiles := $(wildcard *$(CCExtension)) $(LocalSrcFiles)
ObjFiles := $(addprefix $(ObjDir),$(SrcFiles:$(CCExtension)=.o))
Vol3DLib := vol3d/lib/$(MACHTYPE)/libvol3d25a.a


all: DirCheck $(Target)

DirCheck: $(ObjDir) $(BinDir)

$(ObjDir):
	$(InstallCmd) $(ObjDir)

$(InstallDir):
	$(InstallCmd) $(InstallDir)

$(BinDir):
	$(InstallCmd) $(BinDir)

$(ObjDir)%.o: %$(CCExtension)
	$(CC) $(Includes) -c $< -o $@

install: $(ObjDir) $(BinDir) $(Target) $(InstallDir)
	cp $(Target) $(InstallDir)
	(cd $(InstallDir); ln -f -s $(LongName) $(Name); ln -f -s $(LongName) $(Name)$(VersionNum))

$(Target): $(ObjDir) $(BinDir) $(ObjFiles) $(Vol3DLib)
	$(CC) $(LocalLibDirs) $(ObjFiles) $(AuxObjs) -o $(Target) $(LocalLibs) -lvol3d25a -lm -lz

lib: $(Vol3DLib)

$(Vol3DLib):
	make -C vol3d

run: $(Target)
	$(Target)

build: $(Target)

link: deltarget $(Target)

deltarget:
	rm -f $(Target)

depend:
	makedepend  -p$(ObjDir) -f makedep $(Includes) *$(CCExtension)
	rm -f makedep.bak

clean:
	rm -f $(ObjFiles)

makedep:
	touch makedep

distclean: clean
	rm -f $(Target) makedep
	make -C vol3d distclean

include makedep

