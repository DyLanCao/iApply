/*
 * arm_gdbstub.h
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
#include <bitstring.h>
#include <sys/time.h>

/*******************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers */
#define G_PKG_SIZE (2 * ( 16 /*std regs*/ * 4 + 8 /*fp regs*/ * 12 + 1 /*fp stat reg*/ * 4 + 1 /*prog stat reg*/ * 4) + 32)
#if G_PKG_SIZE > (400 - 1)
#define BUFDEFAULTMAX G_PKG_SIZE
#else
#define BUFDEFAULTMAX (400)
#endif
#define BUFMAX (16*1024)


/*
 * Data structure's for memory map's
 */

/*posible memory Types*/
enum memMapType {
	MMAP_T_UNUSED = 0, 
	MMAP_T_FLASH, 
	MMAP_T_APPLICATION_FLASH, 
	MMAP_T_ROM, 
	MMAP_T_RAM,
	MMAP_T_UNREAL,
	MMAP_T_SFA, 
	MMAP_T_IO, 
	MMAP_T_WORKSPACE, 
	MMAP_T_CACHE
};

enum pendingACT {
	ACTION_NON			= 0x00,
	ACTION_INPROCESS		= 0x01,
	ACTION_TERMINATE		= 0x02,
	ACTION_ERROR			= 0x04,
	ACTION_VERIFY 			= 0x10,
	ACTION_VERIFY_INPROCESS		= 0x11, // verify | inprocess
	ACTION_VERIFY_EQUAL		= 0x12, // verify | terminate (-> no error)
	ACTION_VERIFY_DIFF		= 0x16, // verify | terminate | error
	ACTION_PROGRAM			= 0x20,
	ACTION_PROGRAM_INPROCESS	= 0x21, // program | inprocess
	ACTION_PROGRAM_SUCCEED		= 0x22, // program | terminate (-> no error)
	ACTION_PROGRAM_FAIL		= 0x26, // program | terminate | error
	ACTION_ERASE			= 0x40,
	ACTION_ERASE_INPROCESS		= 0x41,	// erase | inprocess
	ACTION_ERASE_SUCCEED		= 0x42,	// erase | terminate (-> no error)
	ACTION_ERASE_FAIL		= 0x46,	// erase | terminate | error
	ACTION_MEMMAPINFO		= 0x80,
	ACTION_MEMMAPINFO_INPROCESS	= 0x81, // memMapInfo | inprocess
	ACTION_MEMMAPINFO_SUCCEED	= 0x82, // memMapInfo | terminate (-> no error)
	ACTION_MEMMAPINFO_FAIL		= 0x86, // memMapInfo | terminate | error
};

enum workspaceState {
	WORKSPACE_UNTESTED	= 0,
	WORKSPACE_BROKEN,
	WORKSPACE_FREE,
	WORKSPACE_ALGO_DUMMY,
	WORKSPACE_ALGO_READ,
	WORKSPACE_ALGO_WRITE,
	WORKSPACE_ALGO_CHECK,
	WORKSPACE_ALGO_FLASH_AMD_8_L,
	WORKSPACE_ALGO_FLASH_AMD_16_L,
	WORKSPACE_ALGO_FLASH_AMD_32_L,
	WORKSPACE_ALGO_FLASH_AMD_8_B,
	WORKSPACE_ALGO_FLASH_AMD_16_B,
	WORKSPACE_ALGO_FLASH_AMD_32_B,
	WORKSPACE_ALGO_FLASH_PHILIPS,
	WORKSPACE_ALGO_FLASH_ATMEL,
	WORKSPACE_ALGO_FLASH_ST
};

enum workspaceInfo {
	STAND_ALONE_WORKSPACE = 0,
	TOP_OF_RAM_WORKSPACE,
	BOTTOM_OF_RAM_WORKSPACE	
};

/*The size of each page is  4 * 32 Byte = 128 Byte = 64 Halfwords = 32 Words*/
#define RAM_PAGE_SIZE	(32*sizeof(uint32_t))

