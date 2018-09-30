/*
 * jt_arm.h
 *
 * ARM7TDMI Definitions
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


/* ARM THUMB opcodes */
#define THUMB_MOV_R0_PC		0x46784678
#define THUMB_STR(Rn,Rd)	(((0x6000L|((uint32_t)(Rn)<<3)|(uint32_t)(Rd))<<16)|(0x6000L|((uint32_t)(Rn)<<3)|(uint32_t)(Rd)))
#define THUMB_NOP		0x46C046C0
#define THUMB_BX_PC		0x47784778
#define THUMB_LD_R0_R2		0x68106810
#define THUMB_LD_R0_R1		0x68086808
#define THUMB_LD_R0_R0		0x68006800
#define THUMB_LD_R0_R7		0x68386838
#define THUMB_LD_R0_PC		0x48004800

/* ARM opcodes */
#define ARM_NOP			0xe1a00000
#define ARM_BX_R0		0xe12fff10
#define ARM_LD_R0_PC		0xe59f0000
#define ARM_LD_R1_PC		0xe59f1000
#define ARM_LD_R0_R0		0xe5900000
#define ARM_LD_R1_R0		0xe5901000
#define ARM_LD_LR_R0		0xe590E000
/* byte */
#define ARM_STRB(Rn, Rd)	(0xe5c00000|((uint32_t)(Rn)<<16)|((uint32_t)(Rd)<<12))
#define ARM_LDRB(Rn, Rd)	(0xe5d00000|((uint32_t)(Rn)<<16)|((uint32_t)(Rd)<<12))
/* half-word */
#define ARM_STRH(Rn,Rd)		(0xe1c000b0|((uint32_t)(Rn)<<16)|((uint32_t)(Rd)<<12))
#define ARM_LDRH(Rn,Rd)		(0xe1d000b0|((uint32_t)(Rn)<<16)|((uint32_t)(Rd)<<12))

	/* LDM,STM */
#define ARM_STR(Rn,Rd)		(0xe5800000|((uint32_t)(Rn)<<16)|((uint32_t)(Rd)<<12))
#define ARM_STMDB(Rn,Rl)	(0xe9000000|((uint32_t)(Rn)<<16)|(Rl))	/* post-decr. store */
#define ARM_STMDA(Rn,Rl)	(0xe8000000|((uint32_t)(Rn)<<16)|(Rl))	/* post-decr. store */
#define ARM_STMDA_S(Rn,Rl)	(0xe8400000|((uint32_t)(Rn)<<16)|(Rl))	/* post-decr. store */
#define ARM_STMIA(Rn,Rl)	(0xe8800000|((uint32_t)(Rn)<<16)|(Rl))  /* post-incr. store */
#define ARM_STMIA_BANK(Rn,Rl)	(0xe8c00000|((uint32_t)(Rn)<<16)|(Rl))  /* post-incr. store */
#define ARM_LDMIA(Rn,Rl)	(0xe8900000|((uint32_t)(Rn)<<16)|(Rl))  /* post-incr. load */
#define ARM_LDMIA_UPDATE(Rn,Rl)	(0xe8b00000|((uint32_t)(Rn)<<16)|(Rl))  /* post-incr. load update Rn*/

#define ARM_LD_C(R,C)		(0xe3a00000|((uint32_t)(R)<<12)|(C))
#define ARM_MOV(Rd,Rs)		(0xe1a00000|((uint32_t)(Rd)<<12)|(uint32_t)(Rs))

#define ARM_MRS_R0_CPSR		0xe10f0000
#define ARM_MRS_R0_SPSR		0xe14f0000
#define ARM_MSR_CPSR_R0		0xe129f000 /* c and f since I have no knowlage what is livving in x and s */


#define ARM_MRC_CP15_R1_C0_IDREG	0xee101f10
#define ARM_MRC_CP15_R2_C0_CACHEREG	0xee102f30
#define ARM_MRC_CP15_R3_C0_CNTRREG	0xee103f11

