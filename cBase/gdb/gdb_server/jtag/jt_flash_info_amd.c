/*
 * jt_flash_info_amd.c
 * 
 * Flash support functions
 *
 * Copyright (C) 2005,2006
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.
 * 
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_flash.h"


/*
 *  AMD 
 */
#define AMD_VID_8BIT	0x01
#define AMD_VID_2x8BIT	0x0101		/* 2x 8 - Bit mode */
#define AMD_VID_16BIT	0x0001		/* 16 - Bit mode */
#define AMD_VID_4x8BIT	0x01010101	/* 4x 8 - Bit mode */
#define AMD_VID_2x16BIT	0x00010001	/* 2x 16 - Bit mode */

#define AMD_CID_8BIT_AM29LV040B		0x4F
#define AMD_CID_2x8BIT_AM29LV040B	0x4F4F
#define AMD_CID_4x8BIT_AM29LV040B	0x4F4F4F4F
static inline void AMD_AM29LV040B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV040B\n" );}
static inline void AMD_2x_AM29LV040B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV040B\n" );}
static inline void AMD_4x_AM29LV040B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29LV040B\n" );}

#define AMD_CID_8BIT_AM29F080B		0xD5
#define AMD_CID_2x8BIT_AM29F080B	0xD5D5
#define AMD_CID_4x8BIT_AM29F080B	0xD5D5D5D5
static inline void AMD_AM29F080B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29F080B\n" );}
static inline void AMD_2x_AM29F080B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29F080B\n" );}
static inline void AMD_4x_AM29F080B(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29F080B\n" );}

#define AMD_CID_8BIT_AM29LV640D		0xD7
#define AMD_CID_16BIT_AM29LV640D	0x22D7
#define AMD_CID_2x16BIT_AM29LV640D	0x22D722D7
static inline void AMD_AM29LV640D(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV640D/Am29LV641D/Am29LV642D\n" );}
static inline void AMD_2x_AM29LV640D(void){dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV640D/Am29LV641D/Am29LV642D\n" );}


/*
 * 1 MByte total
 * Bottom Boot Blocks - 16K, 8K, 8K, 32K
 * 15 Blocks - 64K
 */
#define AMD_CID_8BIT_AM29LV800BB	0x5B
#define AMD_CID_2x8BIT_AM29LV800BB	0x5B5B
#define AMD_CID_16BIT_AM29LV800BB	0x225B
#define AMD_CID_4x8BIT_AM29LV800BB	0x5B5B5B5B
#define AMD_CID_2x16BIT_AM29LV800BB	0x225B225B
static inline void AMD_AM29LV800BB(void)
{
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV800BB (bottom BB)""\n" );	
	bottom_boot_size		= 16 * 1024;	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 32 * 1024;	
	block_size			= 64 * 1024;	
	num_of_blocks			= 15;		
}
static inline void AMD_2x_AM29LV800BB(void)
{ 						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV800BB (bottom BB)\n" );	
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 32 * 1024;
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 15;		
}
static inline void AMD_4x_AM29LV800BB(void)
{ 						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29LV800BB (bottom BB)\n" );	
	bottom_boot_size		= 4 * 16 * 1024;
	bottom_parameter_size		= 4 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 4 * 32 * 1024;
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 15;		
}

/*
 * 1 MByte total
 * 15 Blocks - 64K
 * Top Blocks - 32K, 8K, 8K, 16K
 */
#define AMD_CID_8BIT_AM29LV800BT	0xDA
#define AMD_CID_2x8BIT_AM29LV800BT	0xDADA
#define AMD_CID_16BIT_AM29LV800BT	0x22DA
#define AMD_CID_4x8BIT_AM29LV800BT	0xDADADADA
#define AMD_CID_2x16BIT_AM29LV800BT	0x22DA22DA
static inline void AMD_AM29LV800BT(void)
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV800BT (top BB)\n" ); 		
	block_size			= 64 * 1024;	
	num_of_blocks			= 15;		
	top_inter_size			= 32 * 1024;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 16 * 1024;	
}
static inline void AMD_2x_AM29LV800BT(void)
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV800BT (top BB)\n" ); 		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 15;		
	top_inter_size			= 2 * 32 * 1024;
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 2 * 16 * 1024;
}
static inline void AMD_4x_AM29LV800BT(void)
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29LV800BT (top BB)\n" ); 		
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 15;		
	top_inter_size			= 4 * 32 * 1024;
	top_parameter_size		= 4 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 4 * 16 * 1024;
}

