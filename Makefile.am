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


AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I build

AM_CFLAGS  =
AM_CFLAGS += -Wall
AM_CFLAGS += -Wextra
AM_CFLAGS += -g
AM_CFLAGS += -pedantic
AM_CFLAGS += -std=gnu99
AM_CFLAGS += $(DEPS_CFLAGS)

core_headers :=
#core_headers += src/config/config_file.h
core_headers += src/util/logger.h
core_headers += src/util/daemonize.h
core_headers += src/pingd.h

core_src :=
#core_src += src/config/parse.y
#core_src += src/config/scanner.l
core_src += src/util/logger.c
core_src += src/util/daemonize.c
core_src += src/pingd.c

AM_LFLAGS = --header-file --yylineno
AM_YFLAGS = -d

#src/scanner.c: src/config/scanner.l src/server.h src/config/config_file.h
#	$(LEXCOMPILE) --outfile=$@ $<

#src/parse.c: src/config/parse.y src/server.h src/config/config_file.h
#	$(YACCCOMPILE) --output-file=$@ $<

pingd_SOURCES = $(core_headers) src/main.c $(core_src)
pingd_LDADD   = $(LDADD) -lrt
sbin_PROGRAMS   = pingd

dist_man_MANS  =

EXTRA_DIST = Changelog README.md

man/%.1: man/%.1.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.5: man/%.5.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.7: man/%.7.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.8: man/%.8.pod
	./man/build $< @PACKAGE_VERSION@ > $@
