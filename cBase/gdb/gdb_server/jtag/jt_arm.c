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
 * 
 * 
 * The code I've created is inspirated by routines written 
 * by R.Longo and Erwin Authried (they are using a GNU licence 
 * but it is definde which)
 * These sources can be found at cvs.home.at and www.sf.net
 * 
 * Since both codes did not work at my platform I've decide to
 * rewrite it compleatly.
 * This did not mean, that those code is not working at all.
 * Nor did this mean, that my differnt code will work at any other platform.
 * The resulting stuff based on heavy experminetation I've done.
 * Since it dose not match with the ARM documentation it is very likely
 * that that are a plenty of bugs in there.
 * So you should be very careful by using this stuff.
 *
 * For additional info see ARM7TDMI (rev4) Technical Reference Manual
 * document ARM DDI 0210A somewere at www.arm.com
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_instr.h"

/*
 * Note: We support ARM7TDMI only.
 * The ARM7TDMI core uses scan chain 1 with a wide of 33 bits to send instructions
 * and exchange data values to the debug unit.
 * The ARM9TDMI core uses scan chain 1 with a wide of 67 bits (32 bit instr. 32 bit data and 3 bit control). 
 * The ARM10TDMI core does not have this kind of unit at chain 1. (It's using a different mechanism.)
 */

/*
 * scan chain 0 (113 bits) - Macrocell Scan Test
 * scan chain 1 ( 33 bits) - Debug at ARM7TDMI
 * scan chain 1 ( 67 bits) - Debug at ARM9TDMI
 * scan chain 2 ( 38 bits) - Embedded ICE logic
 */
int scan_chain = -1;
enum scan_mode scan_mode;

/* INTERNAL CPU REGISTERS & ASSOCIATED VARIABLES */
struct reg_set		CPU;
struct reg_set_ext	CPU_ext;
struct ice_state ice_state = {0,0,0,0,0,0,0};
struct arm_info arm_info = {2,0,0,0,0,0,0,0,0,0,0,0,0};

/*
 * Read out ARM CPU core registers R0..R15 and CPSR.
 * The values are stored inside of the global structure named CPU.
 */
