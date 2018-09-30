/*
 * lpc_example.c
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

#include "lpc_env.h"
#include "lpc21xx.h"
#include "lpc_syscall.h"

int useless = 17;

int main(void);

/*
 *
 */
int main(void)
{
	/* Start timer  */ 
	T1_TCR=0x1;

	enableInterrupt();
	
	while(1)
	{
		useless++;
		if(useless == 0)
			syscall(1,2,3,4);
	}
}

