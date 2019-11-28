#
# Copyright © 2012-2019 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src

AM_CFLAGS   = -Wall -pedantic

bin_PROGRAMS            = gpioctl

gpioctl_SOURCES         = gpioctl.c
gpioctl_LDADD           = $(top_builddir)/src/libugpio.la