void jtag_arm_ReadCpuRegs(int reset_inst_counter)
{
	uint32_t instruction;
	int i,status;
	int old_break;
	int enter_from_thumb = 0;		/* THUMB mode flag */
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state\n");
		return;
	}
	/* Read IceRT Status reg. to see if we enter from THUMB or ARM state. */
	status = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);	// so now using chain 2
	
	jtag_arm_set_chain(1);		/* Select Debug Scan chain */
	jtag_send_instr(instr_intest);
	scan_mode = INTEST;

	if (0x10 & status)
    	{ 
		/* The CPU is in THUMB state */
		enter_from_thumb = 1; 	

		/* NOTE: value returned shows BREAKPT/WATCHPT */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, &old_break, THUMB_STR(0,0) , READ_WRITE);
		ice_state.is_watchpoint = old_break;		
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_MOV_R0_PC, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       WRITE_ONLY);
		CPU.regs.r[0] = 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       READ_WRITE);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_STR(0,0) , WRITE_ONLY); 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       WRITE_ONLY);
		CPU.regs.r[15] = 
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       READ_WRITE);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_BX_PC,     WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, THUMB_NOP,       WRITE_ONLY);
		jtag_eos();

		// adjust PC
		//maybe I'm totaly wrong but I see 4 intst in Thumb and the PC is allways two 16 Bit addr. values ahead
		CPU.regs.r[15] -= 6*2; 
		if((ice_state.is_debugrequest&2) != 0 && !ice_state.is_step_mode) // entered by DBGREQ
			CPU.regs.r[15] += 2;
		if(ice_state.is_watchpoint && !ice_state.is_step_mode) // watchpoint has finished the instuction that cause the break
			CPU.regs.r[15] -= 2;
		if( arm_info.has_stepbug && !ice_state.ignore_stepbug)
			CPU.regs.r[15] -= 4;
	}
	else
		enter_from_thumb = 0;
	
	/* We are in ARM state now*/
	if (enter_from_thumb)						/* STM R0,{Rx..Ry}       */
		instruction = ARM_STMIA(0,0x7FFE);			/* -> R1-R14 only in THUMB */
	else
	    	instruction = ARM_STMIA(0,0xFFFF);			/* -> R0-R15 in ARM        */
		
	/* Send opcode & read out break/watchpoint -- if entered in ARM */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, &old_break, instruction, READ_WRITE);	
	
	/* Value returned shows BREAKPT/WATCHPT */
	if (enter_from_thumb == 0)
		ice_state.is_watchpoint = old_break;			

	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_MRS_R0_CPSR,   WRITE_ONLY);		/* MRS R0,CPSR   */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_STR(0,0) ,     WRITE_ONLY);		/* STR R0,R0     */
	
	/* Read out registers while STM is being executed: */
	if (enter_from_thumb)			/* In THUMB state only regs. R1-R14 will be recovered now. */
	{
		for (i=1; i<15; i++)
			CPU.regs.r[i] = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE); /* Copy register contents    */
	}
	else
	{
		for (i=0; i<16; i++)
			CPU.regs.r[i] = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE); /* Copy register contents    */
		// adjust PC
		// maybe I'm totaly wrong but I see 3 intst in ARM and the PC is allways 2 addr.values ahead
		// hm. well if we treat the 16 nop's as one instr we got in total 6
		CPU.regs.r[15] -= 6*4; 
		if((ice_state.is_debugrequest&2) != 0 && !ice_state.is_step_mode) // entered by DBGREQ
			CPU.regs.r[15] += 4;
		if(ice_state.is_watchpoint && !ice_state.is_step_mode) // watchpoint has finished the instuction that cause the break
			CPU.regs.r[15] -= 4;
		if( arm_info.has_stepbug && !ice_state.ignore_stepbug)
			CPU.regs.r[15] -= 8;
	}

	/* next 2 instructions are just NOP's: */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
	CPU.CPSR = 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);

	if (enter_from_thumb)
		CPU.CPSR |= 0x20; // corret value since we collect it in ARM mode
	else
	{
		/*make sure that we ignore the stepbug on entering of an exception*/
		if( arm_info.has_stepbug && !ice_state.ignore_stepbug && (CPU_ext.prev_CPSR & 0x1F) != (CPU.CPSR & 0x1F))
		{
			uint32_t addr;
			uint8_t  mode;

			addr = CPU.regs.r[15] + 8uL;
			mode = CPU.CPSR & 0x1F;
			
			if (
			      ( addr == 0x00uL && mode == 0x13 ) // ? reset	(mode Supervisor)
			   || ( addr == 0x04uL && mode == 0x1b ) // ? instr.	(mode Undef Instr.)
			   || ( addr == 0x08uL && mode == 0x13 ) // ? swi	(mode Supervisor)
			   || ( addr == 0x0CuL && mode == 0x17 ) // ? prefetch	(mode Abort)
			   || ( addr == 0x10uL && mode == 0x17 ) // ? data	(mode Abort)
			   || ( addr == 0x18uL && mode == 0x12 ) // ? irq	(mode irq)
			   || ( addr == 0x1CuL && mode == 0x11 ) // ? fiq	(mode fiq)
			   )
				CPU.regs.r[15] = addr;
		}
	}
	/*read SPSR*/
	if(CPU_ext.prev_CPSR != CPU.CPSR)
	{
		uint8_t mode;
		
		mode = CPU.CPSR & 0x1F;
	
		if(  mode == 0x17 // Abort
		  || mode == 0x12 // irq
		  || mode == 0x11 // fiq
		  || mode == 0x13 // Supervisor
		  || mode == 0x1b)// Undef Instr.
		{
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STMIA_BANK(0,0x6000), WRITE_ONLY);	/* STM banked register r13 and r14*/
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_MRS_R0_SPSR,   WRITE_ONLY);		/* MRS R0,SPSR   */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_STR(0,0) ,     WRITE_ONLY);		/* STR R0,R0     */
			CPU_ext.sp_usr =
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_NOP, READ_WRITE);
			CPU_ext.lr_usr =
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_NOP, READ_WRITE);
			jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
			CPU_ext.SPSR = 
			jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_NOP, READ_WRITE);
		}
		else
			CPU_ext.SPSR = 0;
	}
	
	/*Force setting PC to 0. This makes sure that we can access the Memory*/
	if(reset_inst_counter)
	{
		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,  WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);
		/* address = 0 -> R0 */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, 0uL,           WRITE_ONLY);	
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,       WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_MOV(15,0), WRITE_ONLY);	/* MOV pc,r0 */
	
		jtag_arm_chain1_sysspeed_restart();
	}

	/*finish step instruction*/
	ice_state.is_step_mode = 0;
	ice_state.ignore_stepbug = 0;
 
	return ;
}

/*
 * Dump global CPU registers to the screen.
 */