/* instr. that store data to memory*/
#define ARM_STORE_MSK			0x0c100000 /* 5.2 - addr mode 2 */
#define ARM_STORE			0x04000000
#define ARM_STORE_MISC_MSK		0x0e100090 /* 5.3 - addr mode 3 */
#define ARM_STORE_MISC			0x00000090 /* including swap; double; half; byte - signed ... */
#define ARM_STORE_MULTIPLE_MSK		0x0e100000 /* 5.4 - addr mode 4 */
#define ARM_STORE_MULTIPLE		0x08000000
#define ARM_STORE_COPROCESSOR_REG_MSK	0x0e100000 /* 5.5 - addr mode 5 */
#define ARM_STORE_COPROCESSOR_REG	0x0c000000

#define THUMB_STORE_A_MSK		0xf800
#define THUMB_STORE_A_1			0x6000
#define THUMB_STORE_A_3			0x9000
#define THUMB_STORE_A_BYTE_1		0x7000
#define THUMB_STORE_A_HALF_1		0x8000
#define THUMB_STORE_A_MULTIPLE		0xc000
#define THUMB_STORE_B_MSK		0xfe00
#define THUMB_STORE_B_2			0x5000
#define THUMB_STORE_B_BYTE_2		0x5400
#define THUMB_STORE_B_HALF_2		0x5200
#define THUMB_STORE_B_PUSH		0xb400

/* thumb bl and blx instr. */
#define THUMB_BL_MSK			0xf800
#define THUMB_BL_OFFSET_MSK		0x07ff
#define THUMB_BL_FIRST_PART		0xf000
#define THUMB_BL_SECOND_PART		0xf800
#define THUMB_BLX_MSK			0xf801
#define THUMB_BLX_SECOND_PART		0xe801

/* chain 1*/
#define DEBUG_SPEED			0
#define SYSTEM_SPEED			1
#define DEBUG_REPEAT_SPEED		2
#define RESTART_SPEED			4
#define DEBUG_NO_EOS_SPEED		8

#define WRITE_ONLY	0
#define READ_WRITE	1

/* ICERT register */
#define ICERT_REG_DEBUG_CONTROL			0
#define ICERT_REG_DEBUG_STATUS			1
#define ICERT_REG_ABORT_STATUS			2
#define ICERT_REG_DCC_CONTROL			4
#define ICERT_REG_DCC_DATA			5
#define ICERT_REG_WATCHPOINT_0_ADDRESS		8
#define ICERT_REG_WATCHPOINT_0_ADDRMASK		9
#define ICERT_REG_WATCHPOINT_0_DATA		10
#define ICERT_REG_WATCHPOINT_0_DATAMASK		11
#define ICERT_REG_WATCHPOINT_0_CONTROL		12
#define ICERT_REG_WATCHPOINT_0_CONTROLMASK	13
#define ICERT_REG_WATCHPOINT_1_ADDRESS		16
#define ICERT_REG_WATCHPOINT_1_ADDRMASK		17
#define ICERT_REG_WATCHPOINT_1_DATA		18
#define ICERT_REG_WATCHPOINT_1_DATAMASK		19
#define ICERT_REG_WATCHPOINT_1_CONTROL		20
#define ICERT_REG_WATCHPOINT_1_CONTROLMASK	21

struct ice_state {
	int is_debugrequest;		/* User requested debug (not brkp.)*/
	int is_watchpoint;		/* Breakpoint=0, watchpoint=1  */
	int watchpoint0;		/* free = 0, HW = 1, SW = 2, WP = 4*/
	int watchpoint1;
	int is_step_mode;
	int high_speed_mclk;
	int ignore_stepbug;
};
extern struct ice_state ice_state;

