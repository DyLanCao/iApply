/*
 * arm_gdbserver.c
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
#include <setjmp.h>
#include <errno.h>

#include <poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <ctype.h>
#include <sysexits.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_tap.h"

#define DO_NOT_CREATE_IO_FUNCTIONS
#include "jt_io.h"

#ifndef INADDR_LOOPBACK
#define	INADDR_LOOPBACK		(uint32_t)0x7F000001 // 127.0.0.1
#endif

static uint16_t		port = 0;
static uint32_t		host = 0;
static int		force_rawIoTest = 0;
static enum DriverId	driverId = NO_DRIVER;
static uint32_t		testLevel = 0;

int gdb_main_loop (int fd);

/*
 *
 */
static void arm_gdbserver_usage(void)
{
	printf	(	
		"Usage:\tjtag_server [OPTIONS] -driver NAME HOST:PORT\n"
		"NAME: name of a cable driver\n"
		"\ttls\t\tTINKER_LEVEL_SHIFTER\n"
		"\ttbdm\t\tTINKER_BDM\n"
		"\ttlongo\t\tTINKER_LONGO\n"
		"\tomsp\t\tOLIMEX_MSP430\n"
		"\twiggler\t\tOCDEMON_WIGGLER\n"
		"\tbblst\t\tALTERA_BYTEBLASTER\n"
		"\tispl\t\tLATTICE_ISPDLC\n"
		"\tdlc\t\tXILINX_DLC\n"
		"\tapod\t\tAMONTEC_EPP_ACCELERATOR\n"
		"PORT: listen at \"PORT\" for a TCP connection.\n"
		"HOST: allow only client with \"HOST\" address to be connect\n"
		"      if no HOST address is given defaults to 127.0.0.1 (local host)\n"
		"OPTIONS:\n"
		"\t-?\t\tprint this info\n"
		"\t-V\t\tDisplay this program's version number\n"
		"\t-v\t\tsend a more verbose info string to gdb\n"
		"\t-d#\t\tlevel of debug messages, where # is a number [0..9]\n"
		"\t-7\t\tARM7TDMI Core\n"
		"\t-9\t\tARM9TDMI Core\n"
		"\t-be\t\tBig Endian\n"
		"\t-le\t\tLittle Endian\n"
		"\t-lptbase 0x###\tport address eiher 0x3BC, 0x378 or 0x278\n"
		"\t-rstgrant #\ttime in milliseconds -- GRANT SRESET line\n"
		"\t-rstsettle #\ttime in milliseconds -- CPU init time\n"
		"\t-iotest\t\traw io test\n"
		"\t-testrambase 0x###\t\n"
		"\t-testflashbase 0x###\t\n"
		"\t-testjtag #[,#]\tsimple JTAG-ARM test\n"
		"\t\t\t#: 0 detect JTAG-ID\n"
		"\t\t\t#: 1 show ICE and CPU Regs\n"
		"\t\t\t#: 2 ICE modify\n"
		"\t\t\t#: 3 Flash info/read (requires flashbase)\n"
		"\t\t\t#: 4 RAM read/write test (requires rambase)\n"
		"\t\t\t#: 5 RAM program test (requires rambase)\n"
		"\t\t\t#: 6 read RAM (requires rambase)\n"
		"\t\t\t#: 7 Write CPU Regs\n"
		"\t\t\t#: 8 do 15 step intruczions from address 0x0\n"
		"\t\t\t#: 9 release\n"
		);
	exit(EX_USAGE);
}

/*
 *
 */