void jtag_arm_DumpCPUregs(void)
{
	int i;
	
	IF_DBG(DBG_LEVEL_GDB_ARM_INFO)
	{
		/*Print some debug info*/
		printf   ("CPSR = %08X\t<", CPU.CPSR);
		if((CPU.CPSR & 0x20 )== 0x20)
			printf("from THUMB \t");
		else
			printf("from ARM \t");
		switch(CPU.CPSR & 0x1fL)
		{
		case 0x00: printf("(26) User\t"); break;
		case 0x01: printf("(26) FIQ\t"); break;
		case 0x02: printf("(26) IRQ\t"); break;
		case 0x03: printf("(26) Supervisor\t"); break;
		case 0x10: printf("(32) User\t"); break;
		case 0x11: printf("(32) FIQ\t"); break;
		case 0x12: printf("(32) IRQ\t"); break;
		case 0x13: printf("(32) Supervisor\t"); break;
		case 0x17: printf("(32) Abort\t"); break;
		case 0x1b: printf("(32) Undef Instr.\t"); break;
		case 0x1f: printf("(32) System\t"); break;
		default: printf("invalid mode 0x%X\t",(int)(CPU.CPSR & 0x1fuL));
		}
		
		if(ice_state.is_watchpoint)
			printf("> is watchpoint\n");
		else
			printf("> is break\n");
	
		printf   ("SPSR = %08X\t<", CPU_ext.SPSR);
		if (CPU_ext.SPSR != 0)
		{
			if((CPU_ext.SPSR & 0x20 )== 0x20)
				printf("from THUMB \t");
			else
				printf("from ARM \t");
		}
		switch(CPU_ext.SPSR & 0x1fL)
		{
		case 0x00: printf(" (non existance)\n"); break;
		case 0x10: printf("(32) User\n"); break;
		case 0x11: printf("(32) FIQ\n"); break;
		case 0x12: printf("(32) IRQ\n"); break;
		case 0x13: printf("(32) Supervisor\n"); break;
		case 0x17: printf("(32) Abort\n"); break;
		case 0x1b: printf("(32) Undef Instr.\n"); break;
		case 0x1f: printf("(32) System\n"); break;
		default: printf("invalid mode 0x%X\n",(int)(CPU.CPSR & 0x1fuL));
		}
		if (CPU_ext.SPSR != 0)
		{
			printf ("SP_usr = %08X \t", CPU_ext.sp_usr);
			printf ("LR_usr = %08X \n", CPU_ext.lr_usr);
		}
	
		/*show contens of registers*/
		for (i=0; i<8; i++)
			printf ("R%2.2d=%08X ",i, CPU.regs.r[i]);
		printf("\n");
		for (i=8; i<16; i++)
			printf ("R%2.2d=%08X ",i, CPU.regs.r[i]);
		if(CPU.regs.r[15] == 0L)
			printf("enter <Reset>");
		else if(CPU.regs.r[15] == 0x4L)
			printf("enter <Undefinded Intruction>");
		else if(CPU.regs.r[15] == 0x8L)
			printf("enter <Software Interrupt>");
		else if(CPU.regs.r[15] == 0xCL)
			printf("enter <Instruction fetch Abort>");
		else if(CPU.regs.r[15] == 0x10L)
			printf("enter <Data access Abort>");
		else if(CPU.regs.r[15] == 0x14L)
			printf("enter <26 bit Mode Address overflow (>64MB)>");
		else if(CPU.regs.r[15] == 0x18L)
			printf("enter <IRQ>");
		else if(CPU.regs.r[15] == 0x1CL)
			printf("enter <FIQ>");
		
		printf("\n");
	}
	return;
}

/*
 * Write back the stored CPU register to the ARM core.
 * (This function is used for test only.)
 */
void jtag_arm_WriteCpuRegs(void)
{ 
	int i;  
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	/* Recover any overwritten registers: LDM R0,{R0-R15} */
 	jtag_arm7_mov_chain1_data(DEBUG_SPEED       , NULL, ARM_LDMIA(0,0xFfff), WRITE_ONLY);	 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP     , WRITE_ONLY);

	for (i=0; i<16; i++)
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[i], WRITE_ONLY);
	
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP     , WRITE_ONLY);
	// repeat instr only to be in sync next time registers are read out again
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP     , WRITE_ONLY);
	return ;
}

/*
 *
 */
void jtag_arm_ClearPC(void)
{
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,  WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,WRITE_ONLY);
	/* address = 0 -> R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, 0uL,           WRITE_ONLY);	
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,       WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_MOV(15,0), WRITE_ONLY);	/* MOV pc,r0 */
	
	jtag_arm_chain1_sysspeed_restart();
	return;
}

