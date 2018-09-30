/*
 * arm_gdbstub_rcmd.c
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
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <ctype.h>
#include <sysexits.h>

#include "dbg_msg.h"
#include "jt_instr.h" 
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"


/************************************************************************/
struct SymbolInfo symbolMain = {0,0,0,0};

enum AsignToken {
	/* with '=' somewhere in string*/	
	TOKEN_INVAL,
	TOKEN_MEMMAP,			/*MemMap.MaxNum            = .. */
					/*MemMap.MaxEntrys[MapNum] = .. */
					/*MemMap[MapNum][Entry]    = MemType, BusSize, BaseAddress, AddressLength */
					/*                 MemType -> cache,flash,io,ram,sfa,workspace,unreal or delete*/
					/*                 BusSize -> 8,16 or 32*/
	TOKEN_CMDSEQ,			/*CmdSequence.MaxNum            = .. */
					/*CmdSequence.MaxEntrys[CmdNum] = .. */
					/*CmdSequence[CmdNum][Entry]    = BusAccess, Addr, Val*/
	TOKEN_CMDSEQ_BIND,		/*CmdSequence.Bind[CmdNum] = StartPeriphery */
					/*CmdSequence.Bind[CmdNum] = StopPeriphery */
	TOKEN_REGISTER,			/*Register[RegNum] = .. */
	TOKEN_INTERRUPT,		/*Interrupt	= .. */
					/*		0 -> disable Interrupt in step mode*/
					/*		1 -> enable " */
	TOKEN_MODE_MCLK,		/*modeMCLK = .. */
					/*		0 -> low speed MCLK*/
					/*		x -> linger factor to next debug mode command after a system speed command*/
	TOKEN_MODE_FLASH,		/*modeFLASH = .. */
					/*		0 -> low speed FLASH Download with completion check*/
					/*		1 -> low speed FLASH Download without completion check*/
					/*		2 -> faster speed FLASH Download with completion check*/
					/*		3 -> faster speed FLASH Download without completion check*/
	TOKEN_FORCE_INVAL_FLASH,	/*InvalFlashWhileEraseing = .. */
					/*		0 -> keep cached memory as is*/
					/*		1 -> invalidate cached memory*/
	TOKEN_FORCE_CHECKEVENINVALSEC,	/*ForceCheckInvalidSector = .. */
					/*		0 -> skip check of currently invalid cache flash memory */
					/*		1 -> check even invalidate cached flash memory (assuming all data is 0xFF)*/
	TOKEN_FAKE_CONTINUE,		/*FakeContinue = .. -> fake_continue_mode*/
	TOKEN_MODE_WORKSPACE,		/*modeWorkspace = ..*/
					/*		0 -> workspace only if defined in MemMap */
					/*		1 -> if not defined in MemMap reuse Top of RAM as workspace too*/
					/*		2 -> if not defined in MemMap reuse Bottom of RAM as workspace too*/
	TOKEN_FORCE_HW_BREAK,		/*forceHWBreak = ..*/
					/*		0 -> do not force*/
					/*		1 -> force setting Hardware beakpoint, even if the address is placed in RAM*/
	TOKEN_TEMP_MAIN_BREAK,		/*forceTempHWBreakAtMain = ..*/
					/*		0 -> do not force*/
					/*		1 -> force setting Hardware beakpoint at symbol main */
					/*		     This will be the only present breakpoint. */
					/*		     After hit, forceTempHWBreakAtMain = 0 and other breakpoints will be allowed.*/
	TOKEN_LPC_FREQUENCE,		/*LPCfrequence = .. -> CPU ckock frequency in kHz used by Philips LPC 21xx and LPC 22xx types */
	TOKEN_SAM7_FREQUENCE,		/*SAM7frequence = .. -> CPU ckock frequency in kHz */
	TOKEN_DBG_MASK,			/*DbgMask = .. */
	TOKEN_VERBOSE			/*verbose = .. */
					/*		0 -> be less verbose (in fake_continue_mode)*/
					/*		1 -> be more verbose (in fake_continue_mode)*/
};

	
enum FunctionToken {
	/* no '=' in string*/
	F_TOKEN_INVAL,
	F_TOKEN_MEMMAP,		/*MemMap*/
	F_TOKEN_CMDSEQ,		/*CmdSequence*/
	F_TOKEN_CMDSEQ_BIND,	/*CmdSequence.Bind*/
	F_TOKEN_CMD,		/*Cmd[..]*/
				/*		sequenceNumber*/
	F_TOKEN_ACTIVATE,	/*UseMemMap[..]*/
				/*		mapNumber*/
	F_TOKEN_DEACTIVATE,	/*NoMemMap*/
	F_TOKEN_RESTART,	/*Restart*/
	F_TOKEN_DELAY,		/*Delay[..]*/
	F_TOKEN_PROG_FL,	/*ProgramFlash .. */
				/*		baseaddress numberOfSectors*/
	F_TOKEN_VERIFY_FL,	/*VerifyFlash .. */
				/*		baseaddress numberOfSectors*/
	F_TOKEN_CHKSUM_TEST_FL,	/*CheckFlash .. */
				/*		baseaddress numberOfSectors*/
	F_TOKEN_ERASE_FL,	/*EraseFlash .. */
				/*		baseaddress numberOfSectors*/
	F_TOKEN_FL_STATE,	/*FlashState*/
	F_FORCE_INVAL_FLASH,	/*InvalFlashWhileEraseing*/
	F_TOKEN_SYSINFO,	/*SysInfo*/
	F_TOKEN_INTERRUPT,	/*Interrupt*/
	F_TOKEN_MODE_MCLK,	/*modeMCLK*/
	F_TOKEN_MODE_FLASH,	/*modeFLASH*/
	F_TOKEN_MODE_WORKSPACE,	/*modeWorkspace*/
	F_TOKEN_FORCE_HW_BREAK,	/*forceHWBreak*/
	F_TOKEN_TEMP_MAIN_BREAK,/*forceTempHWBreakAtMain*/
	F_TOKEN_LPC_FREQUENCE,	/*LPCfrequence*/
	F_TOKEN_LPC_CHKSUM,	/*LpcGenVectorCheckSum*/
	F_TOKEN_SAM7_FREQUENCE,	/*SAM7frequence*/
	F_TOKEN_DBG_MASK,	/*DbgMask*/
	F_TOKEN_VERBOSE		/*verbose*/
};



