/*
 * jt_flash_amd.c
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
#include <sys/time.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_instr.h"
#include "jt_flash.h"


/*
 * Support Function to reset Flash back into Read Mode
 */
uint8_t jt_amdflashReadResetByte(uint32_t base_address, uint32_t address)
{
	uint8_t data;

	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteByte(base_address + 0x555L,0xAA);
	jtag_arm_WriteByte(base_address + 0x2AAL,0x55);
	jtag_arm_WriteByte(base_address + 0x555L,0xF0);

	/*Read the address*/
	data = jtag_arm_ReadByte(address);

	return data;
}

/*
 * Support Function to reset Flash back into Read Mode
 */
void jt_amdflashResetByte(uint32_t base_address)
{
	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteByte(base_address + 0x555L,0xF0);

	return;
}


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
int jt_amdflashProgByte(uint32_t base_address, uint32_t address, uint8_t data, unsigned wait)
{
	int error = 0;
	uint8_t rd_data;
	int timeout;
	struct timeval startTime, stopTime;

	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteByte(base_address + 0x555L,0xAA);
	jtag_arm_WriteByte(base_address + 0x2AAL,0x55);
	jtag_arm_WriteByte(base_address + 0x555L,0xA0);

	/*Write data*/
	jtag_arm_WriteByte(address, data);

	gettimeofday((&startTime), NULL);
	/*Wait for completion*/
	while(wait)
	{
		rd_data = jtag_arm_ReadByte(address);

		/*Check if the bit 7 of rd_byte and byte are equal*/
		/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
		if( (data & (1<<7)) == (rd_data & (1<<7)) )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( rd_data & (1<<5) )
		{
			rd_data = jtag_arm_ReadByte(address);
			/*Check if the bit 7 of rd_byte and byte are equal*/
			/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
			if( (data & (1<<7)) == (rd_data & (1<<7)) )
				break; /*pass*/
			else
			{
				/*fail*/
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteByte(base_address + 0x555L,0xF0);
				break;
			}
		}
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1500) // 1.5 sec max
		{
			/*fail*/
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteByte(base_address + 0x555L,0xF0);
			break;
		}

	}
	return error;
}

