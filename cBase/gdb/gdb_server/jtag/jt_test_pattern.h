// Copyright (C) 2005
//  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.

// file format littlearm

unsigned char text_buf[] = { 
// Contents of section .text:
// c000000
			0x13, 0x00, 0x00, 0xea,		// b	start_block
			0xfe, 0xff, 0xff, 0xea, 	// b	.
			0x00, 0x00, 0xa0, 0xe1, 	// nop
			0x00, 0x00, 0xa0, 0xe1,
// c000010
			0x00, 0x00, 0xa0, 0xe1,
			0x00, 0x00, 0xa0, 0xe1,	
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000020
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,		
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000030
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,		
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000040
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,		
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000050
			0x00, 0x00, 0xa0, 0xe1, 	
/*start_block:*/	0xfe, 0xff, 0xff, 0xea,		// b	.
			0x00, 0x00, 0xa0, 0xe1, 	// nop
			0x00, 0x00, 0xa0, 0xe1,
// c000060
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,	
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000070
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,		
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000080 
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,		
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c000090 
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,	
			0x00, 0x00, 0xa0, 0xe1, 	
			0x00, 0x00, 0xa0, 0xe1,
// c0000a0
			0x00, 0x00, 0xa0, 0xe1, 	
			0xfe, 0xff, 0xff, 0xea,		// b	.
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c0000b0
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c0000c0
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c0000d0
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c0000e0
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c0000f0
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c000100
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c000110
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c000120
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c000130
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,	
			0xfe, 0xff, 0xff, 0xea,
// c000140
			0x08, 0x00, 0x9f, 0xe5,		// ldr	r0, =thumb_start
			0x01, 0x00, 0x80, 0xe3,		// orr	r0,r0,#1
			0x10, 0xff, 0x2f, 0xe1,		// bx	r0
			0xfe, 0xff, 0xff, 0xea,		// b	.
// c000150
			0x54, 0x01, 0x00, 0x0c,		// .ltorg (value of thumb_start)
			0x13, 0xe0,	// b	t_block
			0xfe, 0xe7,	// b	.
			0xc0, 0x46,	// nop
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
// c000160
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
// c000170
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
/*t_block:*/		0xfe, 0xe7,	// b	.
// c000180
			0xc0, 0x46,	// nop
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
// c000190
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
// c0001a0
			0xc0, 0x46,
			0xc0, 0x46,
			0xc0, 0x46,
			0xfe, 0xe7,	// b	.
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
// c0001b0 
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
// c0001c0 
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
// c0001d0 
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,		
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
			0xfe, 0xe7,
// c0001e0 
};