static int gdb_monitor_Rcmd(int fd, char *query_str, char *response_str, struct reg_set *raw_regs)
{
	char *sep;
	char *var, *val = NULL;
	int   i;
	int   req_argc, req_arg[2], req_val[4];
	enum FunctionToken f_req_type;
	enum AsignToken    a_req_type;
	struct flashCBContext fcontext;
	struct memMapCBContext mcontext;
	int currMemMapHeadCnt, currEntryCnt;
	struct cmdCallbackEntry *cbEntry;
	struct memMap * mm;
	int diffTime;
	enum CmdSequenceFlag flagOfCmdSequence = CMDSEQUFLAG_WRITE;

	/*search for variable = value string*/
	var = query_str;
	while(*var != 0 && (*var == ' ' || *var == '\t' ))
		var++;
	/*find separator '=' */
	sep = var;
	while(*sep != 0 && *sep != '=')
		sep++;
	if(*sep != 0) // found '='
	{
		*sep = 0;
		val = sep + 1;
		/*clear pending space char*/
		while(sep != var && (*sep == ' ' || *sep =='\t'))
			*sep-- = 0;
		
		/*find start of value string*/
		while(*val != 0 && (*val == ' ' || *val == '\t' ))
			val++;
		if(*val != 0 && *var != 0)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"var = %s ; val = %s\n", var, val);

			/*First we convert the var strings into a more (or less) usable format*/
			if(!strncasecmp(var,"MemMap",sizeof("MemMa")))
			{
				var += sizeof("MemMa"); // sizeof includes the EOS,but we don't
				a_req_type = TOKEN_MEMMAP;
			}
			else if(!strncasecmp(var,"CmdSequence",sizeof("CmdSequenc")))
			{
				var += sizeof("CmdSequenc"); // sizeof includes the EOS,but we don't
				a_req_type = TOKEN_CMDSEQ;
			}
			else if(!strncasecmp(var,"Register",sizeof("Registe")))
			{
				var += sizeof("Registe"); // sizeof includes the EOS,but we don't
				a_req_type = TOKEN_REGISTER;
			}
			else if(!strncasecmp(var,"Interrupt",sizeof("Interrup")))
			{
				var += sizeof("Interrup");
				a_req_type = TOKEN_INTERRUPT;
			}
			else if(!strncasecmp(var,"modeMCLK",sizeof("modeMCL")))
			{
				var += sizeof("modeMCL");
				a_req_type = TOKEN_MODE_MCLK;
			}
			else if(!strncasecmp(var,"modeFLASH",sizeof("modeFLAS")))
			{
				var += sizeof("modeFLAS");
				a_req_type = TOKEN_MODE_FLASH;
			}
			else if(!strncasecmp(var,"InvalFlashWhileEraseing",sizeof("InvalFlashWhileErasein")))
			{
				var += sizeof("InvalFlashWhileErasein");
				a_req_type = TOKEN_FORCE_INVAL_FLASH;
			}
			else if(!strncasecmp(var,"FakeContinue",sizeof("FakeContinu")))
			{
				var += sizeof("FakeContinu");
				a_req_type = TOKEN_FAKE_CONTINUE;
			}
			else if(!strncasecmp(var,"modeWorkspace",sizeof("modeWorkspac")))
			{
				var += sizeof("modeWorkspac");
				a_req_type = TOKEN_MODE_WORKSPACE;
			}
			else if(!strncasecmp(var,"ForceCheckInvalidSe",sizeof("ForceCheckInvalidS")))
			{
				var += sizeof("ForceCheckInvalidS");
				a_req_type = TOKEN_FORCE_CHECKEVENINVALSEC;
			}
			else if(!strncasecmp(var,"forceHWBreak",sizeof("forceHWBrea")))
			{
				var += sizeof("forceHWBrea");
				a_req_type = TOKEN_FORCE_HW_BREAK;
			}
			else if(!strncasecmp(var,"forceTempHWBreakAtMain",sizeof("forceTempHWBreakAtMai")))
			{
				var += sizeof("forceTempHWBreakAtMai");
				a_req_type = TOKEN_TEMP_MAIN_BREAK;
			}
			else if(!strncasecmp(var,"forceTmpHWBreakAtMain",sizeof("forceTmpHWBreakAtMai")))
			{
				var += sizeof("forceTmpHWBreakAtMai");
				a_req_type = TOKEN_TEMP_MAIN_BREAK;
			}
			else if(!strncasecmp(var,"LPCfrequence",sizeof("LPCfrequenc")))
			{
				var += sizeof("LPCfrequenc");
				a_req_type = TOKEN_LPC_FREQUENCE;
			}
			else if(!strncasecmp(var,"SAM7frequence",sizeof("SAM7frequenc")))
			{
				var += sizeof("SAM7frequenc");
				a_req_type = TOKEN_SAM7_FREQUENCE;
			}
			else if(!strncasecmp(var,"DbgMask",sizeof("DbgMas")))
			{
				var += sizeof("DbgMas");
				a_req_type = TOKEN_DBG_MASK;
			}
			else if(!strncasecmp(var,"verbose",sizeof("verbos")))
			{
				var += sizeof("verbos");
				a_req_type = TOKEN_VERBOSE;
			}
			else
				return -1; // Inval

			/*number of numeric arguments (before '=' sign)*/
			if(a_req_type == TOKEN_MEMMAP||a_req_type == TOKEN_CMDSEQ)
			{
				if(*var++ == 0 || *var == 0)
					return -1; // Inval
				if(!strncasecmp(var,"MaxNum",sizeof("MaxNu")))
					req_argc = 0;
				else if(!strncasecmp(var,"MaxEntrys",sizeof("MaxEntry")))
					req_argc = 1;
				else if(a_req_type == TOKEN_CMDSEQ && !strncasecmp(var,"Bind",sizeof("Bin")))
				{
					a_req_type = TOKEN_CMDSEQ_BIND;
					req_argc = 1;
				}
				else
					req_argc = 2;
			}
			else if ( a_req_type == TOKEN_REGISTER)
				req_argc = 1;
			else
				req_argc = 0;
			
			for(i=0 ;i<req_argc ; i++)
			{
				/*find first Number*/
				while( *var < '0' || *var > '9' )
				{
					if(*var == 0)
						return -1;
					if( *var == '-' && var[1] != 0 && var[1] >= '0' && var[1] <= '9')
						break;
					var++;
				}
				if(var[0] == '0' && (var[1] == 'x' || var[1] == 'X'))
					req_arg[i] = strtoul(var, &sep, 16);
				else if(var[0] == '0')
					req_arg[i] = strtoul(var, &sep, 8);
				else if(var[0] == '-')
					req_arg[i] = strtol(var ,&sep ,10);
				else
					req_arg[i] = strtoul(var ,&sep ,10);
				var = sep;
			}
			/*Then we can convert the val strings*/
			if (a_req_type == TOKEN_CMDSEQ_BIND)
			{
				/*find first letter*/
				while( !((*val >= 'a' && *val <= 'z') || (*val >= 'A' && *val <= 'Z')) )
				{
					if(*val == 0)
						return -1;
					val++;
				}
				if(!strncasecmp(val,"StartPeriph",sizeof("StartPerip")))
					req_val[0] = CMDSEQU_START_PERIPH;
				else if(!strncasecmp(val,"StopPeriph",sizeof("StopPerip")))
					req_val[0] = CMDSEQU_STOP_PERIPH;
				else if(!strncasecmp(val,"ResetCPU",sizeof("ResetCP")))
					req_val[0] = CMDSEQU_RESET_CPU;
				else
					req_val[0] = CMDSEQU_UNKOWN;
			}
			else if(req_argc < 2) // there is only one value
			{
				/*find first Number*/
				while( *val < '0' || *val > '9' )
				{
					if(*val == 0)
						return -1;
					if( *val == '-' && val[1] != 0 && val[1] >= '0' && val[1] <= '9')
						break;
					val++;
				}
				if(val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
					req_val[0] = strtoul(val, &sep, 16);
				else if(val[0] == '0')
					req_val[0] = strtoul(val, &sep, 8);
				else if(val[0] == '-')
					req_val[0] = strtol(val ,&sep ,10);
				else
					req_val[0] = strtoul(val ,&sep ,10);

			}
			else if (a_req_type == TOKEN_MEMMAP) // req_argc == 2 -> MemMap[][] = ...
			{
				//MemType 
				switch(*val++)
				{
				case 'a': /*Application Flash*/
				case 'A':
					req_val[0] = MMAP_T_APPLICATION_FLASH;
					break;
				case 'c': /*chache area (niether read nor write)*/
				case 'C':
					req_val[0] = MMAP_T_CACHE;
					break;
				case 'd': /*delete entry*/
				case 'D':
					req_val[0] = MMAP_T_UNUSED;
					break;
				case 'f': /*Flash*/
				case 'F':
					req_val[0] = MMAP_T_FLASH;
					break;
				case 'i': /*IO (non cacheable area)*/
				case 'I':
					req_val[0] = MMAP_T_IO;
					break;
				case 'r': /*RAM or ROM*/
				case 'R':
					if(*val == 'a' || *val == 'A') /*RAM*/
					{
						req_val[0] = MMAP_T_RAM;
						val++;
					}
					else if(*val == 'o' || *val == 'O') /*ROM*/
					{
						req_val[0] = MMAP_T_ROM;
						val++;
					}
					break;
				case 's': /*SFA - Special Function Area*/
				case 'S':
					req_val[0] = MMAP_T_SFA;
					break;
				case 'u': /*unreal entry, e.g. for GDB frame data stuff*/
				case 'U':
					req_val[0] = MMAP_T_UNREAL;
					break;
				case 'w': /*Work space - for the debugger itselfs*/
				case 'W':
					req_val[0] = MMAP_T_WORKSPACE;
					break;
				default: /*unknown entry*/
					req_val[0] = MMAP_T_UNUSED;
					val--; //go back
				}

				//BusSize in Bits
				/*find first Number*/
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					val++;
				}
				req_val[1] = strtol(val, &sep, 10);
				val = sep;
				if(   req_val[1] == 8 
				   || req_val[1] == 16 
				   || req_val[1] == 32 
				   || ((    req_val[0] == MMAP_T_APPLICATION_FLASH
				        || req_val[0] == MMAP_T_FLASH
				      ) 
				      && 
				      (    req_val[1] == 164 // flash ST 
				        || req_val[1] == 132 // flash Atmel
				        || req_val[1] == 128 // flash Philips
				      ))
				   )
					; // OK
				else
					return -1; // wrong Bus size

				if(*val == 0)
					return -1; // too short
				//BaseAddress 
				/*find first Number*/
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					val++;
				}
				if(val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
					req_val[2] = strtoul(val, &sep, 16);
				else if(val[0] == '0')
					req_val[2] = strtoul(val, &sep, 8);
				else
					req_val[2] = strtoul(val, &sep, 10);
				val = sep;
				if(*val == 0)
					return -1; // too short
				//AddressLength
				/*find first Number*/
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					val++;
				}
				if(val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
					req_val[3] = strtol(val, &sep, 16);
				else if(val[0] == '0')
					req_val[3] = strtol(val, &sep, 8);
				else
					req_val[3] = strtol(val, &sep, 10);
				val = sep;
				// skip space
				while( *val != 0 && (*val == ' '||*val == '\t'))
					val++;
				if(*val == 'K' || *val == 'k')  // multiply with K
					req_val[3] *= 1024;
				else if(*val == 'M')		// multiply with M
					req_val[3] *= 1024*1024;
			}
			else if(a_req_type == TOKEN_CMDSEQ)	// req_argc == 2 -> CmdSequence[][] = ...
			{
				//BusAccess
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					val++;
				}
				req_val[0] = strtol(val, &sep, 10);
				val = sep;
				if(req_val[0] == 8 || req_val[0] == 16 || req_val[0] == 32)
					; // OK
				else
					return -1; // wrong Bus size
				//Addr
				/*find first Number*/
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					val++;
				}
				if(val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
					req_val[1] = strtoul(val, &sep, 16);
				else if(val[0] == '0')
					req_val[1] = strtoul(val, &sep, 8);
				else
					req_val[1] = strtoul(val, &sep, 10);
				val = sep;
				if(*val == 0)
					return -1; // too short
				//Val
				/*find first Number or string "read" */
				flagOfCmdSequence = CMDSEQUFLAG_WRITE;
				while(*val < '0' || *val > '9')
				{
					if(*val == 0)
						return -1;
					if(!strncasecmp(val,"read",sizeof("rea")))
					{
						flagOfCmdSequence = CMDSEQUFLAG_READ;
						break;
					}
					val++;
				}
				if(flagOfCmdSequence == CMDSEQUFLAG_WRITE)
					req_val[2] = strtoul(val, NULL, 16);
				else
					req_val[2] = 0;
			}
			else
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"parse error -- should not happen\n");
				exit(0);
			}

			/*now put it all together*/
			switch(a_req_type)
			{
			case TOKEN_MEMMAP: /*MemMap*/
				switch(req_argc & 0xF)
				{
				case 0: /*MemMap.MaxNum            = .. */
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"MaxNum of MemMap -> %d\n",req_val[0]);
					if(AllocMemMaps(req_val[0]) != 0)
						return -1;
					break;
				case 1: /*MemMap.MaxEntrys[MapNum] = .. */
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"MaxEntrys of MemMap[%d] -> %d\n",req_arg[0],req_val[0]);
					if(AllocateMemMapEntrys(searchMemMapHead(req_arg[0]), req_val[0]) != 0)
						return -1;
					break;
				case 2: /*MemMap[MapNum][Entry]    = MemType, BusSize, BaseAddress, AddressLength */
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"MemMap[%d][%d] -> %d %d 0x%X %d\n",req_arg[0],req_arg[1]
						,req_val[0], req_val[1], req_val[2], req_val[3]);
					if(updateMemMap(searchMemMapHead(req_arg[0]) ,req_arg[1] 
							,req_val[0], req_val[1], req_val[2], req_val[3]	) != 0)
						return -1;
					break;
				default:
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"PARSE wrong req_argc %d\n",req_argc);
					return -1;
				}
				break;
			case TOKEN_CMDSEQ: /*CmdSequence*/
				switch(req_argc & 0xF)
				{
				case 0: /*CmdSequence.MaxNum            = .. */
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"MaxNum of seq -> %d\n",req_val[0]);
					if(AllocCmdSeqences(req_val[0]) != 0)
						return -1;
					break;
				case 1: /*CmdSequence.MaxEntrys[CmdNum] = .. */
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"MaxEntrys of seq[%d] -> %d\n",req_arg[0],req_val[0]);
					if(AllocCmdSequenceEntrys(searchCmdSequenceHead(req_arg[0]), req_val[0]) != 0)
						return -1;
					break;
				case 2: /*CmdSequence[CmdNum][Entry]    = BusAccess, Addr, Val*/
					if(flagOfCmdSequence == CMDSEQUFLAG_WRITE)
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(write)");
					else
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(read)");
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
						,"seq[%d][%d] -> %d 0x%X 0x%X\n",req_arg[0],req_arg[1]
						,req_val[0], req_val[1], req_val[2]);
					if(updateCmdSequence(searchCmdSequenceHead(req_arg[0]) ,req_arg[1]
							,req_val[0], req_val[1], req_val[2], flagOfCmdSequence) != 0)
						return -1;
					break;
				default:
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"wrong kind of CmdSequence\n");
					return -1;
				}
				break;
			case TOKEN_CMDSEQ_BIND: /*CmdSequence.Bind[CmdNum] = StartPeripherials*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"CmdSequence.Bind[%d] <- ",req_arg[0]);
				if(req_val[0] == CMDSEQU_START_PERIPH)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"StartPeriphery\n");
					cmdSequenceNumber_StartPeriphery = req_arg[0];
				}
				else if(req_val[0] == CMDSEQU_STOP_PERIPH)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"StopPeriphery\n");
					cmdSequenceNumber_StopPeriphery = req_arg[0];
				}
				else if(req_val[0] == CMDSEQU_RESET_CPU)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"ResetCPU\n");
					cmdSequenceNumber_ResetCPU = req_arg[0];
				}
				else

				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"unknown\n");
					return -1;
				}
				break;
			case TOKEN_REGISTER: /*Register[RegNum] = .. */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Register[%d] <- ",req_arg[0]);
				if ( req_arg[0] < 0 || req_arg[0] > 16 )
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"wrong\n");
					return -1;
				}
				if (req_arg[0] == 16)
					raw_regs->CPSR = req_val[0];
				else
					raw_regs->regs.r[req_arg[0]] = req_val[0];
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X\n",req_val[0]);
				break;
			case TOKEN_INTERRUPT: /*Interrupt*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Interrupt -> %d\n",req_val[0]);
				if(req_val[0])
					allow_intr_in_step_mode = 1;
				else
					allow_intr_in_step_mode = 0;
				break;
			case TOKEN_MODE_MCLK:	/*modeMCLK*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"modeMCLK -> %d\n",req_val[0]);
				if(req_val[0] <= 0)
					ice_state.high_speed_mclk = 0;
				else
					ice_state.high_speed_mclk = req_val[0];
				break;
			case TOKEN_MODE_FLASH: /*modeFLASH*/
				if((req_val[0] & 1) == 0) //wait
					download_wait = 1;
				else // no wait
					download_wait = 0;

				if(req_val[0] < 2) // low speed download
					download_faster = 0;
				else
					download_faster = 1;
				break;

			case TOKEN_FORCE_INVAL_FLASH:	/*InvalFlashWhileEraseing*/
				if(req_val[0] == 0)
					forceInvalMemWhileEraseingFlash = 0;
				else 
					forceInvalMemWhileEraseingFlash = 1;
				break;

			case TOKEN_FORCE_CHECKEVENINVALSEC:	/*ForceCheckInvalidSector*/
				if(req_val[0] == 0)
					forceCheckInvalidSector = 0;
				else 
					forceCheckInvalidSector = 1;
				break;

			case TOKEN_FAKE_CONTINUE: /*FakeContinue */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"FakeContinue -> %d\n",req_val[0]);
				if(req_val[0])
					fake_continue_mode = 1;
				else
					fake_continue_mode = 0;
				break;
			case TOKEN_MODE_WORKSPACE: /*modeWorkspace */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"modeWorkspace -> %d\n",req_val[0]);
				if(req_val[0] >= 0 && req_val[0] < 3 && req_val[0] != workspace_mode)
					changeWorkSpaceMode(req_val[0]);
				break;
			case TOKEN_FORCE_HW_BREAK: /*forceHWBreak*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"forceHWBreak -> %d\n",req_val[0]);
				if(req_val[0])
					force_hardware_breakpoint = 1;
				else
					force_hardware_breakpoint = 0;
				break;
			case TOKEN_TEMP_MAIN_BREAK: /*forceTempHWBreakAtMain*/
				if(symbolMain.state == SYM_PRESENT)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"forceTempHWBreakAtMain -> %d\n",req_val[0]);
					if(req_val[0])
						symbolMain.breakIsActive = 1;
					else
						symbolMain.breakIsActive = 0;
				}
				else
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"forceTempHWBreakAtMain -> 0 (unknown sybmbol main)\n");
					symbolMain.breakIsActive = 0;
					return -1;
				}
			case TOKEN_LPC_FREQUENCE: /*LPCfrequence*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"LPCfrequence -> %d\n",req_val[0]);
				LPC_frequence = req_val[0];
				break;
			case TOKEN_SAM7_FREQUENCE: /*SAM7frequence*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"SAM7frequence -> %d\n",req_val[0]);
				SAM7_frequence = req_val[0];
				break;
			case TOKEN_DBG_MASK: /*DbgMask = .. */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"DbgMask -> 0x%X\n",req_val[0]);
				dbg_msg_msk = req_val[0];
				break;
			case TOKEN_VERBOSE: /*verbose = .. */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"verbose -> 0x%X\n",req_val[0]);
				if(req_val[0] == 0)
					verbose = LESS_VERBOSE;
				else
					verbose = MORE_VERBOSE;
				break;
			case TOKEN_INVAL:
			default:
				return -1;
			}

			return 0;
		}
	}
	else // no separator '=' exists; so it is a plain command
	{
		if(*var != 0)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"var = %s\n", var);
			
			/*First we convert the var strings into a more (or less) usable format*/
			f_req_type = F_TOKEN_INVAL;
			if(!strncasecmp(var,"MemMap",sizeof("MemMa")))
			{
				f_req_type = F_TOKEN_MEMMAP;
				var += sizeof("MemMa"); 
			}
			else if(!strncasecmp(var,"CmdSequence",sizeof("CmdSequenc")))
			{
				f_req_type = F_TOKEN_CMDSEQ;
				var += sizeof("CmdSequenc");
				if(*var++ != 0 && *var != 0 && !strncasecmp(var,"Bind",sizeof("Bin")))
				{
					f_req_type = F_TOKEN_CMDSEQ_BIND;
					var += sizeof("Bin");
				}
			}
			else if(!strncasecmp(var,"Cmd",sizeof("Cm")))
			{
				f_req_type = F_TOKEN_CMD;
				var += sizeof("Cm");
				if(*var == 'S' || *var == 's') // maybe a typing error so better ignore
					f_req_type = F_TOKEN_INVAL;
			} 
			else if(!strncasecmp(var,"UseMemMap",sizeof("UseMemMa")))
			{
				f_req_type = F_TOKEN_ACTIVATE;
				var += sizeof("UseMemMa");
			}
			else if(!strncasecmp(var,"NoMemMap",sizeof("NoMemMa")))
			{
				f_req_type = F_TOKEN_DEACTIVATE;
				var += sizeof("NoMemMa");
			}
			else if(!strncasecmp(var,"Restart",sizeof("Restar")))
			{
				f_req_type = F_TOKEN_RESTART;
				var += sizeof("Restar");
			}
			else if(!strncasecmp(var,"Delay",sizeof("Dela")))
			{
				f_req_type = F_TOKEN_DELAY;
				var += sizeof("Dela");
			}
			else if(!strncasecmp(var,"ProgramFlash",sizeof("ProgramFlas")))
			{
				f_req_type = F_TOKEN_PROG_FL;
				var += sizeof("ProgramFlas");
			}
			else if(!strncasecmp(var,"VerifyFlash",sizeof("VerifyFlas")))
			{
				f_req_type = F_TOKEN_VERIFY_FL;
				var += sizeof("VerifyFlas");
			}else if(!strncasecmp(var,"CheckFlash",sizeof("CheckFlas")))
			{
				f_req_type = F_TOKEN_CHKSUM_TEST_FL;
				var += sizeof("CheckFlas");
			}
			else if(!strncasecmp(var,"EraseFlash",sizeof("EraseFlas")))
			{
				f_req_type = F_TOKEN_ERASE_FL;
				var += sizeof("EraseFlas");
			}
			else if(!strncasecmp(var,"FlashState",sizeof("FlashStat")))
			{
				f_req_type = F_TOKEN_FL_STATE;
				var += sizeof("FlashStat");
			}
			else if(!strncasecmp(var,"InvalFlashWhileEraseing",sizeof("InvalFlashWhileErasein")))
			{
				f_req_type = F_FORCE_INVAL_FLASH;
				var += sizeof("InvalFlashWhileErasein");
			}
			else if(!strncasecmp(var,"SysInfo",sizeof("SysInf")))
			{
				f_req_type = F_TOKEN_SYSINFO;
				var += sizeof("SysInf");
			}
			else if(!strncasecmp(var,"Interrupt",sizeof("Interrup")))
			{
				f_req_type = F_TOKEN_INTERRUPT;
				var += sizeof("Interrup");
			}
			else if(!strncasecmp(var,"modeMCLK",sizeof("modeMCL")))
			{
				f_req_type = F_TOKEN_MODE_MCLK;
				var += sizeof("modeMCL");
			}
			else if(!strncasecmp(var,"modeFLASH",sizeof("modeFLAS")))
			{
				f_req_type = F_TOKEN_MODE_FLASH;
				var += sizeof("modeFLAS");
			}
			else if(!strncasecmp(var,"modeWorkspace",sizeof("modeWorkspac")))
			{
				f_req_type = F_TOKEN_MODE_WORKSPACE;
				var += sizeof("modeWorkspac");
			}
			else if(!strncasecmp(var,"forceHWBreak",sizeof("forceHWBrea")))
			{
				f_req_type = F_TOKEN_FORCE_HW_BREAK;
				var += sizeof("forceHWBrea");
			}
			else if(!strncasecmp(var,"forceTempHWBreakAtMain",sizeof("forceTempHWBreakAtMai")))
			{
				var += sizeof("forceTempHWBreakAtMai");
				f_req_type = F_TOKEN_TEMP_MAIN_BREAK;
			}
			else if(!strncasecmp(var,"forceTmpHWBreakAtMain",sizeof("forceTmpHWBreakAtMai")))
			{
				var += sizeof("forceTmpHWBreakAtMai");
				f_req_type = F_TOKEN_TEMP_MAIN_BREAK;
			}
			else if(!strncasecmp(var,"LPCfrequence",sizeof("LPCfrequenc")))
			{
				f_req_type = F_TOKEN_LPC_FREQUENCE;
				var += sizeof("LPCfrequenc");
			}
			else if(!strncasecmp(var,"LpcGenVectorCheckSum",sizeof("LpcGenVectorCheckSu")))
			{
				f_req_type = F_TOKEN_LPC_CHKSUM;
				var += sizeof("LpcGenVectorCheckSu");
			}
			else if(!strncasecmp(var,"SAM7frequence",sizeof("SAM7frequenc")))
			{
				f_req_type = F_TOKEN_SAM7_FREQUENCE;
				var += sizeof("SAM7frequenc");
			}
			else if(!strncasecmp(var,"DbgMask",sizeof("DbgMas")))
			{
				f_req_type = F_TOKEN_DBG_MASK;
				var += sizeof("DbgMas");
			}
			else if(!strncasecmp(var,"verbose",sizeof("verbos")))
			{
				f_req_type = F_TOKEN_VERBOSE;
				var += sizeof("verbos");
			}
			
			/*collect arg 0 if any*/
			while(*var < '0' || *var > '9')
			{
				if(*var == 0)
				{
					/* cmd, useMemMap and Delay must have an argument */
					if( f_req_type == F_TOKEN_CMD 
					  || f_req_type == F_TOKEN_ACTIVATE 
					  || f_req_type == F_TOKEN_DELAY )
						return -1;
					else
						break;
				}
				var++;
			}
			if(*var != 0)
				req_arg[0] = strtol(var, &var, 0);
			else
				req_arg[0] = -1;
			
			if(*var != 0)
				req_arg[1] = strtol(var, NULL, 0);
			else
				req_arg[1] = -1;
			
			/*Now we start the requested action*/
			switch(f_req_type)
			{
			case F_TOKEN_INVAL: /*This type is not supported we will return with an error message*/
				break;
			case F_TOKEN_MEMMAP:	/* MemMap */
				if((memMapContainer.memStat.pending & ACTION_INPROCESS ) == ACTION_INPROCESS )
				{
					snprintf(response_str,BUFMAX/2,"pending Action\n");
					return sizeof("pending Action\n") - 1;
				}
				if(memMapContainer.numberOfMemMaps <= 0)
				{
					snprintf(response_str,BUFMAX/2,"no MemMaps defined\n");
					return sizeof("no MemMaps defined\n") - 1;
				}
				/*create context of callback*/
				bzero(&mcontext, sizeof(mcontext));
	
				mcontext.max_MemMap_number = memMapContainer.numberOfMemMaps;
				mcontext.curr_Map = memMapContainer.firstMap;

				mcontext.curr_memMap = mcontext.curr_Map->firstMapEntry;
				mcontext.max_MapEntry_number = mcontext.curr_Map->numberOfEntrys;
				if (mcontext.max_MapEntry_number > 0)
				{
					currEntryCnt = 1;  // start with first entry
					mm = mcontext.curr_memMap;
					if (mm->type == MMAP_T_FLASH || mm->type == MMAP_T_APPLICATION_FLASH)
					{
						mcontext.max_secCnt = mm->memBufferType.Flash.numberOfSectors;
						if (mcontext.max_secCnt > 0)
							mcontext.curr_secCnt = 1;  // start with first sector
						else
							mcontext.curr_secCnt = 0;  // seems to be broken
					}
					else
					{
						mcontext.max_secCnt = 0;
						mcontext.curr_secCnt = 0;
					}

				}
				else
				{
					currEntryCnt = 0;  // skip empty Map
					mcontext.max_secCnt = 0;
					mcontext.curr_secCnt = 0;
				}
				currMemMapHeadCnt = 1;

				cbEntry = allocateCmdCallbackEntry(sizeof(mcontext));
				if(cbEntry == NULL)
					return -1;

				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"in Queue Prog 0x%8.8X\n",req_arg[0]);
				inQueueCmdCallbackEntry(
							cbEntry,
							printmemMapCB_info,
							currMemMapHeadCnt,
							currEntryCnt,
							(void *)(&mcontext),
							sizeof(mcontext)
							);
				memMapContainer.memStat.pending = ACTION_MEMMAPINFO_INPROCESS;
				gettimeofday(&(memMapContainer.memStat.startTime), NULL);
				
				/*show active MemMap*/
				if(memMapContainer.activeNumber < 0)
					snprintf(response_str,BUFMAX/2,"no active MemMap\n");
				else
					snprintf(response_str,BUFMAX/2
						,"MemMap[%d] active\n"
						,memMapContainer.activeNumber
						);
				return strlen(response_str);

			case F_TOKEN_CMDSEQ:	/* CmdSequence */
				if( req_arg[0] < 0 || req_arg[1] < 0)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,
						"wrong CmdSequence[%d][%d]\n"
						,req_arg[0]
						,req_arg[1]
						);
					return -1;
				}
				if(verbose)
					snprintf(response_str,BUFMAX/2,
						"[0x%8.8X] <- value @0x%8.8X of CmdSequence[%d][%d]\n"
						,readvalCmdSequence(req_arg[0], req_arg[1])
						,readaddrCmdSequence(req_arg[0], req_arg[1])
						,req_arg[0]
						,req_arg[1]
						);
				else
					snprintf(response_str,BUFMAX/2,
						"[0x%8.8X]\n"
						,readvalCmdSequence(req_arg[0], req_arg[1])
						);
				return strlen(response_str);
		
			case F_TOKEN_CMDSEQ_BIND: /* CmdSequence.Bind */
				snprintf(response_str,BUFMAX/2,
					"bind cmd[%d] to hook StartPeriphery\n"
					"bind cmd[%d] to hook StopPeriphery\n"
					"bind cmd[%d] to hook ResetCPU\n"
					,
					cmdSequenceNumber_StartPeriphery,
					cmdSequenceNumber_StopPeriphery,
					cmdSequenceNumber_ResetCPU
					);

				return strlen(response_str);

			case F_TOKEN_CMD: /* Cmd */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Cmd[%d]\n",req_arg[0]);	
				if( req_arg[0] < 0 || doCmdSequence(req_arg[0]) != 0 )
					return -1;
				return 0;
				
			case F_TOKEN_ACTIVATE: /* UseMemMap */
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"useMemMap[%d]\n",req_arg[0]);
				gdb_writeback_Ram();
				/*deactivateMemMap old then active new MemMap*/
				deactivateMemMap();
				if(activateMemMap(req_arg[0]))
					return -1;
				return 0;
				
			case F_TOKEN_DEACTIVATE: /*NoMemMap*/
				gdb_writeback_Ram();
				deactivateMemMap();
				return 0;
				
			case F_TOKEN_RESTART: /*Restart*/
				gdb_invalidate_Ram_Buffer();
				gdb_invalidate_Rom_Buffer();
				arm_sfa_ResetCPU();
				gdb_restart();
				/*read out the current CPU registers, but we are going to ignore them*/
				jtag_arm_ReadCpuRegs(1); // (0)
				/*setup CPU registers default*/
				bzero(raw_regs,sizeof(struct reg_set));
				raw_regs->CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
				/*reenable temp break at main*/
				if(symbolMain.state == SYM_PRESENT)
					symbolMain.breakIsActive = 1;
				return 0;
				
			case F_TOKEN_DELAY: /*Delay[..]*/
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Delay[%d]\n",req_arg[0]);
				if( req_arg[0] < 0 )
					return -1;
				else
				{
					struct timeval	time_begin;
					struct timeval	time_curr;
					char aliveMsg[1];

					aliveMsg[0] = 0;
					gettimeofday(&time_begin, NULL);
					while(1)
					{
						gettimeofday(&time_curr, NULL);
						/* timeout if more than req_arg[0] msec*/
						if((  (time_curr.tv_sec - time_begin.tv_sec) * 1000 
						    + (time_curr.tv_usec - time_begin.tv_usec)/1000) > req_arg[0] ) 
							break;
						else
							gdb_rcmd_console_output(fd, aliveMsg);
					}
					
				}
				return 0;
				
			case F_TOKEN_PROG_FL: /*ProgramFlash*/
			case F_TOKEN_VERIFY_FL: /*VerifyFlash*/
			case F_TOKEN_CHKSUM_TEST_FL: /*CheckFlash*/
			case F_TOKEN_ERASE_FL: /*EraseFlash*/
				if(req_arg[0] == -1) //correct empty default 
					req_arg[0] = 0;
				mm = findMemMapOfAddr(req_arg[0] & 0xFFFFfffc);
				if(mm == NULL || mm->type != MMAP_T_FLASH || mm->memBufferType.Flash.algo <= FLASH_ALGORITHEM_NOSUPPORT)
					return -1;

				if((memMapContainer.memStat.pending & ACTION_INPROCESS ) == ACTION_INPROCESS )
				{
					snprintf(response_str,BUFMAX/2,"pending Action\n");
					return sizeof("pending Action\n") - 1;
				}

				if(req_arg[1] == 0) //well so there is nothing to do
					return 0;

				/*we must have a workspace to do the CRC-check at target side*/
				if(f_req_type == F_TOKEN_CHKSUM_TEST_FL && getWorkSpace() == NULL)
				{
					struct gdbSprintfBuf *msg_buf;

					/*do we are going to send messages to the gdb console*/
					if(fake_continue_mode)
						msg_buf = allocateGdbSprintfBuf(BUFMAX);
					else
						msg_buf = NULL;

					gdbPrintf(ACTION_NON ,1 ,msg_buf ,"You must have a Workspace at the target side to do a CRC-check\n");
					return 0;
				}
					
				/*create context of callback*/
				bzero(&fcontext, sizeof(fcontext));
				if(req_arg[1] < 0 || req_arg[1] > mm->memBufferType.Flash.numberOfSectors )
					fcontext.lastSectorId = mm->memBufferType.Flash.numberOfSectors - 1;
				else
					fcontext.lastSectorId = req_arg[1] - 1;
				fcontext.whichMemMap = mm;
					
				cbEntry = allocateCmdCallbackEntry(sizeof(fcontext));
				if(cbEntry == NULL)
					return -1;

				fcontext.verbose = verbose | INFO_VERBOSE;
				fcontext.quiet_cnt = 0;
				if(f_req_type == F_TOKEN_PROG_FL)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"in Queue Prog 0x%8.8X\n",req_arg[0]);
					inQueueCmdCallbackEntry(
								cbEntry,
								programFlashCB_write_sector,
								req_arg[0] & 0xFFFFfffc,
								-1,
								(void *)(&fcontext),
								sizeof(fcontext)
								);
					memMapContainer.memStat.pending = ACTION_PROGRAM_INPROCESS;
				}
				else if(f_req_type == F_TOKEN_VERIFY_FL)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"in Queue Verify 0x%8.8X\n",req_arg[0]);
					inQueueCmdCallbackEntry(
								cbEntry,
								verifyFlashCB_read_sector,
								req_arg[0] & 0xFFFFfffc,
								-1,
								(void *)(&fcontext),
								sizeof(fcontext)
								);
					memMapContainer.memStat.pending = ACTION_VERIFY_INPROCESS;
				}
				else if(f_req_type == F_TOKEN_CHKSUM_TEST_FL)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"in Queue CRC-Check at target 0x%8.8X\n",req_arg[0]);
					inQueueCmdCallbackEntry(
								cbEntry,
								checkFlashCB_read_sector,
								req_arg[0] & 0xFFFFfffc,
								-1,
								(void *)(&fcontext),
								sizeof(fcontext)
								);
					memMapContainer.memStat.pending = ACTION_VERIFY_INPROCESS;
				}

				else	//F_TOKEN_ERASE_FL
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"in Queue Erase 0x%8.8X\n",req_arg[0]);
					inQueueCmdCallbackEntry(
								cbEntry,
								eraseFlashCB_sector,
								req_arg[0] & 0xFFFFfffc,
								0,
								(void *)(&fcontext),
								sizeof(fcontext)
								);
					memMapContainer.memStat.pending = ACTION_ERASE_INPROCESS;
				}

				gettimeofday(&(memMapContainer.memStat.startTime), NULL);
				return 0;
				
			case F_TOKEN_FL_STATE: /*FlashState*/
				if(req_arg[0] == -1) //correct empty default 
					req_arg[0] = 0;
				mm = findMemMapOfAddr(req_arg[0] & 0xFFFFfffc);
				if(mm == NULL)
					return -1;
				if(mm->type != MMAP_T_FLASH || mm->memBufferType.Flash.algo <= FLASH_ALGORITHEM_NOSUPPORT)
				{
					snprintf(response_str,BUFMAX/2,"[4] No Flash found\n");
					return sizeof("[4] No Flash found\n") - 1;
				}

				if((memMapContainer.memStat.pending & ACTION_INPROCESS ) == ACTION_INPROCESS )
				{
					if(verbose)
					{
						snprintf(response_str,BUFMAX/2,"[0] pending Action\n");
						return sizeof("[0] pending Action\n") - 1;
					}
					else
					{
						return 0;
					}
				}
				if(memMapContainer.memStat.pending ==  ACTION_VERIFY_EQUAL)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[1] equal (%d msec)\n",diffTime);
					return strlen(response_str);
				}
				else if(memMapContainer.memStat.pending ==  ACTION_VERIFY_DIFF)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[3] diff (%d msec)\n",diffTime);
					return strlen(response_str);
				}
				else if(memMapContainer.memStat.pending ==  ACTION_PROGRAM_SUCCEED)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[1] succeed (%d msec)\n",diffTime);
					return strlen(response_str);
				}
				else if(memMapContainer.memStat.pending ==  ACTION_PROGRAM_FAIL)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[2] fail (%d msec)\n",diffTime);
					return strlen(response_str);
				}					
				else if(memMapContainer.memStat.pending ==  ACTION_ERASE_SUCCEED)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[1] succeed (%d msec)\n",diffTime);
					return strlen(response_str);
				}
				else if(memMapContainer.memStat.pending ==  ACTION_ERASE_FAIL)
				{
					memMapContainer.memStat.pending = ACTION_NON;
					diffTime = (memMapContainer.memStat.stopTime.tv_sec - memMapContainer.memStat.startTime.tv_sec)*1000;
					diffTime += (memMapContainer.memStat.stopTime.tv_usec - memMapContainer.memStat.startTime.tv_usec)/1000 + 1;
					snprintf(response_str,BUFMAX/2,"[2] fail (%d msec)\n",diffTime);
					return strlen(response_str);
				}
				return 0;
			case F_FORCE_INVAL_FLASH:/*InvalFlashWhileEraseing*/
				if(forceInvalMemWhileEraseingFlash == 0)
					snprintf(response_str,BUFMAX/2,"InvalFlashWhileEraseing -> 0\n");
				else
					snprintf(response_str,BUFMAX/2,"InvalFlashWhileEraseing -> 1\n");
				
				return strlen(response_str);	
			case F_TOKEN_SYSINFO:	/*SysInfo*/
				snprintf(response_str,BUFMAX/2,
					"embedded ICE revision %d\n"
					"endian:       %s\n"
					"vendor:       %s\n"
					"core:         %s%s revision %d\n"
					"memory type:  %s\n"
					"capability:   %s\n"
					"with%s Coprocessor CP15\n"
					,
					arm_info.ice_revision,
					(arm_info.bigend == 0? "little" :
					   (arm_info.bigend == 1)?"big":"not yet checked" ),
					arm_info.vendor_string,
					((arm_info.core_number == 7) ? "ARM7"
						: ((arm_info.core_number == 8) ? "ARM7 or ARM9 "
						: ((arm_info.core_number == 9) ? "ARM9" 
						: ((arm_info.core_number == 10)? "ARM10"
						: ((arm_info.core_number == 11)? "ARM11":"unknown" ))))),
					(arm_info.has_thumb ? "TDMI":""),
					arm_info.core_revision,
					arm_info.dd_string,
					arm_info.cap_string,
					(arm_info.has_cp15 ? "" : "out")
					);
					
				return strlen(response_str);	

			case F_TOKEN_INTERRUPT:/*Interrupt*/
				if(allow_intr_in_step_mode == 0)
					snprintf(response_str,BUFMAX/2,"Interrupt -> 0 (disabled while stepping)\n");
				else
					snprintf(response_str,BUFMAX/2,"Interrupt -> 1 (enabled while stepping)\n");
					
				return strlen(response_str);	
				
			case F_TOKEN_MODE_MCLK:/*modeMCLK*/
				snprintf(response_str,BUFMAX/2,"modeMCLK -> %d\n",ice_state.high_speed_mclk);
				return strlen(response_str);	
			case F_TOKEN_MODE_FLASH:/*modeFLASH*/
				snprintf(response_str,BUFMAX/2,
					"modeFLASH -> %d\n",
					(download_wait == 1) ?
						((download_faster == 0) ? 0 : 2 )
					:
						((download_faster == 0) ? 1 : 3 )
					);

				return strlen(response_str);
			case F_TOKEN_MODE_WORKSPACE:/*modeWorkspace*/
				snprintf(response_str,BUFMAX/2,"modeWorkspace -> %d\n",workspace_mode);
				return strlen(response_str);	
			case F_TOKEN_FORCE_HW_BREAK:/*forceHWBreak*/
				snprintf(response_str,BUFMAX/2,"forceHWBreak -> %d\n",force_hardware_breakpoint);
				return strlen(response_str);
			case F_TOKEN_TEMP_MAIN_BREAK: /*forceTempHWBreakAtMain*/
				snprintf(response_str,BUFMAX/2,"forceTempHWBreakAtMain -> %d\n",symbolMain.breakIsActive);
				return strlen(response_str);
			case F_TOKEN_LPC_FREQUENCE:/*LPCfrequence*/
				snprintf(response_str,BUFMAX/2,"LPCfrequence -> %d\n",LPC_frequence);
				return strlen(response_str);	
			case F_TOKEN_LPC_CHKSUM:/*LpcGenVectorCheckSum*/
				{
					uint32_t *buf;
					int cnt = 0;
					uint32_t sum = 0;
					
					if( (buf = gdb_read_mem(0, 8)) == NULL)
						return -1;
					do{
						if(cnt != 5)
							sum += buf[cnt];
					} while (++cnt < 8);
					sum = ~sum + 1;
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"add LPC chksum %X\n",sum);
					
					gdb_write_mem(&sum,5*sizeof(uint32_t),1*sizeof(uint32_t));
					return 0;
				}
			case F_TOKEN_SAM7_FREQUENCE:/*SAM7frequence*/
				snprintf(response_str,BUFMAX/2,"SAM7frequence -> %d\n",SAM7_frequence);
				return strlen(response_str);	
			case F_TOKEN_DBG_MASK: /*DbgMask*/
				snprintf(response_str,BUFMAX/2,"DbgMask -> 0x%X\n",dbg_msg_msk);
				return strlen(response_str);	
			case F_TOKEN_VERBOSE: /*verbose*/
				snprintf(response_str,BUFMAX/2,"verbose -> 0x%X\n",verbose);
				return strlen(response_str);	
			default:
				/*we know somthing but we keep this secret;-)*/
				snprintf(response_str,BUFMAX/2,"well known\n");
				return sizeof("well known\n") - 1;
			}
		}
	}
	return -1;
	
}

