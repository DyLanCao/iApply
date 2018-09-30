/*
 * jt_flash_info_philips.c
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
 * Philips LPC 21xx and 22xx on chip controller
 */

static inline void PHILIPS_128K(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"Philips LPC 21xx 128KByte\n");
	bottom_parameter_size		= 8 * 1024;
	num_of_bottom_parameter_blocks	= 8;
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 7;		
	top_boot_size			= 8 * 1024;
	lock				= 1;
}

static inline void PHILIPS_256K(void) 
{						
	dbgPrintf(DBG_LEVEL_JTAG_ARM,"Philips LPC 21xx 256KByte\n");
	bottom_parameter_size		= 8 * 1024;
	num_of_bottom_parameter_blocks	= 8;
	block_size			= 64 * 1024;
	num_of_blocks			= 2;		
	top_parameter_size		= 8 * 1024;	
	num_of_top_parameter_blocks	= 7;		
	top_boot_size			= 8 * 1024;
	lock				= 1;
}



/*
 *
 */
int jt_philipsflashGetInfo( uint32_t base_address, uint32_t size, struct sector **sector_info )
{
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

	
	if( base_address == 0 )
	{
		if(size == 128 *  1024)
			PHILIPS_128K();
		else if (size == 256 *  1024)
			PHILIPS_256K();
		else
		{
			dbgPrintf(DBG_LEVEL_JTAG_ARM,"unknown size length\n");
			return 0;
		}
	}
	else
	{
		dbgPrintf(DBG_LEVEL_JTAG_ARM,"This can't be an philips Onchip controller\n" );
		return 0;
	}

	/*allocate memory for sector list and fill in its data*/
	if( sector_info != NULL)
		jt_flash_create_sector_info( sector_info );
	return 1;
}


