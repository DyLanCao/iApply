/*
 * jt_flash_info.c
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



int block_size, num_of_blocks; 
int bottom_boot_size;	/* if set assume one block */
int bottom_parameter_size, num_of_bottom_parameter_blocks;
int bottom_inter_size;	/* if set assume one block */
int top_boot_size;	/* if set assume one block */
int top_parameter_size, num_of_top_parameter_blocks;
int top_inter_size;	/* if set assume one block */
int lock;
int num_of_regions;	/*number of atmel regions*/
int num_of_blocks_per_region;	/*number of segments per region*/

int chips_per_bus = 0;


/*
 *
 */
void jt_flash_create_sector_info(struct sector **sector_info )
{
	int num, i, rn, bcnt;
	struct sector *sec;
		
	/*add one more; the last has a size of zero to mark the end of list*/
	num = num_of_blocks
	    + num_of_bottom_parameter_blocks
	    + num_of_top_parameter_blocks
	    + 1;
	if(bottom_boot_size)
		num ++;
	if(bottom_inter_size)
		num ++;
	if(top_boot_size)
		num ++;
	if(top_inter_size)
		num ++;
		
	*sector_info = sec =(struct sector *) malloc(num * sizeof(struct sector));
	if(sec != NULL)
	{
		bzero(sec, num * sizeof(struct sector));
		if(bottom_boot_size)
		{
			sec->size = bottom_boot_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}
		for(i=0; i<num_of_bottom_parameter_blocks; i++)
		{
			sec->size = bottom_parameter_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}
		if(bottom_inter_size)
		{
			sec->size = bottom_inter_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}

		for(i=0,rn=0, bcnt=0; i<num_of_blocks; i++)
		{
			sec->size = block_size;
			if(lock)
				sec->lock = 1;
			sec->sectors_per_region = num_of_blocks_per_region;
			sec->region = rn;
			bcnt++;
			if(bcnt == num_of_blocks_per_region)
			{
				rn++;
				bcnt = 0;
			}
			sec++;
		}
		
		if(top_inter_size)
		{
			sec->size = top_inter_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}
		for(i=0; i<num_of_top_parameter_blocks; i++)
		{
			sec->size = top_parameter_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}
		if(top_boot_size)
		{
			sec->size = top_boot_size;
			if(lock)
				sec->lock = 1;
			sec++;
		}

	}
	else
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Non Mem. Could not generate sector info for FLASH\n");
	return;
}


