/****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or its performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header:          $
 *
 *  Module name:     arm_gdbstub $
 *  Revision:        0.1 $
 *  Date:            May 2004 $
 *  Contributor:     $
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works with jtag target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for FreeBSD by Stu Grossman.
 *  Modified to work as ARM7TDMI server by Thomas Klein <>
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                                      Return value
 *    !             Enable extended mode.                         OK 
 *    ?             What was the last sigval ?                    SNN   (signal NN)
 *    
 *    g             return the value of the CPU registers         hex data or ENN
 *    G             set the value of the CPU registers            OK or ENN
 *
 *    mA..A,LLLL    Read LLLL bytes at address A..A               hex data or ENN
 *    MA..A,LLLL:   Write LLLL hex data bytes at address A..A     OK or ENN
 *    XA..A,LLLL:XX...                                            OK or ENN
 *                  write mem (binary) A..A is address,
 *                  LLLL is number of bytes, XX... is binary data. 
 *                  The characters $, #, and 0x7d are escaped using 0x7d.
 *
 *    c             Resume at current address                     SNN   ( signal NN)
 *    cA..A         Continue at address A..A                      SNN
 *    Cnn;A..A      continue at address A..A with signal nn       SNN
 *
 *    s             Step one instruction                          SNN
 *    sA..A         Step one instruction from A..A                SNN
 *    Snn;A..A      step with signal                              SNN
 *
 *    k             kill
 *    
 *    D             detach                                        OK
 *    
 *    qRcmd,c..c    query remote command c..c                     hex data or ENN or nothing
 *    Qv..v=V..V    Set value of general var v..v to V..V         hex data or ENN or nothing 
 *
 *    z0,A..A,LLLL  remove memory breakpoint                      OK or ENN or nothing
 *    Z0,A..A,LLLL  insert memory breakpoint                      OK or ENN or nothing
 *    z1,A..A,LLLL  remove hardware breakpoint                    OK or ENN or nothing
 *    Z1,A..A,LLLL  insert hardware breakpoint                    OK or ENN or nothing
 *    z2,A..A,LLLL  remove write watchpoint                       OK or ENN or nothing
 *    Z2,A..A,LLLL  insert write watchpoint                       OK or ENN or nothing
 *    z3,A..A,LLLL  remove read watchpoint                        OK or ENN or nothing
 *    Z3,A..A,LLLL  insert read watchpoint                        OK or ENN or nothing
 *    z4,A..A,LLLL  remove read/write watchpoint                  OK or ENN or nothing
 *    Z4,A..A,LLLL  insert read/write watchpoint                  OK or ENN or nothing
 *    
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <setjmp.h>
#include <errno.h>

#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>

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

#define MAX_READ_RETRY 24
#define NUM_REGS (16 /*std regs*/ + 8 /*fp regs*/ + 1 /*fp stat reg*/ + 1 /*prog stat reg*/)

static int wrongbreakpoint = 0;

int	allow_intr_in_step_mode = 0;
int	fake_continue_mode = 0;

volatile char *console_buf;

int gdb_main_loop (int fd);

/*******************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers */
/* at least NUMREGBYTES*2 are needed for register packets                      */
#if ((BUFMAX) < (2 * ( 16*4 + 8*12 + 4 + 4)) )
#error "value of BUFMAX to small"
#endif

char	debugCharReadCache[BUFMAX];
int	debugCharReadCachePos = 0;
int	debugCharReadCacheLen = 0;


/* 
 * write a single character      
 */
static int putDebugChar (int fd, int c)
{
	int val = c;
	int res;
	struct pollfd fds[1];
	int poll_timeout = 0;
	
	fds[0].fd = fd;
	fds[0].events = POLLOUT;

	do {
		if(CALLBACK_EXSIST)
			poll_timeout = 0;	// Do Callback if no yet able to send
		else
			poll_timeout = 100;	// No Callback exists; Do Bussy Wait every 100 msec
		/*check if we can send more data*/
		res = poll(fds, 1, poll_timeout);
		if(res > 0)
			break;
		else if (res < 0)
			return -1;
			
		/*send buffer is full so let's do some bottom half threads*/
		res = doCallback();

		if(res < 0)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panik: Callback function exit with %d\n", res);
			exit(0);
		}			
	}while(1) ;
	
	write (fd, &val, 1);  
	return 1;
}

/*
 *  read and return a single char 
 */
static int getDebugChar (int fd)	
{
	char c;
	int res;
	struct pollfd fds[1];
	int cnt = MAX_READ_RETRY;
	int poll_timeout = 0;
	
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	if(debugCharReadCacheLen > 0)
	{
		c = debugCharReadCache[debugCharReadCachePos];
		debugCharReadCachePos++;
		debugCharReadCacheLen--;
		return c & 0xff;
	}
	
	while (cnt--)
	{
		do {
			if(CALLBACK_EXSIST)
				poll_timeout = 0;	// Do Callback if no new Data arrived
			else
				poll_timeout = 100;	// No Callback exists; Do Bussy Wait every 100 msec
			/*check if new data arrived*/
			res = poll(fds, 1, poll_timeout);
			if(res > 0)
				break;
			else if (res < 0)
				return -1;
			
			/*there is no data so let's do some bottom half threads*/
			res = doCallback();

			if(res < 0)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panik: Callback function exit with %d\n", res);
				exit(0);
			}
				
		}while(1) ;
		
		res = read (fd, debugCharReadCache, BUFMAX);
		if (res < 0)
		{
			if (errno == EAGAIN)     /* fd was set to non-blocking and no data was available */
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"No data and/or wrong Mode(non-blocking)\n");
				return -1;
			}
			if(errno == ECONNRESET ) /*connection reset by peer*/
				return -1;
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"read failed:%d %s\n",errno ,strerror(errno));
			exit(EX_IOERR);
		}
		else if (res == 0)
		{
			//dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"incomplete read\n");
			continue;
		}
		else // res > 0
		{
			c = debugCharReadCache[0];
			debugCharReadCachePos = 1;
			debugCharReadCacheLen = res - 1;			
		}
		return c & 0xff;
	}
	dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Connection closed by client\n");
	return -1;
}

