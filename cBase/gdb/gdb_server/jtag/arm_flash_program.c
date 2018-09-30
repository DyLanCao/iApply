/*
 * arm_gdbstub_mem_flash_program.c
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sysexits.h>
#include <ctype.h>
#include <unistd.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_flash.h"
#include "arm_gdbstub.h"


/********************************/

int download_faster = 1;
int download_wait   = 1;		/* 1 -> Wait for completion after programming*/

/*
 * Callback function to write data into the flash-eeprom
 */
int programFlashCB_write_sector(int currAddr, int cnt, struct context *contextSrc)
{
	struct flashCBContext *context = (struct flashCBContext *)contextSrc;
	struct memMap * mm;
	struct cmdCallbackEntry *cbEntry;
	struct sector	*secList;
	int idx,i;
	uint32_t offset;
	struct gdbSprintfBuf *msg_buf;

	if(context == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing context\n");
		return -1;
	}
	mm = context->whichMemMap;
	
	/*make sure we are still in debug state since we need the traget CPU*/
	if ((ice_state.is_debugrequest & 1) == 1)
	{
		secList = mm->memBufferType.Flash.sectorList;
		if(secList == NULL) //should not happen -- paranoia test
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"missing sector list");
			return -1;
		}
		
		/*do we are going to send messages to the gdb console*/
		if(fake_continue_mode)
			msg_buf = allocateGdbSprintfBuf(BUFMAX);
		else
			msg_buf = NULL;
		
		idx = context->currentSectorId;
		offset = 0;
		for(i=0;i<idx;i++)
			offset += secList[i].size;

		
		/*make sure currAddr is realy within currentSectorId*/
		while( (uint32_t) currAddr >=  mm->baseAddr + offset + secList[idx].size)
		{
			/*correct index and retry*/
			(context->currentSectorId)++;
			idx++;
			offset = 0;
			for(i=0;i<idx;i++)
				offset += secList[i].size;
			if(context->lastSectorId < mm->memBufferType.Flash.numberOfSectors - 1)
				(context->lastSectorId)++;
		}

		/*Check offset alignment*/
		switch(mm->busSize & 0xFF)
		{
		case 8:
			if(offset & 0x7FF) /*2K offset*/
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"offset failure\n");
				offset &= 0xffff800;
			}
			break;
		case 16:
			if(offset & 0xFFF) /*4K offset*/
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"offset failure\n");
				offset &= 0xffffF000;
			}
			break;
		case 32:
		case 128:	// pseudo size for Philips
		case 164:	// pseudo size for ST
			if(offset & 0x1FFF) /*8K offset*/
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"offset failure\n");
				offset &= 0xffffE000;
			}
			break;
		case 132:	// pseudo size for Atmel SAM7
			if(offset & 0x7F) /*128 byte offset for Atmel SAM7*/
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"offset failure\n");
				offset &= 0xffffE000;
			}
			break;
		default:;
		}
		
		/*check if the sector must be written*/
		if(secList[idx].valid == SECTOR_WRITE_DURTY || secList[idx].valid == SECTOR_WRITE_DURTY_ERASED)
		{
			/*First time erase?*/
			if(cnt < 0 && (secList[idx].valid & SECTOR_FLAG_ERASED) == 0)
			{
				if(context->quiet_cnt == 0)
					context->verbose |= INFO_VERBOSE;
				if(++context->quiet_cnt == 10)
					context->quiet_cnt = 0;
				
				if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
					gdbPrintf( ACTION_NON
						,0
						, msg_buf
						,"erase seg %d (%d Bytes) @ Addr 0x%8.8X\t"
						,idx
						,secList[idx].size
						,mm->baseAddr + offset);
				else if (context->verbose)
					gdbPrintf( ACTION_NON
						,0
						, msg_buf
						,"seg %d @ Addr 0x%8.8X\t"
						,idx
						,mm->baseAddr + offset);
				
				context->verbose &= ~INFO_VERBOSE;
				/*erase sector*/
				if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_INTEL)
				{		
					switch(mm->busSize & 0xFF)
					{
					case 8:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
						if(secList[idx].lock)
							jt_intelflashUnlockSectorByte(mm->baseAddr + offset, mm->baseAddr + offset);
						jt_intelflashEraseSectorByte(mm->baseAddr + offset, mm->baseAddr + offset);
						break;
					case 16:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
						switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
						{
						case 1: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset);
							jt_intelflashEraseSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset); 
							break;
						case 2: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset);
							jt_intelflashEraseSectorHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset); 
							break;
						default: 
							gdbPrintf( ACTION_PROGRAM_FAIL
								,1
								, msg_buf
								,"%d Flash Chips per 16 Bit Bus not supported\n"
								,mm->memBufferType.Flash.chipsPerBus);
							return 1;
						}
						break;
					case 32:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
						switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
						{	
						case 1: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord(mm->baseAddr + offset, mm->baseAddr + offset);
							jt_intelflashEraseSectorWord(mm->baseAddr + offset, mm->baseAddr + offset); 
							break;
						case 2: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord_dual(mm->baseAddr + offset, mm->baseAddr + offset);
							jt_intelflashEraseSectorWord_dual(mm->baseAddr + offset, mm->baseAddr + offset); 
							break;
						case 4: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord_quad(mm->baseAddr + offset, mm->baseAddr + offset);
							jt_intelflashEraseSectorWord_quad(mm->baseAddr + offset, mm->baseAddr + offset); 
							break;
						default: 
							gdbPrintf( ACTION_PROGRAM_FAIL
								,1
								, msg_buf
								,"%d Flash Chips per 32 Bit Bus not supported\n"
								,mm->memBufferType.Flash.chipsPerBus);
							return 1;
						}
						break;
					default:	
						gdbPrintf( ACTION_PROGRAM_FAIL
							,1
							, msg_buf
							,"Flash Bus wide of %d not supported\n"
							,mm->busSize);
						return 1;
					}
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"-e-");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD)
				{	
					switch(mm->busSize & 0xFF)
					{
					case 8:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
						if(jt_amdflashEraseSectorByte(mm->baseAddr + offset, mm->baseAddr + offset))
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
							jt_amdflashResetByte(mm->baseAddr + offset);
							return 1;
						}
						jt_amdflashResetByte(mm->baseAddr + offset);
						break;
					case 16:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
						if(jt_amdflashEraseSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset))
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
							jt_amdflashResetHalfword(mm->baseAddr + offset);
							return 1;
						}
						jt_amdflashResetHalfword(mm->baseAddr + offset);
						break;
					case 32:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
						if(jt_amdflashEraseSectorWord(mm->baseAddr + offset, mm->baseAddr + offset))
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
							jt_amdflashResetWord(mm->baseAddr + offset);
							return 1;
						}
						jt_amdflashResetWord(mm->baseAddr + offset);
						break;
					default:
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 , msg_buf
							,"Flash Bus wide of %d not supported\n"
							,mm->busSize);
						return 1; //not supported
					}
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"-e-");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_PHILIPS && (mm->busSize & 0xFF) == 128)
				{
					uint32_t mask;
			
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"128 bit\t");
					mask = jt_philipsflashGenMask( idx, idx, mm->length, LPC_boot_sector_enabled);
					if( idx < mm->memBufferType.Flash.numberOfSectors - 1)
					{
						/*all except the bootsector itself*/
						if(  philipsFlashUnlock( mm->baseAddr + offset,mask) )
						{		
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to unlock\n");
							return 1;
						}
					
						if( philipsFlashEraseAllUnlocked( mm->baseAddr + offset ) )
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"Fail to erase\n");
							return 1;
						}
						/*now Lock and unlock again*/
						if( philipsFlashLock( mm->baseAddr + offset,mask) )
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to lock\n");
							return 1;
						}
	
						if(  philipsFlashUnlock( mm->baseAddr + offset,mask) )
						{		
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to (re)unlock\n");
							return 1;
						}
					}