void gdb_query(int fd, char *query_str, char *response_str, struct reg_set *raw_regs)
{
	int len;
	int ret_val;

	len = strlen(query_str);
	if(len <= 0)
	{
		strncpy(response_str,"E01",sizeof("E01"));
		return;
	}
	if( *query_str == 'C' && len == 1)
	{
		strncpy(response_str,"QC0001",sizeof("QC0001"));
		return;
	}
#if 0	
	if(!strncmp(query_str , "Offsets",sizeof("Offset")))
	{
		strncpy(response_str,"Text=0;Data=0;Bss=0",sizeof("Text=0;Data=0;BSS=0"));
		return;
	}
#endif
	if(!strncmp(query_str , "Symbol",sizeof("Symbo")))
	{
		char 	*ptr, *tptr;
		int	len;
		int	addr;

		ptr = &query_str[sizeof("Symbo")];
		
		/*qSymbol:: -- start symbol lookup*/
		if(*(ptr++) == ':')
		{
			tptr = ptr;
			len  = 0;
			/*search for closing ':'*/
			while(*tptr != ':' )
			{
				if(*tptr == 0)
				{
					/*closing ':' not found*/
					strncpy(response_str,"E02",sizeof("E02"));
					return;
				}
				len++;
				tptr++;
			}
			
			if(len == 0) /* so it was "::"*/
			{
				ptr ++;
				if(*ptr == 0) /*Start*/
				{
					/*
					 * at GDB side "symbol-file ..."
					 * has been issuered a while ago
					 * now we might like to reset all ROM Chache, too.
					 */
					gdb_invalidate_Rom_Buffer();
					
					/*query address of main */
					strncpy(response_str,"qSymbol:",sizeof("qSymbol"));
					response_str += sizeof("qSymbol");
					mem2hex("main",response_str,sizeof("mai"));
					response_str += 2 * sizeof("mai");
					*response_str++ = ':';
					*response_str = 0;
				}
				else /*response (symbol lookup failed) to previous request*/
				{
					hex2mem(ptr, ptr, sizeof("mai"));
					if(!strncmp(ptr, "main", sizeof("mai") ))
					{
						/*receive "unknown" symbol main*/
						symbolMain.addr = 0;
						symbolMain.state = SYM_UNKNOWN;
						symbolMain.breakIsActive = 0;
					}
					strncpy(response_str,"OK",sizeof("OK"));
				}
			}
			else if(hexToInt (&ptr, &addr) && *(ptr++) == ':') /*reponse (symbol with address)*/
			{
				hex2mem(ptr, ptr, sizeof("mai"));
				if(!strncmp(ptr, "main", sizeof("mai") ))
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Symbol main at 0x%8.8X",addr);
#if 0
					if(addr & 1)
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO," (Thumb instr.)\n");
					else
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO," (ARM instr.)\n");
					symbolMain.isThumb = addr & 1;
#else
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO," (ARM or Thumb instr.)\n");
					symbolMain.isThumb = 1; // since gdb did not yet tell us; we assume Thumb
#endif
					symbolMain.addr = (uint32_t) addr & 0xffffFFFEuL;
					symbolMain.state = SYM_PRESENT;
					symbolMain.breakIsActive = 1;
				}
				/*no more requests*/
				strncpy(response_str,"OK",sizeof("OK"));
			}
			else
				strncpy(response_str,"E03",sizeof("E03")); /*wrong hex format off address value*/
		}
		else
			strncpy(response_str,"E01",sizeof("E01")); /*missing first ':'*/
		return;
	}
	/*
	 * User Command at gdb
	 * gdb> monitor ... 
	 */
	if(!strncmp(query_str , "Rcmd,",5)) 
	{
		query_str += 5;
		len -= 5;
		if(len > 1)
		{
			len >>= 1;
			hex2mem(query_str, query_str, len);
			query_str[len] = 0;
			ret_val = gdb_monitor_Rcmd(fd, query_str, query_str, raw_regs);
			if(ret_val > 0)
				mem2hex(query_str, response_str, ret_val);
			else if(ret_val == 0)
				strncpy(response_str,"OK",sizeof("OK"));
			else
				strncpy(response_str,"E01",sizeof("E01"));
		}
		else
			strncpy(response_str,"OK",sizeof("OK"));
		return;
	}
	return;
}


