###############################################################################################
#
# Makefile
#
# Copyright (C) 2005
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

# only one of those required
# 	HAVE_IOPERM          -- so we have libioperm
# 	HAVE_I386_SET_IOPERM -- so we need a kind of libioperm wrapper
#
# required: setting LPT default Port baseaddress
# 	PORT_BASE	-- 0x3BC /*LPT1*/ or
# 	PORT_BASE	-- 0x378 /*LPT2*/ or
# 	PORT_BASE	-- 0x278 /*LPT3*/
#
# 	WITH_ALLOCA	-- allocate temporary space on the Stack
# 	NO_ALLOCA	-- or allocate temporary space on the Heap -- which is a little bit slower
#
# optional:
# 	WITH_TAP_LINGER	-- to make the slow io even slower
# 	WITHOUT_RESET	-- to make sure the we did not touch the RESET

#LPT_BASE = -DPORT_BASE=0x3BC
LPT_BASE = -DPORT_BASE=0x378
#LPT_BASE = -DPORT_BASE=0x278


UNAME := $(shell uname)
# FreeBSD
# If we define HAVE_I386_SET_IOPERM then LIB_IO must be empty, too.
ifeq ($(UNAME),FreeBSD)
 CFLAGS_IO= -DHAVE_I386_SET_IOPERM=1 ${LPT_BASE} 
 CFLAGS_ALLOC= -DWITH_ALLOCA=1
endif

# cygwin @ WinNT
# If we define HAVE_IOPERM then we need also to define LIB_IO.
ifeq ($(findstring CYGWIN_NT,$(UNAME)),CYGWIN_NT)
 CFLAGS_IO= -DHAVE_IOPERM=1 ${LPT_BASE} 
 LIB_IO= -lioperm
 CFLAGS_ALLOC= -DWITH_ALLOCA=1
 SERVER_NAME_EXT=.exe
endif

# cygwin @ W98
# If we nither define HAVE_IOPERM nor HAVE_I386_SET_IOPERM then LIB_IO needs to be empty, too.
ifeq ($(findstring CYGWIN_98,$(UNAME)),CYGWIN_98)
 CFLAGS_IO= ${LPT_BASE} 
 CFLAGS_ALLOC= -DWITH_ALLOCA=1
 SERVER_NAME_EXT=.exe
endif

# Linux
# If we define HAVE_IOPERM we might also to define LIB_IO (or it is in libc too).
ifeq ($(findstring Linux,$(UNAME)),Linux)
 CFLAGS_IO= -DHAVE_IOPERM=1 ${LPT_BASE} 
 LIB_IO= -lc
 CFLAGS_ALLOC= -DWITH_ALLOCA=1
endif


################################################################################################
# existens test
#gnu make uses ifndef and $(error message) while bsd make uses .ifnde and .error "message"
ifndef LPT_BASE
$(error missing LPT_BASE)
endif
ifndef CFLAGS_ALLOC
$(error missing CFLAGS_ALLOC)
endif
ifndef CFLAGS_IO
$(error missing CFLAGS_IO)
endif

CC = gcc
OBJCOPY = objcopy

ARM_CC = arm-elf-gcc
ARM_LD = arm-elf-ld
ARM_STRIP = arm-elf-strip
ARM_OBJCOPY = arm-elf-objcopy

#CFLAGS_I386= -g -O2 -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wparentheses -Wswitch-default -Wswitch-enum -Winline -pedantic -std=gnu99 -I./
CFLAGS_I386= -g -O2 -Wall -W -Wmissing-prototypes -Wstrict-prototypes -I./

SERVER = jtag_gdbserver${SERVER_NAME_EXT}

################################################################################################
#
#