int jt_amdflashProgByte_faster(uint32_t base_address, uint32_t address, uint8_t *data, int numberOfData,unsigned verify)
{
	struct timeval startTime, stopTime;

	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 1;
	}
	if(data == NULL || numberOfData <= 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no data given\n");
		return 1;
	}
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr. */
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xff) , WRITE_ONLY); /* load R0 to R7 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<1), WRITE_ONLY);	// address -> R0 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xAA, WRITE_ONLY);				// value   -> R1
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x2AAL<<1), WRITE_ONLY);	// address -> R2 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0x55, WRITE_ONLY);				// value   -> R3
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<1), WRITE_ONLY);	// address -> R4 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xA0, WRITE_ONLY);				// value   -> R5


	do {
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, address, WRITE_ONLY);	// address -> R6 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, *data, WRITE_ONLY);		// value   -> R7
		
		// write 0xAA -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRB(0,1) , WRITE_ONLY);// R1 -> [R0] --> for Byte
		jtag_arm_chain1_sysspeed_restart();

		// write 0x55 -> [base_address + 0x2AA]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRB(2,3) , WRITE_ONLY);// R3 -> [R2] --> for Byte
		jtag_arm_chain1_sysspeed_restart();
		
		// write 0xA0 -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRB(4,5) , WRITE_ONLY);// R5 -> [R4] --> for Byte
		jtag_arm_chain1_sysspeed_restart();
		
		// write data -> [address]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRB(6,7) , WRITE_ONLY);	// R7 -> [R6] --> for Byte
		jtag_arm_chain1_sysspeed_restart();
		
		gettimeofday((&startTime), NULL);
		while(verify) // wait for compleation
		{
			int timeout;
			uint32_t rd_val;
			uint8_t rd_data;

			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
			jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next LD command at system speed
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_LDRB(6,8) , WRITE_ONLY); // [R6] -> R8
			jtag_arm_chain1_sysspeed_restart();
	
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_STMIA(0,1<<8) , WRITE_ONLY);	/* STM R0,{R8} */
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
			rd_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R8}: */
			rd_data = rd_val & 0xFF;
			/*Check if the bit 7 of rd_data and and bit 7 of data are equal*/
			/*if( ((!(byte ^ rd_data)) & (1<<7)) )*/
			if( (*data & (1<<7)) == (rd_data & (1<<7)) )
				break; /*pass*/
			/*Check if bit 5 (timeout) is set*/
			gettimeofday((&stopTime), NULL);
			timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
				+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
			if( timeout > 1500 || ( (rd_data & (1<<5)) && (rd_data & (1<<7)) != (*data & (1<<7)) ))
			{
				/*fail -- Reset the device*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout remaining %d\n",numberOfData);
				jtag_arm_WriteByte(base_address + (0x555L<<1),0xF0);
				return 1;
			}		
	
		}
		/*next data*/
		if(--numberOfData > 0)
		{
			address ++;
			data++;
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xC0) , WRITE_ONLY); /* load R6 to R7 */ 
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
		}
	} while(numberOfData > 0);
	jtag_arm_WriteByte(base_address + (0x555L<<1),0xF0);
	return 0;
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
int jt_amdflashEraseSectorByte(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint8_t data;
	int timeout;
	struct timeval startTime, stopTime;

	/*Write Erase Command to FlashROM*/
	jtag_arm_WriteByte(base_address + 0x555L,0xAA);
	jtag_arm_WriteByte(base_address + 0x2AAL,0x55);
	jtag_arm_WriteByte(base_address + 0x555L,0x80);
	jtag_arm_WriteByte(base_address + 0x555L,0xAA);
	jtag_arm_WriteByte(base_address + 0x2AAL,0x55);

	/*Tell Flash the sector address*/
	jtag_arm_WriteByte(address,0x30);

	/*Wait for starting the erase algorithem*/
	gettimeofday((&startTime), NULL);
	do
	{
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1000) // 1 sec max
		{
			/*fail - Reset the device*/
			jtag_arm_WriteByte(base_address + 0x555L,0xF0);
			return 1;
		}	
		data = jtag_arm_ReadByte(address); 
	} while(!(data & (1<<3)));

	/*Wait for completion*/
	gettimeofday((&startTime), NULL);
	do
	{
		/*byte already collected*/
		/*Check if bit 7 (data) is set*/
		if( data & (1<<7)  )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( data & (1<<5) )
		{
			data = jtag_arm_ReadByte(address);
			/*Check again if bit 7 (data) is set now*/
			if( data & (1<<7) )
				break; /*pass*/
			else
			{
				/*fail*/
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteByte(base_address + 0x555L,0xF0);
				break;
			}
		}
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 3000) // 3 sec max
		{
			/*fail*/
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteByte(base_address + 0x555L,0xF0);
			break;
		}
		data = jtag_arm_ReadByte(address);
	} while(1);

	return error;
}


/*
 * Support Function to reset Flash back into Read Mode
 * Note: Address Bit A0 is always ignored due to Halfword access
 * At a 16 Bit Flash Data Bits D15-D8 are ignored
 * At two 8 Bit Flashes the comand is always xmit'ed to both parts
 */
uint16_t jt_amdflashReadResetHalfword(uint32_t base_address, uint32_t address)
{
	uint16_t data;

	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xAAAA);
	jtag_arm_WriteHalfword(base_address + (0x2AAL<<1),0x5555);
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);

	/*Read the address*/
	data = jtag_arm_ReadHalfword(address);

	return data;
}

/*
 * Support Function to reset Flash back into Read Mode
 * Note: Address Bit A0 is always ignored due to Halfword access
 * At a 16 Bit Flash Data Bits D15-D8 are ignored
 * At two 8 Bit Flashes the comand is always xmit'ed to both parts
 */
