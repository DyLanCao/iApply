/*
 * jt_flash_info_atmel.c
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
 * Atmel (SAM7)
 */

/*
 * EXT|NVPTYP  | ARCH               |SRAMSIZ    |NVPSIZ2    |NVPSIZ1  |EPROC|VERSION
 *  31|30 29 28|27 26 25 24 23 21 20|19 18 17 16|15 14 13 12|11 10 9 8|7 6 5|4 3 2 1 0
 */
#define	ATMEL_AT91SAM7_DBGU_CIDR			0xFFFFf240
#define	ATMEL_AT91SAM7_DBGU_CIDR_EPROC_MSK		(0x7<<5)
#define	ATMEL_AT91SAM7_DBGU_CIDR_EPROC_ARM7TDMI		(0x2<<5)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_MSK		(0xFF<<8)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_32K		(0x3<<8)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_64K		(0x5<<8)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_128K		(0x7<<8)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_256K		(0x9<<8)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_MSK		(0xF<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_8K		(0x8<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_16K		(0x9<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_32K		(0xA<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_64K		(0xB<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_128K		(0xC<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_SRAMSIZ_256K		(0xD<<16)
#define	ATMEL_AT91SAM7_DBGU_CIDR_ARCH_MSK		(0xFF<<20)
#define	ATMEL_AT91SAM7_DBGU_CIDR_ARCH_SAM7S		(0x70<<20)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVTPY_MSK		(0x7<<28)
#define	ATMEL_AT91SAM7_DBGU_CIDR_NVTPY_EMBEDDED_FLASH	(0x2<<28)
#define	ATMEL_AT91SAM7_DBGU_CIDR_EXT_MSK		(0x1<<31)
#define	ATMEL_AT91SAM7_DBGU_CIDR_EXT_NON		0x0

#define ATMEL_AT91SAM7_FLASH_BASE_ADDRESS		0x100000

static inline void ATMEL_AT91SAM7S32(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"at91sam7s32 32bit (32KByte) 8x 32x 128Byte\n");
	block_size			= 128;
	num_of_blocks			= 256;
	num_of_regions			= 8;
	num_of_blocks_per_region	= 32;
	chips_per_bus			= 1;
	lock				= 1;
}
static inline void ATMEL_AT91SAM7S64(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"at91sam7s64 32bit (64KByte) 16x 32x 128Byte\n");
	block_size			= 128;
	num_of_blocks			= 512;
	num_of_regions			= 16;
	num_of_blocks_per_region	= 32;
	chips_per_bus			= 1;
	lock				= 1;
}
static inline void ATMEL_AT91SAM7S128(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"at91sam7s128 32bit (128KByte) 8x 64x 256Byte\n");
	block_size			= 256;
	num_of_blocks			= 512;
	num_of_regions			= 8;
	num_of_blocks_per_region	= 64;
	chips_per_bus			= 1;
	lock				= 1;
}
static inline void ATMEL_AT91SAM7S256(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"at91sam7s256 32bit (256KByte) 8x 128x 256Byte\n");
	block_size			= 256;
	num_of_blocks			= 1024;
	num_of_regions			= 16;
	num_of_blocks_per_region	= 64;
	chips_per_bus			= 1;
	lock				= 1;
}


/*
 *
 */
int jt_atmelflashGetInfo( uint32_t base_address, struct sector **sector_info )
{
	uint32_t chip_id;

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
	top_inter_size 			= 0;
	lock 				= 0;
	num_of_regions			= 0;
	num_of_blocks_per_region	= 0;

	/*check base addrese resides at known position*/
	if(base_address != ATMEL_AT91SAM7_FLASH_BASE_ADDRESS)
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not an Embedded Flash of AT91SAM7\n wrong Address 0x%X\n", base_address);
		return 0; 
	}
	
	/*read out info*/
	chip_id   = jtag_arm_ReadWord( ATMEL_AT91SAM7_DBGU_CIDR ) ;
	
	/*make sure it is a SAM7 derivat*/
	if((chip_id & 
	    (	  ATMEL_AT91SAM7_DBGU_CIDR_EPROC_MSK
		| ATMEL_AT91SAM7_DBGU_CIDR_ARCH_MSK
		| ATMEL_AT91SAM7_DBGU_CIDR_NVTPY_MSK
		| ATMEL_AT91SAM7_DBGU_CIDR_EXT_MSK
	    )
	   ) != 
	   (
		  ATMEL_AT91SAM7_DBGU_CIDR_EPROC_ARM7TDMI
		| ATMEL_AT91SAM7_DBGU_CIDR_ARCH_SAM7S
		| ATMEL_AT91SAM7_DBGU_CIDR_NVTPY_EMBEDDED_FLASH
		| ATMEL_AT91SAM7_DBGU_CIDR_EXT_NON
	   ))
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not an Embedded Flash of AT91SAM7\nwrong ID 0x%X\n", chip_id );
		return 0; 
	}
	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: Emdedded Flash\n\tManufacturer: Atmel\n\ttype: " );
	switch (chip_id & ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_MSK)
	{
	case ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_32K:  ATMEL_AT91SAM7S32();break;
	case ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_64K:  ATMEL_AT91SAM7S64();break;
	case ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_128K: ATMEL_AT91SAM7S128();break;
	case ATMEL_AT91SAM7_DBGU_CIDR_NVPSIZ_256K: ATMEL_AT91SAM7S256();break;
			
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (0x%X)!\n", chip_id );
		chips_per_bus = 0;
		break;
	}

	/*allocate memory for sector list and fill in its data*/
	if(chips_per_bus != 0 && sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return chips_per_bus;
}


