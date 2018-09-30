/*
 * t_arm_atmelflash.c
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

#define SAM7_FLASH_CMD_WRITE_PAGE		0x01
#define SAM7_FLASH_CMD_SET_LOCK_BIT		0x02
#define SAM7_FLASH_CMD_WRITE_PAGE_AND_LOCK	0x03
#define SAM7_FLASH_CMD_CLEAR_LOCK_BIT		0x04
#define SAM7_FLASH_CMD_ERASE_ALL		0x08
#define SAM7_FLASH_CMD_SET_GP_NVM_BIT		0x0B
#define SAM7_FLASH_CMD_CLEAR_GP_NVM_BIT		0x0D
#define SAM7_FLASH_CMD_SET_SECURTY_BIT		0x0F

struct Sam7FlashCtl {
	volatile uint32_t mode;			//(0xffffFF60)
	volatile uint32_t cmd;			//(0xffffFF64)
	volatile uint32_t status;		//(0xffffFF68)
} ;

extern struct Sam7FlashCtl * const sam7FlashCtl;


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
struct Sam7FlashCtl * const sam7FlashCtl = (struct Sam7FlashCtl *)0xffffFF60;

/*
 * The Start function must be the first "code" line.
 */
void dcc_start(void)
{
	//volatile 
	uint32_t val;	
	uint32_t *address;
	uint32_t len, i;
	uint32_t delay;
	uint32_t dst;
	uint32_t cmd, mode;

	/*in best case we did not use the stack, but if sp should be within the work space*/
	asm("add sp,pc,#512");	// sp = 0x200 + currAddr + 8
	asm("sub sp,sp,#12");	// sp -= 0xc

	/*wait for Start Token*/
	do {
		val = read_dcc_data();
	} while(val != 0x4D524140); // "@ARM" = 40 41 52 4D

	/*get mode and command*/
	mode = read_dcc_data();
	cmd  = read_dcc_data();
	
	/*get address and len*/
	address = (uint32_t *) read_dcc_data();
	len     = read_dcc_data();
	
	sam7FlashCtl->mode = mode;
	/*ready to run ?*/
	do {
		val = sam7FlashCtl->status;
	} while((val & 1) == 0);

	/*ACK*/	
	write_dcc_data(0x216b6361); // "ack!" = 61 63 6b 21
	
	/*colect the data from the debugger ..*/
	for(i=0;i<len;i++)
	{
		val = read_dcc_data();

		/*.. and write data to the write-only latch buffer*/
		*address = val;
		address++;
	}
	/*ready to run ?*/
	do {
		val = sam7FlashCtl->status;
	} while((val & 1) == 0);

	/*start the command*/
	sam7FlashCtl->cmd  = cmd;

	/*wait until the command has been finnished*/
	do {
		val = sam7FlashCtl->status;
	} while((val & 1) == 0);

#if 0
	/*insert additional delay*/
	for(i=0;i<len;i++)
		;
	/*wait until the command has realy finnished*/
	do {
		val = sam7FlashCtl->status;
	} while((val & 1) == 0);
	
	len = sam7FlashCtl->status;
#endif	
	/*is there an error ?*/
	if(val & 0x1C) // one of Bit's 11100 set ?
	{
		/*send error bits*/
		write_dcc_data(val & 0x1C);
	}	
	else
	{
		/*FIN*/
		write_dcc_data(0x216e6966); // "fin!" = 66 69 6e 21
	}

	/*never return*/
	while(1)
		;
	/*not reached*/
}
#endif //GEN_ARM_PART_1