void jt_amdflashResetHalfword(uint32_t base_address)
{
	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);

	return;
}


/*
 * Support Function to write one Halfword to the Flash.
 * This is OK after an erase of a Sector.
 * Note: Address Bit A0 must be 0 always ,due to Halfword access
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
int jt_amdflashProgHalfword(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait)
{
	int error = 0;
	uint16_t rd_data;
	int timeout;
	struct timeval startTime, stopTime;

	/*check alignment*/
	if(address & 1)
		return 1;

	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xAAAA);
	jtag_arm_WriteHalfword(base_address + (0x2AAL<<1),0x5555);
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xA0A0);

	/*Write data*/
	jtag_arm_WriteHalfword(address, data);

	gettimeofday((&startTime), NULL);
	/*Wait for completion*/
	while(wait)
	{
		rd_data = jtag_arm_ReadHalfword(address & 0xffffFFFe);

		/*Check if the bit 7 of rd_byte and and bit 7 of data are equal*/
		/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
		if( (data & (1<<7)) == (rd_data & (1<<7)) && (data & (1<<15)) == (rd_data & (1<<15)) )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( (rd_data & (1<<5)) && (rd_data & (1<<7)) != (data & (1<<7)))
		{
			rd_data = jtag_arm_ReadHalfword(address & 0xffffFFFe);
			/*Check if the bit 7 of rd_byte and byte are equal*/
			/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
			if( (data & (1<<7)) == (rd_data & (1<<7)) && (data & (1<<15)) == (rd_data & (1<<15)) )
				break; /*pass*/
			else
			{
				/*fail*/
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
				break;
			}
		}		
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1500) // 1.5 sec max
		{
			/*fail*/
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
			break;
		}

	}
	return error;
}

