/*
 * s3c44b0x_isr.c
 * 
 * Copyright (C) 2005
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
 */
#include <inttypes.h>
#include "s3c44b0x.h"


void Timer5_ISR(void);

#define TICK_PER_MSEC 10 /*asume 10usec interval Periode with countervalue 10*/
#define TICK_PER_SEC (1000 * TICK_PER_MSEC)
#define TIMETICK_PER_SEC (128) /*Base of RTC - time ticks*/
#define TICK_PER_TIMETICK ( TICK_PER_SEC / TIMETICK_PER_SEC )

extern uint32_t ClockTick;
extern int32_t  TimeTick;


/*
 * Dummy ISR Handler
 */
void no_isr(void)
{
	/*block in endless loop*/
	while(1); 
	return;
}

/*
 * The Timer Interrupt Service Routine code will come here. 
 * Thread Clock
 */
void Timer5_ISR(void)
{
	static int timeTickBase = TICK_PER_TIMETICK;

	ClockTick++;
	if(timeTickBase <= 0)
	{
		TimeTick++;
		timeTickBase = TICK_PER_TIMETICK;
		// Make sure it won't become 0
		//- to prevent us to div by zero somewere else
		if( TimeTick == 0)
			TimeTick = 1; 
	}
	timeTickBase--;
	return;
}

