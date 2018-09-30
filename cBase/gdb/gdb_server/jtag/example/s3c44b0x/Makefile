#############################################################################
#
# s3c44b0x Example
# 

NEWLIBDIR=${HOME}/newlib
CC=arm-elf-gcc
OBJCOPY=arm-elf-objcopy

INCLUDEDIR_FLAGS= -I. -I${NEWLIBDIR}/arm-elf/include

# ARM
CFLAGS_ARM= -mthumb-interwork -msoft-float -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS_ARM= -L. -L${NEWLIBDIR}/arm-elf/lib

# Thumb
CFLAGS= -mthumb -mthumb-interwork -msoft-float -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused
LIBDIR_FLAGS= -L. -L${NEWLIBDIR}/arm-elf/lib/thumb

# asm
AFLAGS= -DPLLONSTART=1 -mthumb -mthumb-interwork -msoft-float -g -Os -fno-builtin -nostdinc -march=armv4t -Wall -Winline -Wmissing-prototypes -Wunused

#-Wa,-D,--gdwarf2 

#LINKFLAGS= -nostdlib -Wl,--script=s3c44b0x.ld ${LIBDIR_FLAGS} -lc
LINKFLAGS= -nostdlib -Wl,--script=s3c44b0x.ld ${LIBDIR_FLAGS} 
LINKBIND= -nostdlib -Wl,--script=s3c44b0x_bind.ld

OBJ=s3c44b0x_crt0.o s3c44b0x_sysinit.o s3c44b0x_except.o s3c44b0x_isr.o s3c44b0x_syscall.o s3c44b0x_example.o

all: s3c44b0x_example.elf s3c44b0x_example.hex

clean:
	rm -f *.o s3c44b0x*[Sch]~ s3c44b0x*ld~ Makefile~ *.gdbscript~ *.gdbinit~

cleanall: clean
	rm -f s3c44b0x_example.elf s3c44b0x_example.bindelf s3c44b0x_example.hex

s3c44b0x_example.elf: ${OBJ}
	${CC} ${LINKFLAGS} -o $@ ${OBJ}

s3c44b0x_example.hex: s3c44b0x_example.elf
	${CC} ${LINKBIND} -o s3c44b0x_example.bindelf s3c44b0x_example.elf
	${OBJCOPY} -O ihex -j .text -j .data s3c44b0x_example.bindelf $@

.S.o:
	${CC} ${AFLAGS} -I. -c  $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDEDIR_FLAGS} -c  $<

s3c44b0x_except.o:s3c44b0x_except.c
	${CC} ${CFLAGS_ARM} -I. -c $<

s3c44b0x_sysinit.o:s3c44b0x_sysinit.c
	${CC} ${CFLAGS_ARM} -I. -c $<