static void parseParameter(int argc, char *argv[])
{
	int	idx;
	char	*ch, c;
	char	*arg_end,*arg_start;
	int	showVersion = 0;

	for (idx=1; idx<argc; idx++)
	{
		ch = argv[idx];
		/*is this an option ?*/
		if (*ch == '-')
		{
			ch++;
			c = *ch++;
			if(c == 0)
				continue;
			else if (c == 'd') // driver or option debug message level
			{
				if(isdigit(*ch)) // option debug message level
				{
					c = *ch - '0';
					if (c == 0)
						dbg_msg_msk = 0;
					else if (c == 1)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR;
					else if (c == 2)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN;
					else if (c == 3)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO;
					else if (c == 4)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO
							    | DBG_LEVEL_JTAG_ARM;
					else if (c == 5)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO
							    | DBG_LEVEL_GDB_ARM_INFO_LOW
							    | DBG_LEVEL_JTAG_ARM
							    | DBG_LEVEL_JTAG_ARM_LOW;
					else if (c == 6)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO
							    | DBG_LEVEL_GDB_ARM_INFO_LOW
							    | DBG_LEVEL_JTAG_ARM
							    | DBG_LEVEL_JTAG_ARM_LOW
							    | DBG_LEVEL_JTAG_ICERT;
					else if (c == 7)
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO
							    | DBG_LEVEL_GDB_ARM_INFO_LOW
							    | DBG_LEVEL_JTAG_ARM
							    | DBG_LEVEL_JTAG_ARM_LOW
							    | DBG_LEVEL_JTAG_ICERT
							    | DBG_LEVEL_JTAG_ICERT_LOW
							    | DBG_LEVEL_JTAG_INSTR;
					else 
						dbg_msg_msk = DBG_LEVEL_GDB_ARM_ERROR
							    | DBG_LEVEL_GDB_ARM_WARN
							    | DBG_LEVEL_GDB_ARM_INFO
							    | DBG_LEVEL_GDB_ARM_INFO_LOW
							    | DBG_LEVEL_JTAG_ARM
							    | DBG_LEVEL_JTAG_ARM_LOW
							    | DBG_LEVEL_JTAG_ICERT
							    | DBG_LEVEL_JTAG_ICERT_LOW
							    | DBG_LEVEL_JTAG_INSTR
							    | DBG_LEVEL_JTAG_TAP;
				}
				else if (!strncasecmp(ch,"river",sizeof("rive"))) // driver
				{
					ch += sizeof("rive");
					/*collect driver NAME*/
					if(*ch == 0) //use next argument if this string terminates here
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) //use next argument if this string terminates here
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					/*check driver types*/
					if (!strncasecmp(ch,"tls",sizeof("tl")))
						driverId = DRIVER_TINKER_LEVEL_SHIFTER;
					else if (!strncasecmp(ch,"tbdm",sizeof("tbd")))
						driverId = DRIVER_TINKER_BDM;
					else if (!strncasecmp(ch,"tlongo",sizeof("tlong")))
						driverId = DRIVER_TINKER_LONGO;
					else if (!strncasecmp(ch,"omsp",sizeof("oms")))
						driverId = DRIVER_OLIMEX_MSP430;
					else if (!strncasecmp(ch,"wiggler",sizeof("wiggle")))
						driverId = DRIVER_OCDEMON_WIGGLER;
					else if (!strncasecmp(ch,"bblst",sizeof("bbls")))
						driverId = DRIVER_ALTERA_BYTEBLASTER;
					else if (!strncasecmp(ch,"ispl",sizeof("isp")))
						driverId = DRIVER_LATTICE_ISPDLC;
					else if (!strncasecmp(ch,"dlc",sizeof("dl")))
						driverId = DRIVER_XILINX_DLC;
					else if (!strncasecmp(ch,"apod",sizeof("apo")))
						driverId = DRIVER_AMONTEC_EPP_ACCELERATOR;
					else
						driverId = NO_DRIVER;
				}
				else
					arm_gdbserver_usage();
			}
			else if (isdigit(c)) // option default core type
			{
				if (c == '7')
					arm_info.core_number = 7; // default ARM7
				else if (c == '9')
					arm_info.core_number = 9; // default ARM9
				else
					arm_gdbserver_usage();
			}
			else if ( (c == 'b' || c == 'B') && (*ch == 'e' || *ch == 'E')) // option default big endian
				arm_info.bigend = 1;
			else if (c == 'l' || c == 'L') // option lptbase or little endian
			{
				if (*ch == 'e' || *ch == 'E') // option default little endian
					arm_info.bigend      = 0;
				else if( !strncasecmp(ch,"ptbase",sizeof("ptbas"))) // option lptbase
				{
					ch += sizeof("ptbas");
					/*collect base addres*/
					if(*ch == 0) //use next argument if this string terminates here
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) //use next argument if this string terminates here
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					port_base = strtoul(ch, &arg_end, 16);
					if ( (arg_end != ch)
					     && ( port_base == 0x3BC  //(LPT1)
					        || port_base == 0x378  //(LPT2) or 
					        || port_base == 0x278 )//(LPT3)
					   )
						; // OK
					else
						arm_gdbserver_usage();
				}
				else
					arm_gdbserver_usage();
			}
			else if (c == 'r' || c == 'R') // option rstgrant or rstsettle
			{
				if (!strncasecmp(ch,"stgrant",sizeof("stgran")))
				{
					ch += sizeof("stgran");
					if(*ch == 0)
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) 
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					tap_grant_sreset_timout = strtoul(ch, &arg_end, 10);
					if (arg_end == ch)
						arm_gdbserver_usage();
					dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO 
						, "change GRAND timeout to %d usec\n"
						, tap_grant_sreset_timout
						);
				}
				else if (!strncasecmp(ch,"stsettle",sizeof("stsettl")))
				{
					ch += sizeof("stsettl");
					if(*ch == 0)
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) 
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					tap_settle_system_timeout = strtoul(ch, &arg_end, 10);
					if (arg_end == ch)
						arm_gdbserver_usage();
					dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO 
						, "change SETTLE timeout to %d usec\n"
						, tap_settle_system_timeout
						);
				}
				else
					arm_gdbserver_usage();
			}
			else if (c == 'i' || c == 'I') // option iotest
			{
				if (!strncasecmp(ch,"otest",sizeof("otes")))
				{
					ch += sizeof("otes");
					force_rawIoTest = 1;
				}
				else
					arm_gdbserver_usage();
			}
			else if (c == 't' || c == 'T') // test...
			{
				if (!strncasecmp(ch,"estrambase",sizeof("estrambas")))
				{
					ch += sizeof("estrambas");
					if(*ch == 0)
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) 
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					ramBase = strtoul(ch, &arg_end, 0);
					if (arg_end == ch)
						arm_gdbserver_usage();
					else
						RAM_BASE_def = 1;
				}
				else if (!strncasecmp(ch,"estflashbase",sizeof("estflashbas")))
				{
					ch += sizeof("estflashbas");
					if(*ch == 0)
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) 
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					flashBase = strtoul(ch, &arg_end, 0);
					if (arg_end == ch)
						arm_gdbserver_usage();
					else
						FLASH_BASE_def = 1;
				}
				else if (!strncasecmp(ch,"estjtag",sizeof("estjta")))
				{
					unsigned long cmd;
					
					ch += sizeof("estjta");
					if(*ch == 0)
					{
						if(++idx>=argc)
							arm_gdbserver_usage();
						ch = argv[idx];
					}
					if(*ch == '=' || *ch == ' ' || *ch == '\t')
					{
						ch++;
						if(*ch == 0) 
						{
							if(++idx>=argc)
								arm_gdbserver_usage();
							ch = argv[idx];
						}
					}
					else
						arm_gdbserver_usage();
					do {
						cmd = strtoul(ch, &arg_end, 0);
						if (arg_end == ch)
							break;
						ch = arg_end;
						switch(cmd & 0xFF)
						{
						case 0: /*detect JTAG-ID*/
							testLevel |= TEST_JTAG_GETID;
							break;
						case 1: /*show ICE and CPU Regs*/
							testLevel |= TEST_JTAG_SHOWREGS;
							break;
						case 2: /*ICE modify*/
							testLevel |= TEST_JTAG_MODIFY_ICE;
							break;
						case 3: /*Flash info/read (requires flashbase)*/
							testLevel |= TEST_JTAG_FLASH_READ;
							break;
						case 4: /*RAM read/write test (requires rambase)*/
							testLevel |= TEST_JTAG_RAM_WRITE;
							break;
						case 5: /*RAM program test (requires rambase)*/
							testLevel |= TEST_JTAG_RAM_PROGRAM;
							break;
						case 6: /*read RAM (requires rambase)*/
							testLevel |= TEST_JTAG_RAM_READ;
							break;
						case 7: /*Write CPU Regs*/
							testLevel |= TEST_JTAG_MODIFY_CPU_REG;
							break;
						case 8: /*do 15 step intruczions from address*/
							testLevel |= TEST_JTAG_START_STEPS;
							break;
						case 9: /*release*/
							testLevel |= TEST_JTAG_RELEASE;
							break;
						default:
							break;
						}
						/*more test follow ?*/
						if(*ch == 0) // no
							break;
						if(*ch == ',') // maybe
							ch++;
						else // no
							break;
						if(*ch == 0)
						{
							if(++idx>=argc)
								break;;
							ch = argv[idx];
						}
					} while(1);
				}
				else
					arm_gdbserver_usage();
			}
			else if (c == 'v')
				verbose = MORE_VERBOSE;
			else if (c == 'V')
			{
				showVersion = 1;
				break;
			}
			else //if (c == '?' || c == 'h' ...)
				arm_gdbserver_usage();
			
		}
		else /*should be host:port*/
		{
			if(force_rawIoTest || testLevel) // not required for tests
				continue;
			
			arg_start = ch;
			while(*ch != 0 && *ch != ':')
				ch++;
	
			if (*ch == 0 || !isdigit(ch[1]))
				arm_gdbserver_usage();
			
			*ch = 0; // ':' is now end of sting
			arg_end = ++ch;
			while(*arg_end >= '0' && *arg_end <= '9')
				arg_end++;
			*arg_end = 0;
			port = atoi(ch);
			port = htons (port);
			
			// setup client address of the gdb-host
			ch = arg_start;
			if (*ch == 0)
				host = htonl(INADDR_LOOPBACK);
			else if ( inet_pton(AF_INET,ch, &host) != 1)
				arm_gdbserver_usage();
		}
	}
	
	if(showVersion)
	{
		printf(	"%s alpha snapshot 2006-01-??\n"
			"Copyright 2005 Free Software Foundation, Inc.\n"
			"This program is free software; you may redistribute it under the terms of\n"
			"the GNU General Public License.  This program has absolutely no warranty.\n"
			,argv[0]);
		exit(0);
	}

	if(!(host || force_rawIoTest || testLevel))
		arm_gdbserver_usage();
	
	return;
}