struct memMap {
	/*set by gdb - monitor - Rcmd*/
	uint32_t		baseAddr;
	uint32_t		length;
	enum memMapType		type;
	uint32_t		busSize; // required by Flash and IO access
	union MemBufferType {
		/*detected (for Flash only)*/
		struct Flash {
			bitstr_t	*writeByteBitmap; // must be equal with Ram
			enum flashAlgo	algo;
			int		numberOfSectors;
			struct sector	*sectorList;
			int		chipsPerBus;
		} Flash;
		/*used by RAM only*/
		struct Ram {
			bitstr_t	*writeByteBitmap; // must be equal with Flash
			bitstr_t	*readPageBitmap;
			enum workspaceState dumy_wstate;
			enum workspaceInfo  dumy_winfo;
		} Ram;
		/*used for our own work space*/
		struct Workspace {
			bitstr_t	*writeByteBitmap; // must be equal with Ram (and so with Flash too)
			bitstr_t	*readPageBitmap; // must be equal with Ram
			enum workspaceState state;
			enum workspaceInfo  info;
			uint32_t	offset;
		} Workspace;
	} memBufferType;
	/*used by gdb read and write operations*/
	union MemBuffer{
		uint8_t  	*Byte;
		uint16_t 	*HalfWord;
		uint32_t 	*Word;
	} memBuffer;
	int 			memBufferLength;
	/*internal stuff*/
	struct memMap		*nextMap;
};

struct memMapHead {
	struct memMapHead	*nextHead;
	int 			numberOfEntrys;
	struct memMap		*firstMapEntry;
};


struct memMapContainer {
	int			numberOfMemMaps;
	struct memMapHead	*firstMap;
	int			activeNumber;
	struct memMapHead	*activeMap;
	struct memMap		*workspace;
	struct MemStat {
		enum pendingACT	pending;
		struct timeval	startTime;
		struct timeval	stopTime;
	} memStat;
};

/*
 * prototype's for memory map
 */
extern struct memMapContainer memMapContainer;

extern int AllocMemMaps(int total_number_of_MemMaps);
extern struct memMapHead * searchMemMapHead(int num);
extern int AllocateMemMapEntrys(struct memMapHead * mh, int total_num);
extern struct memMap * searchMemMapEntry(struct memMapHead * mh,int num);
extern struct memMap * findMemMapOfAddr(uint32_t addr);
extern int updateMemMap(struct memMapHead * mh, int num ,enum memMapType type, uint32_t busSize, uint32_t baseAddr, uint32_t length);
extern int activateMemMap(int num);
extern void deactivateMemMap(void);

/*
 * Data structure's for remote command's
 */
enum CmdSequenceFlag {
	CMDSEQUFLAG_WRITE = 0,
	CMDSEQUFLAG_READ  = 1
};

struct cmdSequence {
	uint32_t		addr;
	uint32_t		val;
	uint32_t		cmdSize;
	enum CmdSequenceFlag	flag;
	int			memMapNumber;
	struct memMap		*memMap;
	struct cmdSequence	*nextSequence;
};

struct cmdSequenceHead {
	struct cmdSequenceHead	*nextHead;
	int			numberOfEntrys;
	struct cmdSequence	*firstSequenceEntry;
};

struct cmdSequenceContainer {
	int			numberOfCmdSequences;
	struct cmdSequenceHead	*firstSequence;
};

enum CmdSequenceBind { 
	CMDSEQU_UNKOWN = 0, 
	CMDSEQU_START_PERIPH, 
	CMDSEQU_STOP_PERIPH,
	CMDSEQU_RESET_CPU
};

/*
 * prototype's for command
 */
extern struct cmdSequenceContainer cmdSequenceContainer;

extern int AllocCmdSeqences(int total_number_of_cmdSequences);
extern struct cmdSequenceHead * searchCmdSequenceHead(int num);
extern int AllocCmdSequenceEntrys(struct cmdSequenceHead * sh, int total_num);
extern struct cmdSequence * searchCmdSequenceEntry(struct cmdSequenceHead * sh, int num);
extern int updateCmdSequence(struct cmdSequenceHead * sh, int num, uint32_t cmdSize, uint32_t addr, uint32_t val, enum CmdSequenceFlag flag);
extern uint32_t readvalCmdSequence(int num, int entryNum);
extern uint32_t readaddrCmdSequence(int num, int entryNum);
extern int doCmdSequence(int num);
extern void arm_sfa_StartPeriphery(void);
extern void arm_sfa_StopPeriphery(void);
extern void arm_sfa_ResetCPU(void);

extern int cmdSequenceNumber_StartPeriphery;
extern int cmdSequenceNumber_StopPeriphery;
extern int cmdSequenceNumber_ResetCPU;

struct context;

/*
 * callback stuff (- instead of multi threading)
 */
struct cmdCallbackEntry {
	int (*fntCB) (int arg, int cnt,struct context *context);
	int arg;
	int cnt;
	struct context *context;
	struct cmdCallbackEntry *nextCBentry;
	struct cmdCallbackEntry *prevCBentry;
};

struct cmdCallbackConainer {
	int numberOfCBentrys;
	struct cmdCallbackEntry *firstCBentry;
	struct cmdCallbackEntry *lastCBentry;
};

/*
 * prototypes for callback
 */
extern struct cmdCallbackConainer cmdCallbackConainer;

