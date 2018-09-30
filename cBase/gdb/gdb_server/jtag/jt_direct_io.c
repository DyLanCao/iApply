/*
 * jt_direct_io.c
 *  
 * ioperm wrapper functions
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
 */
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)

#ifdef HAVE_I386_SET_IOPERM
#include <sys/types.h>
#include <machine/sysarch.h>
#include <err.h>

int ioperm( unsigned long from, unsigned long num, int permit );
//int iopl( int level );

int ioperm( unsigned long from, unsigned long num, int permit )
{
#ifdef __FreeBSD__
	return i386_set_ioperm(from, num, permit);
#else
	u_long ports[32];
	u_long i;

	if (i386_get_ioperm( ports ) == -1)
		return -1;

	for (i = from; i < (from + num); i++)
		if (permit)
			ports[i / 32] &= ~(1 << (i % 32));
		else
			ports[i / 32] |= (1 << (i % 32));

	if (i386_set_ioperm( ports ) == -1)
		return -1;
	return 0;
#endif
}

#if 0
int iopl( int level )
{
#ifdef __FreeBSD__
	return 0;
#else
	return i386_iopl( level );
#endif
}
#endif


#if 0
unsigned char inb( unsigned short int port )
{
	unsigned char _v;

	__asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
	return _v;
}

void outb( unsigned char value, unsigned short int port )
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}
#endif
#endif /* HAVE_I386_SET_IOPERM */

#else
int ioperm( unsigned long from, unsigned long num, int permit )
{
	return 0;
}
#endif /* defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM) */

//extern unsigned short port_base;
unsigned port_base = PORT_BASE;