/*
 *
 * Write back the stored CPU register to the ARM core and restart execution to continue the current program.
 * 
 * -- note on exit we messed up the PC
 *  so we can't access the Memory within the debug state.
 *  But we can correct this next time we read out the CPU register
 */
void jtag_arm_PrepareExitDebug(void)
{ 
	uint32_t instr;
	int i, is_thumb;  
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
		
	is_thumb = (CPU.CPSR & 0x20L) == 0x20L ;
	
	/* write CPSR - as ARM MODE  */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LD_R0_PC,           WRITE_ONLY);	/* LDR R0,PC */  
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_MSR_CPSR_R0,        WRITE_ONLY);	/* MSR CPSR, R0 */
	
	if(is_thumb)		/* Recover any overwritten registers: R1-R14 , R15 -> R0 */
		instr =  ARM_LDMIA(0,0x7fff);		/* LDM R0,{R0-R14} */
	else			/* Recover any overwritten registers: R0-R15 */
		instr =  ARM_LDMIA(0,0xffff);		/* LDM R0,{R0-R15} */
 	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, instr,                  WRITE_ONLY);
	
	/* make sure that we are still in ARM mode */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.CPSR & ~(0x20L) ,   WRITE_ONLY);	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,       WRITE_ONLY);

	if(is_thumb)
	{
		jtag_arm7_mov_chain1_data(DEBUG_SPEED,  NULL, ARM_BX_R0,     WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED,  NULL, ARM_NOP,       WRITE_ONLY);
		/* R15 -> R0 */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[15] | 1L , WRITE_ONLY);
		/* 14 read cycles will be executed */
		for (i=1; i<15; i++)
		{ 
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[i], WRITE_ONLY);
		}
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED,  NULL, ARM_NOP,       WRITE_ONLY);
		
		/* now in THUMB mode: writeback saved R0 */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED,  NULL, THUMB_LD_R0_R7,WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED,  NULL, THUMB_NOP,     WRITE_ONLY);
		
		jtag_arm7_mov_chain1_data(DEBUG_SPEED,  NULL, CPU.regs.r[0], WRITE_ONLY);
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED,  NULL, THUMB_NOP,     WRITE_ONLY);
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, THUMB_NOP,     WRITE_ONLY);	

		/* Calculate branch instruction: 
		 * correct -> B -18 */
		if( arm_info.has_stepbug )
			instr = (- 8L) & ((1L<<11)-1);		// calc immed_11
		else
			instr = (- 7L) & ((1L<<11)-1);		// calc immed_11
		instr = (instr<<16) | instr;	// since we dont know if the higher or lower part is executed
		instr |= 0xe000e000;
	}
	else
	{
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP,                WRITE_ONLY);
		/* 16 read cycles will be executed */
		for (i=0; i<16; i++)
		{ 
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[i], WRITE_ONLY);
		}
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED,  NULL, ARM_NOP,       WRITE_ONLY);
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP,       WRITE_ONLY);
	
		/* Calculate branch instruction: 
		 * 3 for debug request plus 1 for each instruction executed (is allready done), 
		 * so the including the branch itself is still missing. */
		if( arm_info.has_stepbug )
			instr = (unsigned int)(-5L );
		else
			instr = (unsigned int)(-4L );
	
		/* generate the branch field in the instr.: */
		instr &= 0x00ffffff;
		instr |= 0xEA000000;
	}	
	jtag_arm7_mov_chain1_data(RESTART_SPEED, NULL,instr, WRITE_ONLY);  
	
	if( arm_info.has_stepbug )
		ice_state.ignore_stepbug = 1;
	
	ice_state.is_debugrequest = 0;
	//CPU_ext.prev_CPSR = CPU.CPSR;
	CPU_ext.prev_CPSR = 0;
	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"program runs\n");
	return ;
}

void jtag_arm_FinalExitDebug(void)
{
	int reg = 0;

	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
	jtag_eos();

	/* Execute system speed accecss*/
	jtag_send_instr(instr_restart);
	scan_mode = RESTART;
		
	jtag_eos();
	ice_state.is_debugrequest = 0;
	return;
}

/*
 * Write back the stored CPU register to the ARM core and execute one instruction.
 * The instruction has to be executed is given in next_instr.
 */
