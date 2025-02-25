# Copyright (C) 2025 The Regents of the University of California
# Authored by David W. Shattuck, Ph.D.
SHELL := /bin/bash
BaseDir = ./
Name    := $(shell basename `pwd`)
ifndef MACHTYPE
MACHTYPE:=$(shell set | grep ^MACHTYPE= | sed s-.*=--)
endif
VersionNum = 25a
Includes = $(LocalIncludes) -I$(BaseDir)include -Ivol3d/include 
LongName = lib$(Name)$(VersionNum)_$(MACHTYPE).a
ShortName = lib$(Name)$(VersionNum).a
LibDir ?= $(BaseDir)lib/$(MACHTYPE)

CC := clang++ -O3 $(DEBUG) $(BUILDVERSION) -std=c++20
InstallCmd = install -d

Target = $(LibDir)/$(LongName)
ObjDir ?= $(BaseDir)obj/$(MACHTYPE)/$(Name)/

CCExtension = .cpp
SrcFiles := $(wildcard *$(CCExtension))
ObjFiles := $(addprefix $(ObjDir),$(SrcFiles:$(CCExtension)=.o))

all: DirCheck $(Target)

DirCheck: $(LibDir) $(ObjDir)

$(ObjDir):
	$(InstallCmd) $(ObjDir)
	echo Installing $(ObjDir)

$(ObjDir)%.o: %$(CCExtension)
	$(CC) $(Includes) -c $< -o $@

install: $(ObjDir) $(Target)

$(LibDir):
	$(InstallCmd) $(LibDir) 

$(Target): $(LibDir) $(ObjFiles)
	$(AR) r $(Target) $(ObjFiles) $(AuxObjs)
	(cd $(LibDir); ln -f -s $(LongName) $(ShortName))

run:

link: deltarget $(Target)

deltarget:
	rm -f $(Target)

depend:
	makedepend  -p$(ObjDir) -f makedep $(Includes) *$(CCExtension)
	rm -f makedep.bak

clean:
	rm -f $(ObjFiles)

distclean: clean
	rm -f $(Target) makedep

makedep:
	touch makedep

include makedep


