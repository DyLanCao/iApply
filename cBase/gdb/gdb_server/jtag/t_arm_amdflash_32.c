/*
 * t_arm_flash_32.c
 * 
 * DCC - Flash support functions
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

typedef	char	int8_t;
typedef	short	int16_t;
typedef	int	int32_t;

typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned long	uint32_t;

#ifdef __GNUC__
typedef	int __attribute__((__mode__(__DI__)))		 int64_t;
typedef	unsigned int __attribute__((__mode__(__DI__)))	uint64_t;
#else
typedef	long long int64_t;
typedef	unsigned long long uint64_t;
#endif

extern int progWord(uint32_t base_address,uint32_t address,uint32_t data);
extern uint32_t read_dcc_data(void);
extern void write_dcc_data(uint32_t val);

#ifdef GEN_THUMB_PART
/*
 * Support Function to write one Halfword to the Flash.
 * This is OK after an erase of a Sector.
 * Note: Address Bits A1 and A0 must be 0 always ,due to Word access
 * Input: 
 * 		var base_address
 *		var address
 *		var data
 * Output:
 *		none
 * Return:
 *		0 on success
 *		else error
 */
int progWord(uint32_t base_address,uint32_t address,uint32_t data)
{
	volatile uint32_t *dataPtr;
	uint32_t rd_data;

	/*Write Programm Command to FlashROM*/
	*((volatile uint32_t *)(base_address + (0x555uL<<2))) = 0xAAAAaaaa;
	*((volatile uint32_t *)(base_address + (0x2AAuL<<2))) = 0x55555555;
	*((volatile uint32_t *)(base_address + (0x555uL<<2))) = 0xA0A0A0A0;
	/*Write data*/
	dataPtr = (uint32_t *)address;
	*dataPtr = data;
	

	/*Wait for completion*/
	while(1)
	{
		rd_data = *dataPtr;

		/*Check if the bit 7 of rd_byte and and bit 7 of data are equal*/
		/*if( ((!(byte ^ rd_byte)) & (1<<7)) )*/
		if( (data & (1<<7)) == (rd_data & (1<<7))
		 && (data & (1<<15)) == (rd_data & (1<<15)) 
		 && (data & (1<<23)) == (rd_data & (1<<23)) 
		 && (data & (1<<31)) == (rd_data & (1<<31)) 
		 )
			break; /*pass*/
		/*Check if bit 5 (timeout) is set*/
		if( (rd_data & (1<<5)) != 0 && (rd_data & (1<<7)) != (data & (1<<7)))
		{
#if 1
			rd_data = *dataPtr;
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

				/*fail Reset the device*/
				dataPtr = (volatile uint32_t *)(base_address + (0x555L<<2));
				*dataPtr = 0xF0F0F0F0;
				return 1;

			}
#else
			/*fail Reset the device*/
			dataPtr = (volatile uint32_t *)(base_address + (0x555L<<2));
			*dataPtr = 0xF0F0F0F0;
			return 1;
#endif
		}		
	}
	return 0;
}
#endif

#ifdef GEN_ARM_PART_2
/*
 * read word via DCC
 */
uint32_t read_dcc_data(void)
{
	register uint32_t val;
	
	/*wait until data received*/
	do {
		/*read control*/
		__asm__ __volatile__("MRC p14, 0, %0, c0, c0":"=r" (val):);
	} while((val & 1) == 0); 
	/*read data*/
	__asm__ __volatile__("MRC p14, 0, %0, c1, c0":"=r" (val):);
	return val;
}

/*
 * write DCC data
 */
void write_dcc_data(uint32_t val)
{
	register uint32_t not_ready;
	
	/*wait until buffer get free*/
	do {
		__asm__ __volatile__("MRC p14, 0, %0, c0, c0":"=r" (not_ready):);
	}while((not_ready & 2));
	/*write data*/
	__asm__ __volatile__("MCR p14, 0, %0, c1, c0"::"r" (val));
	return;
}
#endif

#ifdef GEN_ARM_PART_1
/*
 * tell the compiler that the function will never return 
 * and that we do manualy the stack related stuff
 */
void dcc_start(void)	__attribute__ ((noreturn,naked)); 

/*
 * The Start function must be the first "code" line.
 */
void dcc_start(void)
{
	register uint32_t val;
	register uint32_t address;
	register uint32_t base_address;
	register uint32_t len;

	/*in best case we did not use the stack, but if sp should be with the work space*/
	asm("add sp,pc,#512");	// sp = 0x200 + currAddr + 8
	asm("sub sp,sp,#12");	// sp -= 0xc

	/*wait for Start Token*/
	do {
		val = read_dcc_data();
	} while(val != 0x4D524140); // "@ARM" = 40 41 52 4D

	/*get address and len*/
	base_address = read_dcc_data();
	address = read_dcc_data();
	len = read_dcc_data();
	
	/*ACK*/	
	write_dcc_data(0x216b6361); // "ack!" = 61 63 6b 21
	
	/*receive data from debuger*/
	for(;0<len;len--)
	{
		val = read_dcc_data();
		if(progWord(base_address,address,val)) 
			goto error_block;
		address+=4;
	}
	/*FIN*/
	write_dcc_data(0x216e6966); // "fin!" = 66 69 6e 21

	/*never return*/
	while(1);
error_block:
	write_dcc_data(0x6b63614e); // "Nack" = 4e 61 63 6b
	while(1)
		;
		//read_dcc_data(); //block
	/*not reached*/
}
#endif