void jtag_arm_Step(uint32_t next_instr)
{ 
	uint32_t instr;
	int i, is_thumb;  

	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
		
	ice_state.is_step_mode = 1;	/* indicate that we are in single step mode */

	is_thumb = (CPU.CPSR & 0x20L) == 0x20L ;
	
	/* write CPSR - as ARM MODE  */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LD_R0_PC,        WRITE_ONLY);	/* LDR R0,PC    */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_MSR_CPSR_R0,     WRITE_ONLY);	/* MSR CPSR, R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY); 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.CPSR & ~(0x20L), WRITE_ONLY);	/* make sure that we are still in ARM mode */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY);
	
	if(is_thumb)		/* Recover any overwritten registers: R1-R14 , R15 -> R0 */
		instr =  ARM_LDMIA(0,0x7fff);				/* LDM R0,{R0-R14} */
	else			/* Recover any overwritten registers: R0-R15 */
		instr =  ARM_LDMIA(0,0xffff);
 	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, instr,               WRITE_ONLY);
	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY);
	
	/* At this point, the LDM R0,{R0-R15} instruction is executed: */
	if(is_thumb)
	{
		/*set stepbug if needed*/
		if( arm_info.has_stepbug )
		{
			if( jt_thumb_instr_modifys_PC(next_instr) || jt_thumb_instr_access_mem(next_instr) )
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%08X ignore stepbug ",next_instr);
				ice_state.ignore_stepbug = 1;
			}
		}
		
		/* R15 -> R0 */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, (CPU.regs.r[15] - 10 ) | 1L , WRITE_ONLY); 
		// 5 Thumb after bx well I can see:
		// - Ld r0,r1
		// - two nop's (before sampling the value)
		// - two before step (to save the value)

		/* 14 write cycles will be executed */
		for (i=1; i<15; i++)
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[i], WRITE_ONLY);

		/* cycle is just a prefetch */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,        WRITE_ONLY);		
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,        WRITE_ONLY);	
		
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_BX_R0,      WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,        WRITE_ONLY);	
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
		
		/* now in THUMB mode: writeback saved R0 */
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, THUMB_LD_R0_PC, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, THUMB_NOP,      WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, THUMB_NOP,      WRITE_ONLY); // no need to harry with jtag_eos();
		
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, CPU.regs.r[0],  WRITE_ONLY);
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, THUMB_NOP,      WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, THUMB_NOP,      WRITE_ONLY); // no need to harry with jtag_eos();
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, THUMB_NOP,      WRITE_ONLY);
		/*modify next intr*/
		next_instr &= 0xFFFF;
		next_instr |= next_instr<<16;
	}
	else
	{
		/*set stepbug if needed*/
		if( arm_info.has_stepbug )
		{
			if ( jt_arm_instr_modifys_PC(next_instr) || jt_arm_instr_access_mem(next_instr) ) 
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%08X ignore stepbug ",next_instr);
				ice_state.ignore_stepbug = 1;
			}
		}
		
		/* 16 write cycles will be executed */
		for (i=0; i<15; i++)
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.regs.r[i], WRITE_ONLY);

		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, CPU.regs.r[15] - 16 , WRITE_ONLY); // - 8
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,              WRITE_ONLY);	
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,              WRITE_ONLY); // no need to harry with jtag_eos();
		
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,              WRITE_ONLY);	
		jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,              WRITE_ONLY); // no need to harry with jtag_eos();
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP,              WRITE_ONLY);
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL,next_instr, WRITE_ONLY);  
	
	/* Execute system speed accecss*/
	jtag_send_instr(instr_restart);
	jtag_eos();
	scan_mode = RESTART;

	CPU_ext.prev_CPSR = CPU.CPSR;
	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"step\n");
	return ;
}

/* 
 * Read out word memory into the given buffer.
 *
 * Max read speed : 19 Kbytes/sec 
 */
