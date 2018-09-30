/*
 * t_arm_philipsflash.c
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

	volatile unsigned clk_ticks_a;		//((unsigned*)0x3fff8000)[10] = frequence / 200 			// erase and  copy ram to flash
	volatile unsigned clk_ticks_b;		//((unsigned*)0x3fff8000)[11] = ((frequence * 25)/ 128) + 1; 	// erase
						//			      = (frequence / 2048) + 1;		// copy ram to flash
} ;

extern struct LpcFlashCtl * const lpcFlashCtl;


/*
 * tell the compiler that the function will never return 
 * and that we do manualy the stack related stuff
 */
void dcc_start(void)	__attribute__ ((noreturn,naked)); 

extern uint32_t read_dcc_data(void);
extern void write_dcc_data(uint32_t val);


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

#endif //GEN_ARM_PART_2

#ifdef GEN_ARM_PART_1
struct LpcFlashCtl * const lpcFlashCtl = (struct LpcFlashCtl *)0x3fff8000;

/*
 * The Start function must be the first "code" line.
 */
void dcc_start(void)
{
	//volatile 
	uint32_t val;
	uint32_t freq_a, freq_b;
	uint32_t delay;
	uint32_t dst;
	uint32_t offset, x;
	int i;

	/*in best case we did not use the stack, but if sp should be with the work space*/
	asm("add sp,pc,#512");	// sp = 0x200 + currAddr + 8
	asm("sub sp,sp,#12");	// sp -= 0xc

	/*wait for Start Token*/
	do {
		val = read_dcc_data();
	} while(val != 0x4D524140); // "@ARM" = 40 41 52 4D

	/*get frequence and delay*/
	dst    = read_dcc_data();
	freq_a = read_dcc_data();
	freq_b = read_dcc_data();
	delay  = read_dcc_data();
	
	/*ready to run ?*/
	if( (lpcFlashCtl->cmd & LPC_FLASH_STATUS_MASK) != 0) 
	{
		lpcFlashCtl->cmd = LPC_FLASH_CMD_STOP;
		//write_dcc_data(0x6b63614e); // "Nack" = 4e 61 63 6b
		write_dcc_data(0x0); // "Nack" = 4e 61 63 6b
		while(1)
			;
			//read_dcc_data(); //block
	}	
	/*ACK*/	
	write_dcc_data(0x216b6361); // "ack!" = 61 63 6b 21
	
	/*what have we to do? erase,unlock,lock or program?*/
	if( dst == 1) // it is not on 128 bit boundary
	{
		/*setup erase*/
		lpcFlashCtl->clk_ticks_a = freq_a; 	// both:     frequence / 200;
		lpcFlashCtl->clk_ticks_b = freq_b;	// program:  (frequence / 2048) + 1;
						// erase:    ((frequence * 25)/ 128) + 1;
		/*erase*/
		lpcFlashCtl->cmd = LPC_FLASH_CMD_ERASE;
	}
	else if ( dst == 2) 
	{
		/*unlock*/
		lpcFlashCtl->mask_active_sectors = freq_a; // set mask
		lpcFlashCtl->mask_locked_sectors = lpcFlashCtl->mask_locked_sectors &  ~freq_a;
		lpcFlashCtl->cmd = LPC_FLASH_CMD_UNLOCK_SECTORS;
	}
	else if ( dst == 3) 
	{
		/*lock*/
		lpcFlashCtl->mask_active_sectors = freq_a; // set mask
		lpcFlashCtl->mask_locked_sectors = lpcFlashCtl->mask_locked_sectors | freq_a;
		lpcFlashCtl->cmd = LPC_FLASH_CMD_LOCK_SECTORS;
	}
	else if ( dst == 4) 
	{
		/*unlock boot sector*/
		lpcFlashCtl->mask_active_sectors = freq_a; // set mask
		lpcFlashCtl->cmd = LPC_FLASH_CMD_UNLOCK_SECTORS;
	}
	else
	{
		/*setup prorgam*/
		lpcFlashCtl->clk_ticks_a = freq_a; 	// both:     frequence / 200;
		lpcFlashCtl->clk_ticks_b = freq_b;	// program:  (frequence / 2048) + 1;
						// erase:    ((frequence * 25)/ 128) + 1;
		/*program*/
		lpcFlashCtl->dst = (unsigned*)dst;
		for ( offset = 0; offset < 512; offset += 16)
		{
			for (i=0; i<4; i++)
				lpcFlashCtl->data[i] = read_dcc_data();
			lpcFlashCtl->dst_offset = offset;
			lpcFlashCtl->cmd = LPC_FLASH_CMD_PROG_128_BITS;
			x = 50;
			while ( x-- != 0)	// = 50			->   0,84 us
				;
			lpcFlashCtl->cmd = LPC_FLASH_CMD_STOP;
		}
	
		x = delay / 2;
		while ( x-- != 0)		// frequence * 0.11	-> 110 us
			;
		lpcFlashCtl->cmd = LPC_FLASH_CMD_PROG_512BYTES_END;
	}
	
	while ( delay-- != 0)			// prorgam: frequence * 0.22	-> 220 us
		;				// erase:   frequence * 80.02	->  80 msec
						// unlock/lock:		= 50	->   0,84 us
	lpcFlashCtl->cmd = LPC_FLASH_CMD_STOP;	
	
	/*FIN*/
	write_dcc_data(0x216e6966); // "fin!" = 66 69 6e 21

	/*never return*/
	while(1)
		;
	/*not reached*/
}
#endif //GEN_ARM_PART_1