#if 0
					else if (LPC_boot_sector_enabled)
					{
						/*bootsector*/
						if(  philipsFlashUnlockBootsector( mm->baseAddr + offset,mask) )
						{		
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to unlock\n");
							return 1;
						}
					
						if( philipsFlashEraseAllUnlocked( mm->baseAddr + offset ) )
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"Fail to erase\n");
							return 1;
						}
					}
#endif
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"Erase of bootsector is not permitted\n");
						return 1;
					}
					
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"-e-");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ATMEL && (mm->busSize & 0xFF) == 132)
				{
					uint32_t mc_fsr;
					
					/*unlock region if necesary*/
					mc_fsr = jtag_arm_ReadWord(0xffffFF68); // read AT91SAM7 Flash Status Register
					if(mc_fsr & (1<<(secList[idx].region + 16)))
					{
						if( atmelFlashUnlock( mm->baseAddr + offset, idx) )
						{
							gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "fail -- unlock\n");
							return 1;
						}
					}
					
					if(atmelFlashErase( mm->baseAddr + offset, idx))
					{		
						gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "fail -- Erase\n");
						return 1;
					}
					
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"-e-");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ST && (mm->busSize & 0xFF) == 164)
				{
					/*XXX todo disable write protection */
					if(stFlashErase( mm->baseAddr + offset, idx))
					{
						gdbPrintf(ACTION_ERASE_FAIL, 1, msg_buf, "fail -- Erase\n");
						return 1;
					}
					
					if(context->verbose)
						gdbPrintf( ACTION_NON, 0, msg_buf, "OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"-E-");
				}
				else
				{
					gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "Erase failure -- unknown Flash\n");
					return 1;
				}
				cnt = 0;
			}
			else if(cnt < 0) /*First time but already erased ?*/
			{
				if(context->quiet_cnt == 0)
					context->verbose |= INFO_VERBOSE;
				if(++context->quiet_cnt == 10)
					context->quiet_cnt = 0;

				if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
					gdbPrintf( ACTION_NON
						,0
						, msg_buf
						,"prepare seg %d (%d Bytes) @ Addr 0x%8.8X\t"
						,idx
						,secList[idx].size
						,mm->baseAddr + offset);
				else if (context->verbose)
					gdbPrintf( ACTION_NON
						,0
						, msg_buf
						,"seg %d @ Addr 0x%8.8X\t"
						,idx
						,mm->baseAddr + offset);
				context->verbose &= ~INFO_VERBOSE;
				/*unlock sector*/
				if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_INTEL)
				{		
					switch(mm->busSize & 0xFF)
					{
					case 8:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
						if(secList[idx].lock)
							jt_intelflashUnlockSectorByte(mm->baseAddr + offset, mm->baseAddr + offset);
						break;
					case 16:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
						switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
						{
						case 1: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorHalfword(mm->baseAddr + offset, mm->baseAddr + offset);
							break;
						case 2: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset);
							break;
						default: 
							gdbPrintf( ACTION_PROGRAM_FAIL
								,1
								, msg_buf
								,"%d Flash Chips per 16 Bit Bus not supported\n"
								,mm->memBufferType.Flash.chipsPerBus);
							return 1;
						}
						break;
					case 32:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
						switch(mm->memBufferType.Flash.chipsPerBus & 0xF)
						{	
						case 1: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord(mm->baseAddr + offset, mm->baseAddr + offset);
							break;
						case 2: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord_dual(mm->baseAddr + offset, mm->baseAddr + offset);
							break;
						case 4: 
							if(secList[idx].lock)
								jt_intelflashUnlockSectorWord_quad(mm->baseAddr + offset, mm->baseAddr + offset);
							break;
						default: 
							gdbPrintf( ACTION_PROGRAM_FAIL
								,1
								, msg_buf
								,"%d Flash Chips per 32 Bit Bus not supported\n"
								,mm->memBufferType.Flash.chipsPerBus);
							return 1;
						}
						break;
					default:	
						gdbPrintf( ACTION_PROGRAM_FAIL
							,1
							, msg_buf
							,"Flash Bus wide of %d not supported\n"
							,mm->busSize);
						return 1;
					}
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"pre");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD)
				{	
					switch(mm->busSize & 0xFF)
					{
					case 8:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"8 bit\t");
						jt_amdflashResetByte(mm->baseAddr + offset);
						break;
					case 16:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"16 bit\t");
						jt_amdflashResetHalfword(mm->baseAddr + offset);
						break;
					case 32:
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"32 bit\t");
						jt_amdflashResetWord(mm->baseAddr + offset);
						break;
					default:
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 , msg_buf
							,"Flash Bus wide of %d not supported\n"
							,mm->busSize);
						return 1; //not supported
					}
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"pre");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_PHILIPS && (mm->busSize & 0xFF) == 128)
				{
					uint32_t mask;
			
					dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"128 bit\t");
					mask = jt_philipsflashGenMask( idx, idx, mm->length, LPC_boot_sector_enabled);
					if( idx < mm->memBufferType.Flash.numberOfSectors - 1)
					{
						/*all except the bootsector itself*/
						if(  philipsFlashUnlock( mm->baseAddr + offset,mask) )
						{		
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to unlock\n");
							return 1;
						}
					}