void jtag_arm_ReadWordMemory(uint32_t address, int howmanywords, uint32_t *buf)
{
	int i; 
	int thisloop;
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
	/* check for word alignment */
	if(address & 0x3)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ReadWordMemory 0x%8X not align\t",address);
		address &= 0xffffFFFC;
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"using 0x%8X instead\n",address);
	}
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"ReadMemoryBuf 0x%0lx count=%d\n",address,howmanywords);

	/* Load R0 with the base address to read: */  
	jtag_arm7_mov_chain1_data(DEBUG_SPEED,        NULL, ARM_LD_R0_PC, WRITE_ONLY);	/* R0 <- addr by LDR R0,PC */  
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP,      WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED,        NULL, address,      WRITE_ONLY);

	while (howmanywords > 0)
	{
		/*Fill up intrtuction pipe with NOP's and prepare system speed access*/ 	
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED,        NULL, ARM_NOP, WRITE_ONLY);
		
		if(howmanywords == 15) // if this is the last we can read into all Regeister
		{
			thisloop=15;
			
			/* System speed access: LDMIA R0,{R0-Rn} */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA(0, (1L<<thisloop)-1) , WRITE_ONLY);
			jtag_arm_chain1_sysspeed_restart();

			/* STM R0,{R0-Rn} */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_STMIA(0, (1L<<thisloop)-1) , WRITE_ONLY); 
		}
		else // read r1..rn, r0 is the base pointer which is auto incremented after the register load operation
		{
			if(howmanywords >= 14)
				thisloop = 14;
			else 
				thisloop = howmanywords;
		
			/* System speed access: LDMIA R0!,{R1-Rn} */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LDMIA_UPDATE(0, ((1L<<thisloop)-1)<<1 ) , WRITE_ONLY);
			jtag_arm_chain1_sysspeed_restart();

			/* STM R0,{R1-Rn} */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_STMIA(0, ((1L<<thisloop)-1)<<1 ) , WRITE_ONLY); 
		}
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);
		
		if (buf==NULL) 
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"%08x: ",address);
		
		/* Execute STM R1,{R1-Rn}: */
		for (i=0; i<thisloop; i++)
		{
			uint32_t value;
			
			value = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE); 
				
			if (buf==NULL) 	
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"%08x  ", value);
			else
				*buf++ = value;
		}
		if (buf==NULL) 
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"\n");
			address += 4*thisloop;		/* Go for next memory address */
		}
		howmanywords -= thisloop; /*remainder*/
	}
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED, NULL, ARM_NOP, WRITE_ONLY);

	return;
}


/*
 * 32 Bit Word Read
 * CPU must be in debug state 
 */
uint32_t jtag_arm_ReadWord(uint32_t address)
{ 
	uint32_t ret_val;
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 0;
	}
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	/* check for word alignment */
	if(address & 0x3)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ReadWord 0x%8X not align\t",address);
		address &= 0xffffFFFC;
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"using 0x%8X instead\n",address);
	}
	/* Scan in instructions in the data bus */ 
	/* Load R0 with the address to read: */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,      WRITE_ONLY);	/* LDR R0,PC */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,           WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,           WRITE_ONLY);
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,           WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,           WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDMIA(0,0x02), WRITE_ONLY);	/* LD R1,[R0] */
	jtag_arm_chain1_sysspeed_restart();

	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STMIA(0,0x02) , WRITE_ONLY);	/* STM R0,{R1} */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,            WRITE_ONLY);
	ret_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R1}: */
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"read WORD %08x: %08x  \n",address, ret_val);
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,            WRITE_ONLY);

	return ret_val;
}

/*
 * 16 Bit Halfword Read
 */
uint32_t jtag_arm_ReadHalfword(uint32_t address)
{ 
	uint32_t ret_val;
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 0;
	}
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	/* check for half word alignment */
	if(address & 0x1)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"ReadHalfword 0x%8X not align\t",address);
		address &= 0xffffFFFE;
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"using 0x%8X instead\n",address);
	}
	/* Load R0 with the address to read: */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,  WRITE_ONLY);	/* LDR R0,PC */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,       WRITE_ONLY);
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,       WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDRH(0,1), WRITE_ONLY);	/* LDH R1,[R0] */
	jtag_arm_chain1_sysspeed_restart();
	
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STMIA(0,0x02), WRITE_ONLY);	/* STM R0,{R1} */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);
	ret_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R1}: */
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"read HALFWORD %08x: %04x  \n",address, ret_val);
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,       WRITE_ONLY);

	return ret_val;
}

/*
 * 8 Bit Byte Read
 */
uint32_t jtag_arm_ReadByte(uint32_t address)
{ 
	uint32_t ret_val;
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return 0;
	}
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	/* Load R0 with the address to read: */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,     WRITE_ONLY);	/* LDR R0,PC */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,          WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,          WRITE_ONLY);
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,          WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,          WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDRB(0,1),    WRITE_ONLY);	/* LDRB R1,[R0] */
	jtag_arm_chain1_sysspeed_restart();

	jtag_arm7_mov_chain1_data(DEBUG_SPEED        ,  NULL, ARM_STMDA(0,0x02),WRITE_ONLY);	/* STM R0,{R1} */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED ,  NULL, ARM_NOP,          WRITE_ONLY);
	ret_val = jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP, READ_WRITE);	/* Execute STM R0,{R1}: */
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"read WORD %08x: %08x  \n",address, ret_val);
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED ,  NULL, ARM_NOP,          WRITE_ONLY);

	return ret_val;
}


