#
# cookies
#
# Copyright (c) 2005 Javeed Shaikh and Jeff Nettleton
# Copyright (c) 2005 Mikkel Krautz
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#
                   
CC =		gcc

MAKE =		make

CFLAGS =	-pipe -O2 -D_GNU_SOURCE -pedantic		\
		-W -Wall -Wno-unused-parameter 	-Wno-long-long	\
		`pkg-config --cflags gtk+-2.0 libmpd-0.01`

LDFLAGS =	`pkg-config --libs gtk+-2.0 libmpd-0.01`

DEBUG =		-g3 -ggdb

OBJ =		objects/mpdwrapper.o				\
		objects/main.o					\
		objects/tree.o					\
		objects/window.o

.PHONY: default
default:
	if [ ! -d objects/ ]; then				\
		mkdir -p objects/;				\
		touch objects/.normalbuild;			\
		$(MAKE) cookies;				\
	elif [ -f objects/.normalbuild ]; then			\
		$(MAKE) cookies;				\
	elif [ -f objects/.debugbuild ]; then			\
		$(MAKE) CFLAGS="$(DEBUG) $(CFLAGS)" cookies;	\
	fi

.PHONY: debug
debug:
	if [ ! -f objects/.debugbuild ]; then			\
		$(MAKE) clean;					\
		mkdir -p objects/;				\
		touch objects/.debugbuild;			\
		$(MAKE) CFLAGS="$(DEBUG) $(CFLAGS)" cookies;	\
	else							\
		$(MAKE) CFLAGS="$(DEBUG) $(CFLAGS)" cookies;	\
	fi

.PHONY: clean
clean:
	rm -rf objects/ cookies

objects/mpdwrapper.o: src/mpdwrapper.c src/window.h src/cookie.h
	$(CC) $(CFLAGS) -c src/mpdwrapper.c -o $@

objects/tree.o:	src/tree.c src/tree.h
	$(CC) $(CFLAGS) -c src/tree.c -o $@

objects/window.o: src/window.c src/window.h
	$(CC) $(CFLAGS) -c src/window.c -o $@

objects/main.o: src/main.c src/cookie.h src/window.h src/mpdwrapper.h
	$(CC) $(CFLAGS) -c src/main.c -o $@

cookies: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@
