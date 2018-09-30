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

#define SET_WPG		0x20000000
#define SET_DWPG	0x10000000
#define SET_SER		0x08000000
#define START		0x80000000
#define MSK_LOCK_BSY	0x16

struct StFlashCtl {
	volatile uint32_t Cr0;			//((unsigned*)0x40100000)[0] -- 
	volatile uint32_t Cr1;
	volatile uint32_t Dr0;
	volatile uint32_t Dr1;
	volatile uint32_t Ar;
	volatile uint32_t Er;
};

extern struct StFlashCtl * const stFlashCtl;


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
struct StFlashCtl * const stFlashCtl = (struct StFlashCtl *)0x40100000;

/*
 * The Start function must be the first "code" line.
 */
void dcc_start(void)
{
	//volatile 
	uint32_t val;
	uint32_t addr;
	uint32_t dst;
	uint32_t len;

	/*in best case we did not use the stack, but if sp should be with the work space*/
	asm("add sp,pc,#512");	// sp = 0x200 + currAddr + 8
	asm("sub sp,sp,#12");	// sp -= 0xc

	/*wait for Start Token*/
	do {
		val = read_dcc_data();
	} while(val != 0x4D524140); // "@ARM" = 40 41 52 4D

	/*get addr and dst*/
	addr = read_dcc_data();
	val  = read_dcc_data();
	dst  = addr & 0x3;

	/*make sure previous operation has been finished*/
	while(stFlashCtl->Cr0 & MSK_LOCK_BSY)
			;
	
	/*ACK*/	
	write_dcc_data(0x216b6361); // "ack!" = 61 63 6b 21

	/*clear previous error; if any*/
	stFlashCtl->Er = 0;
	
	/*what have we to do? erase or program?*/
	if( dst == 1)
	{
		/*setup erase*/
		stFlashCtl->Cr0 |= SET_SER;
		stFlashCtl->Cr1 |= val;
		/*erase*/
		stFlashCtl->Cr0 |= START;
		while(stFlashCtl->Cr0 & MSK_LOCK_BSY)
			;
		if(stFlashCtl->Cr1)
		{
			val = stFlashCtl->Er;
			write_dcc_data(val); // send erase Error
			while(1)
				;
		}
	}
	else
	{
		len = val;
		for ( ; len; len--)
		{
			/*setup prorgam*/
			stFlashCtl->Cr0 |= SET_DWPG;
			stFlashCtl->Ar  = addr;
			val  = read_dcc_data();
			stFlashCtl->Dr0 = val;
			val  = read_dcc_data();
			stFlashCtl->Dr1 = val;
			/*program*/
			stFlashCtl->Cr0 |= START;
			while(stFlashCtl->Cr0 & MSK_LOCK_BSY)
				;
			if(stFlashCtl->Er)
			{
				val = stFlashCtl->Er;
				write_dcc_data(val); // send prorgam Error
				while(1)
					;
			}
			addr += 8;
		}
	}
	
	/*FIN*/
	write_dcc_data(0x216e6966); // "fin!" = 66 69 6e 21

	/*never return*/
	while(1)
		;
	/*not reached*/
}
#endif //GEN_ARM_PART_1