/*
 *  32-bit memory write 
 */
void jtag_arm_WriteWord(uint32_t address, uint32_t value)
{
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"write 0x%0lx -> 0x%08lx\n",value,address);
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDMIA(0,3), WRITE_ONLY);	/* load R0 and R1 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,        WRITE_ONLY);	/* address -> R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, value,          WRITE_ONLY);	/* value   -> R1 */
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STMDA(0,2), WRITE_ONLY);	/* R1 -> [R0] */

	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,          WRITE_ONLY);
	return;
}

/*
 *  16-bit memory write 
 */
void jtag_arm_WriteHalfword(uint32_t address, uint16_t value)
{
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"write 0x%0lx -> 0x%08lx\n",value,address);
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDMIA(0,3), WRITE_ONLY);	/* load R0 and R1 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,        WRITE_ONLY);	/* address -> R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, value,          WRITE_ONLY);	/* value   -> R1 */
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STRH(0,1),  WRITE_ONLY);	/* R1 -> [R0] --> for halfwords */

	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,          WRITE_ONLY);
	return;
}

/*
 *  8-bit memory write 
 */
void jtag_arm_WriteByte(uint32_t address, uint16_t value)
{
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}

	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"write 0x%0lx -> 0x%08lx\n",value,address);
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDMIA(0,3), WRITE_ONLY);	/* load R0 and R1 */ 
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,        WRITE_ONLY);	/* address -> R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, value,          WRITE_ONLY);	/* value   -> R1 */
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,        WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STRB(0,1),  WRITE_ONLY);	/* R1 -> [R0] --> for Byte */

	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);
	return;
}

/*
 * Write back word memory from the given buffer.
 */
void jtag_arm_WriteMemoryBuf(uint32_t address, int howmanywords, uint32_t *buf)
{
	int i;
	int cnt;
	
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"WriteMemoryBuf 0x%0lx count=%d\n",address,howmanywords);
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}

	while (howmanywords > 0)
	{  
		cnt=(howmanywords>14) ? 15 :  howmanywords + 1; /* including addr at R0 */

		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LDMIA(0,(1L<<cnt)-1) , WRITE_ONLY);	/* load R0,R1,... except R13..R15 */
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address, WRITE_ONLY);          /* R0 <- addr */
	
		for(i=1; i<cnt; i++)	/* R1..Rn */
			jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, *buf++, WRITE_ONLY); 
	
		/* Prepare system speed accecss*/
		jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP, WRITE_ONLY);
		jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_STMIA(0,(( (1L<<(cnt-1))- 1 )<<1) ) , WRITE_ONLY);      /* R1..Rn -> [R0] */

		jtag_arm_chain1_sysspeed_restart();
		
		address += 4*(cnt-1);                         /* Go for next memory address */
		howmanywords -= cnt - 1;
	}
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,        WRITE_ONLY);

	return;
}

/*
 * Start execution of a program at a given address.
 */
void jtag_arm_RunProgram(uint32_t address)
{
	dbgPrintf(DBG_LEVEL_JTAG_ARM_LOW,"Run program at 0x%08x\n",address);
	
	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr. */
		scan_mode = INTEST;
	}
	/* write CPSR - as ARM System MODE - disabe IRQ and FIQ  */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,    WRITE_ONLY);	/* LDR R0,PC */  
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_MSR_CPSR_R0, WRITE_ONLY);	/* MSR CPSR, R0 */
 	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_NOP,         WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, 0xdf ,           WRITE_ONLY);	/* cpsr val (= 0xdf) -> R0*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,         WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_LD_R0_PC,    WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,         WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, address,         WRITE_ONLY);	/* address -> R0 */
	/* Prepare system speed accecss*/
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,         WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED       , NULL, ARM_NOP,         WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED        , NULL, ARM_MOV(15,0),   WRITE_ONLY); 	/* MOV pc,r0 */
	/* Execute system speed access */
	jtag_send_instr(instr_restart);
	jtag_eos();
	CPU_ext.prev_CPSR = 0;
	scan_mode = RESTART;
	return;
}

/*
 * check if CP15 is present and if so read out the ARM Sysinfo (and dump this to screen).
 */