/*
 *
 */
int main(int argc, char *argv[])
{
	struct sockaddr_in address[1];
	int                sock, fd, i;
	socklen_t          addrLength[1];

	if (argc < 2)
		arm_gdbserver_usage();
	
	// Set default values
	// in jt_direct_io.c set port_base = PORT_BASE;
	verbose = LESS_VERBOSE;
	arm_info.bigend      = 0; // default little endian
	arm_info.core_number = 7; // default ARM7
	dbg_msg_msk          = DBG_LEVEL_GDB_ARM_INFO 
		             | DBG_LEVEL_GDB_ARM_WARN 
			     | DBG_LEVEL_GDB_ARM_ERROR
			     //| DBG_LEVEL_JTAG_ICERT
			     | DBG_LEVEL_JTAG_ARM_LOW
			     | DBG_LEVEL_JTAG_ARM;
	
	
	/*run through parameter list*/
	parseParameter(argc, argv);

	if(driverId == NO_DRIVER || tap_driver_init(driverId))
		arm_gdbserver_usage();
	
	/*check if we can access the  IO-port*/
	tap_probe();
	
	/*ckeck if we like to perform a raw IO TEST instead*/
	if(force_rawIoTest)
	{
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO 
		, "LPT-Port 0x%X DbgMsk(0x%X)\n"
		, port_base
		, dbg_msg_msk);
		return tap_raw_io_test();
	}

	dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO 
		, "LPT-Port 0x%X %sendian ARM%d DbgMsk(0x%X)\n"
		, port_base
		, arm_info.bigend ? "Big":"Little"
		, arm_info.core_number
		, dbg_msg_msk);

	/*ckeck if we like to perform a TEST at JTAG level instead**/
	if(testLevel)
		return jtag_test(testLevel);
		
	/*now we try to setup the server*/
	if ((sock = socket (PF_INET, SOCK_STREAM, 0)) < 0)
		error2 ("Can't create socket: %s", strerror (errno));    
	
	i = 1;
	setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i));
	memset (&address, 0, sizeof (address));
	address->sin_family = AF_INET;
	address->sin_port = port;
	//memset (&address->sin_addr, 0, sizeof (address->sin_addr));
	if (host == htonl(INADDR_LOOPBACK))
		address->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	if (bind (sock, (struct sockaddr *)address, sizeof (address)))
		error2 ("Can not bind socket: %s", strerror (errno));
	
	while (1)
	{
		if (listen (sock, 1))
		{
			if( errno == EINTR)
				break;          /* EINTR will cause listen to be interrupted */
			error ("Can not listen on socket");
			return -1;
		}
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO
			, "Waiting on port %d for gdb client to connect...\n", port);
		
		/* accept() needs this set, or it fails (sometimes) */
		addrLength[0] = sizeof (struct sockaddr);
		
		/* We only want to accept a single connection, thus don't need a loop. */
		fd = accept (sock, (struct sockaddr *)address, addrLength);
		if (fd < 0)
		{
			if( errno == EINTR)
				break;          /* EINTR will cause accept to be interrupted */
			error ("Accept connection failed");
		}
		/*is it the right host ?*/
		if( address->sin_addr.s_addr != host )
		{
			close(fd);
			continue;
		}
		/* Tell TCP not to delay small packets.  
		 * This greatly speeds up interactive response. 
		 * WARNING: If TCP_NODELAY is set on, then gdb may timeout in mid-packet if 
		 * the (gdb)packet is not sent within a single (tcp)packet, thus all outgoing 
		 * (gdb)packets _must_ be sent with a single call to write. 
		 * (see Stevens "Unix Network Programming", Vol 1, 2nd Ed, page 202 for more info) */
		i = 1;
		setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &i, sizeof (i));
		/* If we got this far, we now have a client connected and can start processing. */
		
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO
			,"Connection opened by host %s, peer %hd.\n"
			,inet_ntoa (address->sin_addr), ntohs (address->sin_port));

		i = gdb_main_loop (fd);
		if( errno != 0 || i != 0)
		{
			close (fd);
			break;
		}
		close (fd);
	}
	close (sock);
	return 0;
}

