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
LD=arm-elf-ld
OBJCOPY=arm-elf-objcopy

INCLUDEDIR_FLAGS= -I. -I./include -I${NEWLIBDIR}/arm-elf/include

MCULIB=71x
MCULIB_DIR=libsrc
CRT=${MCULIB_DIR}/71x_crt0.o

# ARM
#CFLAGS_ARM= -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
CFLAGS_ARM= -mthumb-interwork -g -Os -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS_ARM= -L. -L${NEWLIBDIR}/arm-elf/lib

# Thumb
#AFLAGS= -mthumb -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
AFLAGS= -mthumb -Os -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
#CFLAGS= -mthumb -mthumb-interwork -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
CFLAGS= -mthumb -mthumb-interwork -g -Os -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS= -L. -L./${MCULIB_DIR} -L${NEWLIBDIR}/arm-elf/lib/thumb

#LINKFLAGS= -g -static -nostdlib -Wl,--script=str7.ld
LINKFLAGS= --script=str7.ld

OBJ=str7_presysinit.o str7_sysinit.o str7_except.o str7_example.o str7_irq.o str7_syscall.o

all: str7_example.elf str7_example.hex

clean:
	rm -f *.o str7*[Sch]~ str7*.ld~ *.gdbscript~ Makefile~ 71x_conf.h~
	cd ${MCULIB_DIR}; ${MAKE} clean

cleanall: clean
	rm -f str7_example.elf str7_example.hex *.core
	cd ${MCULIB_DIR}; ${MAKE} cleanall

str7_example.elf: ${OBJ} ${CRT} ${MCULIB_DIR}/lib${MCULIB}.a
	${LD} -o $@ ${LINKFLAGS} ${CRT} ${OBJ} ${LIBDIR_FLAGS} -l${MCULIB}

str7_example.hex: str7_example.elf
	${OBJCOPY} -O ihex -j .remap -j .text -j .data str7_example.elf $@
	
.S.o:
	${CC} ${AFLAGS} -I. -c  $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c  $<

str7_except.o:str7_except.c
	${CC} ${CFLAGS_ARM} ${INCLUDEDIR_FLAGS} -c $<

str7_sysinit.o:str7_sysinit.c
	${CC} ${CFLAGS_ARM} ${INCLUDEDIR_FLAGS} -c $<

str7_presysinit.o:str7_presysinit.c
	${CC} ${CFLAGS_ARM} ${INCLUDEDIR_FLAGS} -c $<

${CRT}: 
	cd ${MCULIB_DIR}; ${MAKE} crt

${MCULIB_DIR}/lib${MCULIB}.a:
	cd ${MCULIB_DIR}; ${MAKE} lib

