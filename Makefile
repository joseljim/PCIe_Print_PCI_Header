# Makefile for The PCI Utilities
# (c) 1998--2020 Martin Mares <mj@ucw.cz>

OPT=-O2
CFLAGS=$(OPT) -Wall -W -Wno-parentheses -Wstrict-prototypes -Wmissing-prototypes -g

VERSION=3.7.0
DATE=2020-05-31

# Host OS and release (override if you are cross-compiling)
HOST=
RELEASE=
CROSS_COMPILE=

# Support for compressed pci.ids (yes/no, default: detect)
ZLIB=

# Support for resolving ID's by DNS (yes/no, default: detect)
DNS=

# Build libpci as a shared library (yes/no; or local for testing; requires GCC)
SHARED=no

# Use libkmod to resolve kernel modules on Linux (yes/no, default: detect)
LIBKMOD=

# Use libudev to resolve device names using hwdb on Linux (yes/no, default: detect)
HWDB=

# ABI version suffix in the name of the shared library
# (as we use proper symbol versioning, this seldom needs changing)
ABI_VERSION=.3

# Installation directories
PREFIX=/usr/local
SBINDIR=$(PREFIX)/sbin
SHAREDIR=$(PREFIX)/share
IDSDIR=$(SHAREDIR)
MANDIR:=$(shell if [ -d $(PREFIX)/share/man ] ; then echo $(PREFIX)/share/man ; else echo $(PREFIX)/man ; fi)
INCDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
PKGCFDIR=$(LIBDIR)/pkgconfig

# Commands
INSTALL=install
DIRINSTALL=install -d
STRIP=-s
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib

# Base name of the library (overridden on NetBSD, which has its own libpci)
LIBNAME=libpci

-include lib/config.mk

PCIINC=lib/config.h lib/header.h lib/pci.h lib/types.h lib/sysdep.h
PCIINC_INS=lib/config.h lib/header.h lib/pci.h lib/types.h

export

all:  pciheader

lib/$(PCILIB): $(PCIINC) force
	$(MAKE) -C lib all

force:

lib/config.h lib/config.mk:
	cd lib && ./configure

# pciheader
pciheader: pciheader.o lib/$(PCILIB)
pciheader.o: pciheader.c $(PCIINC)

%: %.o
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

ctags:
	rm -f tags
	find . -name '*.[hc]' -exec ctags --append {} +

TAGS:
	rm -f TAGS
	find . -name '*.[hc]' -exec etags --append {} +

clean:
	rm -f `find . -name "*~" -o -name "*.[oa]" -o -name "\#*\#" -o -name TAGS -o -name core -o -name "*.orig"`
	rm -f lib/config.* pciheader


.PHONY: all clean