int jt_amdflashProgHalfword_faster(uint32_t base_address, uint32_t address, uint16_t *data, int numberOfData,unsigned verify)
{
	struct timeval startTime, stopTime;

	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 1;
	}
	if(data == NULL || numberOfData <= 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no data given\n");
		return 1;
	}
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr. */
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xff) , WRITE_ONLY); /* load R0 to R7 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<1), WRITE_ONLY);	// address -> R0 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xAAAA, WRITE_ONLY);			// value   -> R1
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x2AAL<<1), WRITE_ONLY);	// address -> R2 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0x5555, WRITE_ONLY);			// value   -> R3
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<1), WRITE_ONLY);	// address -> R4 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xA0A0, WRITE_ONLY);			// value   -> R5


	do {
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, address, WRITE_ONLY);		// address -> R6 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, *data, WRITE_ONLY);		// value   -> R7
		
		// write 0xAAAA -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRH(0,1) , WRITE_ONLY);// R1 -> [R0] --> for halfwords
		jtag_arm_chain1_sysspeed_restart();

		// write 0x5555 -> [base_address + 0x2AA]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRH(2,3) , WRITE_ONLY);// R3 -> [R2] --> for halfwords
		jtag_arm_chain1_sysspeed_restart();
		
		// write 0xA0A0 -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRH(4,5) , WRITE_ONLY);// R5 -> [R4] --> for halfwords
		jtag_arm_chain1_sysspeed_restart();
		
		// write data -> [address]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STRH(6,7) , WRITE_ONLY);// R7 -> [R6] --> for halfwords
		jtag_arm_chain1_sysspeed_restart();
		
		gettimeofday((&startTime), NULL);
		while(verify) // wait for compleation
		{
			int timeout;
			uint32_t rd_val;
			uint16_t rd_data;

			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
			jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next LD command at system speed
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_LDRH(6,8) , WRITE_ONLY); // [R6] -> R8
			jtag_arm_chain1_sysspeed_restart();
	
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STMIA(0,1<<8) , WRITE_ONLY);	/* STM R0,{R8} */
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
			rd_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R8}: */
			rd_data = rd_val & 0xFFFF;
			/*Check if the bit 7 of rd_data and and bit 7 of data are equal*/
			/*if( ((!(byte ^ rd_data)) & (1<<7)) )*/
			if( (*data & (1<<7)) == (rd_data & (1<<7)) && (*data & (1<<15)) == (rd_data & (1<<15)) )
				break; /*pass*/
			/*Check if bit 5 (timeout) is set*/
			gettimeofday((&stopTime), NULL);
			timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
				+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
			if( timeout > 1500 || ( (rd_data & (1<<5)) /*&& (rd_data & (1<<7)) != (*data & (1<<7)) */))
			{
				/*fail -- Reset the device*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout remaining %d\n",numberOfData);
				jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
				return 1;
			}		
	
		}
		/*next data*/
		if(--numberOfData > 0)
		{
			address += 2;
			data++;
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xC0) , WRITE_ONLY); /* load R6 to R7 */ 
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
		}
	} while(numberOfData > 0);
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
	return 0;
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
int jt_amdflashEraseSectorHalfword(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint16_t data;
	int timeout;
	struct timeval startTime, stopTime;

	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW," base %8.8X addr %8.8X ",base_address,address);
	/*Write Erase Command to FlashROM*/
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xAAAA);
	jtag_arm_WriteHalfword(base_address + (0x2AAL<<1),0x5555);
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0x8080);
	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xAAAA);
	jtag_arm_WriteHalfword(base_address + (0x2AAL<<1),0x5555);

	/*Tell Flash the sector address*/
	jtag_arm_WriteHalfword(address,0x3030);

	/*Wait for starting the erase algorithem*/
	gettimeofday((&startTime), NULL);
	do
	{
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1000) // 1 sec max
		{
			/*fail - Reset the device*/
			dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"(start timeout)");
			jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
			return 1;
		}	
		data = jtag_arm_ReadHalfword(address & 0xffffFFFe); 
	} while((data & (1<<3))==0);
	data = jtag_arm_ReadHalfword(address & 0xffffFFFe); 

	/*Wait for completion*/
	gettimeofday((&startTime), NULL);
	do
	{
		/*byte already collected*/
		/*Check if bit 7 (data) is set*/
		if( (data & (1<<7)) && (data & (1<<15)) )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( (data & (1<<5)) && !(data & (1<<7)) )
		{
			data = jtag_arm_ReadHalfword(address & 0xffffFFFe);
			/*Check again if bit 7 (data) is set now*/
			if( (data & (1<<7)) &&  (data & (1<<15)))
				break; /*pass*/
			else
			{
				/*fail*/
				dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"(end error)");
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
				break;
			}
		}
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 3000) // 3 sec max
		{
			/*fail*/
			dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"(end timeout)");
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
			break;
		}
		data = jtag_arm_ReadHalfword(address & 0xffffFFFe);
	} while(1);

	jtag_arm_WriteHalfword(base_address + (0x555L<<1),0xF0F0);
	return error;
}


/*
 * Support Function to reset Flash back into Read Mode
 * Note: Address Bit A0 and A1 is always ignored due to Word access
 * At a 16 Bit Flash Data Bits D15-D8 are ignored (we migth have two of them)
 * At four 8 Bit Flashes the comand is always xmit'ed to all parts
 */
uint32_t jt_amdflashReadResetWord(uint32_t base_address, uint32_t address)
{
	uint32_t data;

	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xAAAAAAAA);
	jtag_arm_WriteWord(base_address + (0x2AAL<<2),0x55555555);
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);

	/*Read the address*/
	data = jtag_arm_ReadWord(address);

	return data;
}

/*
 * Support Function to reset Flash back into Read Mode
 * Note: Address Bit A0 and A1 is always ignored due to Word access
 * At a 16 Bit Flash Data Bits D15-D8 are ignored (we migth have two of them)
 * At four 8 Bit Flashes the comand is always xmit'ed to all parts
 */
