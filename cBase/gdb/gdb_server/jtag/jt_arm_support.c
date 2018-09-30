/*
 *
 * ARM7TDMI Interface
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
#include <unistd.h>
#include <stdio.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_instr.h"


/*
 * support function to create a bit string
 */
void jtag_supp_int2bitstr_MSB_First(int len, long val, char *str)
{
	char *cp;

	cp = str + len - 1;

	while(len > 0)
	{
		if(val & 1)
		{
			*cp = 1;
		}
		else
		{
			*cp = 0;
		}
		len--;
		cp--;

		val = val >> 1;
	}
	return;
}

void jtag_supp_int2bitstr_LSB_First(int len, long val, char *str)
{
	char *cp;

	cp = str;

	while(len > 0)
	{
		if(val & 1)
		{
			*cp = 1;
		}
		else
		{
			*cp = 0;
		}
		len--;
		cp++;

		val = val >> 1;
	}
	return;
}

/*
 * support function to generate an integer value out of the bit string
 */
void jtag_supp_bitstr2int_LSB_First(int len, long *val, char *str)
{
	char *cp;

	*val = 0;
	cp = str + len - 1;

	while(len > 0)
	{
		*val = *val << 1;
		if(*cp == 1)
		{
			*val |= 1;
		}
		len--;
		cp--;
	}
}

void jtag_supp_bitstr2int_MSB_First(int len, long *val, char *str)
{
	char *cp;

	*val = 0;
	cp = str ;

	while(len > 0)
	{
		*val = *val << 1;
		if(*cp == 1)
		{
			*val |= 1;
		}
		len--;
		cp++;
	}
}


/*
 * support function to check if this is the correct device 
 * return 1 if well known device
 */
int jtag_arm_verify(void)
{
	int err;

	err = jtag_identify_devices();
	if(err > 0)
	{
#ifdef NO_DEBUG
		dbgPrintf(DBG_LEVEL_JTAG_INSTR,"ok device: %s\n",idcode_string[err]);
#endif
		err = 1;
	}
	return err;
}


/*
 * support function to set the JTAG scann chain
 *
 * scan chain 0 (113 bits) - Macrocell Scan Test
 * scan chain 1 ( 33 bits) - Debug - at ARM7TDMI
 * scan chain 1 ( 67 bits) - Debug - at ARM9TDMI
 * scan chain 2 ( 38 bits) - Embedded ICE logic
 * note:
 * scan chain 3 number of bits are processor dependend
 * 
 */
void jtag_arm_set_chain(int chain)
{
	char chain_str[5];

	jtag_supp_int2bitstr_MSB_First(4, chain, chain_str);
	jtag_send_instr(instr_scan_n);
	jtag_exchange_data(4, chain_str, chain_str);

	scan_chain = chain;
	scan_mode = NON;
}


/* 
 * function to send instruction data and the breakpt bit via scan chain 1 to the CPU
 * 
 * NOTE: Scan chain 1 bit order is (from DIN to DOUT):
 *          D0..D31, BREAKPT
 *
 * break_bit =  DEBUG_SPEED (=0)	; clear BREAKPT , go to Run/Idle
 * break_bit =  SYSTEM_SPEED (=1)	; set BREAKPT , go to Run/Idle
 * break_bit =	DEBUG_REPEAT_SPEED (=2)	; clear BREAKPT , go to Run/Idle (twice)
 * break_bit =	RESTART_SPEED (=4)	; clear BREAKPT, prepare to leave debug mode and prepare restart
 *          
 * The previous BREAKPT value is storen to old_break (useful to know the reason we entered DEBUG)
 * RETURNS:
 *          returns 32 bits read out (result of a previous instruction)
 */