/*
 * simple method to convert a Hex-Nibble to its ASCII char value
 */
static const char hexchars[]="0123456789abcdef";

/*
 * convert single hexadecimal encoded ASCII char to its intger value
 */
static int hex(char ch)
{
  if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
  if ((ch >= '0') && (ch <= '9')) return (ch-'0');
  if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
  return (-1);
}

/* 
 * scan for the sequence $<data>#<checksum>     
 */
static void getpacket (int fd, char *buffer)
{
	unsigned char checksum;
	unsigned char xmitcsum;
	int i;
	int count;
	int ch;
	do
	{      
		/* wait around for the start character, ignore all other characters */
		do 
		{
			ch = getDebugChar(fd);
			if(ch == 0x03 || ch == 0x04) // gdb send interrupt signal c-c
			{
				buffer[0] = 0x03; //etx
				buffer[1] = 0;
				return;
			}
			else if(ch < 0) // gdb closed connection
			{
interrupt_exit:			buffer[0] = 0x04; //eot -- hoping that gdb does not send this
				buffer[1] = 0;
				return;
			}
		} while (ch  != '$');
		checksum = 0;
		xmitcsum = -1;
		count = 0;
		
		/* now, read until a # or end of buffer is found */
		while (1)
		{
			ch = getDebugChar(fd);
			if(ch == 0x7d) // Escape char '}' ignore #
			{
				checksum = checksum + ch;
				ch = getDebugChar(fd);
				checksum = checksum + ch;
				ch ^= 0x20;
				if (ch < 0)
					goto interrupt_exit;
			}
			else if (ch == '#')
				break;
			else if (ch < 0)
				goto interrupt_exit;
			else
				checksum = checksum + ch;
			buffer[count] = ch;
			if(count < BUFMAX - 1)
				count ++;
		}
		buffer[count] = 0;
		
		if((ch = getDebugChar(fd)) < 0) goto interrupt_exit;
		xmitcsum = hex(ch) << 4;
		if((ch = getDebugChar(fd)) < 0) goto interrupt_exit;
		xmitcsum += hex(ch);
		if (checksum != xmitcsum)
			putDebugChar(fd,'-');  /* failed checksum */
		else
		{
			putDebugChar(fd,'+'); /* successful transfer */
			/* if a sequence char is present, reply the sequence ID */
			if (buffer[2] == ':')
			{
				putDebugChar(fd,buffer[0]);
				putDebugChar(fd,buffer[1]);
				/* remove sequence chars from buffer */
	      			count = strlen (buffer);
	      			for (i=3; i <= count; i++)
		    			buffer[i-3] = buffer[i];
			}
		}	
    
	}  
	while (checksum != xmitcsum);
}

/* 
 * send the packet in buffer.  
 */
static void putpacket (int fd, char *buffer)
{
	unsigned char checksum;
	int count;  
	unsigned char ch;
	int ret_c;
	char send_buf[BUFMAX + 2 + 2 + 1]; // +"$" +"#" + 2 char "chksum" +"eos"
	char *cp;
	int res;
	struct pollfd fds[1];
	int poll_timeout = 0;
	
	fds[0].fd = fd;
	fds[0].events = POLLOUT;

	/*  $<packet info>#<checksum>. */  
	do    
	{
		cp = send_buf;
		*cp++ = '$';
		checksum = 0;
		count = 0;      
		
		while ((ch=buffer[count]) != 0)
		{
			*cp++ = ch;	  
			checksum += ch;
			count ++;
			if(count >= BUFMAX)
				break;
		}      
		*cp++ = '#';
		*cp++ = hexchars[checksum >> 4];
		*cp++ = hexchars[checksum & 0xf];
		*cp   = 0;
		count += 4;

		do {
			if(CALLBACK_EXSIST)
				poll_timeout = 0;	// Do Callback if no yet able to send
			else
				poll_timeout = 100;	// No Callback exists; Do Bussy Wait every 100 msec
			/*check if we can send more data*/
			res = poll(fds, 1, poll_timeout);
			if(res > 0)
				break;
			else if (res < 0) // receiving signal EINTR
				return ;
			
			/*send buffer is full so let's do some bottom half threads*/
			res = doCallback();

			if(res < 0)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panik: Callback function exit with %d\n", res);
				exit(0);
			}			
		}while(1) ;
		
		write (fd,send_buf,count);
		ret_c = getDebugChar (fd);
	} while ((ret_c & 0x7f) != '+' && ret_c >= 0);
	return;
}

/* 
 * convert the memory pointed to by mem into hex, placing result in buf 
 * return a pointer to the last char put in buf (null) 
 */
