/*
 * jt_instr.c
 *
 * Intructions send to the TAP Controller
 *  
 * Support functions to access the JTAG interface via parallel port
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
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "dbg_msg.h"
#include "jt_tap.h"
#include "jt_instr.h"
#include "jt_arm.h"

#include "jedec_vendorID.h"

static char jedec_unknown[] = "vendor unknown";

const char instr_extest[]  = {0,0,0,0,'$'};
const char instr_scan_n[]  = {0,0,1,0,'$'}; /*alias USER 1*/
const char instr_sample[]  = {0,0,1,1,'$'}; /*alias USER 2 don't use*/
const char instr_restart[] = {0,1,0,0,'$'};
const char instr_clamp[]   = {0,1,0,1,'$'};
const char instr_highz[]   = {0,1,1,1,'$'};
const char instr_clampz[]  = {1,0,0,1,'$'};
const char instr_intest[]  = {1,1,0,0,'$'};
const char instr_idcode[]  = {1,1,1,0,'$'};
const char instr_bypass[]  = {1,1,1,1,'$'};

const int instr_length = 4;


/* 
 * extern int scan_chain;
 * extern int data_length_of_scan_chain[];
 */
char jtag_data_IDcode[32];
const int data_length_of_IDCODE = 32; /* (4 bit)-Version (16 bit)-Part number (11 bit)-VerdorID (1 bit)-End marker*/
int active_jtag_device = 0;

/*
# bits 11-1 of the Device Identification Register
	any legal 11 Bit JEDEC ID
	0,0,1,0,0,1,1,1,0,1,1,	ARM (JEDEC ID)
	1,1,1,1,0,0,0,0,1,1,1,  ARM (ilegal non JEDEC ID)
*/

/*
# bits 19-12 device number
	0,0,0,0,0,0,0,0		Core only
	0,0,1,0,0,0,0,0		with MMU
	0,0,1,0,0,0,1,0		with MMU and half size cache
	0,0,1,0,0,1,1,0		with MMU and tightly coupled memory
	0,1,0,0,0,0,0,0		with Memory Protection Unit
	0,1,0,0,0,1,1,0		with Memory Protection Unit and tightly coupled memory
	0,1,1,0,0,1,1,0		with tightly coupled memory
   	0,0,1,1,0,1,1,0		with V6 MMU

	1,1,1,1,0,0,0,0		presents of MMU is not defined
*/
#define ARM_DEV_NUM_CORE_ONLY			0
#define ARM_DEV_NUM_WITH_MMU			0x20
#define ARM_DEV_NUM_WITH_MMU_HALFSIZE_CACHE	0x22
#define ARM_DEV_NUM_WITH_MMU_TIGHTLY_COUP_MEM	0x26
#define ARM_DEV_NUM_WITH_MPU			0x40
#define ARM_DEV_NUM_WITH_MPU_TIGHTLY_COUP_MEM	0x46
#define ARM_DEV_NUM_WITH_TIGHTLY_COUP_MEM	0x66
#define ARM_DEV_NUM_WITH_MMU_V6			0x36

/*
# bits 23-20 family
	0,1,1,1			ARM7
	1,0,0,1			ARM9
	1,0,1,0			ARM10
	1,0,1,1			ARM11
	in case bits 27 - 24 are 1,1,1,1 and the presents of MMU is not defined
	0,0,0,0			ARM7 or ARM9
	0,0,0,1			ARM9 

	in case vendor is OKI 0x2E (0x05D)
	0,0,1,0			ML674001
*/
#define ARM_FAMILY_ARM7		7
#define ARM_FAMILY_ARM9		9
#define ARM_FAMILY_ARM10	10
#define ARM_FAMILY_ARM11	11

//#define ARM_FAMILY_OKI		2

