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
# SAM7 Example
# 

NEWLIBDIR=${HOME}/newlib
CC=arm-elf-gcc
OBJCOPY=arm-elf-objcopy

INCLUDEDIR_FLAGS= -I. -I${NEWLIBDIR}/arm-elf/include

# ARM
CFLAGS_ARM= -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS_ARM= -L. -L${NEWLIBDIR}/arm-elf/lib

# Thumb
AFLAGS= -mthumb -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
CFLAGS= -mthumb -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS= -L. -L${NEWLIBDIR}/arm-elf/lib/thumb

#LINKFLAGS= -g -static -nostdlib -Wl,--script=sam7.ld ${LIBDIR_FLAGS} -lc
LINKFLAGS= -g -static -nostdlib -Wl,--script=sam7.ld ${LIBDIR_FLAGS}

OBJ=sam7_crt0.o sam7_presysinit.o sam7_sysinit.o sam7_except.o sam7_example.o sam7_irq.o sam7_syscall.o

all: sam7_example.elf sam7_example.hex

clean:
	rm -f *.o sam7*[Sch]~ sam7*.ld~ *.gdbscript~ Makefile~

cleanall: clean
	rm -f sam7_example.elf sam7_example.hex *.core

sam7_example.elf: ${OBJ}
	${CC} ${LINKFLAGS} -o $@ ${OBJ}

sam7_example.hex: sam7_example.elf
	${OBJCOPY} -O ihex -j .remap -j .text -j .data \
	sam7_example.elf $@
	
#	--change-section-lma .text+0x100000  \
#	--change-section-lma .data+0x100000  \

.S.o:
	${CC} ${AFLAGS} -I. -c  $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c  $<

sam7_except.o:sam7_except.c
	${CC} ${CFLAGS_ARM} -I. -c $<

sam7_sysinit.o:sam7_sysinit.c
	${CC} ${CFLAGS_ARM} -I. -c $<

sam7_presysinit.o:sam7_presysinit.c
	${CC} ${CFLAGS_ARM} -I. -c $<