unsigned int jtag_arm7_mov_chain1_data(int break_bit, int *old_break, int data, int read_prev_data)
{
	char instr[33];                    /* 33 bit collecting from the device */
	char outstr[33];                   /* 33 bit senting to device */
	long ret_val;

	if(scan_chain != 1 || 	scan_mode != INTEST)	/* check Scan chain */
		return 0;
	
	if(break_bit == SYSTEM_SPEED)			/*set breakpoint bit*/
		outstr[32] = 1;
	else
		outstr[32] = 0;
	
	jtag_supp_int2bitstr_LSB_First(32, data, &outstr[0]); /*put 32 bit on output string*/
	
	if(read_prev_data) // read in previous data from chain 1
	{
		jtag_exchange_data(33, outstr, instr); 
		jtag_eos();			/* Run-Test/idle => exec instr. */
	
		if(old_break != NULL)
			*old_break = instr[32];

		jtag_supp_bitstr2int_LSB_First(32, &ret_val, &instr[0]); /* Only use 32 LSB bits out of 33 */

		return ret_val;  /* Return 32 bits readout */
	}
	// else // write only into chain

	if(break_bit == RESTART_SPEED)
	{
		jtag_exchange_data(33, outstr, NULL); 
		jtag_eos();
	}
	else if(break_bit == DEBUG_REPEAT_SPEED)
	{
		jtag_exchange_data(33, outstr, NULL); 
		jtag_eos();
	}
	if(break_bit != DEBUG_NO_EOS_SPEED)
	{
		jtag_exchange_data(33, outstr, NULL); 
		jtag_eos();			/* Run-Test/idle => exec instr. */
	}

	return 0;
}

/* 
 * function to send instruction, data and the sysspeed bit via scan chain 1 to the CPU
 * 
 * NOTE: Scan chain 1 bit order is (from DIN to DOUT):
 *          Dat_0..Dat_31,Unused,WPT,SYSSPEED,Instr_31..Instr_0
 *          
 * The previous BREAKPT value is storen to old_sysspeed (useful to know the reason we entered DEBUG)
 * RETURNS:
 *          returns 32 bits read out (result of a previous instruction)
 */
unsigned int jtag_arm9_mov_chain1_data(int sysspeed_bit, int *old_sysspeed, int data_val, int cmd_val, int read_prev_data)
{
	char instr[67];                    /* 67 bit collecting from the device */
	char outstr[67];                   /* 67 bit senting to device */
	long ret_val;

	if(scan_chain != 1 || 	scan_mode != INTEST)                /* check Scan chain */
		return 0;
	
	outstr[32] = 0;
	outstr[33] = 0;
	if(sysspeed_bit)		/*set speed-mode bit*/
		outstr[34] = 1;
	else
		outstr[34] = 0;
	
	jtag_supp_int2bitstr_LSB_First(32, data_val, &outstr[0]); /*put 32 bit on output string*/
	jtag_supp_int2bitstr_MSB_First(32, cmd_val, &outstr[35]); /*put 32 bit on output string*/

	if(read_prev_data) // read in previous data from chain 1
	{
		jtag_exchange_data(67, outstr, instr); 
		jtag_eos();			/* Run-Test/idle => exec instr. */
	
		if(old_sysspeed != NULL)
			*old_sysspeed = instr[34];

		jtag_supp_bitstr2int_LSB_First(32, &ret_val, &instr[0]); /* Only use 32 LSB bits out of 33 */

		return ret_val;  /* Return 32 bits readout */
	}
	// else // write only into chain
	jtag_exchange_data(67, outstr, NULL); 

	jtag_eos();			/* Run-Test/idle => exec instr. */

	return 0;
	

}

/*
 * Perform a single system speed access.
 * Enter a RESTART instr. in the TAP controller.
 * Wait to get ready.
 * Go back to INTEST.
 */
void jtag_arm_chain1_sysspeed_restart(void)
{
	jtag_send_instr(instr_restart);		/* Enter RESTART instr. */
	jtag_eos();				/* goto Run-Test/Idle   */
#if 0	
	if(ice_state.high_speed_mclk)
	{
		/*sometimes we are much to fast so we linger here*/
		jtag_linger(ice_state.high_speed_mclk * 128);
		jtag_send_instr(instr_intest);	/* Enter INTEST instr.  */
	}
	else
#endif		
	{
		/*at slow MCLK speed make sure we are back in debug state*/
		while((jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS) & 0x09) != 0x09)
			;

		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr. */
		scan_mode = INTEST;
	}

	return;
}

/*
 * support function to check type of instuction
 */
int is_arm_store_instr(uint32_t step_instr)
{
	if(   (step_instr & ARM_STORE_MSK) == ARM_STORE 
	   || (step_instr & ARM_STORE_MISC_MSK) == ARM_STORE_MISC
	   || (step_instr & ARM_STORE_MULTIPLE_MSK) == ARM_STORE_MULTIPLE
	   || (step_instr & ARM_STORE_COPROCESSOR_REG_MSK) == ARM_STORE_COPROCESSOR_REG
	   )
		return 1;
	return 0;
}

/*
 * support function to check type of instuction
 */
