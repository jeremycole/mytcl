#
#   MyTCL, package to integrate TCL with MySQL databases
#   Copyright (C) 2003  Jeremy Cole
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Makefile for MyTCL

OBJS	   = manager.o connection.o mytcl.o

CC	= g++
LD	= g++
FLAGS	= -Wall

INSTALL_DIR = /usr/lib/tcl8.4

MYSQL_LIB = -L/usr/local/lib/mysql
MYSQL_INC = -I/usr/local/include/mysql

TCL_LIB = -L/usr/local/lib
TCL_INC = -I/usr/local/include/tcl8.4

LIBS = -lmysqlclient -ltcl84 -lz

CC_FLAGS = -ggdb
LD_FLAGS = -ggdb -L/usr/local/lib $(MYSQL_LIB) $(TCL_LIB) $(LIBS)
INCLUDE  = -I/usr/local/include $(MYSQL_INC) $(TCL_INC)

# Linux:
EXTRA_FLAGS    = -fPIC 
EXTRA_LD_FLAGS = -shared -lgcc -lm

# Solaris:
# EXTRA_FLAGS    = -fPIC -shared
# EXTRA_LD_FLAGS = -L/usr/local/mysql -lnsl -lsocket -shared
# LD	= ld
# You will need to modify the following paths to your libmysqlclient.so
# and libgcc.a
# EXTRA_LINKS = /usr/local/mysql/lib/libmysqlclient.a \
#        /opt/GCC2721/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2.1/libgcc.a

# FreeBSD:
# EXTRA_LD_FLAGS = -lstdc++ -lm -lgcc_pic -shared

# Don't touch below this line, that's what variables are for.

all: mytcl.so

.cc.o:
	$(CC) $(CC_FLAGS) -c $(INCLUDE) $(EXTRA_FLAGS) $(FLAGS) -o $@ $<

mytcl.so:	$(OBJS)
	$(LD) $(OBJS) $(EXTRA_LINKS) $(LD_FLAGS) $(EXTRA_LD_FLAGS) -o mytcl.so

install: mytcl.so
	cp mytcl.so mytcl1.0/
	cp -R mytcl1.0 $(INSTALL_DIR)

clean:
	rm -f *~ *.o mytcl.so mytcl1.0/mytcl.so