OBJ=	jt_arm.o\
	jt_arm_support.o\
	jt_arm_iceRT.o\
	jt_flash_amd.o\
	jt_flash_intel.o\
	jt_flash_philips.o\
	jt_flash_info.o\
	jt_flash_info_amd.o\
	jt_flash_info_intel.o\
	jt_flash_info_philips.o\
	jt_flash_info_atmel.o\
	jt_flash_info_st.o\
	jt_instr.o\
	jt_direct_io.o\
	jt_tap.o \
	jt_tap_tls.o \
	jt_tap_tbdm.o \
	jt_tap_tlongo.o \
	jt_tap_omsp.o \
	jt_tap_wiggler.o \
	jt_tap_bblst.o \
	jt_tap_ispl.o \
	jt_tap_dlc.o \
	jt_tap_apod.o \
	jt_tap_raw_out_test_tls.o \
	jt_tap_raw_out_test_tbdm.o \
	jt_tap_raw_out_test_tlongo.o \
	jt_tap_raw_out_test_omsp.o \
	jt_tap_raw_out_test_wiggler.o \
	jt_tap_raw_out_test_bblst.o \
	jt_tap_raw_out_test_ispl.o \
	jt_tap_raw_out_test_dlc.o \
	jt_tap_raw_out_test_apod.o \
	jt_jtag_test.o \
	dbg_msg.o \
	arm_gdbserver.o\
	arm_gdbstub.o\
	arm_gdbstub_rcmd.o\
	arm_gdbstub_callback.o\
	arm_gdbstub_break.o\
	arm_memory_mmap.o\
	arm_memory_read.o\
	arm_memory_write.o\
	arm_memory_workspace.o\
	arm_dcc_dummy.o\
	arm_flash_readchk.o\
	arm_flash_verify.o\
	arm_flash_check.o\
	arm_flash_erase.o\
	arm_flash_program.o\
	arm_flash_amd.o\
	arm_flash_philips.o\
	arm_flash_atmel.o\
	arm_flash_st.o\
	arm_sfa_cmdseq.o\
	ldsym.o\
	fl_amd8_386.elf\
	fl_amd16_386.elf\
	fl_amd32_386.elf \
	fl_philipsflash_386.elf\
	fl_atmelflash_386.elf\
	fl_stflash_386.elf\
	dcc_read_386.elf\
	dcc_write_386.elf\
	dcc_check_386.elf\
	dcc_dummy_386.elf

################################################################################################
#
#

all: ${SERVER}

${SERVER}: ${OBJ} 
	${CC} -o ${SERVER} ${OBJ} ${LIB_IO}

#
# jt_..
#

jt_ar%.o:jt_ar%.c
	${CC} ${CFLAGS_I386} -c $<

jt_flash_%.o:jt_flash_%.c  
	${CC} ${CFLAGS_I386} -c $<

jt_instr.o:jt_instr.c  
	${CC} ${CFLAGS_I386} ${CFLAGS_ALLOC} -c $<

jt_tap.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DWRAPPER -c $<

jt_tap_tls.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_LEVEL_SHIFTER -o $@ -c $<

jt_tap_raw_out_test_tls.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_LEVEL_SHIFTER -o $@ -c $<

jt_tap_tbdm.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_BDM -o $@ -c $<

jt_tap_raw_out_test_tbdm.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_BDM -o $@ -c $<

jt_tap_tlongo.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_LONGO -o $@ -c $<

jt_tap_raw_out_test_tlongo.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DTINKER_LONGO -o $@ -c $<

jt_tap_omsp.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DOLIMEX_MSP430 -o $@ -c $<

jt_tap_raw_out_test_omsp.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DOLIMEX_MSP430 -o $@ -c $<

jt_tap_wiggler.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DOCDEMON_WIGGLER -o $@ -c $<

jt_tap_raw_out_test_wiggler.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DOCDEMON_WIGGLER -o $@ -c $<

jt_tap_bblst.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DALTERA_BYTEBLASTER -o $@ -c $<

jt_tap_raw_out_test_bblst.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DALTERA_BYTEBLASTER -o $@ -c $<

jt_tap_ispl.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DLATTICE_ISPDLC -o $@ -c $<

jt_tap_raw_out_test_ispl.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DLATTICE_ISPDLC -o $@ -c $<

jt_tap_dlc.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DXILINX_DLC -o $@ -c $<

jt_tap_raw_out_test_dlc.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DXILINX_DLC -o $@ -c $<

jt_tap_apod.o:jt_tap.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DAMONTEC_EPP_ACCELERATOR -DLPT_EPP_MODE -o $@ -c $<

jt_tap_raw_out_test_apod.o:jt_tap_raw_out_test.c jt_io.h
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -DAMONTEC_EPP_ACCELERATOR -DLPT_EPP_MODE -o $@ -c $<

jt_direct_io.o:jt_direct_io.c
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -c $<

jt_jtag_test.o:jt_jtag_test.c  
	${CC} ${CFLAGS_I386} -c $<
	