#if 0
					else if (LPC_boot_sector_enabled)
					{
						/*bootsector*/
						if(  philipsFlashUnlockBootsector( mm->baseAddr + offset,mask) )
						{		
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to unlock\n");
							return 1;
						}
					}
#endif
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"Erase of bootsector is not permitted\n");
						return 1;
					}
					
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"pre");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ATMEL && (mm->busSize & 0xFF) == 132)
				{
					uint32_t mc_fsr;
					
					/*unlock region if necesary*/
					mc_fsr = jtag_arm_ReadWord(0xffffFF68); // read AT91SAM7 Flash Status Register
					if(mc_fsr & (1<<(secList[idx].region + 16)))
					{
						if( atmelFlashUnlock( mm->baseAddr + offset, idx) )
						{
							gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "fail -- unlock\n");
							return 1;
						}
					}
					if (context->verbose)
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"OK\n");
					else
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"pre");
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ST && (mm->busSize & 0xFF) == 164)
				{
					;/*XXX todo disable write protection */
				}
				else
				{
					gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "Unlock failure -- unknown Flash\n");
					return 1;
				}

				cnt = 0;
			}
			else
			{
				uint32_t data;
				
				/*program single data*/
				if(cnt == 0)
				{
					if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
						gdbPrintf( ACTION_NON 
							,0 
							,msg_buf 
							,"prog seg %d (%d Bytes) @ Addr 0x%8.8X\t"
							,idx,secList[idx].size
							,mm->baseAddr + offset);
				}
				else if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
				{
					gdbPrintf( ACTION_NON ,0 ,msg_buf ,".");
					if((cnt & 0x1F) == 0)
					{
						struct memMap *ws = getWorkSpace();
						if( ws == NULL || ws->memBufferType.Workspace.state == WORKSPACE_BROKEN )
							gdbPrintf( ACTION_NON ,0 ,msg_buf ,"\n");
					}
				}

				if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_INTEL)
				{	
					switch(mm->busSize & 0xFF)
					{
					case 8:
						/*prog byte*/
						data = mm->memBuffer.Byte[offset + cnt];
						if((data & 0xFF) != 0xFF)
						{
							dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"%02X ",data);
							if(jt_intelflashProgByte(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data & 0xff, 1))
							{
								gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
								return 1;
							}
						}
						/*next addr*/
						cnt++; 
						break;
					case 16:
						/*prog halfword*/
						data = mm->memBuffer.HalfWord[(offset + cnt)/2];
						if((data & 0xFFFF) != 0xFFFF)
						{
							dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"%04X ",data);
							switch(mm->memBufferType.Flash.chipsPerBus)
							{
							case 1: 
								if(jt_intelflashProgHalfword(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data, 1))
								{
									gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
									return 1;
								}
								break;
							case 2: 
								if(jt_intelflashProgHalfword_dual(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data, 1))
								{
									gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
									return 1;
								}
								break;
							default: 
								gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"%d Flash Chips per 16 Bit Bus not supported\n",mm->memBufferType.Flash.chipsPerBus);
								return 1;
							}
						}
						/*next addr*/
						cnt+=2; 
						break;

					case 32:
						/*prog word*/
						data = mm->memBuffer.Word[(offset + cnt)/4];
						if(data != 0xffffFFFF)
						{
							dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"%08X ",data);
							switch(mm->memBufferType.Flash.chipsPerBus)
							{
							case 1: 
								if(jt_intelflashProgWord(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data, 1))
								{
									gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
									return 1;
								}
								break;
							case 2: 
								if(jt_intelflashProgWord_dual(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data, 1))
								{
									gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
									return 1;
								}
								break;
							case 4: 
								if(jt_intelflashProgWord_quad(mm->baseAddr + offset, mm->baseAddr + offset + cnt, data, 1))
								{
									gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
									return 1;
								}
								break;
							default: 
								dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"%d Flash Chips per 32 Bit Bus not supported\n",mm->memBufferType.Flash.chipsPerBus);
								gettimeofday(&(memMapContainer.memStat.stopTime),NULL);
								memMapContainer.memStat.pending = ACTION_PROGRAM_FAIL;
								return 1;
							}

						}
						/*next addr*/
						cnt+=4; 
						break;
					default:
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"Flash Bus wide of %d not supported\n",mm->busSize);
						return 1; //not supported
					}

				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD)
				{	
					switch(mm->busSize & 0xFF)
					{
					case 8:
						/*prog byte*/
						i = prog_flash_amd8(mm->baseAddr + offset
							, mm->baseAddr + offset + cnt 
							, secList[idx].size - cnt
							, &mm->memBuffer.Byte[offset + cnt]);
						break;
					case 16:
						/*prog half word*/
						i = prog_flash_amd16(mm->baseAddr + offset
							, mm->baseAddr + offset + cnt 
							, secList[idx].size - cnt
							, &mm->memBuffer.HalfWord[(offset + cnt)/2]);
						break;
					case 32:
						/*prog word*/
						i = prog_flash_amd32(mm->baseAddr + offset
							, mm->baseAddr + offset + cnt 
							, secList[idx].size - cnt
							, &mm->memBuffer.Word[(offset + cnt)/4]);
						break;
					default:
						gdbPrintf( ACTION_NON ,0 ,msg_buf ,"Flash Bus wide of %d not supported\t",mm->busSize);
						i = 0; // not supported
						break;
					}
					/*next addr*/
					if(i > 0)
						cnt += i;
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
						return 1;
					}
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_PHILIPS && (mm->busSize & 0xFF) == 128)
				{
					i = philipsFlashProgram(mm->baseAddr + offset + cnt, secList[idx].size - cnt , &mm->memBuffer.Word[(offset + cnt)/4]);
					/*next addr*/
					if(i > 0)
						cnt += i;
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
						return 1;
					}
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ATMEL && (mm->busSize & 0xFF) == 132)
				{
					i = atmelFlashProgramOnly(mm->baseAddr + offset + cnt, idx, secList[idx].size - cnt, &mm->memBuffer.Word[(offset + cnt)/4]);
					/*next addr*/
					if(i > 0)
						cnt += i;
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
						return 1;
					}
				}
				else if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_ST && (mm->busSize & 0xFF) == 164)
				{
					i = stFlashProgram(mm->baseAddr + offset + cnt, secList[idx].size - cnt, &mm->memBuffer.Word[(offset + cnt)/4]);
					/*next addr*/
					if(i > 0)
						cnt += i;
					else
					{
						gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"fail\n");
						return 1;
					}
				}
				
				IF_DBG( DBG_LEVEL_ALL )
					fflush(stdout);
									
				/*is the next addr within this segment?*/
				if(cnt >= secList[idx].size)
				{
					secList[idx].valid &= ~(SECTOR_FLAG_ERASED | SECTOR_FLAG_DURTY);
					/*make sure we are back in read mode*/
					if(  mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_AMD
					  /*&& download_wait == 0 */
					  /*&& (getWorkSpace() != NULL || download_faster)*/)
					{
						switch(mm->busSize & 0xFF)
						{
						case 8:
							jt_amdflashResetByte(mm->baseAddr + offset);
							break;
						case 16:
							jt_amdflashResetHalfword(mm->baseAddr + offset);
							break;
						case 32:
							jt_amdflashResetWord(mm->baseAddr + offset);
							break;
						default:;
						}

					}
					if(mm->memBufferType.Flash.algo == FLASH_ALGORITHEM_PHILIPS && (mm->busSize & 0xFF) == 128)
					{
						uint32_t mask;
			
						mask = jt_philipsflashGenMask( idx, idx, mm->length, LPC_boot_sector_enabled);
						if( philipsFlashLock( mm->baseAddr + offset,mask) )
						{
							gdbPrintf( ACTION_PROGRAM_FAIL ,1 ,msg_buf ,"BUSSY not able to lock sector again\n");
							return 1;
						}
					}
					if(!context->verbose)
						gdbPrintf(ACTION_NON, 0, msg_buf,"[P]");
					goto next_seg;
				}
			}
			IF_DBG( DBG_LEVEL_ALL )
				fflush(stdout);
		}
		else if(secList[idx].valid == SECTOR_WRITE)
		{
			gdbPrintf(ACTION_PROGRAM_FAIL, 1, msg_buf, "use command \"monitor VerifyFlash\" or read data from Flash before you try to [re]programm the Flash\n");
			return 1;
		}
		else
		{
			if(context->quiet_cnt == 0)
				context->verbose |= INFO_VERBOSE;
			if(++context->quiet_cnt == 10)
				context->quiet_cnt = 0;
			
			if((context->verbose & MORE_VERBOSE) == MORE_VERBOSE)
				gdbPrintf( ACTION_NON
					   ,0
					   , msg_buf
					   ,"nothing to do at seg %d (%d Bytes) @ Addr 0x%8.8X"
					   ,idx
					   ,secList[idx].size
					   ,mm->baseAddr + offset);
			else if (context->verbose)
				gdbPrintf( ACTION_NON
					   ,0
					   , msg_buf
					   ,"seg %d @ Addr 0x%8.8X\t[-]"
					   ,idx
					   ,mm->baseAddr + offset);
			else
				gdbPrintf(ACTION_NON, 0, msg_buf,"[-]");
				
			context->verbose &= ~INFO_VERBOSE;
next_seg:
			if(context->verbose || !context->quiet_cnt)
				gdbPrintf(ACTION_NON, 0, msg_buf,"\n");
			
			cnt = -1; //set erase as the default of the next commad	
		
			/*next Addr*/
			currAddr = (int)(mm->baseAddr + offset + secList[idx].size);
	
			if(context->currentSectorId < context->lastSectorId)
				(context->currentSectorId)++;
			else
			{
				/*last segment*/
				gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
				memMapContainer.memStat.pending = ACTION_PROGRAM_SUCCEED;
				if(msg_buf)
					inQueueGdbSprintfBuf(msg_buf);
				return 0;
			}
		}
		if(msg_buf)
			inQueueGdbSprintfBuf(msg_buf);
	}
	else
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"must be in debug state\n");
		memMapContainer.memStat.pending = ACTION_PROGRAM_FAIL;
		return 1;
	}
		
	/*create context of next callback*/
	cbEntry = allocateCmdCallbackEntry(sizeof(struct flashCBContext));
	if(cbEntry == NULL)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"no Mem for new Entry\n");
		return -1;
	}

	inQueueCmdCallbackEntry(
				cbEntry,
				programFlashCB_write_sector,
				currAddr & 0xFFFFfffc,
				cnt,
				contextSrc,
				sizeof(struct flashCBContext)
				);

	return 0;
}

