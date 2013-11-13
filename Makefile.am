ACLOCAL_AMFLAGS=-I m4

AUTOMAKE_OPTIONS = foreign

if ENABLE_UNITTEST
SUBDIRS = utils debug test tools
else
SUBDIRS = optimized tools
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mclinker.pc

EXTRA_DIST = ./docs/MCLinker.dia ./autogen.sh mclinker.pc.in
DISTCLEANFILES = mclinker.pc