/*
 * 512 KByte total
 * Boot Blocks - 16K, 8K, 8K, 32K
 * 7 Blocks - 64K
 */
#define AMD_CID_8BIT_AM29LV400BB	0xBA
#define AMD_CID_2x8BIT_AM29LV400BB	0xBABA
#define AMD_CID_16BIT_AM29LV400BB	0x22BA
#define AMD_CID_4x8BIT_AM29LV400BB	0xBABABABA
#define AMD_CID_2x16BIT_AM29LV400BB	0x22BA22BA
static inline void AMD_AM29LV400BB(void)
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV400BB (bottom BB)\n" );		
	bottom_boot_size		= 16 * 1024;	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 32 * 1024;	
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
}
static inline void AMD_2x_AM29LV400BB(void)
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV400BB (bottom BB)\n" );	
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 32 * 1024;
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
}
static inline void AMD_4x_AM29LV400BB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29LV400BB (bottom BB)\n" );	
	bottom_boot_size		= 4 * 16 * 1024;
	bottom_parameter_size		= 4 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 4 * 32 * 1024;
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 7;		
}

/*
 * 512 KByte total
 * 7 Blocks - 64K
 * Top Blocks - 32K, 8K, 8K, 16K
 */
#define AMD_CID_8BIT_AM29LV400BT	0xB9
#define AMD_CID_2x8BIT_AM29LV400BT	0xB9B9
#define AMD_CID_16BIT_AM29LV400BT	0x22B9
#define AMD_CID_4x8BIT_AM29LV400BT	0xB9B9B9B9
#define AMD_CID_2x16BIT_AM29LV400BT	0x22B922B9
static inline void AMD_AM29LV400BT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Am29LV400BT (top BB)\n" );		
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
	top_inter_size			= 32 * 1024;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 16 * 1024;	
}
static inline void AMD_2x_AM29LV400BT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x Am29LV400BT (top BB)\n" );		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
	top_inter_size			= 2 * 32 * 1024;
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 2 * 16 * 1024;
}
static inline void AMD_4x_AM29LV400BT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x Am29LV400BT (top BB)\n" );		
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 7;		
	top_inter_size			= 4 * 32 * 1024;
	top_parameter_size		= 4 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 4 * 16 * 1024;
}

			
/* Atmel */
#define ATMEL_VID_8BIT 		0x1F
#define ATMEL_VID_2x8BIT 	0x1F1F
#define ATMEL_VID_16BIT 	0x001F
#define ATMEL_VID_16BIT_ALT 	0x161F
#define ATMEL_VID_4x8BIT 	0x1F1F1F1F
#define ATMEL_VID_2x16BIT 	0x001F001F
#define ATMEL_VID_2x16BIT_ALT 	0x161F161F

/*
 * 2 MByte total
 * 8 Boot Blocks - 8K,
 * 31 Blocks - 64K
 */
#define ATMEL_CID_8BIT_AT49LV1604A	0xC0
#define ATMEL_CID_2x8BIT_AT49LV1604A	0xC0C0
#define ATMEL_CID_16BIT_AT49LV1604A	0x00C0
#define ATMEL_CID_4x8BIT_AT49LV1604A	0xC0C0C0C0
#define ATMEL_CID_2x16BIT_AT49LV1604A	0x00C000C0
static inline void ATMEL_AT49LV1604A(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "AT49BV/LV16x4A (bottom BB)\n" );	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 64 * 1024;	
	num_of_blocks			= 31;		
}
static inline void ATMEL_2x_AT49LV1604A(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x AT49BV/LV16x4A (bottom BB)\n" );	
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 31;		
}
static inline void ATMEL_4x_AT49LV1604A(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x AT49BV/LV16x4A (bottom BB)\n" );
	bottom_parameter_size		= 4 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 31;		
}

/*
 * 2 MByte total
 * 31 Blocks - 64K
 * 8 Top Blocks - 8K, 
 */
