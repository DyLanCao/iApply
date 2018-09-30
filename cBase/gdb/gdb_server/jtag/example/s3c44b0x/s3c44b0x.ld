/*
 *  This file contains directives for the GNU linker which are specific
 *  to the ARM XXX board.
 *
 * Memory layout:
 *
 * 0x0000_0000 -> 0x0003_FFFF : BOOT ROM / ISR - Entrypoint
 * 0x01C0_0000 -> 0x01FF_FFFF : CPU special function register
 * 0x0C00_0000 -> 0x0FFF_FFFF : text,data,bss and user_stack section in extenal SDRAM
 * 0x1000_0000 -> 0x1000_0FFF : 4K Cache Area
 * 0x1000_1000 -> 0x1000_1FFF : ISR_text,ISR_data, ISR_bss and ISR_stacks section in internal RAM
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")
OUTPUT_ARCH(arm)
	ENTRY(_start)
SECTIONS
{
	_ldinfo_rom_start = 0x0;
	
	_ldinfo_sdram_start = 0x0C000000;
	_ldinfo_sdram_size  = 0x00200000; /*2 MByte*/
	
	_ldinfo_iram_start = 0x10001000;
	_ldinfo_iram_size  = 0x1000; /*4 KByte*/
	
	/******************************************************
	 * 
	 * Init Segments not copied - stored in ROM area
	 * 
	 ******************************************************/
	
	/* Init text */
	.text0 _ldinfo_rom_start :
	{
		_init_text_start = .;
		*(.init);
		. = ALIGN (4) ;
		_init_text_end = .;
	} 
	_init_text_size	= _init_text_end - _init_text_start; 
	
	/* Init data */
	.data0 _init_text_end : 
	{
		_init_data_start = .;
		*(.datainit) ;
		. = ALIGN (4) ;
		_init_data_end = .;
	}
	_init_data_size		= _init_data_end - _init_data_start;
	
	/******************************************************
	 * 
	 * Second Init phase:
	 *	Text segments not copied 
	 *	Data segments copied from ROM to SDRAM
	 * 
	 ******************************************************/		
	_ldinfo_phase2_init_text = _init_data_end;

	/* phase 2 init text */
	.text1 _ldinfo_phase2_init_text : 
	{
		_phase2_init_text_start = .;
		*(.textinit2) ;
		. = ALIGN (4) ;
		_phase2_init_text_end = .;
	} 
	_phase2_init_text_size		= _phase2_init_text_end - _phase2_init_text_start; 

	/* phase 2 init data */
	.data1 _ldinfo_sdram_start : AT( _phase2_init_text_end ) 
	{
		_phase2_init_data_start = .;
		*(.datainit2);
		. = ALIGN (4) ;
		_phase2_init_rodata_start = . ;
	}
	/* read only data */
	.rodata : AT (_phase2_init_text_end + _phase2_init_rodata_start - _phase2_init_data_start)
	{
		_rodata_start = .;
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		_erodata = ALIGN( 0x10 ) ;
		_phase2_init_data_end = .;
	}
	_phase2_init_data_size		= _phase2_init_data_end - _phase2_init_data_start;
	
	/* data section */
        .data  _phase2_init_data_end : AT( _phase2_init_text_end + _phase2_init_data_size ) 
        {
		_sdata = .;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN (4) ;
		_edata = .;
   	}
	_data_start  = _sdata ;
	 data_start  = _data_start ;
        _data_size   = _edata - _sdata ;

	 /* text section */
        .text _edata : AT( _phase2_init_text_end + _phase2_init_data_size + _data_size )
        {
		sym_start = . ;
		CREATE_OBJECT_SYMBOLS

		text_start = . ;
	        _text_start = . ;
        	*(.text ) 
		*(.text.*)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t.*)
		*(.glue_7t) 
		*(.glue_7)		
		. = ALIGN (16);

		*(.eh_fram)
		. = ALIGN (16);

		/*
		 * C++ constructors
		 */
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		LONG(0)
		__CTOR_END__ = .;
		. = ALIGN (4) ;
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;
		_etext = ALIGN( 0x10 );
		_endtext = _etext ; 
        }
	_text_size  = _endtext - sym_start;

	/**************************************************************************************
	 * bss section:
	 *
	 * The bss section is the last section in SDRAM.  
	 *************************************************************************************/
	.bss _etext :
	{
		/*. = _etext;*/
		_bss_start = .;
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON);
		_ebss = ALIGN(0x10);
		end = _ebss;
		_end = end;
		__end = end;
	}
        _bss_size  = _ebss - _bss_start ;


	/********************************************************
	 * 
	 * ISR Segments - copied from ROM into buidin RAM area
	 * 
	 ********************************************************/
	_isr_text_rom = _phase2_init_text_end + _phase2_init_data_size + _data_size + _text_size;
	/* ISR text */
	.text2 _ldinfo_iram_start : AT( _isr_text_rom)
	{
		_isr_text_start = .;
		*(.isr.text)
		*(.isr_text*)
		. = ALIGN (4);
		_isr_text_end = .;
	}
	_isr_text_size		= _isr_text_end - _isr_text_start;

	/* ISR data */
	.data2 _isr_text_end : AT( _isr_text_rom + _isr_text_size )
	{
		_isr_data_start = .;
		*(.isr.data)
		*(.isr_data*)
		. = ALIGN (4) ;
		_isr_data_end = .;
	}
	_isr_data_size  = _isr_data_end - _isr_data_start;

	/* ISR uninizialiced data*/
	.bss2 _isr_data_end : 
	{
		_isr_bss_start = .;
		. = ALIGN (4) ;
		*(.isr.bss)
		*(.isr_bss*)
		. = ALIGN (16) ;
		_isr_bss_end = .;
	}
	/*_isr_bss_size       = _isr_bss_end - _isr_bss_start;*/

	/*********************************************************
	 * General variables:
	 *
	 * The stack_size variable is customizable here.  
	 * The heap is located directly after the stack in RAM.  
	 ********************************************************/
    	stack_size   = 0x80000 ;
	stack_origin = end + stack_size ;
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