int is_thumb_store_instr(uint32_t step_instr)
{
	step_instr &= 0xFFFF;

	switch(step_instr & THUMB_STORE_A_MSK)
	{
	case THUMB_STORE_A_1:
	case THUMB_STORE_A_3:
	case THUMB_STORE_A_BYTE_1:
	case THUMB_STORE_A_HALF_1:
	case THUMB_STORE_A_MULTIPLE:
		return 1;
	default:
		break;
	}

	switch(step_instr & THUMB_STORE_B_MSK)
	{
	case THUMB_STORE_B_2:
	case THUMB_STORE_B_BYTE_2:
	case THUMB_STORE_B_HALF_2:
	case THUMB_STORE_B_PUSH:
		return 1;
	default:
		break;
	}
	return 0;
}

/*
 * check ARM conditon flags
 * cpsr flags: N Z C V
 */
int jt_arm_condition_pass (uint32_t step_instr)
{
	step_instr >>= 28;

	switch(step_instr & 0xF)
	{
	case 0: // EQ (Z is set)
		if ( (CPU.CPSR & 0x40000000) != 0)
			return 1;
		break;
	case 1: // NE (Z is clear)
		if ( (CPU.CPSR & 0x40000000) == 0)
			return 1;
		break;
	case 2: // CS (C is set)
		if ( (CPU.CPSR & 0x20000000) != 0)
			return 1;
		break;
	case 3: // CC (C is clear)
		if ( (CPU.CPSR & 0x20000000) == 0)
			return 1;
		break;
	case 4: // MI (N is set)
		if ( (CPU.CPSR & 0x80000000) != 0)
			return 1;
		break;
	case 5: // PL (N is clear)
		if ( (CPU.CPSR & 0x80000000) == 0)
			return 1;
		break;
	case 6: // VS overflow (V is set)
		if ( (CPU.CPSR & 0x10000000) != 0)
			return 1;
		break;
	case 7: // VC (V is claer)
		if ( (CPU.CPSR & 0x10000000) == 0)
			return 1;
		break;
	case 8: // HI (C set, Z clear)
		if ( (CPU.CPSR & 0x60000000) == 0x20000000)
			return 1;
		break;
	case 9: // LS (C clear, Z set)
		if ( (CPU.CPSR & 0x60000000) == 0x40000000)
			return 1;
		break;
	case 10: // GE ( N is equal with V )
		if ( (CPU.CPSR & 0x90000000) == 0x90000000 || (CPU.CPSR & 0x90000000) == 0 )
			return 1;
		break;
	case 11: // LT ( N is not equal with V )
		if ( (CPU.CPSR & 0x90000000) == 0x80000000 || (CPU.CPSR & 0x90000000) == 1 )
			return 1;
		break;
	case 12: // GT ( Z clear and N is equal with V )
		if ( (CPU.CPSR & 0x40000000) == 0 && ((CPU.CPSR & 0x90000000) == 0x90000000 || (CPU.CPSR & 0x90000000) == 0 ) )
			return 1;
		break;
	case 13: // LE ( Z clean and N is not equal with V )
		if ( (CPU.CPSR & 0x40000000) != 0 && ( (CPU.CPSR & 0x90000000) == 0x80000000 || (CPU.CPSR & 0x90000000) == 1 ) )
			return 1;
		break;
	case 14: // Always
		return 1;
	default:
		break;
	}
	return 0;
}

/*
 * support function to check if the given instruction will modify the program counter
 */