#define ATMEL_CID_8BIT_AT49LV1604AT	0xC2
#define ATMEL_CID_2x8BIT_AT49LV1604AT	0xC2C2
#define ATMEL_CID_16BIT_AT49LV1604AT	0x00C2
#define ATMEL_CID_4x8BIT_AT49LV1604AT	0xC2C2C2C2
#define ATMEL_CID_2x16BIT_AT49LV1604AT	0x00C200C2
static inline void ATMEL_AT49LV1604AT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "AT49BV/LV16x4AT (top BB)\n" );		
	block_size			= 64 * 1024;	
	num_of_blocks			= 31;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void ATMEL_2x_AT49LV1604AT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x AT49BV/LV16x4AT (top BB)\n" );	
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 31;		
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void ATMEL_4x_AT49LV1604AT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x AT49BV/LV16x4AT (top BB)\n" );	
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 31;		
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

/*
 * 512 KByte total
 * 1 Boot Blocks - 16K, 8K, 8K
 * 1 Blocks - 480K
 */
#define ATMEL_CID_8BIT_AT49LV4096A 0x92
#define ATMEL_CID_16BIT_AT49LV4096A 0x1692
#define ATMEL_CID_2x16BIT_AT49LV4096A 0x1692
static inline void ATMEL_AT49LV4096A(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "AT49BV/LV4096A (bottom BB)\n" );	
	bottom_boot_size		= 16 * 1024; 	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 480 * 1024;	
	block_size			= 0;		
	num_of_blocks			= 0;		
}
static inline void ATMEL_2x_AT49LV4096A(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x AT49BV/LV4096A (bottom BB)\n" );	
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 480 * 1024;
	block_size			= 0;		
	num_of_blocks			= 0;		
}

/* 
 * SGS TOMSON 
 */
#define STM_VID_8BIT 	0x20
#define STM_VID_2x8BIT 	0x2020
#define STM_VID_16BIT 	0x0020
#define STM_VID_4x8BIT 	0x20202020
#define STM_VID_2x16BIT	0x00200020

/*
 * 512 KByte total
 * Boot Blocks - 16K, 8K, 8K, 32K
 * 7 Blocks - 64K
 */
#define STM_CID_8BIT_M29F400BB		0xD6
#define STM_CID_16BIT_M29F400BB		0x00D6
#define STM_CID_2x16BIT_M29F400BB	0x00D600D6
static inline void STM_M29F400BB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29F400BB (bottom BB)\n" );		
	bottom_boot_size		= 16 * 1024;	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 32 * 1024;	
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
}
static inline void STM_2x_M29F400BB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29F400BB (bottom BB)\n" );		
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 32 * 1024;
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
}

/*
 * 512 KByte total
 * 7 Blocks - 64K
 * Top Blocks - 32K, 8K, 8K, 16K
 */
#define STM_CID_8BIT_M29F400BT		0xD5
#define STM_CID_16BIT_M29F400BT		0x00D5
#define STM_CID_2x16BIT_M29F400BT	0x00D500D5
static inline void STM_M29F400BT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29F400BT (top BB)\n" );		
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
	top_inter_size			= 32 * 1024;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 16 * 1024;	
} 
static inline void STM_2x_M29F400BT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29F400BT (top BB)\n" );		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
	top_inter_size			= 2 * 32 * 1024;
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 2 * 16 * 1024;
}

/*
 * 512 KByte total
 * Boot Blocks - 16K, 8K, 8K, 32K
 * 7 Blocks - 64K
 */
#define STM_CID_8BIT_M29W400DB		0xEF
#define STM_CID_2x8BIT_M29W400DB	0xEFEF
#define STM_CID_16BIT_M29W400DB		0x00EF
#define STM_CID_4x8BIT_M29W400DB	0xEFEFEFEF
#define STM_CID_2x16BIT_M29W400DB	0x00EF00EF
static inline void STM_M29W400DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29W400DB (bottom BB)\n" );		
	bottom_boot_size		= 16 * 1024;	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 32 * 1024;	
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
}
static inline void STM_2x_M29W400DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29W400DB (bottom BB)\n" );		
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 32 * 1024;
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
}
static inline void STM_4x_M29W400DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x m29W400DB (bottom BB)\n" );		
	bottom_boot_size		= 4 * 16 * 1024;
	bottom_parameter_size		= 4 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 4 * 32 * 1024;
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 7;		
}