char * mem2hex (char *mem, char *buf, int count)
{
	int i;      
	int ch;      
	
	for (i=0;i<count;i++)       
	{
		ch = *mem++;
		*buf++ = hexchars[(ch >> 4) & 0xF];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	return(buf);
}

/* 
 * convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte written 
 */
char * hex2mem (char *buf, char * mem, int count)
{
	int i;
	int ch;
	
	if(count > BUFMAX/2) // try to prevent buffer overrun
		count = BUFMAX/2;
	
	for (i=0;i<count;i++)
	{
		ch = hex(*buf++) << 4;
		ch = ch + hex(*buf++);
		*mem++ = ch;
	}
	return(buf);
}

/*
 * copy count bytes of memory from buf to mem.
 *
 * note: using
 *	memcpy(mem, buf, count);
 *	would be a better solution.
 */
static char * bin2mem(char *buf, char * mem, int count)
{
	int i;
	
	for (i=0;i<count;i++)
	{
		*mem++ = *buf++;
	}
	return(buf);
}


/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 * (a call of 
 *    val = strtol(string, &string, 16);
 *  might do the same.
 *  except we don't know how many char's we're visiting)
 */
int hexToInt(char **ptr, int *intValue)
{
	int numChars = 0;
	int hexValue;
	*intValue = 0;
	
	while (**ptr && numChars < 8)
	{
		hexValue = hex(**ptr);
		if (hexValue >=0)
		{
			*intValue = (*intValue <<4) | hexValue;
			numChars ++;
		}
		else
			break;
		(*ptr)++;
	}
	return (numChars);
}


/* 
 * this function takes the exception vector and attempts to translate this 
 * number into a unix compatible signal value.
 * (Well, but we don't have an exception vector so we do something similare.)
 */
static int computeSignal (int exceptionVector)
{
	int sigval;
	
	switch (exceptionVector & 0xFF)
	{
		case 0: sigval = TARGET_SIGNAL_FPE;  break; /*  8 divide by zero or coprocessor not available */
		case 1: sigval = TARGET_SIGNAL_TRAP; break; /*  5 debug exception or breakpoint */
		case 2: sigval = TARGET_SIGNAL_ILL;  break; /*  4 Invalid opcode */
		case 3: sigval = TARGET_SIGNAL_SEGV; break; /* 11 double fault - Segmentation fault*/
		case 4: sigval = TARGET_SIGNAL_BUS;  break; /* 10 coprocessor segment overrun - Bus error*/
		case 5: sigval = TARGET_SIGNAL_ABRT; break; /*  6 Aborted */
		case 6: sigval = TARGET_SIGNAL_KILL; break; /*  9 Killed */
		case 7: sigval = TARGET_SIGNAL_INT;  break; /*  2 Interrupt*/
		case 8: sigval = TARGET_SIGNAL_QUIT; break; /*  3 Quit*/
		case 10:sigval = TARGET_SIGNAL_EMT;  break; /*  7 software generated - Emulation trap*/
		case 11:sigval = TARGET_SIGNAL_TSTP; break; /* 18 Stop signal*/
		default:
			sigval = TARGET_SIGNAL_0;
	}
	return (sigval);
}


/* 
 * reply to host that an exception has occurred 
 */
static int gdb_reply_exception (int fd, struct reg_set *raw_regs, int type)
{
	int    sigval;
	char * ptr;
	char   outBuffer[BUFMAX];
	
	sigval = computeSignal (type);
	
	ptr = outBuffer;
	*ptr++ = 'T';
	*ptr++ = hexchars[sigval >> 4];
	*ptr++ = hexchars[sigval & 0xf];
		
	if(raw_regs->regs.r[GDB_REG_POS_PC] & 1) // Thumb Frame pointer
	{
		*ptr++ = hexchars[GDB_REG_POS_THUMB_FP >> 4];
		*ptr++ = hexchars[GDB_REG_POS_THUMB_FP & 0xf];
		*ptr++ = ':';
		ptr = mem2hex ((char *)&raw_regs->regs.r[GDB_REG_POS_THUMB_FP], ptr, 4);
		*ptr++ = ';';
	}
	else // ARM Frame pointer
	{
		*ptr++ = hexchars[GDB_REG_POS_ARM_FP >> 4];
		*ptr++ = hexchars[GDB_REG_POS_ARM_FP & 0xf];
		*ptr++ = ':';
		ptr = mem2hex ((char *)&raw_regs->regs.r[GDB_REG_POS_ARM_FP], ptr, 4);
		*ptr++ = ';';
	}
	
	*ptr++ = hexchars[GDB_REG_POS_SP >> 4];
	*ptr++ = hexchars[GDB_REG_POS_SP & 0xf];
	*ptr++ = ':';
	ptr = mem2hex ((char *)&raw_regs->regs.r[GDB_REG_POS_SP], ptr, 4);
	*ptr++ = ';';
	
	*ptr++ = hexchars[GDB_REG_POS_LR >> 4];
	*ptr++ = hexchars[GDB_REG_POS_LR & 0xf];
	*ptr++ = ':';
	ptr = mem2hex ((char *)&raw_regs->regs.r[GDB_REG_POS_LR], ptr, 4);
	*ptr++ = ';';

	*ptr++ = hexchars[GDB_REG_POS_PC >> 4];
	*ptr++ = hexchars[GDB_REG_POS_PC & 0xf];
	*ptr++ = ':';
	ptr = mem2hex ((char *)&raw_regs->regs.r[GDB_REG_POS_PC], ptr, 4);
	*ptr++ = ';';
	
	*ptr++ = hexchars[GDB_REG_POS_PS >> 4];
	*ptr++ = hexchars[GDB_REG_POS_PS & 0xf];
	*ptr++ = ':';
	ptr = mem2hex ((char *)&raw_regs->CPSR, ptr, 4);
	*ptr++ = ';';
	
	*ptr = 0;
	
	putpacket (fd, outBuffer);
	return sigval;
}

/*
 * send a message to the console while doing a rcmd
 */
void gdb_rcmd_console_output(int fd, char *mem)
{

	int  cnt;  
	char val;
	char *cptr;
	char buffer[BUFMAX]; 
	
	cptr = buffer;
	*cptr++ = 'O';
	for(cnt=0; cnt < BUFMAX-2; cnt+=2) // remember 'O' and terminating '\0'
	{
		val = *mem;
		if(val == 0)
			break;
		*cptr++ = hexchars[(val>>4) & 0xf];
		*cptr++ = hexchars[val & 0xf];
	}

	*cptr = 0;
	putpacket (fd, buffer);
	return;
}

/*
 * Write data to GDB's console
 * -- this is only posible when we are performing a continue request.
 * -- but we don't like to do this at the real target.
 * -- so this is a fake.
 */
static int gdb_handle_fake_continue (int fd, struct reg_set *raw_regs, int type)
{
	char   outBuffer[BUFMAX];
	char   inBuffer[BUFMAX];
	struct gdbSprintfBuf *console_buf;
	int    is_break, res;
	int    base_adr, data_len, idx;
	int    addr, length;
	char * ptr;

	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"doing fake continue\n");
	packGdbSprintBuf(); // reduce number of F-calls
	console_buf = deQueueGdbSprintfBuf();
	/*is there anything to write to the console*/
	while(
		(memMapContainer.memStat.pending & ACTION_INPROCESS ) == ACTION_INPROCESS 
		|| console_buf != NULL
	     )
	{
		is_break = 0;
		if(console_buf != NULL && console_buf->len > 0)
		{
			/*(send write command) -> Fwrite,STDOUT,BASE_ARD,LEN*/
			base_adr = (int)console_buf->string;
			data_len = console_buf->len;
			if(data_len > BUFMAX/2)
				data_len = BUFMAX/2;
			idx = 0;
			snprintf(outBuffer, BUFMAX,"Fwrite,1,%x,%x", base_adr, data_len);
			
			putpacket (fd, outBuffer);
			
			do {
				/*fill default reply as empty string -> ""*/
				outBuffer[0] = 0;
				
				/*gather request from host*/
				getpacket (fd, inBuffer);
		
				/*handle host request*/
				switch (inBuffer[0])
				{
				case 0x03:		/* c-c signal */
					/*remove any pending callback function*/
					removeAllCallbacks();
					if((memMapContainer.memStat.pending & ACTION_INPROCESS ) == ACTION_INPROCESS)
					{
						memMapContainer.memStat.pending =  ACTION_PROGRAM_FAIL;
						gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
					}
					// make sure that we break the do-loop too and that we do not reponse to this request
					data_len = 0;
					is_break = 1;
					type = 7; // Interrupt ??
					break;
				case 0x04:		/* (self generated)gdb close signal */
					return -1;	// we can't anything more since we lost connection to our host
					
				case 'm':		/* (receive data request) <- mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
					ptr = &inBuffer[1];
					
					/*collect addr and length from inBuffer string*/
					if (hexToInt (&ptr, &addr) && *(ptr++) == ',' && hexToInt (&ptr, &length))
					{
						/* Try to read at addr, length bytes.  */
						if (length <= BUFMAX/2 && length <= data_len && addr == base_adr)
						{
							mem2hex(&(console_buf->string[idx]) , outBuffer, length);
							/*calculate remainder*/
							idx += length;
							data_len -= length;
							base_adr += length;
						}
						else
							strcpy (outBuffer, "E02");
						break;
					}
					else
						strcpy (outBuffer, "E01");
					break;
				default: /*unsupported*/
					break;
				}
				/*(send data) -> XXXXXX *** reply to the request */
				putpacket (fd, outBuffer);

				/*let's do some bottom half threads (might be done in putpacket and getpacket too)*/
				res = doCallback();

				if(res < 0)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panik: Callback function exit with %d\n", res);
					exit(0);
				}

			} while(data_len > 0); // every thing written ? 
			
			if(!is_break)
			{
				/*gather written bytes from host*/
				getpacket (fd, inBuffer);
				if(inBuffer[0] != 'F')
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"wrong answer\n");
			}
			
		}
		
		/*free buffer*/
		if(console_buf != NULL)
			freeGdbSprintfBuf(console_buf);
		
		if(is_break)
			break;
		else
		{
			/*let's do some bottom half threads (might be done in putpacket and getpacket too)*/
			res = doCallback();

			if(res < 0)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panik: Callback function exit with %d\n", res);
				exit(0);
			}
		}

		packGdbSprintBuf(); // reduce number of F-calls
		/*next string message for gdb's stderr*/
		console_buf = deQueueGdbSprintfBuf();
	}

	/*End of Fake -> signal exception*/
	return gdb_reply_exception (fd, raw_regs, type);
}

