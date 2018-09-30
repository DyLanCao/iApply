/*
 * jt_flash_intel.c
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
#include "jt_instr.h"
#include "jt_flash.h"



/*
 * Support Function to write one Byte to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgByte(uint32_t base_address, uint32_t address, uint8_t data, unsigned wait)
{
	int error = 0;
	uint8_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteByte(base_address ,0x50);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteByte(address ,0x40); //or 0x10

	/*Write data*/
	jtag_arm_WriteByte(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadByte(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteByte(base_address,0xFF);

	return error;
}

/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorByte(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint8_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteByte(base_address ,0x50);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteByte(address ,0x60);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteByte(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadByte(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x38) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteByte(base_address,0xFF);

	return error;
}

/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashEraseSectorByte(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint8_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteByte(base_address ,0x50);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteByte(address ,0x20);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteByte(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadByte(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x2A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteByte(base_address,0xFF);

	return error;
}


/*
 * Support Function to write one Halfword to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgHalfword(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x50);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteHalfword(address ,0x40); //or 0x10

	/*Write data*/
	jtag_arm_WriteHalfword(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFF);
	return error;
}

/*
 * Support Function to write one Halfword to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgHalfword_dual(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x5050);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteHalfword(address ,0x4040); //or 0x1010

	/*Write data*/
	jtag_arm_WriteHalfword(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x8080) != 0) // is bit 7 set
		{
			if( (status & 0x1A1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFFFF);
	return error;
}

/*
 * Support Function to Unlock a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be unlocked
 *		var address - Addres within the sector which has to to be unlocked
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorHalfword(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x50);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteHalfword(address ,0x60);
	
	/*confirm starts the erase algorithem*/
	jtag_arm_WriteHalfword(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x38) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFF);
	return error;
}

/*
 * Support Function to Unlock a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be unlocked
 *		var address - Addres within the sector which has to to be unlocked
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorHalfword_dual(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x5050);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteHalfword(address ,0x6060);
	
	/*confirm starts the erase algorithem*/
	jtag_arm_WriteHalfword(address ,0xD0D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x8080) != 0) // is bit 7 set
		{
			if( (status & 0x3838) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFFFF);
	return error;
}



/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashEraseSectorHalfword(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x50);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteHalfword(address ,0x20);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteHalfword(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x2A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFF);
	return error;
}

/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashEraseSectorHalfword_dual(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteHalfword(base_address ,0x5050);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteHalfword(address ,0x2020);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteHalfword(address ,0xD0D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadHalfword(base_address);
		if( (status & 0x8080) != 0) // is bit 7 set
		{
			if( (status & 0x2A2A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteHalfword(base_address,0xFFFF);
	return error;
}

/*
 * Support Function to write one Word to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgWord(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait)
{
	int error = 0;
	uint32_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteWord(address ,0x40); //or 0x10

	/*Write data*/
	jtag_arm_WriteWord(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xff);
	return error;
}



/*
 * Support Function to write one Word to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgWord_dual(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait)
{
	int error = 0;
	uint32_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x00500050);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteWord(address ,0x00400040); //or 0x00100010

	/*Write data*/
	jtag_arm_WriteWord(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x00800080) != 0) // is bit 7 set
		{
			if( (status & 0x001A1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0x00ff00FF);
	return error;
}

/*
 * Support Function to write one Word to the Flash.
 * This is OK after an erase of a Sector.
 * 
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 *		var wait
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashProgWord_quad(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait)
{
	int error = 0;
	uint32_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50505050);
	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteWord(address ,0x40404040); //or 0x10101010

	/*Write data*/
	jtag_arm_WriteWord(address, data);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	while(wait)
	{
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80808080) != 0) // is bit 7 set
		{
			if( (status & 0x1A1A1A1A) != 0) // is bit 4,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	}
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xffffFFFF);
	return error;
}

/*
 * Support Function to Unlock a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be unlocked
 *		var address - Addres within the sector which has to to be unlocked
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorWord(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x60);
	
	/*confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x38) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xff);
	return error;
}

/*
 * Support Function to Unlock a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be unlocked
 *		var address - Addres within the sector which has to to be unlocked
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorWord_dual(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x00500050);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x00600060);
	
	/*confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0x00D000D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x00800080) != 0) // is bit 7 set
		{
			if( (status & 0x00380038) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0x00ff00ff);
	return error;
}


/*
 * Support Function to Unlock a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be unlocked
 *		var address - Addres within the sector which has to to be unlocked
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashUnlockSectorWord_quad(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50505050);
	
	/*Write Unlock Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x60606060);
	
	/*confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0xD0D0D0D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80808080) != 0) // is bit 7 set
		{
			if( (status & 0x38383838) != 0) // is bit 5,4 or 3 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xffffFFFF);
	return error;
}



/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int jt_intelflashEraseSectorWord(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x20);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0xD0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80) != 0) // is bit 7 set
		{
			if( (status & 0x2A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xff);
	return error;
}

/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */

int jt_intelflashEraseSectorWord_dual(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x00500050);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x00200020);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0x00D000D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x00800080) != 0) // is bit 7 set
		{
			if( (status & 0x002A002A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0x00ff00FF);
	return error;
}

/*
 * Support Function to Erase a Sector within the Flash
 *
 * Input: 
 * 		var base_address - First address of the Flash that has to be erased
 *		var address - Addres within the sector which has to to be erased
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */

int jt_intelflashEraseSectorWord_quad(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t status;
	int timeout;

	/*Clear status register*/
	jtag_arm_WriteWord(base_address ,0x50505050);
	
	/*Write Erase Setup Command to FlashROM and Tell Flash the sector address*/
	jtag_arm_WriteWord(address ,0x20202020);
	
	/*Erase confirm starts the erase algorithem*/
	jtag_arm_WriteWord(address ,0xD0D0D0D0);

	timeout = 1000; // maybe up to 6 seconds
	/*Wait for completion*/
	do {
		status = jtag_arm_ReadWord(base_address);
		if( (status & 0x80808080) != 0) // is bit 7 set
		{
			if( (status & 0x2A2A2A2A) != 0) // is bit 5,3 or 1 set
				error = 1;
			break;
		}
		else if(--timeout <= 0)
		{
			/*fail*/
			error = 1;
			break;
		}
	} while(1);
	/*Reset the device*/
	jtag_arm_WriteWord(base_address,0xffffFFFF);
	return error;
}