/*
# bits 27 - 24 ARM Core ID and Capability Bits 
	0,0,0,0		ARM Processor without E extension - hard macrocell
	0,0,0,1		ARM Processor without E extension - soft macrocell
	0,0,1,X		Reserved
	0,1,0,0		ARM processor with E extension - hard macrocell
	0,1,0,1		ARM processor with E extension - soft macrocell
	0,1,1,0		ARM Processor with J extension - hard macrocell
	0,1,1,1		ARM Processor with J extension - soft macrocell
	1,0,0,0		Reserved
	1,0,0,1		Not a recognised executable ARM device
	1,0,1,0		Reserved
	1,0,1,1		ARM Trace Buffer (so it is not the core it self)
	1,1,0,0		Reserved
	1,1,0,1		Reserved
	1,1,1,0		Reserved
	1,1,1,1		maybe includes Boundary Scan Ids for ARMs.
*/
#define ARM_CAP_HC	0x0
#define ARM_CAP_SC	0x1
#define ARM_CAP_E_HC	0x4
#define ARM_CAP_E_SC	0x5
#define ARM_CAP_J_HC	0x6
#define ARM_CAP_J_SC	0x7

/*
# bits 31 - 28 revison number
*/

/*
# bits 27 - 12 Part number alternative description
 */
#define ARM_PART_ARM7_OR_ARM9		0xF0F0
#define ARM_PART_ARM7_SOFT_CORE		0xF1F0

struct description {
	char string[sizeof("with Memory Protection Unit and tightly coupled memory")];
};
static struct description dev_description[] = {
	{"core only"},
	{"with MMU"},
	{"with MMU and half size cache"},
	{"with MMU and tightly coupled memory"},
	{"with Memory Protection Unit"},
	{"with Memory Protection Unit and tightly coupled memory"},
	{"with tightly coupled memory"},
	{"with V6 MMU"}
};

static struct description cap_description[] = {
	{""},
	{"enhanced DSP extensions"},
	{"Java support"},
	{"(soft core)"},
	{"(soft core)enhanced DSP extensions"},
	{"(soft core)Java support"},
};

static char unknown_description[] = "unknown";

/*
 * Note function is called in "human readable notation" with
 * MSB shiftet in first and MSB shifted out first
 * while Hardware makes in real live;
 * LSB shifted in first and LSB shifted out first
 */

/*
 * return : 1 on success else 0
 */
int jtag_send_instr(const char * cmd)
{
	char *cmd_str,*cp;
	int   i,j;
	struct chain_info *curr;

#ifdef WITH_ALLOCA
	/* Stack allocation */
	cmd_str = (char *) alloca(sizeof(char)*chain_head.total_length);
#else
	/* Heap allocation */
	cmd_str = (char *) malloc(sizeof(char)*chain_head.total_length);
#endif

	if(cmd_str == NULL)
		return 0;

	/*create command for all devices*/
	cp = cmd_str;
	curr = chain_head.last;
	for(i=chain_head.num_of_devices-1;i>=0;i--)
	{
		if(i == active_jtag_device)
		{
			for(j=0;j<instr_length;j++)
			{
				*cp = cmd[instr_length - 1 - j];
				cp++;
			}
		}
		else
		{
			for(j=0; j < curr->instr_len;j++)
			{
				*cp = 1; // bypass
				cp++;
			}
		}
		curr = curr->prev;
	}
	/*send command*/
	tap_instr(chain_head.total_length, cmd_str, NULL);

#ifndef NO_DUMP
	IF_DBG ( DBG_LEVEL_JTAG_INSTR )
	{
		int j;
		printf("\tinstr (lsb first) -> ");
		j = chain_head.total_length;
		cp = cmd_str;
		for(i=0; i<j; i++)
		{
			printf("%c", (*cp + '0'));
			cp++;
		}
		printf("\n");
	}
#endif

#ifndef WITH_ALLOCA
	free(cmd_str);
#endif

	return 1;
}


/*
 * return : 1 on success else 0
 */