void jt_amdflashResetWord(uint32_t base_address)
{
	/*Write ReadReset Command to FlashROM*/
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);

	return;
}

/*
 * Support Function to write one Word to the Flash.
 * This is OK after an erase of a Sector.
 * Note: Address Bit A0 and A1 must always be 0 ,due to Word access
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
int jt_amdflashProgWord(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait)
{
	int error = 0;
	uint32_t rd_data;
	int timeout;
	struct timeval startTime, stopTime;

	/*check alignment*/
	if(address & 0x3)
		return 1;

	/*Write Programm Command to FlashROM*/
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xAAAAAAAA);
	jtag_arm_WriteWord(base_address + (0x2AAL<<2),0x55555555);
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xA0A0A0A0);

	/*Write data*/
	jtag_arm_WriteWord(address, data);

	gettimeofday((&startTime), NULL);
	/*Wait for completion*/
	while(wait)
	{
		rd_data = jtag_arm_ReadWord(address & 0xffffFFFc);

		/*Check if the bit 7 of rd_byte and byte are equal*/
		/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
		if( (data & (1<<7)) == (rd_data & (1<<7)) 
		 && (data & (1<<15)) == (rd_data & (1<<15)) 
		 && (data & (1<<23)) == (rd_data & (1<<23)) 
		 && (data & (1<<31)) == (rd_data & (1<<31)) 
		 )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( (rd_data & (1<<5)) /*&& (rd_data & (1<<7)) != (data & (1<<7))*/)
		{
			rd_data = jtag_arm_ReadWord(address & 0xffffFFFc);
			/*Check if the bit 7 of rd_byte and byte are equal*/
			/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
			if( (data & (1<<7)) == (rd_data & (1<<7))
			  && (data & (1<<15)) == (rd_data & (1<<15)) 
			  && (data & (1<<23)) == (rd_data & (1<<23)) 
			  && (data & (1<<31)) == (rd_data & (1<<31))
			  )
				break; /*pass*/
			else
			{
				/*fail*/
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
				break;
			}
		}		
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1500) // 1.5 sec max
		{
			/*fail*/
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
			break;
		}

	}
	return error;
}