int jt_arm_instr_modifys_PC(uint32_t step_instr)
{
	/*check if this is an undefined instruction*/
	if( (step_instr & 0xFF000000) == 0xFF000000)
		return 1;
	if( (step_instr & 0xFE000000) == 0xF8000000)
		return 1;
	if( (step_instr & 0xF8000000) == 0xF0000000)
		return 1;

	/*check blx  -- not at arm v4 (e.g. ARM7)*/
	if( (step_instr & 0xFE000000) == 0xFA000000)
		return 1;
	
	/*now take a look at the condition field*/
	if( jt_arm_condition_pass(step_instr & 0xF0000000) )
	{
		/*check swi*/
		if( (step_instr & 0x0F000000) == 0x0F000000)
			return 1;
		/*check bl*/
		if( (step_instr & 0x0E000000) == 0x0A000000)
			return 1;
		/*load multiple*/
		if( (step_instr & 0x0E100000) == 0x08100000 )
		{
			/*is pc on register list*/
			if ( (step_instr & 0x8000) == 0x8000 )
				return 1;
			else
				return 0;
		}
		
		/*conditional undefined instructions*/
		if ( (step_instr & 0x0FB00000) == 0x03000000 )
			return 1;
		if ( (step_instr & 0x0E000010) == 0x06000010 )
			return 1;
		/*bx*/
		if ( (step_instr & 0x0FFFFFF0) == 0x012FFF10 )
			return 1;
		/*blx (not arm v4)*/
		if ( (step_instr & 0x0FFFFFF0) == 0x012FFF30 )
			return 1;
		/*software breakpoint (not arm v4)*/
		if ( (step_instr & 0x0FF000F0) == 0x01200070 )
			return 1;
		/*instr. where destination register is at position bit 19-16 */
		if (  (step_instr & 0x0FC000F0) == 0x00000090 /*multiply*/
		   || (step_instr & 0x0F900090) == 0x01000080 /*enhanced DSP multiply*/
		   || (step_instr & 0x0D200000) == 0x05200000 /*ldr/str rd, [PC], #n*/
		   //|| (step_instr & 0x0F200090) == 0x01200090 /*ldr/str Rd, [PC,#n]*/
		   || (step_instr & 0x0E000090) == 0x00000090 /*ldr/str Rd, [PC,#n]*/
		   )
		{
			step_instr >>= 16;
			if( (step_instr & 0xF) == 0xF)
				return 1;
			else
				return 0;
		}
		/*instr. where destination register is at position bit 15-12 */
		if (  (step_instr & 0x0FBF0FFF) == 0x010F0000 /*move status to register*/
		   || (step_instr & 0x0FFF0FF0) == 0x016F0F10 /*count leading zeros*/
		   || (step_instr & 0x0F900FF0) == 0x01000050 /*enhanced DSP add/sub*/
		   || (step_instr & 0x0F000000) == 0x00000000 /*data proc insrt. (and/eor/sub/rsb/add/adc/sbc/rsc) with dest reg.*/
		   || (step_instr & 0x0F800000) == 0x01800000 /*data proc insrt. (orr/mov/bic/mvn)with dest reg.*/
		   || (step_instr & 0x0F000000) == 0x02000000 /*data proc insrt. immediate (and/eor/sub/rsb/add/adc/sbc/rsc) with dest reg.*/
		   || (step_instr & 0x0F800000) == 0x03800000 /*data proc insrt. immediate (orr/mov/bic/mvn) with dest reg.*/
		   || (step_instr & 0x0C100000) == 0x04100000 /*Load register ldr PC, [...]*/
		   )
		{
			step_instr >>= 12;
			if( (step_instr & 0xF) == 0xF)
				return 1;
			else
				return 0;
		}
		/*ldr/str Rn, [Rd , PC]*/
		if (  (step_instr & 0x0E00000F) == 0x0600000F
		   || (step_instr & 0x0E000F9F) == 0x0000009F
		   )
			return 1;
	}
	return 0;
}

/*
 * support function to check if the given instruction will modify the program counter
 */
int jt_thumb_instr_modifys_PC(uint32_t step_instr)
{
	step_instr &= 0xFFFF;

	/*XXX bl and blx can't be handeled within singel step instr. So we skip this here*/

	/*check undefined instr.*/
	if( (step_instr & 0xF800) == 0xE800)
		return 1;
	if( (step_instr & 0xFF00) == 0xDE00)
		return 1;
	/*swi*/
	if( (step_instr & 0xFF00) == 0xDF00)
		return 1;
	/*branch*/
	if( (step_instr & 0xF800) == 0xE000)
		return 1;
	/*software break*/
	if( (step_instr & 0xFF00) == 0xBE00)
		return 1;

	/*conditional branch*/
	if( (step_instr & 0xF000) == 0xD000)
	{
		if( jt_arm_condition_pass( (step_instr & 0x0F00) << 20) )
			return 1;
		else
			return 0;
	}
	/*Add to pc*/
	if( (step_instr & 0xF800) == 0xA000)
		return 1;
	/*special data to pc*/
	if( (step_instr & 0xFC87) == 0x4487)
		return 1;
	/*bx*/
	if( (step_instr & 0xFF07) == 0x4700)
		return 1;
	/*pop {..,PC}*/
	if( (step_instr & 0xFF00) == 0xBD00)
		return 1;
	
	return 0;
}

/*
 * check if instr. doing a access to the Memory.
 */
