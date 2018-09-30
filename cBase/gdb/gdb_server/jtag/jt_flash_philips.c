/*
 * jt_flash_philips.c
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
#include "jt_arm.h"
#include "jt_instr.h"
#include "jt_flash.h"

#define LPC_FLASH_CMD_STOP		0
#define LPC_FLASH_CMD_PROG_512BYTES_END	0x01
#define LPC_FLASH_CMD_ERASE		0x02
#define LPC_FLASH_CMD_UNLOCK_SECTORS	0x04
#define LPC_FLASH_CMD_LOCK_SECTORS	0x08
#define LPC_FLASH_CMD_PROG_128_BITS	0x10
#define LPC_FLASH_STATUS_MASK		0x1F

struct LpcFlashCtl {
	volatile unsigned init;			//((unsigned*)0x3fff8000)[0] -- part ID 0xFFF0FF32

	volatile unsigned* dst;			//((unsigned *)(0x3fff8000))[1]  write flash destination address
	volatile unsigned data[4];		//((unsigned *)(0x3fff8000))[2]  write flash source data Word 0
						//((unsigned *)(0x3fff8000))[3]  write flash source data Word 1
						//((unsigned *)(0x3fff8000))[4]  write flash source data Word 2
						//((unsigned *)(0x3fff8000))[5]  write flash source data Word 3

	volatile unsigned cmd;			//(unsigned *)(0x3fff8000))[6]	read status & 0x1F => BUSSY
						//(unsigned *)(0x3fff8000))[6]	write ctrl 	

	volatile unsigned mask_active_sectors;	//((unsigned*)0x3fff8000)[7]	active_mask 0x80000000 Bootsector

	volatile unsigned dst_offset;		//((unsigned *)(0x3fff8000))[8]	write flash destination word offset


	volatile unsigned mask_locked_sectors;	//((unsigned *)0x3fff8000)[9]	lock_mask

	volatile unsigned clk_ticks_a;		//((unsigned*)0x3fff8000)[10] = frequence / 200 ;[erase and  copy ram to flash]
	volatile unsigned clk_ticks_b;		//((unsigned*)0x3fff8000)[11] = ((frequence * 25)/ 128) + 1 ;[erase]
						//			      = (frequence / 2048) + 1 ;[copy ram to flash]
} ;

struct LpcFlashCtl * const lpcFlashCtl = (struct LpcFlashCtl *)0x3fff8000;

/*
 * Support Function to generate the sector mask of the Flash.
 * 
 * Input: 
 * 		var start_sector
 *		var stop_sector
 *		var flash_size
 *		var boot_sector_enabled
 * Output:
 *		none
 * Return:
 *		mask of sectors that can be used
 */
uint32_t jt_philipsflashGenMask( int start_sector, int stop_sector, uint32_t flash_size, int boot_sector_enabled)
{
	uint32_t msk = 0;
	int LpcHighMaskOffset, LpcFirstHigherSector, LpcLastSector;

	/* set default*/
	if(flash_size == 128 * 1024)
	{
		LpcLastSector = 15;
		LpcFirstHigherSector = 8;
		LpcHighMaskOffset = 16;
	}
	else if (flash_size == 256 * 1024)
	{
		LpcLastSector = 17;
		LpcFirstHigherSector = 10;
		LpcHighMaskOffset = 16;
	}
	else
		return 0;
	
	/*check if parameter are usable*/
	if( start_sector < 0 || start_sector > stop_sector || stop_sector > LpcLastSector )
		return 0;
	
	/*generate mask*/
	if(start_sector < LpcLastSector)
	{
		// but without boot sector
		if (stop_sector == LpcLastSector)
			stop_sector = LpcLastSector - 1;
		
		while ( start_sector <= stop_sector) 
		{
			if ( start_sector >= LpcFirstHigherSector)  
				msk |= 1 << ( LpcHighMaskOffset + start_sector );
			else
				msk |= 1 << start_sector;
			start_sector ++;
		}
	}
	else if (boot_sector_enabled)
		msk = 0x80000000;

	return msk;
}

#if 0
/*
 * Support Function to locate the start and stop sector of the Flash.
 * 
 * Input: 
 * 		var destination
 * 		var length
 * 		pointer to sector_info
 * 		pointer to start_sector
 *		pointer to stop_sector
 *		var flash_size
 *		var boot_sector_enabled
 * Output:
 *		none
 * Return:
 *		mask of sectors that can be used
 */
