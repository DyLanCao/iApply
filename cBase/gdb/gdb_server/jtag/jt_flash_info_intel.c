/*
 * jt_flash_info_intel.c
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
 * Intel
 */
#define	INTEL_VID_8BIT		0x89
#define	INTEL_VID_2x8BIT	0x8989
#define INTEL_VID_16BIT		0x0089
#define INTEL_VID_2x16BIT	0x00890089

#define INTEL_CID_8BIT_28F004B3_T	0xD4
#define INTEL_CID_8BIT_28F008B3_T	0xD2
#define INTEL_CID_8BIT_28F016B3_T	0xD0
#define INTEL_CID_8BIT_28F004B3_B	0xD5
#define INTEL_CID_8BIT_28F008B3_B	0xD3
#define INTEL_CID_8BIT_28F016B3_B	0xD1
#define INTEL_CID_2x8BIT_28F004B3_T	0xD4D4
#define INTEL_CID_2x8BIT_28F008B3_T	0xD2D2
#define INTEL_CID_2x8BIT_28F016B3_T	0xD0D0
#define INTEL_CID_2x8BIT_28F004B3_B	0xD5D5
#define INTEL_CID_2x8BIT_28F008B3_B	0xD3D3
#define INTEL_CID_2x8BIT_28F016B3_B	0xD1D1

static inline void INTEL_28F004B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F004B3 8bit top (512KByte) 14x 32KByte, 8x 8KByte\n");
	block_size			= 32 * 1024;
	num_of_blocks			= 14;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_28F004B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F004B3 8bit bottom (512KByte 8x 8KByte, 14x 32KByte)\n");
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 14;		
}
static inline void INTEL_28F008B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F008B3 8bit top (1MByte) 30x 32KByte, 8x 8KByte\n");
	block_size			= 32 * 1024;
	num_of_blocks			= 30;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_28F008B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F008B3 8bit bottom (1MByte 8x 8KByte, 30x 32KByte)\n");
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 30;		
}
static inline void INTEL_28F016B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F016B3 8bit top (2MByte) 62x 32KByte, 8x 8KByte\n");
	block_size			= 32 * 1024;
	num_of_blocks			= 62;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_28F016B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"28F016B3 8bit bottom (2MByte 8x 8KByte, 62x 32KByte)\n");
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 62;		
}



static inline void INTEL_2x_28F004B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F004B3 8bit top (512KByte) 14x 32KByte, 8x 8KByte\n");
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 14;		
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_2x_28F004B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F004B3 8bit bottom (512KByte 8x 8KByte, 14x 32KByte)\n");
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 14;		
}
static inline void INTEL_2x_28F008B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F008B3 8bit top (1MByte) 30x 32KByte, 8x 8KByte\n");
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 30;		
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_2x_28F008B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F008B3 8bit bottom (1MByte 8x 8KByte, 30x 32KByte)\n");
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 30;		
}
static inline void INTEL_2x_28F016B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F016B3 8bit top (2MByte) 62x 32KByte, 8x 8KByte\n");
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 62;		
	top_parameter_size		= 2 * 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}
static inline void INTEL_2x_28F016B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"2x 28F016B3 8bit bottom (2MByte 8x 8KByte, 62x 32KByte)\n");
	bottom_parameter_size		= 2 * 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 2 * 32 * 1024;
	num_of_blocks			= 62;		
}


#define INTEL_CID_16Bit_28F400B3_T	0x8894
#define INTEL_CID_16Bit_28F800B3_T	0x8892
#define INTEL_CID_16Bit_28F160B3_T	0x8890
#define INTEL_CID_16Bit_28F320B3_T	0x8896
#define INTEL_CID_16Bit_28F640B3_T	0x8898

static inline void INTEL_28F400B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F400B3 16Bit top (512KByte) 14x 32KByte, 8x 8KByte\n" );
	block_size			= 32 * 1024;
	num_of_blocks			= 14;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

static inline void INTEL_28F800B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F800B3 16Bit top (1MByte) 30x 32KByte, 8x 8KByte\n" );
	block_size			= 32 * 1024;
	num_of_blocks			= 30;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

static inline void INTEL_28F160B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F160B3 16Bit top (2MByte) 62x 32KByte, 8x 8KByte\n" );
	block_size			= 32 * 1024;
	num_of_blocks			= 62;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

static inline void INTEL_28F320B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F320B3 16Bit top (4MByte) 126x 32KByte, 8x 8KByte\n" );
	block_size			= 32 * 1024;
	num_of_blocks			= 126;	
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