/*
 * This function does all command procesing for interfacing with gdb.
 *
 * returns -2 indicates to exit this program (kill) - the target program stops
 *         -1 indicates to leave this program (detach) - the target program still running
 *          0 indicates to continue target procedure
 *          1 indicates a single step operation
 */
static int gdb_handle_exception (int fd, struct reg_set *raw_regs, int type)
{
	int    sigval;
	int    addr, length;
	char * ptr;
	char   outBuffer[BUFMAX];
	char   inBuffer[BUFMAX];
	int    regno;
	int    i,ret_val;

	/* reply to host that an exception has occurred */
	if (type >= 0)
		sigval = gdb_reply_exception (fd, raw_regs, type);
	else
		sigval = computeSignal (1);
	
	/*gdb serial protocol loop*/
	while (1)
	{
		/*fill default reply as empty string -> ""*/
		outBuffer[0] = 0;
		
		/*gather request from host*/
		getpacket (fd, inBuffer);
		
		/*handle host request*/
		switch (inBuffer[0])
		{
		case 0x03:		/* c-c signal */
			/*remove any pending callback function*/
			removeAllCallbacks();
			if(memMapContainer.memStat.pending == ACTION_ERASE_INPROCESS)
			{
				memMapContainer.memStat.pending =  ACTION_PROGRAM_FAIL;
				gettimeofday(&(memMapContainer.memStat.stopTime), NULL);
			}
			break;
		case 0x04:		/* gdb close signal */
			return -1;
		case '!':		/* set into advanced mode */
			strcpy (outBuffer, "OK");
			break;
		case '?':		/*request signal number so send it */
			outBuffer[0] = 'S';
			outBuffer[1] = hexchars[sigval >> 4];
			outBuffer[2] = hexchars[sigval % 16];
			outBuffer[3] = 0;
			break;
		case 'D':		/* detach; say OK and turn off gdb */
			strcpy (outBuffer, "OK");
			putpacket(fd, outBuffer);
			return -2;
		case 'k':		/* kill */
			// reset all register values
			bzero(raw_regs,sizeof(struct reg_set));
			raw_regs->CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
			/* write back modifid RAM Memory*/
			gdb_writeback_Ram();
			//peer will close the connection
			return -1;
		case 'g':		/* return the value of the CPU registers */
			mem2hex ((char *)raw_regs, outBuffer, sizeof(struct reg_set));
			break;
		case 'G':		/* set the value of the CPU registers - return OK */
			hex2mem (&inBuffer[1], (char *)raw_regs, sizeof(struct reg_set));
			strcpy (outBuffer, "OK");
			break;
		case 'p':		/* get the value of one register */
			ptr = &inBuffer[1];
			if (hexToInt (&ptr, &regno) && regno < NUM_REGS)
			{
				if(regno <= 15) /*r0 .. r15*/
					mem2hex ((char *) &raw_regs->regs.r[regno], outBuffer, sizeof(uint32_t));
				else if(regno <= 23) /*f0 .. f7*/
					mem2hex ((char *) &raw_regs->f[regno-16], outBuffer, sizeof(struct fp_reg));
				else if(regno == 24) /*fps*/
					mem2hex ((char *) &raw_regs->fps, outBuffer, sizeof(uint32_t));
				else /*if(regno == 25)*/ /*CPSR*/
					mem2hex ((char *) &raw_regs->CPSR, outBuffer, sizeof(uint32_t));
			}
			else
				strcpy (outBuffer, "E01");
			break;
		case 'P':		/* Set the value of one register */
			ptr = &inBuffer[1];
			
			if (hexToInt (&ptr, &regno) && *ptr++ == '=' && regno < NUM_REGS)
			{
				if(regno <= GDB_REG_POS_PC)
					hex2mem (ptr, (char *)&raw_regs->regs.r[regno], 4);
				else if(regno == GDB_REG_POS_PS)
				{
					hex2mem (ptr, (char *)&raw_regs->CPSR, 4);
					raw_regs->CPSR &= 0xf80000FF; // only flags [31-27] and control [7-0]
				}
				else
				{
					/*floating point stuff not yet supported*/
					strcpy (outBuffer, "E02");
					break;
				}
				strcpy(outBuffer,"OK");
			}
			else
				strcpy (outBuffer, "E01");
			break;
		case 'm':		/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
			ptr = &inBuffer[1];
			
			/*collect addr and length from inBuffer string*/
			if (hexToInt (&ptr, &addr) && *(ptr++) == ',' && hexToInt (&ptr, &length))
			{
				int *mem_val;

				/* Try to read at addr, length bytes.  */
				if (length <= BUFMAX/2 && (mem_val = gdb_read_mem(addr, length)) != NULL)
					mem2hex( (char *) mem_val, outBuffer, length);
				else
					strcpy (outBuffer, "E03");
				break;
			}
			else
				strcpy (outBuffer, "E01");
			break;
		case 'M':		/* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			ptr = &inBuffer[1];
			
			/*collect addr and length from inBuffer string*/
			if (hexToInt(&ptr,&addr) && *(ptr++) == ',' && hexToInt(&ptr, &length) && *(ptr++) == ':')
			{
				int *mem_val;
#ifdef WITH_ALLOCA
				/* Stack allocation */
				mem_val = alloca(length);
#else
				/* Heap allocation */
				mem_val = malloc(length);
#endif
				/* Try to write (into chache buffer) at addr, length bytes  */
				if ( mem_val != NULL 
				  && hex2mem(ptr, (char *)mem_val, length) == NULL
				  && gdb_write_mem(mem_val, addr, length) == 0 )
				{
					strcpy (outBuffer, "OK");
				}
				else
					strcpy (outBuffer, "E01");
#ifndef WITH_ALLOCA
				if(mem_val != NULL)
					free(mem_val);
#endif
			}
			else
				strcpy (outBuffer, "E02");
			break;
		case 'X':		/* XAA..AA,LLLL: Write binary bytes at address AA.AA return OK */
			ptr = &inBuffer[1];
			
			/*collect addr and length from inBuffer string*/
			if (hexToInt(&ptr,&addr) && *(ptr++) == ',' && hexToInt(&ptr, &length) && *(ptr++) == ':')
			{
				int *mem_val;
#ifdef WITH_ALLOCA
				/* Stack allocation */
				mem_val = alloca(length);
#else
				/* Heap allocation */
				mem_val = malloc(length);
#endif

				/* Try to write (into chache buffer) at addr, length bytes  */
				if ( mem_val != NULL 
				  && bin2mem(ptr, (char *) mem_val, length) != NULL
				  && gdb_write_mem(mem_val, addr, length) == 0 )
				{
					strcpy (outBuffer, "OK");
				}
				else
					strcpy (outBuffer, "E01");
#ifndef WITH_ALLOCA
				if(mem_val != NULL)
					free(mem_val);
#endif
			}
			else
				strcpy (outBuffer, "E02");
			break;
		case 'C':
		case 'S':		/*SNN;AA..AA*/
			ptr = &inBuffer[1];
			while(*ptr != 0) // skip SIGNAL number NN
			{
				if(*ptr == ';'|| *ptr == ':' )
				{
					ptr++;
					break;
				}
				ptr++;
			}
			goto CS_common;
		case 'c' :		/* cAA..AA    Continue at address AA..AA(optional) */
		case 's' :		/* sAA..AA   Step one instruction from AA..AA(optional) */
			ptr = &inBuffer[1];
CS_common:		
			/*make sure the desired temporary startup break is set*/
			if(symbolMain.breakIsActive && symbolMain.state == SYM_PRESENT)
			{
				if(symbolMain.isThumb)
					ret_val = InsertBreakpoint(symbolMain.addr, 2, 0);
				else
					ret_val = InsertBreakpoint(symbolMain.addr, 4, 0);
				if(ret_val > 0)
				{
					strcpy (outBuffer, "E03");
					break;
				}
			}

			/*check if we like to perfome a console IO instead*/
			if(fake_continue_mode)
			{
				type = 11; /*response with TSTP(or emulation trap?) when finish*/
				sigval = gdb_handle_fake_continue(fd, raw_regs, type);
				if(sigval < 0) // lost connection ?
					return -1;
				continue; // we have done the response already
			}
			
			/* try to read optional parameter, pc unchanged if no parm */
			if (hexToInt(&ptr,&addr))
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"change pc\n");
				raw_regs->regs.name.pc = addr;
			}
			
			/* write back modifid RAM Memory*/
			gdb_writeback_Ram();
			
			/* tell if we're stepping */
			if (inBuffer[0] == 's' || inBuffer[0] == 'S')
				return 1; // -> action = step
			return 0; // -> action = continue
		case 'q':		/*e.g. qRcmd,c..c    query remote command c..c */
			gdb_query(fd, &inBuffer[1], outBuffer, raw_regs);
			break;
		case 'Q':		/* Qv..v=V..V    Set value of general var v..v to V..V  */
			break;
		case 'B':		/* BA..A,[SC] deprecated version to insert/remove breakpoints*/
			ptr = &inBuffer[1];
			/*collect addr from inBuffer string*/
			if (hexToInt (&ptr, &addr) && *(ptr++) == ',')
			{
				i = hex(*ptr);
				if(i == 'S')
				{
					if(addr & 0x1uL) // not sure if gdb send it this way
						ret_val = InsertBreakpoint(addr, 2, 0);
					else
						ret_val = InsertBreakpoint(addr, 4, 0);
				}
				else if (i == 'C')
				{
					if(addr & 0x1uL) // not sure if gdb send it this way
						ret_val = RemoveBreakpoint(addr, 2, 0);
					else
						ret_val = RemoveBreakpoint(addr, 4, 0);
				}
				else
					ret_val = 1;
				
				if(ret_val == 0)
					strcpy (outBuffer, "OK");
				else
					strcpy (outBuffer, "E01");
				
			}
			break;
		case 'z':		/* z0,A..A,LLLL  remove memory breakpoint */
			 		/* z1,A..A,LLLL  remove hardware breakpoint */
					/* z2,A..A,LLLL  remove write watchpoint */
					/* z3,A..A,LLLL  remove read watchpoint */
					/* z4,A..A,LLLL  remove read/write watchpoint */
			if(inBuffer[1] == 0 || inBuffer[2] != ',' )
			{
				strcpy (outBuffer, "E01");
				break;
			}
			i = hex(inBuffer[1]);
			ptr = &inBuffer[3];
			/*collect addr and length from inBuffer string*/
			if (hexToInt (&ptr, &addr) && *(ptr++) == ',' && hexToInt (&ptr, &length))
			{
				if(i>=0 && i<=1)
					ret_val = RemoveBreakpoint(addr, length, 0);
				else if(i>=2 && i<=4)
					ret_val = RemoveBreakpoint(addr, length, i-1);
				else
					ret_val = 1; //default error "E03"
				
				if(ret_val > 0)
					strcpy (outBuffer, "E03");
				else if(ret_val == 0)
					strcpy (outBuffer, "OK");
				// else (ret_val < 0) -> default ""
			}
			else
				strcpy (outBuffer, "E02");
			break;		
		case 'Z':		/* Z0,A..A,LLLL  insert memory breakpoint */
					/* Z1,A..A,LLLL  insert hardware breakpoint */
					/* Z2,A..A,LLLL  insert write watchpoint */
					/* Z3,A..A,LLLL  insert read watchpoint */
					/* Z4,A..A,LLLL  insert read/write watchpoint */
			if(inBuffer[1] == 0 || inBuffer[2] != ',' )
			{
				strcpy (outBuffer, "E01");
				break;
			}
			i = hex(inBuffer[1]);
			ptr = &inBuffer[3];
			/*collect addr and length from inBuffer string*/
			if (hexToInt (&ptr, &addr) && *(ptr++) == ',' && hexToInt (&ptr, &length))
			{
				if(i>=0 && i<=1)
					ret_val = InsertBreakpoint(addr, length, 0);
				else if(i>=2 && i<=4)
					ret_val = InsertBreakpoint(addr, length, i-1);
				else
					ret_val = 2; //default error
				
				if(ret_val > 0)
					strcpy (outBuffer, "E03");
				else if(ret_val == 0)
					strcpy (outBuffer, "OK");
				else if(ret_val == 1)
				{
					// we must tell gdb OK even if this is wrong
					// but we should stop doing anything
					wrongbreakpoint = 1;
					strcpy (outBuffer, "OK");
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"GDB break at 0x%X not possible\n",addr);
				}
				// else (ret_val < 0) -> default ""
			}
			else
				strcpy (outBuffer, "E02");
			break;
		default:
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"GDB command %c not supported\n",inBuffer[0]);
		} /* switch */
		
		/* reply to the request */
		putpacket (fd, outBuffer);
	}
	return -1;
}

