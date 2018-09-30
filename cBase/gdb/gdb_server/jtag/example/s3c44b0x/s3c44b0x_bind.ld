/*
 *  This file contains directives for the GNU linker which are specific
 *  to the ARM XXX board.
 */
SECTIONS
{
	.text :
	{
		KEEP(*(.text0));
		KEEP(*(.data0));

		KEEP(*(.text1));
		KEEP(*(.data1));
		KEEP(*(.rodata));

		KEEP(*(.data));
		KEEP(*(.text));

		KEEP(*(.text2));
		KEEP(*(.data2));
	}
	.bss :
	{
		*(.bss);
		*(.bss2);
		*(.comment);
		*(.glue_7);
		*(.glue_7t);
	}
}