static inline void INTEL_28F640B3_T(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F640B3 16Bit top (8MByte) 254x 32KByte, 8x 8KByte\n" );
	block_size			= 32 * 1024;
	num_of_blocks			= 254;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 8;		
}

		
#define INTEL_CID_16Bit_28F400B3_B	0x8895
#define INTEL_CID_16Bit_28F800B3_B	0x8893
#define INTEL_CID_16Bit_28F160B3_B	0x8891
#define INTEL_CID_16Bit_28F320B3_B	0x8897
#define INTEL_CID_16Bit_28F640B3_B	0x8899

static inline void INTEL_28F400B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F400B3 16Bit bottom (512KByte 8x 8KByte, 14x 32KByte)\n" );
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 14;		
}
static inline void INTEL_28F800B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F800B3 16Bit bottom (1MByte 8x 8KByte, 30x 32KByte)\n" );
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 30;		
}
static inline void INTEL_28F160B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F160B3 16Bit bottom (2MByte 8x 8KByte, 62x 32KByte)\n" );
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 62;		
}
static inline void INTEL_28F320B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F320B3 16Bit bottom (4MByte 8x 8KByte, 126x 32KByte)\n" );
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 126;		
}
static inline void INTEL_28F640B3_B(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F640B3 16Bit bottom (8MByte 8x 8KByte, 254x 32KByte)\n" );
	bottom_parameter_size		= 8 * 1024;	
	num_of_bottom_parameter_blocks	= 8;		
	block_size			= 32 * 1024;
	num_of_blocks			= 254;		
}

#define INTEL_CID_8BIT_28F320J3		0x16
#define INTEL_CID_2x8BIT_28F320J3	0x1616
#define INTEL_CID_4x8BIT_28F320J3	0x16161616
#define INTEL_CID_16BIT_28F320J3	0x0016
#define INTEL_CID_2x16BIT_28F320J3	0x00160016
static inline void INTEL_28F320J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F320J3 4MByte 32x128K (lock)\n" );
	block_size			= 128 * 1024;
	num_of_blocks			= 32;
	lock = 1;
}
static inline void INTEL_2x_28F320J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "dual 28F320J3 4MByte 32x128K (lock)\n" );
	block_size			= 2 * 128 * 1024;
	num_of_blocks			= 32;		
	lock = 1;
}
static inline void INTEL_4x_28F320J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "quad 28F320J3 4MByte 32x128K (lock)\n" );
	block_size			= 4 * 128 * 1024;
	num_of_blocks			= 32;		
	lock = 1;
}

#define INTEL_CID_8BIT_28F640J3		0x17
#define INTEL_CID_2x8BIT_28F640J3	0x1717
#define INTEL_CID_4x8BIT_28F640J3	0x17171717
#define INTEL_CID_16BIT_28F640J3	0x0017
#define INTEL_CID_2x16BIT_28F640J3	0x00170017
static inline void INTEL_28F640J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F640J3 8MByte 64x128K (lock)\n" );
	block_size			= 128 * 1024;
	num_of_blocks			= 64;
	lock = 1;
}
static inline void INTEL_2x_28F640J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "dual 28F640J3 8MByte 64x128K (lock)\n" );
	block_size			= 2 * 128 * 1024;
	num_of_blocks			= 64;		
	lock = 1;
}
static inline void INTEL_4x_28F640J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "quad 28F640J3 8MByte 64x128K (lock)\n" );
	block_size			= 4 * 128 * 1024;
	num_of_blocks			= 64;		
	lock = 1;
}

#define INTEL_CID_8BIT_28F128J3		0x18
#define INTEL_CID_2x8BIT_28F128J3	0x1818
#define INTEL_CID_4x8BIT_28F128J3	0x18181818
#define INTEL_CID_16BIT_28F128J3	0x0018
#define INTEL_CID_2x16BIT_28F128J3	0x00180018
static inline void INTEL_28F128J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F128J3 16MByte 128x128K (lock)\n" );
	block_size			= 128 * 1024;
	num_of_blocks			= 128;
	lock = 1;
}
static inline void INTEL_2x_28F128J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "dual 28F128J3 16MByte 128x128K (lock)\n" );
	block_size			= 2 * 128 * 1024;
	num_of_blocks			= 128;		
	lock = 1;
}
static inline void INTEL_4x_28F128J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "quad 28F128J3 16MByte 128x128K (lock)\n" );
	block_size			= 4 * 128 * 1024;
	num_of_blocks			= 128;		
	lock = 1;
}