int jtag_exchange_data(int len, char *data_out, char *data_in)
{
	char *d_pt, *cp;
	int i,j;
#ifndef NO_DUMP
	char *tmp_in = NULL;
#endif

	if(len <= 0)
		return 0;

#ifdef WITH_ALLOCA
	d_pt = (char *) alloca(sizeof(char)*len + chain_head.num_of_devices - 1);
#ifndef NO_DUMP
	IF_DBG ( DBG_LEVEL_JTAG_INSTR )
		tmp_in = (char *) alloca(sizeof(char)*len);
#endif
#else
	d_pt = (char *) malloc(sizeof(char)*len + chain_head.num_of_devices - 1);
#ifndef NO_DUMP
	IF_DBG ( DBG_LEVEL_JTAG_INSTR )
		tmp_in = (char *) malloc(sizeof(char)*len);
#endif
#endif

	/*move data_out into correct position*/
	cp = d_pt;
	for(i=chain_head.num_of_devices-1;i>=0;i--)
	{
		if(i == active_jtag_device)
		{
			for(j=0;j<len;j++)
			{
				*cp = data_out[len - 1 - j];
				cp++;
			}
		}
		else
		{
			*cp = 0;
			cp++;
		}
	}
#ifndef NO_DUMP
	IF_DBG ( DBG_LEVEL_JTAG_INSTR )
	{
		printf("\tXchng (lsb first) -> ");
		j = len+ chain_head.num_of_devices - 1;
		cp = d_pt;
		for(i=0; i<j; i++)
		{
			printf("%c", (*cp + '0'));
			cp++;
		}
		printf("\n");
		if(data_in == NULL)
			data_in = tmp_in;
	}
#endif
	if(data_in == NULL) // we did not need to read any data
	{
		tap_data(len + chain_head.num_of_devices - 1, d_pt, NULL);
	}
	else // we want to have the previous data from the scan chain
	{
		/*exchange data*/
		tap_data(len + chain_head.num_of_devices - 1, d_pt, d_pt);

		/*move data back*/
		cp = d_pt;
		for(i=chain_head.num_of_devices-1;i>=0;i--)
		{
			if(i == active_jtag_device)
			{
				for(j=0;j<len;j++)
				{
					data_in[len - 1 - j] = *cp;
					cp++;
				}
			}
			else
			{
				cp++;
			}
		}
#ifndef NO_DUMP
		IF_DBG ( DBG_LEVEL_JTAG_INSTR )
		{
			printf("\tXchng (msb first) <- ");
			j = len+ chain_head.num_of_devices - 1;
			cp = d_pt;
			for(i=0; i<j; i++)
			{
				printf("%c", (*cp + '0'));
				cp++;
			}
			printf("\n");
#ifndef WITH_ALLOCA
			free(tmp_in);
#endif
		}
#endif
#ifndef WITH_ALLOCA
		free(d_pt);
#endif
	}
	return 1;
}

/* 
 * show info of IDCODE
 * return 1 if it is a valid idcode
 * otherwise return 0
 */
