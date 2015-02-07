dnl
dnl Copyright © 2012-2015 Michael Heimpold <mhei@heimpold.de>
dnl
dnl SPDX-License-Identifier: LGPL-2.1+
dnl

dnl
dnl Process this file with autoconf to produce a configure script.
dnl

dnl libugpio package version number, (as distinct from shared library version)
dnl An odd micro number indicates in-progress development from Git
dnl An even micro number indicates a released version
dnl
dnl Making a point release:
dnl - increase libugpio_version_micro to the next even number
dnl
dnl After the release:
dnl - increase libugpio_version_minor to the next odd number
dnl
dnl Take care to update the libtool versioning when required (LIBUGPIO_LD_*).
dnl http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html
dnl
m4_define([libugpio_version_major], [0])
m4_define([libugpio_version_minor], [0])
m4_define([libugpio_version_micro], [6])

m4_define([libugpio_release_status],
    [m4_if(m4_eval(libugpio_version_minor % 2), [1], [snapshot], [release])])

m4_define([libugpio_version],
    [libugpio_version_major.libugpio_version_minor.libugpio_version_micro])

AC_PREREQ([2.63])
AC_INIT([libugpio], [libugpio_version], [https://github.com/mhei/libugpio/issues])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2])
# enable nice build output on automake1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

LIBUGPIO_VERSION_MAJOR=libugpio_version_major
LIBUGPIO_VERSION_MINOR=libugpio_version_minor
LIBUGPIO_VERSION_MICRO=libugpio_version_micro
LIBUGPIO_VERSION=libugpio_version
AC_SUBST(LIBUGPIO_VERSION_MAJOR)
AC_SUBST(LIBUGPIO_VERSION_MINOR)
AC_SUBST(LIBUGPIO_VERSION_MICRO)
AC_SUBST(LIBUGPIO_VERSION)

# ABI version
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
LIBUGPIO_LD_CURRENT=1
LIBUGPIO_LD_REVISION=0
LIBUGPIO_LD_AGE=0
LIBUGPIO_LT_VERSION_INFO=$LIBUGPIO_LD_CURRENT:$LIBUGPIO_LD_REVISION:$LIBUGPIO_LD_AGE
AC_SUBST(LIBUGPIO_LT_VERSION_INFO)

PACKAGE_DESCRIPTION="Library for using the sysfs gpio api"
AC_SUBST(PACKAGE_DESCRIPTION)

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
LT_INIT
# Checks for header files
AC_HEADER_STDC

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/ugpio-version.h
        tests/Makefile
        libugpio.pc
])
AC_OUTPUT