extern struct cmdCallbackEntry *allocateCmdCallbackEntry(int byteSizeOfExtraContext);
extern void freeCmdCallbackEntry(struct cmdCallbackEntry *cb);
void inQueueCmdCallbackEntry(struct cmdCallbackEntry *entry, int (*fntCB) (int arg, int cnt,struct context *context),
		int argSrc, int cntSrc, struct context *contextSrc, int contextSize);
struct cmdCallbackEntry * deQueueCmdCallbackEntry(void);
extern int doCallback(void);
extern void removeAllCallbacks(void);

#define CALLBACK_EXSIST (cmdCallbackConainer.numberOfCBentrys > 0)

/*
 * some predefined callback functions
 */
struct flashCBContext {
	int			firstSectorId;
	int			lastSectorId;
	int			currentSectorId;
	char			*temp_buffer;
	struct memMap		*whichMemMap;
	enum VerboseLevel	verbose;
	int			quiet_cnt;
};

struct memMapCBContext {
	int 			max_MemMap_number;
	struct memMapHead	*curr_Map;
	int 			max_MapEntry_number;
	struct memMap		*curr_memMap;
	int			max_secCnt;
	int			curr_secCnt;
};

extern int verifyFlashCB_read_sector(int currAddr, int cnt, struct context *flashCBcontext);
extern int checkFlashCB_read_sector(int currAddr, int cnt, struct context *flashCBcontext);
extern int programFlashCB_write_sector(int currAddr, int cnt, struct context *flashCBcontext);
extern int eraseFlashCB_sector(int currAddr, int cnt, struct context *flashCBcontext);
extern int printmemMapCB_info(int currMemMapHeadCnt, int currEntryCnt, struct context *contextSrc);

/*
 * Flash Progamming function that are used by programFlashCB_write_sector
 */
extern int prog_flash_amd8(uint32_t base, uint32_t addr, int maxSize, uint8_t *data);
extern int prog_flash_amd16(uint32_t base, uint32_t addr, int maxSize, uint16_t *data);
extern int prog_flash_amd32(uint32_t base, uint32_t addr, int maxSize, uint32_t *data);
extern int philipsFlashProgram(uint32_t addr, int maxSize, uint32_t *data);
extern int philipsFlashUnlock(uint32_t flash_base_addr, uint32_t mask);
extern int philipsFlashLock(uint32_t flash_base_addr, uint32_t mask);
extern int philipsFlashEraseAllUnlocked(uint32_t flash_base_addr);
extern int atmelFlashEraseAndProgram(uint32_t addr, int pageNumber, int pageSize, uint32_t *data);
extern int atmelFlashProgramOnly(uint32_t addr, int pageNumber, int pageSize, uint32_t *data);
extern int atmelFlashUnlock(uint32_t addr, int pageNumber);
extern int atmelFlashLock(uint32_t addr, int pageNumber);
extern int atmelFlashErase(uint32_t addr, int pageNumber);
extern int stFlashProgram(uint32_t addr, int maxSize, uint32_t *data);
extern int stFlashErase(uint32_t addr, int sectorNumber);


/*set water mark for starting DCC*/
#define MIN_DCC_DATA_SIZE	64

#define WORKSPACE_SEGMENT_SIZE		0x200 // 512 Bytes
#define WORKSPACE_STACK_SEGMENT_SIZE	18*4  // 18 word
#define WORKSPACE_TEXT_SEGMENT_SIZE	(WORKSPACE_SEGMENT_SIZE - (WORKSPACE_STACK_SEGMENT_SIZE)) 

#define SMALL_PAGE_SIZE 1024
#define PAGE_SIZE SMALL_PAGE_SIZE * 4 // 4096 Bytes = 2048 halfwords = 1024 Words


/*
 * send Stringbuffer to gdb's stderr while doing a fake conituing operation
 */
struct gdbSprintfBuf {
	struct gdbSprintfBuf	*next;
	int			len;
	char			*string;
};

struct gdbSprintfBufContainer {
	struct gdbSprintfBuf	*first;
	struct gdbSprintfBuf	*last;
};

extern struct gdbSprintfBufContainer gdbSprintfBufContainer;

extern void gdbPrintf(int set_pending_action, int doInQueue, struct gdbSprintfBuf *msg_buf, const char *format, ...);
extern struct gdbSprintfBuf *allocateGdbSprintfBuf(int size_of_string);
extern void freeGdbSprintfBuf(struct gdbSprintfBuf *buf);
extern void inQueueGdbSprintfBuf(struct gdbSprintfBuf *buf);
extern struct gdbSprintfBuf *deQueueGdbSprintfBuf(void);
extern void packGdbSprintBuf(void);

/*
 * Data structure's for breakpoint / watchpoint handling
 */