#ifndef WITHOUT_ENDIAN
/*
 *
 */
static void gdb_check_endian_at_reset_vector(void)
{
	uint32_t tst_word;
	uint8_t tst_byte[4];
	int i, maybe_LE, maybe_BE;

	tst_word = jtag_arm_ReadWord(0);
       	for(i=0;i<4;i++)
		tst_byte[i] = jtag_arm_ReadByte(i);

	/*can this be little endian*/
	if(
	       (tst_word                    & 0xFF) == tst_byte[0]
	  && (((tst_word &     0xFF00)>>8)  & 0xFF) == tst_byte[1]
	  && (((tst_word &   0xFF0000)>>16) & 0xFF) == tst_byte[2]
	  && (((tst_word & 0xFF000000)>>24) & 0xFF) == tst_byte[3]
	  )
		maybe_LE = 1;
	else
		maybe_LE = 0;
	
	/*can this be big endian*/
	if(
	       (tst_word                    & 0xFF) == tst_byte[3]
	  && (((tst_word &     0xFF00)>>8)  & 0xFF) == tst_byte[2]
	  && (((tst_word &   0xFF0000)>>16) & 0xFF) == tst_byte[1]
	  && (((tst_word & 0xFF000000)>>24) & 0xFF) == tst_byte[0]
	  )
		maybe_BE = 1;
	else
		maybe_BE = 0;

	if(maybe_LE && maybe_BE)
		arm_info.bigend = 2; //still unkown since both fits
	else if(maybe_LE && !maybe_BE)
		arm_info.bigend = 0; //little endian
	else if(!maybe_LE && maybe_BE)
		arm_info.bigend = 1; //big endian
	else
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Unexpected result broken device\n");
		exit(EX_UNAVAILABLE);
	}
	return;
}
#endif

