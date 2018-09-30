/*
 * str7_irq.c
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
#include "71x_conf.h"
#include "71x_lib.h"
#include "71x_it.h"

/*
 *
 */
void Default_IRQHandler(void)
{
	while(1)
		;
	return;
}

/*
 *
 */
void T0TIMI_IRQHandler(void)
{
	/*   
	 * The Timer Interrupt Service Routine code will come here. 
	 * The interrupt needs to be cleared in Timer0.
	 * Here the user could  blink a LED or toggle some 
	 * port pins as an indication of being in the ISR  
	 */
	TIM_FlagClear(TIM0,TIM_TOF | TIM_OCFB);
	/* invert the GPIO1 pin P1.8 (LED) output level.*/
	if(GPIO_BitRead(GPIO1,8) == 0)
		GPIO_BitWrite(GPIO1,8,1);
	else
		GPIO_BitWrite(GPIO1,8,0);
	return;
}