struct breakpointEntry {
	uint32_t		addr;		// address 
	int			len;		// len = 4 (byte)-> ARM length or len = 2 (byte)-> THUMB length
	uint32_t		instr;		// real intruction (only if SW Break)
	struct breakpointEntry	*nextBkptEntry;
};

struct breakpointContainer {
	int numberOfHWBkptEntrys;	
	int numberOfSWBkptEntrys;
	int numberOfWatchEntrys;
	// we can have one of the three solutions:
	//		2 HWBkpt and nither SWBkpt nor Watch
	// alternative	2 Watch and nither HWBkpt nor SWBkpt
	// alternative  unlimited SWBkpt and either one HWBkpt or one SWBkpt
	struct breakpointEntry *firstHWBkptEntry;
	struct breakpointEntry *firstSWBkptEntry;
	struct breakpointEntry *firstWatchEntry;
};

/* for temporary break at main*/
enum SymbolState{
	SYM_UNKNOWN = 0, 
	SYM_PRESENT, 
	SYM_NOT_EXSIST
};

struct SymbolInfo {
	uint32_t		addr;
	enum SymbolState	state;
	int			isThumb;
	int			breakIsActive;
};

extern struct SymbolInfo symbolMain;

/*
 * prototype's for breakpoint functions
 */
extern struct breakpointContainer breakpointList;

extern int InsertBreakpoint(uint32_t addr, int len, int watchORbreak);
extern int RemoveBreakpoint(uint32_t addr, int len, int watchORbreak);
extern int RemoveAllBreakpoints(void);

extern int isAddrOnBreakpointList(uint32_t addr);
extern int gdbSetupJtagICE_RTbreakpoint(void);
extern int gdbWriteSoftwareBreakToRAM(void);

extern uint32_t gdbLockupThumbInstr(uint32_t addr);
extern uint32_t gdbLockupArmInstr(uint32_t addr);

/*
 * general memory handling
 */
extern int *gdb_read_mem(int addr, int length);
extern int gdb_write_mem(int *mem_val, int addr, int length);
extern int gdb_writeback_Ram(void);
extern int gdb_invalidate_Ram_Buffer(void);
extern int gdb_invalidate_Rom_Buffer(void);

extern int gdb_check_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf);
	
extern void gdb_read_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf);
extern void gdb_writeback_memory_block(uint32_t addr, uint32_t word_len, uint32_t *buf);

extern void gdb_dcc_dummy(uint32_t len);
/*
 * general support functions
 */
extern char * mem2hex (char *mem, char *buf, int count);
extern char * hex2mem (char *buf, char * mem, int count);
extern int  hexToInt(char **ptr, int *intValue);
extern void gdb_rcmd_console_output(int fd, char *mem);
extern void gdb_query(int fd, char *query_str, char *response_str, struct reg_set *raw_regs);
extern int gdb_restart (void);

/*
 * rcmd setting general variables
 */
extern int allow_intr_in_step_mode;
extern int fake_continue_mode;
extern int download_faster;
extern int download_wait;
extern int forceInvalMemWhileEraseingFlash;
extern int forceCheckInvalidSector;
extern int workspace_mode;
extern int force_hardware_breakpoint;
extern uint32_t LPC_frequence;
extern int LPC_boot_sector_enabled;
extern uint32_t SAM7_frequence;

/*
 * workspace
 */
extern int disable_workspace_size_test;
extern int is_workspace_big_enough (uint32_t length);
extern struct memMap * getWorkSpace(void);
extern void changeWorkSpaceMode( int mode );
extern int useWorkspace(int algo, uint32_t addr, unsigned length);

/*
 * text sector of target DCC routines
 */
extern unsigned int dcc_read_start;
extern int dcc_read_size;
extern unsigned int dcc_write_start;
extern int dcc_write_size;
extern unsigned int dcc_check_start;
extern int dcc_check_size;
extern unsigned int dcc_dummy_start;
extern int dcc_dummy_size;

extern unsigned int dcc_fl_amd8_start;
extern int dcc_fl_amd8_size;
extern unsigned int dcc_fl_amd16_start;
extern int dcc_fl_amd16_size;
extern unsigned int dcc_fl_amd32_start;
extern int dcc_fl_amd32_size;

extern unsigned int dcc_fl_philips_start;
extern int dcc_fl_philips_size;
extern unsigned int dcc_fl_atmel_start;
extern int dcc_fl_atmel_size;
extern unsigned int dcc_fl_st_start;
extern int dcc_fl_st_size;

uint32_t arm_checksum_crc32(uint32_t *block, int bit_len);
uint32_t arm_checksum_crc16(uint16_t *block, int bit_len);