int jt_amdflashProgWord_faster(uint32_t base_address, uint32_t address, uint32_t *data, int numberOfData,unsigned verify)
{
	struct timeval startTime, stopTime;

	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 1;
	}
	if(data == NULL || numberOfData <= 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"no data given\n");
		return 1;
	}
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr. */
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xff) , WRITE_ONLY); /* load R0 to R7 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<2), WRITE_ONLY);	// address -> R0 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xAAAAaaaa, WRITE_ONLY);			// value   -> R1
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x2AAL<<2), WRITE_ONLY);	// address -> R2 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0x55555555, WRITE_ONLY);			// value   -> R3
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, base_address + (0x555L<<2), WRITE_ONLY);	// address -> R4 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, 0xA0A0A0A0, WRITE_ONLY);			// value   -> R5


	do {
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, address, WRITE_ONLY);	// address -> R6 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, *data, WRITE_ONLY);		// value   -> R7
		
		// write 0xAAAAaaaa -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STR(0,1) , WRITE_ONLY);	// R1 -> [R0] 
		jtag_arm_chain1_sysspeed_restart();

		// write 0x55555555 -> [base_address + 0x2AA]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STR(2,3) , WRITE_ONLY);	// R3 -> [R2]
		jtag_arm_chain1_sysspeed_restart();
		
		// write 0xA0A0A0A0 -> [base_address + 0x555]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STR(4,5) , WRITE_ONLY);	// R5 -> [R4]
		jtag_arm_chain1_sysspeed_restart();
		
		// write data -> [address]
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED      , NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next STR command at system speed
		jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STR(6,7) , WRITE_ONLY);	// R7 -> [R6]
		jtag_arm_chain1_sysspeed_restart();
		
		gettimeofday((&startTime), NULL);
		while(verify) // wait for compleation
		{
			int timeout;
			uint32_t rd_val;
			uint32_t rd_data;

			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// prefech
			jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP, WRITE_ONLY);	// Perpare to execute next LD command at system speed
			/*XXX we should use the all instead off the following halfword*/
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDRH(6,8) , WRITE_ONLY); // [R6] -> R8
			jtag_arm_chain1_sysspeed_restart();
	
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_STMIA(0,1<<8) , WRITE_ONLY);	/* STM R0,{R8} */
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
			rd_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R8}: */
			rd_data = rd_val & 0xFFFF;
			/*Check if the bit 7 of rd_data and and bit 7 of data are equal*/
			/*if( ((!(byte ^ rd_data)) & (1<<7)) )*/
			if( (*data & (1<<7)) == (rd_data & (1<<7)) && (*data & (1<<15)) == (rd_data & (1<<15)) )
				break; /*pass*/
			/*Check if bit 5 (timeout) is set*/
			gettimeofday((&stopTime), NULL);
			timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
				+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
			if( timeout > 1500 || ( (rd_data & (1<<5)) /*&& (rd_data & (1<<7)) != (*data & (1<<7))*/ ))
			{
				/*fail -- Reset the device*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"timeout remaining %d\n",numberOfData);
				jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
				return 1;
			}		
	
		}
		/*next data*/
		if(--numberOfData > 0)
		{
			address += 4;
			data++;
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0,0xC0) , WRITE_ONLY); /* load R6 to R7 */ 
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
		}
	} while(numberOfData > 0);
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
	return 0;
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
int jt_amdflashEraseSectorWord(uint32_t base_address, uint32_t address)
{
	int error = 0;
	uint32_t data;
	int timeout;
	struct timeval startTime, stopTime;

	/*Write Erase Command to FlashROM*/
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xAAAAAAAA);
	jtag_arm_WriteWord(base_address + (0x2AAL<<2),0x55555555);
	jtag_arm_WriteWord(base_address + (0x555L<<2),0x80808080);
	jtag_arm_WriteWord(base_address + (0x555L<<2),0xAAAAAAAA);
	jtag_arm_WriteWord(base_address + (0x2AAL<<2),0x55555555);

	/*Tell Flash the sector address*/
	jtag_arm_WriteWord(address,0x30303030);

	/*Wait for starting the erase algorithem*/
	gettimeofday((&startTime), NULL);
	do
	{
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 1000) // 1 sec max
		{
			/*fail - Reset the device*/
			jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
			return 1;
		}	
		data = jtag_arm_ReadWord(address & 0xffffFFFc); 
	} while(!(data & (1<<3)));

	/*Wait for completion*/
	gettimeofday((&startTime), NULL);
	do
	{
		/*byte already collected*/
		/*Check if bit 7 (data) is set*/
		if( (data & (1<<7)) 
		  && (data & (1<<15)) 
		  && (data & (1<<23)) 
		  && (data & (1<<31)) 
		  )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( (data & (1<<5)) && !(data & (1<<7)) )
		{
			data = jtag_arm_ReadWord(address & 0xffffFFFc);
			/*Check again if bit 7 (data) is set now*/
			if( (data & (1<<7)) 
			  &&  (data & (1<<15))
			  && (data & (1<<23)) 
			  && (data & (1<<31)) 
			  )
				break; /*pass*/
			else
			{
				/*fail*/
				error = 1;
				/*Reset the device*/
				jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
				break;
			}
		}
		gettimeofday((&stopTime), NULL);
		timeout = (stopTime.tv_sec - startTime.tv_sec) * 1000
			+ (stopTime.tv_usec - startTime.tv_usec) / 1000;
		if( timeout > 3000) // 3 sec max
		{
			/*fail*/
			error = 1;
			/*Reset the device*/
			jtag_arm_WriteWord(base_address + (0x555L<<2),0xF0F0F0F0);
			break;
		}
		data = jtag_arm_ReadWord(address & 0xffffFFFc);
	} while(1);

	return error;
}