#
# arm_gdb..
#
arm_gdbserver.o:arm_gdbserver.c
	${CC} ${CFLAGS_I386} ${CFLAGS_IO} -c $<

arm_gdbstu%.o:arm_gdbstu%.c           
	${CC} ${CFLAGS_I386} -c $<

# arm_memory..

arm_memory_%.o:arm_memory_%.c
	${CC} ${CFLAGS_I386} -c $<

arm_dcc_dummy.o:arm_dcc_dummy.c
	${CC} ${CFLAGS_I386} -c $<

# arm_flash..

arm_flash_%.o:arm_flash_%.c
	${CC} ${CFLAGS_I386} -c $<

# arm_sfa..

arm_sfa_cmdseq.o:arm_sfa_cmdseq.c
	${CC} ${CFLAGS_I386} -c $<

# dbg_msg

dbg_msg.o:dbg_msg.c
	${CC} ${CFLAGS_I386} -c $<
	
#
# t_arm ..
#

fl_amd8.elf fl_amd8_386.elf:t_arm_amdflash_8.c
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4t -c t_arm_amdflash_8.c -DGEN_ARM_PART_1=1 -o fl_amd8_arm_1.o
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4t -c t_arm_amdflash_8.c -DGEN_ARM_PART_2=1 -o fl_amd8_arm_2.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_amd8_arm_1.o fl_amd8_arm_2.o -o fl_amd8.elf
	${ARM_STRIP} -s -R .comment -o fl_amd8.ram fl_amd8.elf
	${ARM_OBJCOPY} -O binary fl_amd8.ram fl_amd8.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_amd8.bin fl_amd8_386.elf

fl_amd16.elf fl_amd16_386.elf:t_arm_amdflash_16.c
	${ARM_CC} -g -Os -mlittle-endian -mcpu=arm7tdmi -c t_arm_amdflash_16.c -DGEN_ARM_PART_1=1 -o fl_amd16_arm_1.o
	${ARM_CC} -g -Os -mlittle-endian -mcpu=arm7tdmi -c t_arm_amdflash_16.c -DGEN_ARM_PART_2=1 -o fl_amd16_arm_2.o
	${ARM_CC} -g -Os -mthumb -mlittle-endian -mcpu=arm7tdmi -mcallee-super-interworking -c t_arm_amdflash_16.c -DGEN_THUMB_PART=1 -o fl_amd16_thumb.o
#	${ARM_CC} -g -Os -mthumb -mlittle-endian -mcpu=arm7tdmi -mthumb-interwork -c t_arm_amdflash_16.c -DGEN_THUMB_PART=1 -o fl_amd16_thumb.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_amd16_arm_1.o fl_amd16_arm_2.o fl_amd16_thumb.o -o fl_amd16.elf
	${ARM_STRIP} -s -R .comment -o fl_amd16.ram fl_amd16.elf
	${ARM_OBJCOPY} -O binary fl_amd16.ram fl_amd16.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_amd16.bin fl_amd16_386.elf

fl_amd32.elf fl_amd32_386.elf:t_arm_amdflash_32.c
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4t -c t_arm_amdflash_32.c -DGEN_ARM_PART_1=1 -o fl_amd32_arm_1.o
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4t -c t_arm_amdflash_32.c -DGEN_ARM_PART_2=1 -o fl_amd32_arm_2.o
	${ARM_CC} -g -Os -fPIC -mthumb -mlittle-endian -mapcs -march=armv4t -mcallee-super-interworking -c t_arm_amdflash_32.c -DGEN_THUMB_PART=1 -o fl_amd32_thumb.o
#	${ARM_CC} -g -Os -fPIC -mthumb -mlittle-endian -mapcs -march=armv4t -mthumb-interwork -c t_arm_amdflash_32.c -DGEN_THUMB_PART=1 -o fl_amd32_thumb.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_amd32_arm_1.o fl_amd32_arm_2.o fl_amd32_thumb.o -o fl_amd32.elf
	${ARM_STRIP} -s -R .comment -o fl_amd32.ram fl_amd32.elf
	${ARM_OBJCOPY} -O binary fl_amd32.ram fl_amd32.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_amd32.bin fl_amd32_386.elf