#define INTEL_CID_8BIT_28F256J3		0x1D
#define INTEL_CID_2x8BIT_28F256J3	0x1D1D
#define INTEL_CID_4x8BIT_28F256J3	0x1D1D1D1D
#define INTEL_CID_16BIT_28F256J3	0x001D
#define INTEL_CID_2x16BIT_28F256J3	0x001D001D
static inline void INTEL_28F256J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F256J3 32MByte 128x128K (lock)\n" );
	block_size			= 128 * 1024;
	num_of_blocks			= 256;
	lock = 1;
}
static inline void INTEL_2x_28F256J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "dual 28F256J3 32MByte 128x128K (lock)\n" );
	block_size			= 2 * 128 * 1024;
	num_of_blocks			= 256;		
	lock = 1;
}
static inline void INTEL_4x_28F256J3(void) 
{	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "quad 28F256J3 32MByte 128x128K (lock)\n" );
	block_size			= 4 * 128 * 1024;
	num_of_blocks			= 256;		
	lock = 1;
}



/*
 *
 */
int jt_intelflashGetInfoByte( uint32_t base_address, struct sector **sector_info )
{
	uint8_t vendor_id, chip_id;
	uint8_t raw[3];

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
	raw[0] = jtag_arm_ReadByte( base_address +(0x00uL) ) ;
	raw[1] = jtag_arm_ReadByte( base_address +(0x01uL) ) ;

	/* Clear Status Register */
	jtag_arm_WriteByte(base_address , 0x50 );

	/* Read Identifier Command */
	jtag_arm_WriteByte(base_address , 0x90 );

	/*read out info*/
	vendor_id = jtag_arm_ReadByte( base_address +(0x00uL<<1) ) ;
	chip_id   = jtag_arm_ReadByte( base_address +(0x01uL<<1) ) ;
	
	/*back to Read Array mode*/
	jtag_arm_WriteByte(base_address , 0xFF );
	
	if(raw[0] == vendor_id && raw[1] == chip_id )
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not realy Intel\nFlash base is equal vendor (ID 0x%02x) and devive (ID 0x%02x)\n", vendor_id, chip_id );
		return 0; /* it is possible but not realistic that the ID's placed here */
	}
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: Intel Flash\n\tManufacturer: " );
	switch (vendor_id) 
	{
	case INTEL_VID_8BIT:
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Intel (8Bit)\n\ttype: ");
		switch (chip_id) 
		{
		case INTEL_CID_8BIT_28F004B3_T: INTEL_28F004B3_T(); break;
		case INTEL_CID_8BIT_28F008B3_T: INTEL_28F008B3_T(); break;
		case INTEL_CID_8BIT_28F016B3_T: INTEL_28F016B3_T(); break;
		case INTEL_CID_8BIT_28F004B3_B: INTEL_28F004B3_B(); break;
		case INTEL_CID_8BIT_28F008B3_B: INTEL_28F008B3_B(); break;
		case INTEL_CID_8BIT_28F016B3_B: INTEL_28F016B3_B(); break;
		case INTEL_CID_8BIT_28F320J3:	INTEL_28F320J3(); break;
		case INTEL_CID_8BIT_28F640J3:	INTEL_28F640J3(); break;
		case INTEL_CID_8BIT_28F128J3:	INTEL_28F128J3(); break;
		case INTEL_CID_8BIT_28F256J3:	INTEL_28F256J3(); break;

		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown (0x%02X)!\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		break;
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown manufacturer (0x%04X)!\n", vendor_id);
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
int jt_intelflashGetInfoHalfword( uint32_t base_address, struct sector **sector_info )
{
	uint16_t vendor_id, chip_id;
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

	/* Clear Status Register */
	jtag_arm_WriteHalfword(base_address , 0x5050 );

	/* Read Identifier Command */
	jtag_arm_WriteHalfword(base_address , 0x9090 );

	/*read out info*/
	vendor_id = jtag_arm_ReadHalfword( base_address +(0x00uL<<1) ) ;
	chip_id   = jtag_arm_ReadHalfword( base_address +(0x01uL<<1) ) ;
	
	/*back to Read Array mode*/
	jtag_arm_WriteHalfword(base_address , 0xFFFF );
	
	if(raw[0] == vendor_id && raw[1] == chip_id )
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"not realy Intel\nFlash base is equal vendor (ID 0x%04x) and devive (ID 0x%04x)\n", vendor_id, chip_id );
		return 0; /* it is possible but not realistic that the ID's placed here */
	}
	
	dbgPrintf(DBG_LEVEL_JTAG_ARM, "Chip: Intel Flash\n\tManufacturer: " );
	switch (vendor_id) 
	{
	case INTEL_VID_2x8BIT:
		chips_per_bus = 2;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Intel (2x 8Bit)\n\ttype: ");
		switch (chip_id) 
		{
		case INTEL_CID_2x8BIT_28F004B3_T: INTEL_2x_28F004B3_T(); break;
		case INTEL_CID_2x8BIT_28F008B3_T: INTEL_2x_28F008B3_T(); break;
		case INTEL_CID_2x8BIT_28F016B3_T: INTEL_2x_28F016B3_T(); break;
		case INTEL_CID_2x8BIT_28F004B3_B: INTEL_2x_28F004B3_B(); break;
		case INTEL_CID_2x8BIT_28F008B3_B: INTEL_2x_28F008B3_B(); break;
		case INTEL_CID_2x8BIT_28F016B3_B: INTEL_2x_28F016B3_B(); break;
		case INTEL_CID_2x8BIT_28F320J3:	 INTEL_2x_28F320J3(); break;
		case INTEL_CID_2x8BIT_28F640J3:	 INTEL_2x_28F640J3(); break;
		case INTEL_CID_2x8BIT_28F128J3:	 INTEL_2x_28F128J3(); break;
		case INTEL_CID_2x8BIT_28F256J3:	 INTEL_2x_28F256J3(); break;

		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (0x%02X)!\n", chip_id );
			chips_per_bus = 0;
			break;
		}

	case INTEL_VID_16BIT:
		chips_per_bus = 1;
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Intel (16 Bit)\n\ttype: ");
		switch (chip_id) 
		{
		case INTEL_CID_16Bit_28F400B3_T: INTEL_28F400B3_T(); break;
		case INTEL_CID_16Bit_28F800B3_T: INTEL_28F800B3_T(); break;
		case INTEL_CID_16Bit_28F160B3_T: INTEL_28F160B3_T(); break;
		case INTEL_CID_16Bit_28F320B3_T: INTEL_28F320B3_T(); break;
		case INTEL_CID_16Bit_28F640B3_T: INTEL_28F640B3_T(); break;
		case INTEL_CID_16Bit_28F400B3_B: INTEL_28F400B3_B(); break;
		case INTEL_CID_16Bit_28F800B3_B: INTEL_28F800B3_B(); break;
		case INTEL_CID_16Bit_28F160B3_B: INTEL_28F160B3_B(); break;
		case INTEL_CID_16Bit_28F320B3_B: INTEL_28F320B3_B(); break;
		case INTEL_CID_16Bit_28F640B3_B: INTEL_28F640B3_B(); break;
		case INTEL_CID_16BIT_28F320J3:	 INTEL_28F320J3(); break;
		case INTEL_CID_16BIT_28F640J3:	 INTEL_28F640J3(); break;
		case INTEL_CID_16BIT_28F128J3:	 INTEL_28F128J3(); break;
		case INTEL_CID_16BIT_28F256J3:	 INTEL_28F256J3(); break;
	
		case 0x8801:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F640K3 64MB 64x 128K -16Bit-3V-(lock)\n" );
			break;
		case 0x8802:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F128K3 128MB 128x 128K -16Bit-3V-(lock)\n" );
			break;
		case 0x8803:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F256K3 256MB 256x 128K -16Bit-3V-(lock)\n" );
			break;
		case 0x8805:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F640K18 64MB 64x 128K -16Bit-1.8V-(lock)\n" );
			break;
		case 0x8806:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F128K18 128MB 128x 128K -16Bit-1.8V-(lock)\n" );
			break;
		case 0x8807:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "28F256K18 256MB 256x 128K -16Bit-1.8V-(lock)\n" );
			break;
		default:
			dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown device (0x%02X)!\n", chip_id );
			chips_per_bus = 0;
			break;
		}
		break;
	default:
		dbgPrintf(DBG_LEVEL_JTAG_ARM, "Unknown manufacturer (0x%04X)!\n", vendor_id);
		chips_per_bus = 0;
		break;
	}

	/*allocate memory for sector list and fill in its data*/
	if(chips_per_bus != 0 && sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return chips_per_bus;
}

/*
INTEL_CID_4x8BIT_28F320J3
INTEL_CID_4x8BIT_28F640J3
INTEL_CID_4x8BIT_28F128J3
INTEL_CID_4x8BIT_28F256J3

INTEL_CID_2x16BIT_28F320J3
INTEL_CID_2x16BIT_28F640J3
INTEL_CID_2x16BIT_28F128J3
INTEL_CID_2x16BIT_28F256J3
*/