void jt_philipsflashFindStartStopSectors(uint32_t dst, int len, struct sector *sec, int *start_sector, int *stop_sector, uint32_t flash_size, int boot_sector_enabled)
{
	int LpcLastSector;
	uint32_t size;

	/* set default*/
	if(flash_size == 128 * 1024)
		LpcLastSector = 15;
	else if (flash_size == 256 * 1024)
		LpcLastSector = 17;
	else
		return;
	
	if(boot_sector_enabled == 0)
		LpcLastSector--;
	
	/*check if parameter are usable*/
	if( sec == NULL || start_sector == NULL || stop_sector == NULL )
		return;
	
	// len in bytes should be multiple of 512 and dst must be in 512 byte boundary
	if ( (len & 0x1FF) != 0 || (dst & 0x1FF) != 0 )
		return;
	
	*start_sector = 0;
	*stop_sector = 0;
	size = 0;

	/*search first match of start sector*/
	while ( *start_sector <= LpcLastSector )
	{
		size += sec->size;
		if ( dst < size )
			break;
		sec++;
		(*start_sector)++;
	}
	
	/*is the remainding stuff located within this or the next sector*/
	if ( (dst + len) >  size ) 
		*stop_sector = *start_sector + 1;
	else
		*stop_sector = *start_sector;
	return;
}
#endif


#if 0
/*
 * Support Function to Prepare a Sector for erasing or programming the Flash
 *
 * Input: 
 * 		var controller_base - address of the Flash controller that has to be used
 *		var mask - sector mask which has to to be used
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_philipsflashUnlockSector(uint32_t mask)
{
	uint32_t lock_msk, status;

	/*read status*/
	status = jtag_arm_ReadWord( (uint32_t)(&lpcFlashCtl->cmd) );
	if ( (status & LPC_FLASH_STATUS_MASK) != 0)
		return 1;
	
	/*read current lock mask*/
	lock_msk = jtag_arm_ReadWord( (uint32_t)(&lpcFlashCtl->mask_locked_sectors) );
	lock_msk &= ~mask;

	jtag_arm_WriteByte( (uint32_t) (&lpcFlashCtl->mask_active_sectors) , mask);
	jtag_arm_WriteByte( (uint32_t) (&lpcFlashCtl->mask_locked_sectors) , lock_msk);
	jtag_arm_WriteByte( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_UNLOCK_SECTORS);
	jtag_arm_WriteByte( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_STOP);
	
	return 0;
}


/*
 * Support Function to Finish a Sector for erasing or programming the Flash
 *
 * Input: 
 * 		var controller_base - address of the Flash controller that has to be used
 *		var mask - sector mask which has to to be used
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_philipsflashLockSector(uint32_t mask)
{
	uint32_t lock_msk, status;

	/*read status*/
	status = jtag_arm_ReadWord( (uint32_t)(&lpcFlashCtl->cmd) );
	if ( (status & LPC_FLASH_STATUS_MASK) != 0)
		return 1;
	
	/*read current lock mask*/
	lock_msk = jtag_arm_ReadWord( (uint32_t)(&lpcFlashCtl->mask_locked_sectors) );
	lock_msk |= mask;

	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->mask_active_sectors) , mask);
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->mask_locked_sectors) , lock_msk);
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_LOCK_SECTORS);
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_STOP);
	
	return 0;
}

/*
 * Support Function to Erase all unlocked Sectors within the Flash
 *
 * Input: 
 * 		var controller_base - address of the Flash controller that has to be used
 *		var frequence - targert processor clock in kHz
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_philipsflashEraseSector(uint32_t frequence)
{
	uint32_t status;

	if ( frequence < 1000 || frequence > 75000 )
	{
		printf("wrong frequence\n");
		return 3;
	}
		
	/*read status*/
	status = jtag_arm_ReadWord( (uint32_t)(&lpcFlashCtl->cmd) );
	if ( (status & LPC_FLASH_STATUS_MASK) != 0)
		return 2;
	
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->clk_ticks_a) , frequence / 200);
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->clk_ticks_b) , ((frequence * 25)/ 128) + 1);
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_ERASE);
	//wait ( delay_erase ) ; // = frequence * 80.02	->  80 msec
	jtag_arm_WriteWord( (uint32_t) (&lpcFlashCtl->cmd) , LPC_FLASH_CMD_STOP);

	return 0;
}
#endif