int jt_arm_instr_access_mem(uint32_t step_instr)
{
	/*take a look at the condition field*/
	if( jt_arm_condition_pass(step_instr & 0xF0000000) )
	{
		/*all ldr/str instr.*/
		if(//   (step_instr & 0x0D000000) == 0x04000000 /*ldr/str rd, [rn], #n*/
		   //|| (step_instr & 0x0F200090) == 0x01200090 /*ldr/str Rd, [rn,#n]*/
		   //|| (step_instr & 0x0E000090) == 0x00000090 /*ldr/str Rd, [Rn,#n] (addr. mode 3)*/
		   //|| (step_instr & 0x0E000000) == 0x04000000 /*ldr/str Rd, [Rn,#n] (immediate)*/
		   //|| (step_instr & 0x0E000010) == 0x06000010 /*ldr/str Rd, [Rn,Rm]*/
		      (step_instr & 0x0E000000) == 0x04000000 /*ldr/str (immediate)*/
		   || (step_instr & 0x0E000010) == 0x06000000 /*ldr/str (register offset)*/

		   || (step_instr & 0x0E400FB0) == 0x000000B0 /*ldr/str halfword */
		   || (step_instr & 0x0E4000B0) == 0x004000B0 /*ldr/str halfword */
		   || (step_instr & 0x0E400FD0) == 0x000000D0 /*ldr/str sign half/sign byte */
		   || (step_instr & 0x0E4000D0) == 0x004000D0 /*ldr/str sign half/sign byte */
		   || (step_instr & 0x0E000000) == 0x08000000 /*ldm/stm */
		  )
			return 1;
	}
	return 0;
}

int jt_thumb_instr_access_mem(uint32_t step_instr)
{
	step_instr &= 0xFFFF;

	/*load from pool*/
	if( (step_instr & 0xF800) == 0x4800)
		return 1;
	
	/*ldr/str register offset*/
	if( (step_instr & 0xF000) == 0x5000)
		return 1;
	
	/*ldr/str immediate offset*/
	if( (step_instr & 0xE000) == 0x6000)
		return 1;
	
	/*ldr/str halfword immediate offset*/
	if( (step_instr & 0xF000) == 0x8000)
		return 1;
	
	/*load/store from/to stack*/
	if( (step_instr & 0xF000) == 0x9000)
		return 1;
	
	/*push/pop*/
	if( (step_instr & 0xF600) == 0xB400)
		return 1;
	
	/*ldm/stm*/
	if( (step_instr & 0xF000) == 0xC000)
		return 1;
	
	return 0;
}

/*
 * check if this is the bx instuction
 * and if so do the bx workaround, now
 */
int jt_arm_instr_bx_workaround(uint32_t step_instr)
{
	uint32_t next_pc;
	uint32_t reg_number;
	
	/*take a look at the condition field*/
	if( jt_arm_condition_pass(step_instr & 0xF0000000) )
	{
		/*bx*/
		if ( (step_instr & 0x0FFFFFF0) == 0x012FFF10 )
		{
			reg_number = step_instr & 0xF;
			next_pc = CPU.regs.r[reg_number];

			if( (reg_number == 15))
			{
				next_pc += 8;
				next_pc &= 0xFFFFFFFC; // force ARM
			}
			
			if (next_pc & 1)
			{
				CPU.CPSR |= (0x20L);  // THUMB
				next_pc &= 0xFFFFFFFE;
			}
			else
			{
				CPU.CPSR &= ~(0x20L); // ARM
				next_pc &= 0xFFFFFFFC;
			}
			
			CPU.regs.r[15] = next_pc;
			return 1;
		}
	}
	return 0;
}

/*
 * check if this is the bx instuction
 * and if so do the bx workaround, now
 */
int jt_thumb_instr_bx_workaround(uint32_t step_instr)
{
	uint32_t next_pc;
	uint32_t reg_number;
	
	/*bx*/
	if ( (step_instr & 0xFF87) == 0x4700 )
	{
		reg_number = ((step_instr & 0x78)>>3);
		next_pc = CPU.regs.r[ reg_number ];
		
		if( (reg_number == 15))
		{
			next_pc += 4;
			next_pc &= 0xFFFFFFFE; // force Thumb
		}
		
		if (next_pc & 1)
		{
			CPU.CPSR |= (0x20L);  // THUMB
			next_pc &= 0xFFFFFFFC;
		}
		else
		{
			CPU.CPSR &= ~(0x20L); // ARM
			next_pc &= 0xFFFFFFFE;
		}
		CPU.regs.r[15] = next_pc;
		return 1;
	}
	return 0;
}