void jtag_arm_ReadCP15(void)
{
	/* Well, nither ARM7TDMI nor ARM7TDMI-S did have the CP15 */
#if 0
	int i, ok = 0;
	uint32_t val;
	uint32_t sav_cpsr;
		
	if (ice_state.is_debugrequest == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"must be in debug state (driven by EICE-RT)\n");
		return;
	}
	
	if( arm_info.has_stepbug && arm_info.core_number == 7 ) // the test did not work with soft cores
	{
		arm_info.has_cp15 = 0;
		return;
	}

	sav_cpsr = CPU.CPSR;

	if(scan_chain != 1 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(1);		/* Select Debug Scan chain */
		jtag_send_instr(instr_intest);
		scan_mode = INTEST;
	}
	
	/* write CPSR - as ARM System MODE - disabe IRQ and FIQ  */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_LD_R0_PC,                WRITE_ONLY);	/* LDR R0,PC */  
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_MSR_CPSR_R0,             WRITE_ONLY);	/* MSR CPSR, R0 */
 	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,                     WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, 0xdf ,                       WRITE_ONLY);	/* cpsr val */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_NOP,                     WRITE_ONLY); /*prefetch*/
	jtag_eos(); // repeat instr
	jtag_eos(); // repeat instr and execute MSR CPSR, R0
 	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,                     WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP,                     WRITE_ONLY);
	//jtag_arm7_mov_chain1_data(DEBUG_NO_EOS_SPEED , NULL, ARM_MRC_CP15_R1_C0_IDREG,    WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_MRC_CP15_R1_C0_IDREG,    WRITE_ONLY);
	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,                     WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP,                     WRITE_ONLY);
	//jtag_arm7_mov_chain1_data(DEBUG_NO_EOS_SPEED , NULL, ARM_MRC_CP15_R2_C0_CACHEREG, WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_MRC_CP15_R2_C0_CACHEREG, WRITE_ONLY);
	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,                     WRITE_ONLY);
	jtag_arm7_mov_chain1_data(SYSTEM_SPEED, NULL, ARM_NOP,                     WRITE_ONLY);
	//jtag_arm7_mov_chain1_data(DEBUG_NO_EOS_SPEED , NULL, ARM_MRC_CP15_R3_C0_CNTRREG,  WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_MRC_CP15_R3_C0_CNTRREG,  WRITE_ONLY);
	jtag_arm_chain1_sysspeed_restart();
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_MRS_R0_CPSR,             WRITE_ONLY); 
	jtag_arm7_mov_chain1_data(DEBUG_SPEED , NULL, ARM_STMIA(0,0xF),            WRITE_ONLY); /* R0-R3 */
	jtag_arm7_mov_chain1_data(DEBUG_REPEAT_SPEED , NULL, ARM_NOP,                     WRITE_ONLY);
	
	/* Read out registers while STM is being executed: */
	/* In THUMB state only regs. R1-R14 will be recovered now. */
	for (i=0; i<=3; i++)
	{
		val = jtag_arm7_mov_chain1_data(0, NULL, ARM_NOP, READ_WRITE); /* Copy register contents    */
		if(i==0)
		{
			dbgPrintf(DBG_LEVEL_JTAG_ARM,"0x%X\n", val );
			if( (val&0x1FL) == 0x1bL)
			{
				dbgPrintf(DBG_LEVEL_JTAG_ARM,"CP15 not present\n");
				arm_info.has_cp15 = 0;
				ok = 0;
			}
			else
			{
				dbgPrintf(DBG_LEVEL_JTAG_ARM,"CP15 present\n");
				arm_info.has_cp15 = 1;
				ok = 1;
			}
		}
		else if(ok)
			dbgPrintf(DBG_LEVEL_JTAG_ARM,"CP15 (%d) 0x%X\n",i-1,val);
	}
	CPU.CPSR = sav_cpsr; // restore original
	/* write CPSR - as ARM MODE  */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_LD_R0_PC,        WRITE_ONLY);	/* LDR R0,PC */  
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_MSR_CPSR_R0,     WRITE_ONLY);	/* MSR CPSR, R0 */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY);
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, CPU.CPSR & ~(0x20L), WRITE_ONLY);	/* make sure that we are still in ARM mode */
	jtag_arm7_mov_chain1_data(DEBUG_SPEED, NULL, ARM_NOP,             WRITE_ONLY);
	jtag_eos(); // repeat instr
	jtag_eos(); // repeat instr and execute MSR CPSR, R0
	return;
#else
	arm_info.has_cp15 = 0;
	return;
#endif
}