static int jt_info_idcode(int val)
{
	int vendor_id;
	int part_id;
	int family_id, dev_num, revision, cap_id;
	int valid = 0;
	int is_arm7 = 0;
	int is_arm9 = 0;
	int expect_core_number = arm_info.core_number;
	int expect_bigendian   = arm_info.bigend;
			
	vendor_id = (val>>1)&0x7FF; // 11 bits that give us the index within the jedec manufactor lookup table
	arm_info.vendor_id = vendor_id;
	
	if(vendor_id == 0x787) // this is ARM's wrong test ID F0F
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"%s\t",jedec_unknown);
		arm_info.vendor_string = jedec_unknown;
		valid = 1;
	}
	else if( (vendor_id & 0x7F) == 0 || (vendor_id & 0x7F) == 0x7F)
	{
		valid = 0;
	}
	else if (vendor_id < 589)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"vendor %s\t" ,jedec_vendor[vendor_id].string);
		arm_info.vendor_string = jedec_vendor[vendor_id].string;
		valid = 1;
	}
	else
		valid = 0;
			
	part_id = (val>>12) & 0xFFFF;
	if(valid && part_id == ARM_PART_ARM7_OR_ARM9)
	{
		arm_info.core_number = 8;
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"ARM7 or ARM9 ");
		arm_info.dd_type = -1;
		arm_info.dd_string = unknown_description;
		arm_info.cap_type = -1;
		arm_info.cap_string = unknown_description;
		arm_info.has_stepbug = 0;
		if (expect_core_number == 7)
		{
			is_arm7 = 1;
			arm_info.core_number = 7;
		}
		else if (expect_core_number == 9)
		{
			is_arm9 = 1;
			arm_info.core_number = 9;
		}
		else
			valid = 0;
	}
	else if(valid && part_id == ARM_PART_ARM7_SOFT_CORE)
	{
		arm_info.core_number = 7;
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"ARM7 softcore type ");
		arm_info.dd_type = -1;
		arm_info.dd_string = unknown_description;
		arm_info.cap_type = -1;
		arm_info.cap_string = unknown_description;
		arm_info.has_stepbug = 1;
		if (expect_core_number == 7)
			is_arm7 = 1;
		else
			valid = 0;
	}
	else if (valid)
	{
		family_id = (val>>20) & 0xF;	/*bits 23-20 family*/			
		arm_info.core_number = family_id;
		
		if(family_id == ARM_FAMILY_ARM7)
		{
			printf("ARM7 ");
			is_arm7 = 1;
		}
		else if(family_id == ARM_FAMILY_ARM9)
		{
			printf("ARM9 ");
			is_arm9 = 1;
		}
		else if(family_id == ARM_FAMILY_ARM10)
		{
			printf("ARM10 ");
			valid = 0;
		}
		else if(family_id == ARM_FAMILY_ARM11)
		{
			printf("ARM11 ");
			valid = 0;
		}
		else
			valid = 0;
				
		dev_num = (val>>12) & 0xFF; 	/*bits 19-12 device number*/
		if(valid && dev_num == ARM_DEV_NUM_CORE_ONLY)
		{
			arm_info.dd_type = 0;
			arm_info.dd_string = dev_description[0].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MMU)
		{
			arm_info.dd_type = 1;
			arm_info.dd_string = dev_description[1].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MMU_HALFSIZE_CACHE)
		{
			arm_info.dd_type = 2;
			arm_info.dd_string = dev_description[2].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MMU_TIGHTLY_COUP_MEM)
		{
			arm_info.dd_type = 3;
			arm_info.dd_string = dev_description[3].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MPU)
		{
			arm_info.dd_type = 4;
			arm_info.dd_string = dev_description[4].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MPU_TIGHTLY_COUP_MEM)
		{
			arm_info.dd_type = 5;
			arm_info.dd_string = dev_description[5].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_TIGHTLY_COUP_MEM)
		{
			arm_info.dd_type = 6;
			arm_info.dd_string = dev_description[6].string;
		}
		else if(valid && dev_num == ARM_DEV_NUM_WITH_MMU_V6)
		{
			arm_info.dd_type = 7;
			arm_info.dd_string = dev_description[7].string;
		}
		else
			valid = 0;

		if(valid)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"%s ",dev_description[arm_info.dd_type].string);

		cap_id = (val>>24) & 0xF;	/*bits 27 - 24 ARM Core ID and Capability Bits*/
		if(valid && cap_id == ARM_CAP_HC )
		{
			arm_info.has_stepbug = 0;
			arm_info.cap_type = 0;
			arm_info.cap_string = cap_description[0].string;
		}
		else if(valid && cap_id == ARM_CAP_E_HC)
		{
			arm_info.has_stepbug = 0;
			arm_info.cap_type = 1;
			arm_info.cap_string = cap_description[1].string;
		}
		else if(valid && cap_id == ARM_CAP_J_HC )
		{
			arm_info.has_stepbug = 0;
			arm_info.cap_type = 2;
			arm_info.cap_string = cap_description[2].string;
		}
		else if(valid && cap_id == ARM_CAP_SC)
		{
			arm_info.has_stepbug = 1;
			arm_info.cap_type = 3;
			arm_info.cap_string = cap_description[3].string;
			arm_info.has_stepbug = 1;
		}
		else if(valid && cap_id == ARM_CAP_E_SC)
		{
			arm_info.has_stepbug = 1;
			arm_info.cap_type = 4;
			arm_info.cap_string = cap_description[4].string;
		}
		else if(valid && cap_id == ARM_CAP_J_SC)
		{
			arm_info.has_stepbug = 1;
			arm_info.cap_type = 5;
			arm_info.cap_string = cap_description[5].string;
		}
		else
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(invalid)\n");
			valid = 0;
		}
		if(valid)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"%s",cap_description[arm_info.cap_type].string);


	}
	revision = (val>>28) & 0xF;	/*bits 31 - 28 revison number*/
	arm_info.core_revision = revision;
		
	if(valid)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"rev%d ",revision);
		
		/*currently we do support arm7tdmi cores only*/
		if(is_arm9 == 1)
			valid = 0;
	}
	else
	{
		// restore original values for next loop
		arm_info.core_number = expect_core_number;
		arm_info.bigend      = expect_bigendian;
	}
	return valid;
			
}

