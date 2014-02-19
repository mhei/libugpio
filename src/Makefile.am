#
# Copyright © 2012-2014 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: LGPL-2.1+
#

lib_LTLIBRARIES = libugpio.la
libugpio_la_SOURCES = \
        ugpio.c \
        gpio.c \
        ugpio.h \
        ugpio-internal.h \
        ugpio-version.h

libugpio_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBUGPIO_LT_VERSION_INFO) \
                      -no-undefined -export-dynamic

libugpioincludedir = $(includedir)/ugpio
libugpioinclude_HEADERS = ugpio.h ugpio-version.h

DISTCLEANFILES = ugpio-version.h
EXTRA_DIST = ugpio-version.h.in
CLEANFILES = *~
