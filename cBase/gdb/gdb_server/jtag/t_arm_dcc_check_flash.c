/*
 * t_arm_check.c
 * 
 * DCC - Flash check support functions
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

/*
 * tell the compiler that the function will never return 
 * and that we do manualy the stack related stuff
 */
void dcc_start(void)	__attribute__ ((noreturn,naked)); 
static inline uint32_t checksum_crc32(uint32_t *block, int bit_len);
static inline uint32_t checksum_crc16(uint16_t *block, int bit_len);
static inline uint32_t read_dcc_data(void) ;
static inline void write_dcc_data(uint32_t val);

/*
 * read word via DCC
 */
static inline uint32_t read_dcc_data(void)
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
static inline void write_dcc_data(uint32_t val)
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


/*
 * The Start function must be the first "code" line.
 */
void dcc_start(void)
{
	uint32_t val;
	uint32_t *address;
	uint32_t len, i;
	
	/*in best case we did not use the stack, but if sp should be with the work space*/
	asm("add sp,pc,#512");	// sp = 0x200 + currAddr + 8
	asm("sub sp,sp,#12");	// sp -= 0xC

	/*wait for Start Token*/
	do {
		val = read_dcc_data();
	} while(val != 0x4D524140); // "@ARM" = 40 41 52 4D

	/*get address and len*/
	address = (uint32_t *) read_dcc_data();
	len = read_dcc_data();
	
	/*ACK*/	
	write_dcc_data(0x216b6361); // "ack!" = 61 63 6b 21
	
	/*transmit the result to the debuger*/
	val = checksum_crc32(address,len);
	write_dcc_data(val);
	
	/*transmit the result to the debuger*/
	val = checksum_crc16((uint16_t *)address,len);
	write_dcc_data(val);

	/*FIN*/
	write_dcc_data(0x216e6966); // "fin!" = 66 69 6e 21

	/*never return*/
	while(1);
}


/*
 *
 * The CRC 32 polynominal is
 *   32   26   23   22  16   12   11   10   8   7   5   4   2   1   0
 *  X  + X  + X  + X  +X  + X  + X  + X  + X + X + X + X + X + X + X 
 *
 *  3  3322 2222  2222 1111  1111 11
 *  2  1098 7654  3210 9876  5432 1098  7654 3210
 *
 *  1  0000 0100  1100 0001  0000 1101  1011 0111
 * 
 *  0x104C10DB7
 */

#define CRC32_POLY 0x04C10DB7

static inline uint32_t checksum_crc32(uint32_t *block, int bit_len)
{
	uint32_t val,next;
	int bit_32,i;
	
	/*initial val is 0*/
	val = 0;
	
	for(i=0;i<bit_len;i++)
	{
		/*remember bit[32] for later use*/
		if(val & 0x80000000uL)
			bit_32 = 1;
		else
			bit_32 = 0;
		/*do left shift*/
		val <<= 1;
		/*do we have the next bit*/
		if((i & 0x1F) == 0) // (i % 32) == 0
		{
			/*next bit is highes from block*/
			next = *block++;
		}
		/*insert next bit*/
		if(next & 0x80000000uL)
			val |= 1;
		next <<= 1;

		/*now the polynominal division*/
		if(bit_32)
			val ^= CRC32_POLY;
	}
	return val;
}

/* The CRC 16 polynominal is
 *   16   12   5   0
 *  x  + x  + x + x
 *
 *  1 1111 11
 *  6 5432 1098 7654 3210
 *
 *  1 0001 0000 0010 0001
 *
 *  0x11021
 */

#define CRC16_POLY 0x1021

static inline uint32_t checksum_crc16(uint16_t *block, int bit_len)
{
	uint32_t val,cnt;
	uint32_t ret_val,next;
	int i;

	/*initial val is 0*/
	val = 0;
	cnt = 0;
	
	for(i=0;i<bit_len;i++)
	{
		/*do left shift*/
		val <<= 1;
		/*do we have the next bit*/
		if((i & 0xF) == 0) // (i % 16) == 0
		{
			/*next bit is highes from block*/
			next = *block++;
		}
		/*insert next bit*/
		if(next & 0x8000)
			val |= 1;
		next <<= 1;

		/*now the polynominal division*/
		if(val & 0x10000)
			val ^= CRC32_POLY;
		else
			cnt ++;
	}
	cnt <<= 16;
	ret_val = cnt | val;
	return ret_val;
}

