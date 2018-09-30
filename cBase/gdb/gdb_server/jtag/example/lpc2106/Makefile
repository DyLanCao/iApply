#############################################################################
#
# Copyright 2005 Free Software Foundation, Inc.
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 
# Written by Thomas Klein <ThRKlein@sf.net>, 2005.
#
# LPC 21xx Example
# 

NEWLIBDIR=${HOME}/newlib
CC=arm-elf-gcc
OBJCOPY=arm-elf-objcopy

INCLUDEDIR_FLAGS= -I. -I${NEWLIBDIR}/arm-elf/include

# ARM
CFLAGS_ARM= -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS_ARM= -L. -L${NEWLIBDIR}/arm-elf/lib

# Thumb
AFLAGS= -mthumb -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
CFLAGS= -mthumb -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS= -L. -L${NEWLIBDIR}/arm-elf/lib/thumb

LINKFLAGS= -g -static -nostdlib -Wl,--script=lpc.ld ${LIBDIR_FLAGS} -lc

OBJ=lpc_crt0.o lpc_sysinit.o lpc_except.o lpc_example.o lpc_irq.o lpc_syscall.o

all: lpc_example.elf lpc_example.hex

clean:
	rm -f *.o lpc*[Sch]~ lpc*.ld~ *.gdbscript~ Makefile~

cleanall: clean
	rm -f lpc_example.elf lpc_example.hex *.core

lpc_example.elf: ${OBJ}
	${CC} ${LINKFLAGS} -o $@ ${OBJ}

lpc_example.hex: lpc_example.elf
	${OBJCOPY} -O ihex -j .text -j .data lpc_example.elf $@

.S.o:
	${CC} ${AFLAGS} -I. -c  $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c  $<

lpc_except.o:lpc_except.c
	${CC} ${CFLAGS_ARM} -I. -c $<

lpc_sysinit.o:lpc_sysinit.c
	${CC} ${CFLAGS_ARM} -I. -c $<

