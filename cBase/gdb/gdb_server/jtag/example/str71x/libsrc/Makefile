#############################################################################
#
# Copyright 2005 Free Software Foundation, Inc.
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
#
# STR7 Example
# 

NEWLIBDIR=${HOME}/newlib
CC=arm-elf-gcc
AR=arm-elf-ar
OBJCOPY=arm-elf-objcopy

INCLUDEDIR_FLAGS= -I. -I../include -I../

# ARM
CFLAGS_ARM= -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Wunused
LIBDIR_FLAGS_ARM= -L. -L${NEWLIBDIR}/arm-elf/lib

# Thumb
AFLAGS= -mthumb -Os -fno-builtin -nostdinc -march=armv4t -Wall -Wunused
CFLAGS= -mthumb -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Wunused
LIBDIR_FLAGS= -L. -L${NEWLIBDIR}/arm-elf/lib/thumb

#LINKFLAGS= -g -static -nostdlib -Wl,--script=../str7.ld ${LIBDIR_FLAGS} -lc
LINKFLAGS= -g -static -nostdlib -Wl,--script=../str7.ld ${LIBDIR_FLAGS}

CRT=71x_crt0
LIB_OBJ=apb.o eic.o gpio.o rccu.o uart.o bspi.o emi.o i2c.o rtc.o \
	wdg.o adc12.o can.o flash.o pcu.o tim.o xti.o

all: lib libdebug crt

crt: ${CRT}.o

lib: ${LIB_OBJ} 71x_lib.o
	${AR} r lib71x.a ${LIB_OBJ} 71x_lib.o

libdebug: ${LIB_OBJ} 71x_lib_debug.o
	${AR} r lib71x_debug.a ${LIB_OBJ} 71x_lib_debug.o

clean:
	rm -f ${LIB_OBJ} 71x_lib*.o Makefile~ *.[chS]~
	rm -f ../include/*.h~

cleanall: clean
	rm -f lib71x*.a ${CRT}.o

71x_lib.o: 71x_lib.c
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c 71x_lib.c

71x_lib_debug.o: 71x_lib.c
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -DDEBUG=1 -o $@ -c 71x_lib.c

${CRT}.o: ${CRT}.S
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c ${CRT}.S

.S.o:
	${CC} ${AFLAGS} -I. -c  $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c  $<