/*
 *
 */
static void gdb_check_thumb_support(void)
{
	struct reg_set sav_regs;
	int i;
	
	/*save current CPU state*/
	for(i=15; i>=0;i--)
		sav_regs.regs.r[i] = CPU.regs.r[i];
	sav_regs.CPSR = CPU.CPSR;
	
	CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
	CPU.regs.r[0] = 65;
	jtag_arm_Step(ARM_BX_R0);
	while(jtag_arm_PollDbgState() == 0) 
		;
	jtag_arm_ReadCpuRegs(1); // (0)
	if((CPU.CPSR & 0xff)== 0xf3 && CPU.regs.r[15] == 64)
	{
		arm_info.has_thumb = 1;
	}

	/*resore original CPU state*/
	for(i=15; i>=0;i--)
		CPU.regs.r[i] = sav_regs.regs.r[i];
	CPU.CPSR = sav_regs.CPSR;
	return;
}


/*
 *
 */
int gdb_restart (void)
{
	static int restart = 0;
	
	if(restart == 0) // this is the first real start -- setup JTAG
	{
		jtag_start();
		if(jtag_arm_verify() == 0)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Unknown JTAG Device\n");
			exit(EX_UNAVAILABLE);
		}
	}
	else
	{
		jtag_hard_reset();	// grant SRESET if present
		jtag_reset();		// reset TAP controller
		jtag_eos();		// enter RUN/IDLE state
	}
	/*prepare to enter debug state*/
	jtag_arm_PutAnyBreakPoint();

	/*wait until debugger enters debug state*/
	while(jtag_arm_PollDbgState() == 0) 
		;

	//jtag_arm_ShowAllIceRT_Regs();

	if(restart == 0) // this is the first real start -- setup CPU
	{
		restart = 1;
		return 0;
		
	}
	return 1;
}


