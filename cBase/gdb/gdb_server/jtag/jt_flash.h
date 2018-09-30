/*
 * jt_flash.h
 * 
 * Flash support functions
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

enum flashAlgo {
	FLASH_ALGORITHEM_UNKOWN = 0,
	FLASH_ALGORITHEM_NOSUPPORT,
	FLASH_ALGORITHEM_AMD,
	FLASH_ALGORITHEM_INTEL,
	FLASH_ALGORITHEM_PHILIPS,
	FLASH_ALGORITHEM_ATMEL,
	FLASH_ALGORITHEM_ST
};

enum sectorState {
	SECTOR_INVALID			= 0x00,
	SECTOR_FLAG_HAS_DATA		= 0x01, /* (Flag - only - to tell if data present) */
	SECTOR_FLAG_RW			= 0x02, /* (Flag - only - write or read) */
	SECTOR_FLAG_DURTY		= 0x04, /* (Flag - only) */
	SECTOR_FLAG_ERASED		= 0x08, /* (Flag - only) */
	SECTOR_READ_VALID		= 0x01, /* read from target- so it is valid */
	SECTOR_READ_ERASED_VALID	= 0x09, /* read from target but all data are erased*/
	SECTOR_WRITE			= 0x03, /* nither yet written, nor checked */
	SECTOR_WRITE_DURTY		= 0x07, /* not yet written, but known to be dury */
	SECTOR_WRITE_DURTY_ERASED	= 0x0f, /* not yet written, but known to be dury and Flash is already erased*/
};

struct sector {
	int			size:24;	/*size of the current sector*/
	int			lock:1;		/*intel lock bit*/
	int			region:7;	/*atmel lock bit region number*/
	int			sectors_per_region:16;	/**/
	enum sectorState	valid:16;	/* if valid host-RAM-Buffer and target real Flash are equal */
};

/*prototyp definiton*/
extern uint16_t jt_flashReadHalfword(uint32_t address);
extern uint8_t jt_flashReadByte_of_Halfword(uint32_t address);

/*AMD*/
extern uint8_t jt_amdflashReadResetByte(uint32_t base_address, uint32_t address);
extern void jt_amdflashResetByte(uint32_t base_address);
extern int jt_amdflashProgByte(uint32_t base_address, uint32_t address, uint8_t data, unsigned wait);
extern int jt_amdflashProgByte_faster(uint32_t base_address, uint32_t address, uint8_t *data, int numberOfData,unsigned verify);
extern int jt_amdflashEraseSectorByte(uint32_t base_address, uint32_t address);

extern uint16_t jt_amdflashReadResetHalfword(uint32_t base_address, uint32_t address);
extern void jt_amdflashResetHalfword(uint32_t base_address);
extern int jt_amdflashProgHalfword(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait);
extern int jt_amdflashProgHalfword_faster(uint32_t base_address, uint32_t address, uint16_t *data, int numberOfData,unsigned verify);
extern int jt_amdflashEraseSectorHalfword(uint32_t base_address, uint32_t address);

extern uint32_t jt_amdflashReadResetWord(uint32_t base_address, uint32_t address);
extern void jt_amdflashResetWord(uint32_t base_address);
extern int jt_amdflashProgWord(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait);
extern int jt_amdflashProgWord_faster(uint32_t base_address, uint32_t address, uint32_t *data, int numberOfData,unsigned verify);
extern int jt_amdflashEraseSectorWord(uint32_t base_address, uint32_t address);


extern int jt_amdflashGetInfoByte( uint32_t base_address, struct sector **sector_info );
extern int jt_amdflashGetInfoHalfword( uint32_t base_address, struct sector **sector_info );
extern int jt_amdflashGetInfoWord( uint32_t base_address, struct sector **sector_info );

/*Intel*/
extern int jt_intelflashProgByte(uint32_t base_address, uint32_t address, uint8_t data, unsigned wait);
extern int jt_intelflashUnlockSectorByte(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorByte(uint32_t base_address, uint32_t address);

extern int jt_intelflashProgHalfword(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait);
extern int jt_intelflashProgHalfword_dual(uint32_t base_address, uint32_t address, uint16_t data, unsigned wait);
extern int jt_intelflashUnlockSectorHalfword(uint32_t base_address, uint32_t address);
extern int jt_intelflashUnlockSectorHalfword_dual(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorHalfword(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorHalfword_dual(uint32_t base_address, uint32_t address);

extern int jt_intelflashProgWord(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait);
extern int jt_intelflashProgWord_dual(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait);
extern int jt_intelflashProgWord_quad(uint32_t base_address, uint32_t address, uint32_t data, unsigned wait);
extern int jt_intelflashUnlockSectorWord(uint32_t base_address, uint32_t address);
extern int jt_intelflashUnlockSectorWord_dual(uint32_t base_address, uint32_t address);
extern int jt_intelflashUnlockSectorWord_quad(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorWord(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorWord_dual(uint32_t base_address, uint32_t address);
extern int jt_intelflashEraseSectorWord_quad(uint32_t base_address, uint32_t address);

extern int jt_intelflashGetInfoByte( uint32_t base_address, struct sector **sector_info );
extern int jt_intelflashGetInfoHalfword( uint32_t base_address, struct sector **sector_info );

/*Philips*/
extern int jt_philipsflashGetInfo( uint32_t base_address, uint32_t size, struct sector **sector_info );
extern uint32_t jt_philipsflashGenMask( int start_sector, int stop_sector, uint32_t flash_size, int boot_sector_enabled);
//extern void jt_philipsflashFindStartStopSectors(uint32_t dst, int len, struct sector *sec, int *start_sector, int *stop_sector, uint32_t flash_size, int boot_sector_enabled);
//extern int jt_philipsflashUnlockSector(uint32_t mask);
//extern int jt_philipsflashLockSector(uint32_t mask);


/*Atmel*/
extern int jt_atmelflashGetInfo( uint32_t base_address, struct sector **sector_info );

/*SGS Thomson*/
extern int jt_stflashGetInfo( uint32_t base_address, uint32_t size, struct sector **sector_info );

extern	int block_size, num_of_blocks; 
extern	int bottom_boot_size;	/* if set assume one block */
extern	int bottom_parameter_size, num_of_bottom_parameter_blocks;
extern	int bottom_inter_size;	/* if set assume one block */
extern	int top_boot_size;	/* if set assume one block */
extern	int top_parameter_size, num_of_top_parameter_blocks;
extern	int top_inter_size;	/* if set assume one block */
extern	int lock;
extern	int num_of_regions;	/*number of atmel regions*/
extern	int num_of_blocks_per_region;	/*number of segments per region*/

extern	int chips_per_bus;

extern void jt_flash_create_sector_info(struct sector **sector_info );