/*
 * 512 KByte total
 * 7 Blocks - 64K
 * Top Blocks - 32K, 8K, 8K, 16K
 */
#define STM_CID_8BIT_M29W400DT		0xEE
#define STM_CID_2x8BIT_M29W400DT	0xEEEE
#define STM_CID_16BIT_M29W400DT		0x00EE
#define STM_CID_4x8BIT_M29W400DT	0xEEEEEEEE
#define STM_CID_2x16BIT_M29W400DT	0x00EE00EE
static inline void STM_M29W400DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29W400DT (top BB)\n" );		
	block_size			= 64 * 1024;	
	num_of_blocks			= 7;		
	top_inter_size			= 32 * 1024;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 16 * 1024;	
}
static inline void STM_2x_M29W400DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29W400DT (top BB)\n" );		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 7;		
	top_inter_size			= 2 * 32 * 1024;
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 2 * 16 * 1024;
}
static inline void STM_4x_M29W400DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x m29W400DT (top BB)\n" );		
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 7;		
	top_inter_size			= 4 * 32 * 1024;
	top_parameter_size		= 4 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 4 * 16 * 1024;
}

/*
 * 1 MByte total
 * Bottom Boot Blocks - 16K, 8K, 8K, 32K
 * 15 Blocks - 64K
 */
#define STM_CID_8BIT_M29W800DB		0x5B
#define STM_CID_2x8BIT_M29W800DB	0x5B5B
#define STM_CID_16BIT_M29W800DB		0x225B
#define STM_CID_4x8BIT_M29W800DB	0x5B5B5B5B
#define STM_CID_2x16BIT_M29W800DB	0x225B225B
static inline void STM_M29W800DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29W800DB (bottom BB)\n" );		
	bottom_boot_size		= 16 * 1024; 	
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 32 * 1024;	
	block_size			= 64 * 1024;	
	num_of_blocks			= 15;		
}
static inline void STM_2x_M29W800DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29W800DB (bottom BB)\n" );		
	bottom_boot_size		= 2 * 16 * 1024;
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 2 * 32 * 1024;
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 15;		
}
static inline void STM_4x_M29W800DB(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x m29W800DB (bottom BB)\n" );		
	bottom_boot_size		= 4 * 16 * 1024;
	bottom_parameter_size		= 4 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 2;		
	bottom_inter_size		= 4 * 32 * 1024;
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 15;		
}

/*
 * 1 MByte total
 * 15 Blocks - 64K
 * Top Blocks - 32K, 8K, 8K, 16K
 */
#define STM_CID_8BIT_M29W800DT		0xD7
#define STM_CID_2x8BIT_M29W800DT	0xD7D7
#define STM_CID_16BIT_M29W800DT		0x22D7
#define STM_CID_4x8BIT_M29W800DT	0xD7D7D7D7
#define STM_CID_2x16BIT_M29W800DT	0x22D722D7
static inline void STM_M29W800DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "m29W800DT (top BB)\n" );		
	block_size			= 64 * 1024;	
	num_of_blocks			= 15;		
	top_inter_size			= 32 * 1024;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 16 * 1024;	
}
static inline void STM_2x_M29W800DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "2x m29W800DT (top BB)\n" );		
	block_size			= 2 * 64 * 1024;
	num_of_blocks			= 15;		
	top_inter_size			= 2 * 32 * 1024;
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 2 * 16 * 1024;
}
static inline void STM_4x_M29W800DT(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "4x m29W800DT (top BB)\n" );		
	block_size			= 4 * 64 * 1024;
	num_of_blocks			= 15;		
	top_inter_size			= 4 * 32 * 1024;
	top_parameter_size		= 4 * 8 * 1024;	
	num_of_top_parameter_blocks	= 2;		
	top_boot_size			= 4 * 16 * 1024;
}


/*
 *
 */
