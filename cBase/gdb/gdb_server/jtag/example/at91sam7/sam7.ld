/*
 * Copyright 2005 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 *
 *
 * This file contains directives for the GNU linker which are specific
 * to the Atmel - ARM At91sam7
 *
 * Memory layout:
 *
 * 0x0000_0000 -> 0x000F_FFFF : OnChip Flash before REMAP - SRAM after REMAP
 * 0x0010_0000 -> 0x001F_FFFF : OnChip Flash
 * 0x0020_0000 -> 0x002F_FFFF : OnChip RAM
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")
OUTPUT_ARCH(arm)
/*	ENTRY(_start) */
	ENTRY(Entry_VectorTable) 
SECTIONS
{
	_remap_data_start = 0x0;
	_rom_data_start   = 0x00100000;
	_ram_data_start   = 0x00200000;
	_ram_data_size    = 0x4000; /*16 KByte*/

	/******************************************************
	 *
	 * Vektor table
	 *    after reset but before remap it it Flash EEPROM
	 *    after remap it is SRAM
	 *
	 ******************************************************/	
	.remap _remap_data_start : AT(_rom_data_start)
	{
		_vector_table_start = .;
		*crt0.o(.vector);
		_vector_table_end = .;
	}
	_vector_table_size = _vector_table_end - _vector_table_start;
	_init_start        = _rom_data_start   + _vector_table_size;
	
	/******************************************************
	 * 
	 * Text Segments not copied - stored in ROM/Flash area
	 * 
	 ******************************************************/	
	.text _init_start : 
	{
		_text_start = .;
		*crt0.o(.init);
		*(SORT(.text.*));
		*(.text);
		. = ALIGN (4) ;
		*(.gnu.linkonce.t*)
		*(.glue_7t) *(.glue_7)
		*(.rodata) 
		*(.rodata.*) 
		*(.gnu.linkonce.r*) 
		*(.rodata1) 
		*(.eh_frame_hdr) 
		_text_end = .;
	} 
	_text_size	= _text_end - _text_start; 
	_flash_text_end	= _init_start + _text_size;

	/****************************************************************************************
	 * data section:
	 *
	 * This section defines the locations of the data section in ROM as well as in RAM.  
	 * crt0.S copies the data section to RAM.
	 ***********************************************************************************/
        _flash_data_start = _flash_text_end;
	_sdata = _ram_data_start + _vector_table_size; 
	
        .data  _sdata : AT( _flash_data_start ) 
        {
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
		. = ALIGN (4) ;
		*(.data1)
		KEEP (*(.eh_frame))
		*(.gcc_except_table)
		*(.sdata) 
		*(.sdata.*)
		*(.gnu.linkonce.s.*)
		_edata = .;
   	}
	_data_start  = _sdata ;
	 data_start  = _data_start ;
        _data_size   = _edata - _sdata ;

	/**************************************************************************************
	 * bss section:
	 *
	 * The bss section is the last section in SDRAM.  
	 *************************************************************************************/
	.bss _edata : 
	{
		_bss_start = .;
		__bss_start = .;
		__bss_start__ = .;
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON);
		_ebss = ALIGN(4);
		end = _ebss;
		_end = end;
		__end = end;
	} /*: NONE*/
	_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;
        _bss_size  = _ebss - _bss_start ;

	/*********************************************************
	 * Stack variables:
	 * The stack_size variable is customizable here.  
	 ********************************************************/
	stack_size  = 0x1000 ; /*4 KByte*/
	stack_origin = end + stack_size ;
	
	/*********************************************************
	 * Heap variables:
	 * The heap is located directly after the stack in RAM.  
	 ********************************************************/
	heap_bottom  = stack_origin + 4 ;

	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }

}