/*
 *
 */
int gdb_main_loop (int fd)
{
	struct reg_set raw_regs;
	int action, i, fd_flag;
	int type = -1;
	struct pollfd fds[1];
	int restart = 0;
	int poll_cnt;

	fd_flag = fcntl (fd, F_GETFL, 0);
	fcntl (fd, F_SETFL, fd_flag | O_NONBLOCK);
	
	bzero(&raw_regs,sizeof(struct reg_set));
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	
	/*setup gdb-server defaults*/
	allow_intr_in_step_mode = 0;
	ice_state.high_speed_mclk = 0;
	
	restart = gdb_restart();
	if(restart == 0) // this is the first real start -- setup CPU
	{
		restart = 1;
		
		/*we collecting some system info*/
		jtag_arm_IceRT_version();
		jtag_arm_ReadCpuRegs(1); // (0)
		jtag_arm_DumpCPUregs();
		jtag_arm_ReadCP15();
#ifndef WITHOUT_ENDIAN
		gdb_check_endian_at_reset_vector();
#endif
		gdb_check_thumb_support();
		/*read out the current CPU registers, but we are going to ignore them*/
		jtag_arm_ReadCpuRegs(1); // (0)
		
		/*setup CPU registers default*/
		bzero(&CPU,sizeof(struct reg_set));
	
		CPU.CPSR = 0xd3; // IRQ and FRQ disabled; Supervisor mode; ARM - State
	}
	else
	{
		/*collect first CPU context after restart*/
		jtag_arm_ReadCpuRegs(1); // (0)
		arm_sfa_StopPeriphery();
	}

	/*debug main loop*/
	do {
		jtag_arm_ShowAllIceRT_Regs();
		jtag_arm_DumpCPUregs();

		for(i=15; i>=0;i--)
			raw_regs.regs.r[i] = CPU.regs.r[i];
		raw_regs.CPSR = CPU.CPSR & 0xf80000ff;

		/*remove temp. Main Break at first hit*/
		if(symbolMain.breakIsActive && symbolMain.state == SYM_PRESENT && raw_regs.regs.r[15] == symbolMain.addr)
		{
			symbolMain.breakIsActive = 0;
			if(symbolMain.isThumb)
				RemoveBreakpoint(symbolMain.addr, 2, 0);
			else
				RemoveBreakpoint(symbolMain.addr, 4, 0);
		}
		
		/*gdb's internal hack of thumb mode detection within the disassembler*/	
		if((CPU.CPSR & 0x20L) == 0x20L ) // THUMB mode
			raw_regs.regs.r[15] |= 1uL;
		
		do {
			wrongbreakpoint = 0;
			action = gdb_handle_exception (fd, &raw_regs, type);
		}while(wrongbreakpoint && action >= 0);

		/*restore registers R0-R15 from raw_regs to CPU*/
		for(i=15; i>=0;i--)
			CPU.regs.r[i] = raw_regs.regs.r[i];

		/*restore register CPSR from raw_regs to CPU*/
		if( (CPU.CPSR & 0xff) != (raw_regs.CPSR & 0xff))
			printf("Change control bits from 0x%02X to 0x%02X\n",CPU.CPSR & 0xff,raw_regs.CPSR & 0xff);
		if( (CPU.CPSR & 0xf8000000) != (raw_regs.CPSR & 0xf8000000))
			printf("Change Flags (NZCVQ)from 0x%02X to 0x%02X\n"
				,((CPU.CPSR & 0xf8000000)>>27)&0xFF
				,((raw_regs.CPSR & 0xff)>>27)&0xFF);
		CPU.CPSR = raw_regs.CPSR & 0xf80000ff;

		/*remove any thumb hack stuff from the PC*/
		if((CPU.CPSR & 0x20L) == 0x20L ) // THUMB mode (half word align)
			CPU.regs.r[15] &= 0xFFFFfffeuL;
		else // ARM mode (word align)
			CPU.regs.r[15] &= 0xFFFFfffcuL;

		/*perform the requested action*/
		if(action < 0) // Detach or kill
			break;
		else if(action == 0) // -> continue
		{
			/* while PC is at any of the breakpoint list -- single step instead (this is a temorary hack)*/
			if(isAddrOnBreakpointList(CPU.regs.r[15]))
				action = 1;
			else
			{
				/*make sure the Softbreak instr. are in Memory*/
				gdbWriteSoftwareBreakToRAM();
				/* if no more Hardbreakpoints required*/
				arm_sfa_StartPeriphery();
				jtag_arm_PrepareExitDebug();
				jtag_arm_ClearAnyBreakPoint();
				/* setup requested Breakpoint*/
				gdbSetupJtagICE_RTbreakpoint();
				
				//jtag_arm_ExitDebug();
				jtag_arm_FinalExitDebug();

				gdb_invalidate_Ram_Buffer();
			}
		}
		if(action == 1) // -> single step
		{
			uint32_t step_instr;
			int instr_modify_mem;
			
			if((CPU.CPSR & 0x20L) == 0x20L ) // THUMB mode
			{
				/*collect the step instruction*/
				step_instr = gdbLockupThumbInstr(CPU.regs.r[15]);
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"0x%8.8X Thumb Step\n",step_instr);

				/* 
				 * BL and BLX have both sequences of two Thumb instructions which can't be
				 * handled within one single step. So we are going to emulate the result instead.
				 */
				if( (step_instr & THUMB_BL_MSK) == THUMB_BL_FIRST_PART )
				{
					uint32_t offset;

					offset = step_instr & THUMB_BL_OFFSET_MSK;
					offset <<= 12;
					
					/*collect the second part of the BL-instruction*/
					step_instr = gdbLockupThumbInstr(CPU.regs.r[15] + 2);
					if( (step_instr & THUMB_BL_MSK) == THUMB_BL_SECOND_PART )
					{
						offset |= (step_instr & THUMB_BL_OFFSET_MSK)<<1;

						CPU.regs.r[14] = (CPU.regs.r[15] + 4) | 1; // LR <- addr of next instr.
						// PC <- PC + 4 +/- offset
						if((offset & 0x400000) == 0x400000) // negative offset
							CPU.regs.r[15] = CPU.regs.r[15] + 4 - ((~offset & 0x3fffff)+1);
						else // positive offset
							CPU.regs.r[15] = CPU.regs.r[15] + 4 + offset;
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"emulate bl\n");
					}
					else if( (step_instr & THUMB_BLX_MSK) == THUMB_BLX_SECOND_PART )
					{
						// allways exchange to ARM ?? is this correct ??
						// well we can't distinguish between ARM and THUMB mode by loking at the offset
						// since Bit 0 is allway zerro and Bit 1 can be zerro even in THUMB address space
						offset |= (step_instr & THUMB_BL_OFFSET_MSK)<<1;

						CPU.regs.r[14] = (CPU.regs.r[15] + 4) | 1; // LR <- addr of next instr.
						// PC <- PC + 4 +/- offset
						if((offset & 0x400000) == 0x400000) // negative offset
							CPU.regs.r[15] = (CPU.regs.r[15] + 4 - ((~offset & 0x3fffff) +1 )) & 0xffffFFFC;
						else // positive offset
							CPU.regs.r[15] = (CPU.regs.r[15] + 4 + offset) & 0xffffFFFC;
						CPU.CPSR &= ~0x20L; // switch into ARM - mode
						dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"emulate blx\n");
					}
					else 
						dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"incomplete bl instr.\n");
					
					//type = 10;	/*type = software generated - Emulation trap */
					type = 1;	/*type = trap */
					continue;	// continue and handle with this emulated instruction
				}
				else
					instr_modify_mem = is_thumb_store_instr(step_instr);
			}
			else // ARM mode
			{
				/*collect the step instruction*/
				step_instr = gdbLockupArmInstr(CPU.regs.r[15]);
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"0x%8.8X ARM Step\n",step_instr);
				instr_modify_mem = is_arm_store_instr(step_instr);
			}
			// allow interrupt only in user or system mode
			if(allow_intr_in_step_mode 
			   && ( (CPU.CPSR & 0x1fL) == 0x10L || (CPU.CPSR & 0x1fL) == 0x1fL ) // user or system
			   && ( (CPU.CPSR & 0x80L) == 0x0L || (CPU.CPSR & 0x40L) == 0x0L )   // enabled IRQ or FIQ
			  )
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO_LOW,"enable Interrupt while doing Step\n");
				jtag_arm_enable_Intr();
				//jtag_arm_ShowAllIceRT_Regs();
			}
			arm_sfa_StartPeriphery();
			jtag_arm_Step(step_instr);

			/* does instr. modify Memory ?*/
			if(instr_modify_mem)
				gdb_invalidate_Ram_Buffer();
		}
		
		poll_cnt = 3;
		/*poll state of RT ICE via JTAG*/
		while(1)
		{
			if(jtag_arm_PollDbgState())
			{
				//jtag_arm_ShowAllIceRT_Regs();

				/*make sure we stop after every single instuction*/
				jtag_arm_PutAnyBreakPoint();
				
				/*collect current CPU context*/
				jtag_arm_ReadCpuRegs(1); // (0)
				arm_sfa_StopPeriphery();

				/*if PC is at software breakpoint list tell this*/
				if(   isAddrOnBreakpointList(CPU.regs.r[15])
				   || action == 1) // it was a single step
				{
					/*make sure that the previous PC differ to the current PC*/
					if(raw_regs.regs.r[15] == CPU.regs.r[15])
						type = 10;/*type = emulation trap */
					else
						type = 1;/*type = debug exception or breakpoint */
				}
				else
					type = 11;/*type = Stop signal */
				break;
			}
			/* also poll gdb for <cntr><C> - Interrupt request*/
			else if( poll(fds, 1, 0) > 0)
			{	
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"gdb request ??\n");
				type = 7; /*interrupt*/
				jtag_arm_PutAnyBreakPoint();
				while(jtag_arm_PollDbgState() == 0)
					;
				/*collect current CPU context*/
				jtag_arm_ReadCpuRegs(1); // (0)
				break;
			}
			/* since there is nothing to do try one of our bottom half activities if any*/
			else if(CALLBACK_EXSIST)
			{
				int res;
				
				res = doCallback();
				if(res < 0)
				{
					dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Panic: Callback function exit with %d\n", res);
					exit(0);
				}
			}
			else
			{
				if(--poll_cnt<0)
				{
					/*suspend for one timeslice (100 microseconds)*/
					usleep(100);
					poll_cnt = 3;
				}
			}
		}
	} while(1); /*debug main loop*/
	
	if(action == -2)
	{
		// Detach 
		arm_sfa_StartPeriphery();
		jtag_arm_PrepareExitDebug();
		jtag_arm_ClearAnyBreakPoint();
		jtag_arm_FinalExitDebug();
		return 1;
	}
	return 0;
}