struct arm_info {
	int	bigend;			/* endian 1 = big, 0 = little, 2 = not destinguishable(due to wrong test pattern)*/
	int	vendor_id;		/* jedec vendor id*/
	char	*vendor_string;		/* jedec string*/
	int	core_number;		/* 2 = OKI unknown, 7 = ARM7, 8 = ARM7 or ARM9, 9 = ARM9, 10 = ARM10, 11 = ARM11*/
	int	core_revision;		/* revision number of ARM core*/
	int	ice_revision;		/* revision number of Embedded ICE*/
	int	dd_type;		/* device description e.g with or without mmu*/
	char	*dd_string;		/* string of dev description */
	int	cap_type;		/* standard, E or J */
	char	*cap_string;		/* string '' */
	int	has_cp15;		/* 1 = has CP15, 0 = has not */
	int	has_thumb;
	int	has_stepbug;		/* this dose increeses the PC with extra steps*/
};
extern struct arm_info arm_info;

/*floting point register 96 bit to hold 80 bit IEEE value - requested by gdb */
struct fp_reg {
      uint32_t pos[3];
} ;

/*register set - requested by gdb */
struct reg_set {
	union Regs {
		uint32_t r[16];
		struct Name {
			uint32_t r0;	/* 32 bit r0 */
			uint32_t r1;	/* 32 bit r1 */
			uint32_t r2;	/* 32 bit r2 */
			uint32_t r3;	/* 32 bit r3 */
			uint32_t r4;	/* 32 bit r4 */
			uint32_t r5;	/* 32 bit r5 */
			uint32_t r6;	/* 32 bit r6 */
			uint32_t r7;	/* 32 bit r7 - Thumb Frame pointer */
			uint32_t r8;	/* 32 bit r8 */
			uint32_t r9;	/* 32 bit r9 */
			uint32_t r10;	/* 32 bit r10 */
			uint32_t r11;	/* 32 bit r11 - ARM Frame pointer */
			uint32_t r12;	/* 32 bit r12 */
			uint32_t sp;	/* 32 bit r13 - Stack pointer */
			uint32_t lr;	/* 32 bit r14 - link register (fnct. return address) */
			uint32_t pc;	/* 32 bit r15 - program counter */
		} name;
	} regs;
	struct fp_reg f[8];		/* 8 x 96 bit f0 .. f7 */
	uint32_t fps;			/* 32 bit fps - floating point status register */
	uint32_t CPSR;			/* 32 bit cpsr - current program status */
};

struct reg_set_ext {
	uint32_t prev_CPSR;		/* 32 bit cpsr - current program status of previous intruction*/
	uint32_t SPSR;			/* 32 bit spsr - saved program status */
	uint32_t sp_usr;		/* 32 bit r13 - Stack pointer */
	uint32_t lr_usr;		/* 32 bit r14 - link register (fnct. return address) */
};

extern struct reg_set CPU;
extern struct ice_state ice_state;

/*
 * register positions within the (raw-gdb) register file
 */
#define GDB_REG_POS_THUMB_FP	7
#define GDB_REG_POS_ARM_FP	11
#define GDB_REG_POS_SP		13
#define GDB_REG_POS_LR		14
#define GDB_REG_POS_PC		15
#define GDB_REG_POS_PS		25

/*
 *  These need to match the same in gdb/include/gdb/signals.h
 */

#define TARGET_SIGNAL_0		0
#define TARGET_SIGNAL_INT	2
#define TARGET_SIGNAL_QUIT	3
#define TARGET_SIGNAL_ILL	4
#define TARGET_SIGNAL_TRAP	5
#define TARGET_SIGNAL_ABRT	6
#define TARGET_SIGNAL_EMT	7
#define TARGET_SIGNAL_FPE	8
#define TARGET_SIGNAL_KILL	9
#define TARGET_SIGNAL_BUS	10
#define TARGET_SIGNAL_SEGV	11
#define TARGET_SIGNAL_TSTP	18

/*
 * scan chain 0 (113 bits) - Macrocell Scan Test
 * scan chain 1 ( 33 bits) - Debug
 * scan chain 2 ( 38 bits) - Embedded ICE logic
 */
extern int scan_chain;

enum scan_mode {NON , INTEST, EXTEST, SYSTEM, RESTART};
extern enum scan_mode scan_mode;