int jt_amdflashGetInfoByte( uint32_t base_address, struct sector **sector_info )
{
	uint8_t vendor_id, chip_id, prot;
	uint8_t raw[3];
	int block_size, num_of_blocks; 

	/*set default info to no info*/
	if(sector_info != NULL)
		*sector_info = NULL;
	block_size			= 0;
	num_of_blocks			= 0;
	bottom_boot_size		= 0;
	bottom_parameter_size		= 0;
	num_of_bottom_parameter_blocks	= 0;
	bottom_inter_size		= 0;
	top_boot_size			= 0;
	top_parameter_size		= 0;
	num_of_top_parameter_blocks	= 0;
	top_inter_size			= 0;
	lock				= 0;
	num_of_regions			= 0;
	num_of_blocks_per_region	= 0;

	/*read plain memory*/
	raw[0] = jtag_arm_ReadByte( base_address + 0x00uL ) ;
	raw[1] = jtag_arm_ReadByte( base_address + 0x01uL ) ;
	raw[2] = jtag_arm_ReadByte( base_address + 0x02uL ) ;

	/*request chip info*/
	jtag_arm_WriteByte(base_address + 0x555uL, 0xaa ); 
	jtag_arm_WriteByte(base_address + 0x2AAuL, 0x55 );
	jtag_arm_WriteByte(base_address + 0x555uL, 0x90 );
	
	/*read out info*/
	vendor_id = jtag_arm_ReadByte( base_address + 0x00uL ) ;
	chip_id   = jtag_arm_ReadByte( base_address + 0x01uL ) ;
	prot      = jtag_arm_ReadByte( base_address + 0x02uL ) ;
	
	/*Reset the device*/
	jtag_arm_WriteByte(base_address ,0xF0);
	
	if(raw[0] == vendor_id && raw[1] == chip_id && raw[2] == prot)
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not realy AMD Flash base is equal vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		return 0; /* it is possible but not realistic that the ID's placed here */
	}

	/*show result*/
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: AMD/Fujitsu Flash\n\tManufacturer: " );
	switch (vendor_id) 
	{
	case AMD_VID_8BIT:
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "AMD \n\ttype: " );
		switch (chip_id) 
		{
		case AMD_CID_8BIT_AM29LV040B:	AMD_AM29LV040B(); break;
		case AMD_CID_8BIT_AM29F080B:	AMD_AM29F080B(); break;
		case AMD_CID_8BIT_AM29LV640D:	AMD_AM29LV640D(); break;
		case AMD_CID_8BIT_AM29LV800BB:	AMD_AM29LV800BB(); break;		
		case AMD_CID_8BIT_AM29LV800BT:	AMD_AM29LV800BT(); break;
		case AMD_CID_8BIT_AM29LV400BB:	AMD_AM29LV400BB(); break;
		case AMD_CID_8BIT_AM29LV400BT:	AMD_AM29LV400BT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "\n\tProtected: %04x\n", prot );
		break;
	case ATMEL_VID_8BIT:
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"Atmel \n\ttype: ");
		switch (chip_id)
		{
		case ATMEL_CID_8BIT_AT49LV1604A:	ATMEL_AT49LV1604A(); break;
		case ATMEL_CID_8BIT_AT49LV1604AT:	ATMEL_AT49LV1604AT(); break;
		case ATMEL_CID_8BIT_AT49LV4096A:	ATMEL_AT49LV4096A(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
	case STM_VID_8BIT: 
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"STM \n\ttype: ");
		switch (chip_id) 
		{
		case STM_CID_8BIT_M29F400BB:	STM_M29F400BB(); break;
		case STM_CID_8BIT_M29F400BT:	STM_M29F400BT(); break;		
		case STM_CID_8BIT_M29W400DB:	STM_M29W400DB(); break;
		case STM_CID_8BIT_M29W400DT:	STM_M29W400DT(); break;
		case STM_CID_8BIT_M29W800DB:	STM_M29W800DB(); break;
		case STM_CID_8BIT_M29W800DT:	STM_M29W800DT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		break;
	
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		chips_per_bus = 0;
		break;
	}
	/*allocate memory for sector list and fill in its data*/
	if(chips_per_bus != 0 && sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return chips_per_bus;
}


/*
 *
 */
int jt_amdflashGetInfoHalfword( uint32_t base_address, struct sector **sector_info )
{
	uint16_t vendor_id, chip_id, prot;
	uint16_t raw[3];

	/*set default info to no info*/
	if(sector_info != NULL)
		*sector_info = NULL;
	block_size			= 0;
	num_of_blocks			= 0;
	bottom_boot_size		= 0;
	bottom_parameter_size		= 0;
	num_of_bottom_parameter_blocks	= 0;
	bottom_inter_size		= 0;
	top_boot_size			= 0;
	top_parameter_size		= 0;
	num_of_top_parameter_blocks	= 0;
	top_inter_size			= 0;
	lock				= 0;
	num_of_regions			= 0;
	num_of_blocks_per_region	= 0;
	
	/*read plain memory*/
	raw[0] = jtag_arm_ReadHalfword( base_address +(0x00uL<<1) ) ;
	raw[1] = jtag_arm_ReadHalfword( base_address +(0x01uL<<1) ) ;
	raw[2] = jtag_arm_ReadHalfword( base_address +(0x02uL<<1) ) ;

	/*request chip info*/
	jtag_arm_WriteHalfword(base_address +(0x555uL<<1), 0xAAAA ); 
	jtag_arm_WriteHalfword(base_address +(0x2AAuL<<1), 0x5555 );
	jtag_arm_WriteHalfword(base_address +(0x555uL<<1), 0x9090 );
	
	/*read out info*/
	vendor_id = jtag_arm_ReadHalfword( base_address +(0x00uL<<1) ) ;
	chip_id   = jtag_arm_ReadHalfword( base_address +(0x01uL<<1) ) ;
	prot      = jtag_arm_ReadHalfword( base_address +(0x02uL<<1) ) ;
	
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address ,0xF0F0);
	
	if(raw[0] == vendor_id && raw[1] == chip_id && raw[2] == prot)
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not realy AMD Flash base is equal vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		return 0; /* it is possible but not realistic that the ID's placed here */
	}
	/*show result*/
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: AMD/Fujitsu Flash\n\tManufacturer: " );
	switch (vendor_id) 
	{
	case AMD_VID_2x8BIT: /* 2x 8 - Bit mode	*/
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "AMD (8 Bit mode)\n\ttype: " );
		switch (chip_id) 
		{
		case AMD_CID_2x8BIT_AM29LV040B:		AMD_2x_AM29LV040B(); break;
		case AMD_CID_2x8BIT_AM29F080B:		AMD_2x_AM29F080B(); break;
		case AMD_CID_2x8BIT_AM29LV800BB:	AMD_2x_AM29LV800BB(); break;
		case AMD_CID_2x8BIT_AM29LV800BT:	AMD_2x_AM29LV800BT(); break;
		case AMD_CID_2x8BIT_AM29LV400BB:	AMD_2x_AM29LV400BB(); break;
		case AMD_CID_2x8BIT_AM29LV400BT:	AMD_2x_AM29LV400BT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "\n\tProtected: %04x\n", prot );
		break;	
	case AMD_VID_16BIT: /* 16 - Bit mode */
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "AMD (16 Bit mode)\n\ttype: " );
		switch (chip_id) 
		{
		case AMD_CID_16BIT_AM29LV640D:	AMD_AM29LV640D(); break;
		case AMD_CID_16BIT_AM29LV800BB:	AMD_AM29LV800BB(); break;
		case AMD_CID_16BIT_AM29LV800BT:	AMD_AM29LV800BT(); break;
		case AMD_CID_16BIT_AM29LV400BB:	AMD_AM29LV400BB(); break;
		case AMD_CID_16BIT_AM29LV400BT:	AMD_AM29LV400BT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "\n\tProtected: %04x\n", prot );
		break;	
	case ATMEL_VID_2x8BIT: /* 2x 8 - Bit mode */
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"Atmel (8 Bit mode)\n\ttype: ");
		switch (chip_id)
		{
		case ATMEL_CID_2x8BIT_AT49LV1604A:	ATMEL_2x_AT49LV1604A(); break;
		case ATMEL_CID_2x8BIT_AT49LV1604AT:	ATMEL_2x_AT49LV1604AT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
	case ATMEL_VID_16BIT:     /* 16 - Bit mode */
	case ATMEL_VID_16BIT_ALT: /* 16 - Bit mode */
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"Atmel (16 Bit mode)\n\ttype: ");
		switch (chip_id)
		{
		case ATMEL_CID_16BIT_AT49LV1604A:	ATMEL_AT49LV1604A(); break;
		case ATMEL_CID_16BIT_AT49LV1604AT:	ATMEL_AT49LV1604AT(); break;
		case ATMEL_CID_16BIT_AT49LV4096A:	ATMEL_AT49LV4096A(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}		
	case STM_VID_2x8BIT: /* 2x 8 - Bit mode	*/
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"STM (8 Bit mode)\ttype: ");
		switch (chip_id) 
		{
		case STM_CID_2x8BIT_M29W400DB:	STM_2x_M29W400DB(); break;
		case STM_CID_2x8BIT_M29W400DT:	STM_2x_M29W400DT(); break;
		case STM_CID_2x8BIT_M29W800DB:	STM_2x_M29W800DB(); break;
		case STM_CID_2x8BIT_M29W800DT:	STM_2x_M29W800DT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
	case STM_VID_16BIT: /* 16 - Bit mode */
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"STM (16 Bit mode)\ttype: ");
		switch (chip_id) 
		{
		case STM_CID_16BIT_M29F400BB:	STM_M29F400BB(); break;
		case STM_CID_16BIT_M29F400BT:	STM_M29F400BT(); break;
		case STM_CID_16BIT_M29W400DB:	STM_M29W400DB(); break;
		case STM_CID_16BIT_M29W400DT:	STM_M29W400DT(); break;
		case STM_CID_16BIT_M29W800DB:	STM_M29W800DB(); break;
		case STM_CID_16BIT_M29W800DT:	STM_M29W800DT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		break;
	
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		chips_per_bus = 0;
		break;
	}
	/*allocate memory for sector list and fill in its data*/
	if(chips_per_bus != 0 && sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return chips_per_bus;
}

