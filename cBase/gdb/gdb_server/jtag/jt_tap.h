/*
 * jt_tap.h
 *
 * Test Access Port Controller Interface
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
/* The tap controller changes its internal states on TMS at rising edge off TCK */
/* All Inputs (TDI) are sampled at the rising edge of TCK*/
/* All Outputs (TDO) are propageted at falling edge of TCK*/

#ifdef TINKER_LEVEL_SHIFTER
#define DRV_IO_FNCT(_X_) _X_ ## _tls
#endif
#ifdef TINKER_BDM
#define DRV_IO_FNCT(_X_) _X_ ## _tbdm
#endif
#ifdef TINKER_LONGO
#define DRV_IO_FNCT(_X_) _X_ ## _tlongo
#endif
#ifdef OLIMEX_MSP430
#define DRV_IO_FNCT(_X_) _X_ ## _omsp
#endif
#ifdef OCDEMON_WIGGLER
#define DRV_IO_FNCT(_X_) _X_ ## _wiggler
#endif
#ifdef ALTERA_BYTEBLASTER
#define DRV_IO_FNCT(_X_) _X_ ## _bblst
#endif
#ifdef LATTICE_ISPDLC
#define DRV_IO_FNCT(_X_) _X_ ## _ispl
#endif
#ifdef XILINX_DLC
#define DRV_IO_FNCT(_X_) _X_ ## _dlc
#endif
#ifdef AMONTEC_EPP_ACCELERATOR
#define DRV_IO_FNCT(_X_) _X_ ## _apod
#endif


#define TEST_JTAG_MODIFY_ICE		0x80000000uL	/* ICE modify */
//					0x40000000uL
//					0x20000000uL
//					0x10000000uL
#define TEST_JTAG_FLASH_READ		0x08000000uL	/* Flash info/read */
#define TEST_JTAG_RAM_WRITE		0x04000000uL	/* RAM read/write test */
#define TEST_JTAG_RAM_PROGRAM		0x02000000uL	/* RAM program test */
#define TEST_JTAG_RAM_READ		0x01000000uL	/* read RAM */
#define TEST_JTAG_MODIFY_CPU_REG	0x00800000uL	/* Write CPU Regs */
#define TEST_JTAG_START_STEPS		0x00400000uL	/* step 15 */
//					0x00200000uL
#define TEST_JTAG_RELEASE		0x00100000uL	/* release */

#define TEST_JTAG_SHOWREGS		0x00000002uL	/* show ICE and CPU Regs */
#define TEST_JTAG_GETID			0x00000001uL	/* get JTAG-ID */

extern uint32_t ramBase; /*for TEST JTAG*/
extern int RAM_BASE_def; /*for TEST JTAG*/

extern uint32_t flashBase; /*for TEST JTAG*/
extern int FLASH_BASE_def; /*for TEST JTAG*/
extern int jtag_test(uint32_t level);

struct chain_info {
	int instr_len;
	int has_idcode;
	char idcode[32];
	struct chain_info *next;
	struct chain_info *prev;
};

struct chain_head {
	int num_of_devices;
	int total_length;
	struct chain_info * first;
	struct chain_info * last;
};

extern struct chain_head chain_head;

enum DriverId {
	NO_DRIVER = 0,
	DRIVER_TINKER_LEVEL_SHIFTER,
	DRIVER_TINKER_BDM,
	DRIVER_TINKER_LONGO,
	DRIVER_OLIMEX_MSP430,
	DRIVER_OCDEMON_WIGGLER,
	DRIVER_ALTERA_BYTEBLASTER,
	DRIVER_LATTICE_ISPDLC,
	DRIVER_XILINX_DLC,
	DRIVER_AMONTEC_EPP_ACCELERATOR
};

extern int tap_grant_sreset_timout;
extern int tap_settle_system_timeout;

extern void tap_probe(void);
extern void tap_delay(void);
extern void tap_hard_reset(void);
extern void tap_reset(void);
extern void tap_start(void);
extern void tap_stop(void);
extern void tap_idle(void);
extern void tap_instr(int num_bits, char *to_dev, char *from_dev);
extern void tap_data(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain(void);
extern int tap_raw_io_test(void);

extern int tap_driver_init(enum DriverId driverId);

