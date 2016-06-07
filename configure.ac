#
# Copyright 2016 Dan Molik <dan@d3fy.net>
#
# This file is part of Pingd
#
# Pingd is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Pingd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Pingd.  If not, see <http://www.gnu.org/licenses/>.
#


AC_PREREQ(2.63)
AC_INIT([pingd], [0.0.1], [software@d3fy.net], , [https://github.com/d3fy/pingd/])

AC_CONFIG_SRCDIR([src/pingd.h])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC
AC_PROG_YACC
AC_PROG_LEX
AC_PROG_INSTALL

AC_HAVE_LIBRARY(pthread,,     AC_MSG_ERROR(Missing pthread library))

AC_C_INLINE

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