/*
 *
 */
int jt_amdflashGetInfoWord( uint32_t base_address, struct sector **sector_info )
{
	uint32_t vendor_id, chip_id, prot;
	uint32_t raw[3];

	/*set default info to no info*/
	if(sector_info != NULL)
		*sector_info = NULL;
	block_size			= 0;
	num_of_blocks			= 0;
	bottom_boot_size		= 0;
	bottom_parameter_size		= 0;
	num_of_bottom_parameter_blocks	= 0;
	bottom_inter_size		= 0;
	top_boot_size			= 0;
	top_parameter_size		= 0;
	num_of_top_parameter_blocks	= 0;
	top_inter_size			= 0;
	lock				= 0;
	num_of_regions			= 0;
	num_of_blocks_per_region	= 0;
	
	/*read plain memory*/
	raw[0] = jtag_arm_ReadWord( base_address +(0x00uL<<2) ) ;
	raw[1] = jtag_arm_ReadWord( base_address +(0x01uL<<2) ) ;
	raw[2] = jtag_arm_ReadWord( base_address +(0x02uL<<2) ) ;

	/*request chip info*/
	jtag_arm_WriteWord(base_address +(0x555uL<<2), 0xAAAAAAAA); 
	jtag_arm_WriteWord(base_address +(0x2AAuL<<2), 0x55555555);
	jtag_arm_WriteWord(base_address +(0x555uL<<2), 0x90909090);
	
	/*read out info*/
	vendor_id = jtag_arm_ReadWord( base_address +(0x00uL<<2) ) ;
	chip_id   = jtag_arm_ReadWord( base_address +(0x01uL<<2) ) ;
	prot      = jtag_arm_ReadWord( base_address +(0x02uL<<2) ) ;
	
	/*Reset the device*/
	jtag_arm_WriteWord(base_address ,0xF0F0F0F0);
	
	if(raw[0] == vendor_id && raw[1] == chip_id && raw[2] == prot)
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not realy AMD Flash base is equal vendor (ID 0x%08x) and devive (ID 0x%08x)\n", vendor_id, chip_id );
		return 0; /* it is possible but not realistic that the ID's placed here */
	}
	/*show result*/
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: AMD/Fujitsu Flash\n\tManufacturer: " );
	switch (vendor_id) 
	{
	case AMD_VID_4x8BIT: /* 4x 8 - Bit mode	*/
		chips_per_bus = 4;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "AMD (8 Bit mode)\n\ttype: " );
		switch (chip_id) 
		{
		case AMD_CID_4x8BIT_AM29LV040B:		AMD_4x_AM29LV040B(); break;
		case AMD_CID_4x8BIT_AM29F080B:		AMD_4x_AM29F080B(); break;
		case AMD_CID_4x8BIT_AM29LV800BB:	AMD_4x_AM29LV800BB(); break;
		case AMD_CID_4x8BIT_AM29LV800BT:	AMD_4x_AM29LV800BT(); break;
		case AMD_CID_4x8BIT_AM29LV400BB:	AMD_4x_AM29LV400BB(); break;
		case AMD_CID_4x8BIT_AM29LV400BT:	AMD_4x_AM29LV400BT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "\n\tProtected: %04x\n", prot );
		break;	
	case AMD_VID_2x16BIT: /* 2x 16 - Bit mode */
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "AMD (16 Bit mode)\n\ttype: " );
		switch (chip_id) 
		{
		case AMD_CID_2x16BIT_AM29LV640D:	AMD_2x_AM29LV640D(); break;
		case AMD_CID_2x16BIT_AM29LV800BB:	AMD_2x_AM29LV800BB(); break;
		case AMD_CID_2x16BIT_AM29LV800BT:	AMD_2x_AM29LV800BT(); break;
		case AMD_CID_2x16BIT_AM29LV400BB:	AMD_2x_AM29LV400BB(); break;
		case AMD_CID_2x16BIT_AM29LV400BT:	AMD_2x_AM29LV400BT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "\n\tProtected: %04x\n", prot );
		break;	
	case ATMEL_VID_4x8BIT: /* 4x 8 - Bit mode */
		chips_per_bus = 4;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"Atmel (8 Bit mode)\n\ttype: ");
		switch (chip_id)
		{
		case ATMEL_CID_4x8BIT_AT49LV1604A:	ATMEL_4x_AT49LV1604A(); break;
		case ATMEL_CID_4x8BIT_AT49LV1604AT:	ATMEL_4x_AT49LV1604AT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
	case ATMEL_VID_2x16BIT:     /* 2x 16 - Bit mode */
	case ATMEL_VID_2x16BIT_ALT: /* 2x 16 - Bit mode */
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"Atmel (16 Bit mode)\n\ttype: ");
		switch (chip_id)
		{
		case ATMEL_CID_2x16BIT_AT49LV1604A:	ATMEL_2x_AT49LV1604A(); break;
		case ATMEL_CID_2x16BIT_AT49LV1604AT:	ATMEL_2x_AT49LV1604AT(); break;
		case ATMEL_CID_2x16BIT_AT49LV4096A:	ATMEL_2x_AT49LV4096A(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}		
	case STM_VID_4x8BIT: /* 4x 8 - Bit mode	*/
		chips_per_bus = 4;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"STM (8 Bit mode)\ttype: ");
		switch (chip_id) 
		{
		case STM_CID_4x8BIT_M29W400DB:	STM_4x_M29W400DB(); break;
		case STM_CID_4x8BIT_M29W400DT:	STM_4x_M29W400DT(); break;
		case STM_CID_4x8BIT_M29W800DB:	STM_4x_M29W800DB(); break;
		case STM_CID_4x8BIT_M29W800DT:	STM_4x_M29W800DT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
	case STM_VID_2x16BIT: /* 2x 16 - Bit mode */
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"STM (16 Bit mode)\ttype: ");
		switch (chip_id) 
		{
		case STM_CID_2x16BIT_M29F400BB:	STM_2x_M29F400BB(); break;
		case STM_CID_2x16BIT_M29F400BT:	STM_2x_M29F400BT(); break;
		case STM_CID_2x16BIT_M29W400DB:	STM_2x_M29W400DB(); break;
		case STM_CID_2x16BIT_M29W400DT:	STM_2x_M29W400DT(); break;
		case STM_CID_2x16BIT_M29W800DB:	STM_2x_M29W800DB(); break;
		case STM_CID_2x16BIT_M29W800DT:	STM_2x_M29W800DT(); break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (ID 0x%04x)\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		break;
	
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		chips_per_bus = 0;
		break;
	}
	/*allocate memory for sector list and fill in its data*/
	if(chips_per_bus != 0 && sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return chips_per_bus;
}


