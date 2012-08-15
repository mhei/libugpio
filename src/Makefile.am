#
# Copyright © 2012 Michael Heimpold <mhei@heimpold.de>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

lib_LTLIBRARIES = libugpio.la
libugpio_la_SOURCES = \
        ugpio.c \
        ugpio.h \
        ugpio-version.h

libugpio_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBUGPIO_LT_VERSION_INFO) \
                      -no-undefined -export-dynamic

libugpioincludedir = $(includedir)/upgio
libugpioinclude_HEADERS = ugpio.h ugpio-version.h

DISTCLEANFILES = ugpio-version.h
EXTRA_DIST = ugpio-version.h.in
CLEANFILES = *~