/*
 * return (element pos of IDcode in list starting at 1) if matches with one known device IDCODE else 0
 * the IDCODE is stored in global variable jtag_data_IDcode
 */
int jtag_identify_devices(void)
{
	char *dev_str,*cmp_str;
	int   i,j;
	long  val;
	struct chain_info *curr;
	
	tap_discover_chain();
	
	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Number of devices in Jtag-chain: %d\n",chain_head.num_of_devices);
	curr = chain_head.first;
	for(i=0;i<chain_head.num_of_devices;i++)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"[%d]\tinstr. length: %d\t",i,curr->instr_len);
		if(curr->has_idcode == 0)
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"(no JEDEC ID Code available)\n");
		else
		{
			jtag_supp_bitstr2int_MSB_First(32, &val, curr->idcode);
			val = (val>>1)&0x7FF; // 10 bits that give us the index within the jedec manufactor lookup table
			dbgPrintf(DBG_LEVEL_JTAG_INSTR,"0x%3.3x ",val);
			if(val == 0x787) // this is ARM's wrong test ID F0F
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(JEDEC illigal) ARM ID\t");
			else if (val < 589)
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"JEDEC %s\t" ,jedec_vendor[val].string);
			else if ( (val & 0x7F) == 0)
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"JEDEC invalid Start of table[%ld]\t", (val & 0x380)>>7 );
			else if ( (val & 0x7F) == 0x7F)
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"JEDEC invalid Continuation Code (End of table[%ld]\t", (val & 0x380)>>7 );
			else
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"unknown index %ld\t",val);
			
			if(curr->instr_len == instr_length)
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(maybe an ARMxTDMI device)\n");
			else
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(other device type)\n");
		}
		curr = curr->next;
	}
	curr = chain_head.first;
	for(active_jtag_device = 0; active_jtag_device < chain_head.num_of_devices; active_jtag_device++)
	{
		if(curr->instr_len != instr_length || curr->has_idcode == 0)
		{
			curr = curr->next;
			continue;
		}

		memset(jtag_data_IDcode,1,data_length_of_IDCODE);
		/*send command*/
		if(jtag_send_instr(instr_idcode) == 0)
			return 0;
		/*read ID code*/
		if(jtag_exchange_data(data_length_of_IDCODE, jtag_data_IDcode, jtag_data_IDcode) == 0)
			return 0;

		/*check that the current reponse is equal to the after reset respose*/
		i = 0; // mark as OK		
		dev_str = jtag_data_IDcode;
		cmp_str = curr->idcode;
		for(j=0;j<data_length_of_IDCODE;j++)
		{
			if(*cmp_str != *dev_str)
			{
				i = 1;
				break;
			}
			dev_str++;
			cmp_str++;
		}

		if(i)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ups telling device %d some unknown stuff\n",active_jtag_device);
			/*paranoia reset to make sure that nothing get to worst*/
			tap_reset();
			tap_idle();
			curr = curr->next;
			continue;
		}

#ifndef NO_DUMP
		/*dump string of ID that was delivered through JTAG*/
		IF_DBG ( DBG_LEVEL_GDB_ARM_INFO )
		{
			dev_str = jtag_data_IDcode;
			for(j=0;j<data_length_of_IDCODE;j++)
			{
				putchar(*dev_str + '0');
				dev_str++;
			}
			printf("\treading device %d in chain\n",active_jtag_device);
		}
#endif

		/*show what we got*/
		jtag_supp_bitstr2int_MSB_First(32, &val, curr->idcode);
		if(jt_info_idcode(val))
		{
			printf("[%d] OK\n",active_jtag_device);
			return 1;
		}

		curr = curr->next;
	}
	return 0;
}

/*
 * End Of Sequence 
 * This Signal set TAP controler into run/idle state.
 * e.g. this indicates an end of a debug communication and releases the CPU to work in nonblocking normal mode.
 */
void jtag_eos(void)
{
	tap_idle();
	return;
}

void jtag_start(void)
{
	tap_start();
	return;
}

void jtag_reset(void)
{
	tap_reset();
	return;
}

void jtag_hard_reset(void)
{
	tap_hard_reset();
	return;
}


void jtag_linger(int delay)
{
	int i;

	for(i=0;i<delay;i++)
		tap_delay();
	return;
}