fl_philips.elf fl_philipsflash_386.elf:t_arm_philipsflash.c
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_philipsflash.c -DGEN_ARM_PART_1=1 -o fl_philips_part1.o
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_philipsflash.c -DGEN_ARM_PART_2=1 -o fl_philips_part2.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_philips_part1.o fl_philips_part2.o -o fl_philips.elf
	${ARM_STRIP} -s -R .comment -o fl_philips.ram fl_philips.elf
	${ARM_OBJCOPY} -O binary fl_philips.ram fl_philips.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_philips.bin fl_philipsflash_386.elf

fl_atmel.elf fl_atmelflash_386.elf:t_arm_atmelflash.c
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_atmelflash.c -DGEN_ARM_PART_1=1 -o fl_atmel_part1.o
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_atmelflash.c -DGEN_ARM_PART_2=1 -o fl_atmel_part2.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_atmel_part1.o fl_atmel_part2.o -o fl_atmel.elf
	${ARM_STRIP} -s -R .comment -o fl_atmel.ram fl_atmel.elf
	${ARM_OBJCOPY} -O binary fl_atmel.ram fl_atmel.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_atmel.bin fl_atmelflash_386.elf

fl_st.elf fl_stflash_386.elf:t_arm_stflash.c
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_stflash.c -DGEN_ARM_PART_1=1 -o fl_st_part1.o
	${ARM_CC} -g -Os -fPIC -mlittle-endian -mapcs -march=armv4 -c t_arm_stflash.c -DGEN_ARM_PART_2=1 -o fl_st_part2.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start fl_st_part1.o fl_st_part2.o -o fl_st.elf
	${ARM_STRIP} -s -R .comment -o fl_st.ram fl_st.elf
	${ARM_OBJCOPY} -O binary fl_st.ram fl_st.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 fl_st.bin fl_stflash_386.elf

dcc_read.elf dcc_read_386.elf:t_arm_dcc_read_mem.c
	${ARM_CC} -O2 -fPIC  -mlittle-endian -march=armv4 -mapcs -fomit-frame-pointer -c t_arm_dcc_read_mem.c -o dcc_read.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start dcc_read.o -o dcc_read.elf
	${ARM_STRIP} -s -R .comment -o dcc_read.ram dcc_read.elf
	${ARM_OBJCOPY} -O binary dcc_read.ram dcc_read.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 dcc_read.bin dcc_read_386.elf

dcc_write.elf dcc_write_386.elf:t_arm_dcc_write_mem.c
	${ARM_CC} -O2 -fPIC -finline-functions -mlittle-endian -march=armv4 -mapcs -fomit-frame-pointer -c t_arm_dcc_write_mem.c -o dcc_write.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start dcc_write.o -o dcc_write.elf
	${ARM_STRIP} -s -R .comment -o dcc_write.ram dcc_write.elf
	${ARM_OBJCOPY} -O binary dcc_write.ram dcc_write.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 dcc_write.bin dcc_write_386.elf

dcc_check.elf dcc_check_386.elf:t_arm_dcc_check_flash.c
	${ARM_CC} -O2 -fPIC -finline-functions -mlittle-endian -march=armv4 -mapcs -fomit-frame-pointer -c t_arm_dcc_check_flash.c -o dcc_check.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start dcc_check.o -o dcc_check.elf
	${ARM_STRIP} -s -R .comment -o dcc_check.ram dcc_check.elf
	${ARM_OBJCOPY} -O binary dcc_check.ram dcc_check.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 dcc_check.bin dcc_check_386.elf

dcc_dummy.elf dcc_dummy_386.elf:t_arm_dcc_dummy.c
	${ARM_CC} -O2 -fPIC -finline-functions -mlittle-endian -march=armv4 -mapcs -fomit-frame-pointer -c t_arm_dcc_dummy.c -o dcc_dummy.o
	${ARM_LD} -EL -Ttext 0 -e dcc_start dcc_dummy.o -o dcc_dummy.elf
	${ARM_STRIP} -s -R .comment -o dcc_dummy.ram dcc_dummy.elf
	${ARM_OBJCOPY} -O binary dcc_dummy.ram dcc_dummy.bin
	${OBJCOPY} -I binary -B i386 -O elf32-i386 dcc_dummy.bin dcc_dummy_386.elf


#
# ld..
#

ldsym.o:ldsym.S
	${CC} -c $<

#
#
#

clean:
	rm -f *.o *.elf *.bin *.ram *.stackdump *.core

cleanall: clean
	rm -f ${SERVER} *.[chS]~ Makefile~ README~ Notes~ .mmap*.tmp