/*jt_arm_support.c*/
extern void jtag_supp_int2bitstr_MSB_First(int len, long val, char *str);
extern void jtag_supp_int2bitstr_LSB_First(int len, long val, char *str);
extern void jtag_supp_bitstr2int_LSB_First(int len, long *val, char *str);
extern void jtag_supp_bitstr2int_MSB_First(int len, long *val, char *str);

extern int jtag_arm_verify(void);
extern void jtag_arm_set_chain(int chain);
extern unsigned int jtag_arm7_mov_chain1_data(int break_bit, int *old_break, int data, int read_prev_data);
extern unsigned int jtag_arm9_mov_chain1_data(int sysspeed_bit, int *old_sysspeed, int data_val, int cmd_val, int read_prev_data);
extern void jtag_arm_chain1_sysspeed_restart(void);

extern int is_arm_store_instr(uint32_t step_instr);
extern int is_thumb_store_instr(uint32_t step_instr);
extern int jt_arm_condition_pass (uint32_t step_instr);
extern int jt_arm_instr_modifys_PC(uint32_t step_instr);
extern int jt_thumb_instr_modifys_PC(uint32_t step_instr);
extern int jt_arm_instr_access_mem(uint32_t step_instr);
extern int jt_thumb_instr_access_mem(uint32_t step_instr);
extern int jt_arm_instr_bx_workaround(uint32_t step_instr);
extern int jt_thumb_instr_bx_workaround(uint32_t step_instr);

/*jt_arm_iceRT.c*/
extern unsigned int jtag_arm_IceRT_RegRead(int nreg);
extern unsigned int jtag_arm_IceRT_RegRead_Once(int nreg);
extern void jtag_arm_IceRT_RegWrite(int nreg, unsigned int regdata);
extern unsigned int jtag_arm_IceRT_RegWrite_getPrevData(int nreg, unsigned int regdata);
extern void jtag_arm_ShowAllIceRT_Regs(void);
extern unsigned int jtag_arm_PollDbgState(void);
extern int jtag_arm_StopRunningProgram(void);
extern int jtag_arm_IceRT_version(void);
extern void jtag_arm_PutAnyBreakPoint(void);
extern void jtag_arm_PutHWBreakPoint0(uint32_t addr);
extern void jtag_arm_PutHWBreakPoint1(uint32_t addr);
extern void jtag_arm_PutSWBreakPoint0(void);
extern void jtag_arm_PutSWBreakPoint1(void);
extern void jtag_arm_ClearAnyBreakPoint(void);
extern void jtag_arm_ClearPC(void);
extern void jtag_arm_disable_Intr(void);
extern void jtag_arm_enable_Intr(void);
extern void jtag_arm_Mointor2DebugMode(void);
extern void jtag_arm_enterMonitorMode(void);

/*jt_arm.c*/
extern void jtag_arm_ReadCpuRegs(int reset_inst_counter);
extern void jtag_arm_DumpCPUregs(void);
extern void jtag_arm_WriteCpuRegs(void);
extern void jtag_arm_PrepareExitDebug(void);
extern void jtag_arm_FinalExitDebug(void);
extern void jtag_arm_Step(uint32_t next_instr);
extern void jtag_arm_ReadWordMemory(uint32_t address, int howmanywords, uint32_t *buf);
extern uint32_t jtag_arm_ReadWord(uint32_t address);
extern uint32_t jtag_arm_ReadHalfword(uint32_t address);
extern uint32_t jtag_arm_ReadByte(uint32_t address);
extern void jtag_arm_WriteWord(uint32_t address, uint32_t value);
extern void jtag_arm_WriteHalfword(uint32_t address, uint16_t value);
extern void jtag_arm_WriteByte(uint32_t address, uint16_t value);
extern void jtag_arm_WriteMemoryBuf(uint32_t address, int howmanywords, uint32_t *buf);
extern void jtag_arm_RunProgram(uint32_t address);
extern void jtag_arm_ReadCP15(void);


