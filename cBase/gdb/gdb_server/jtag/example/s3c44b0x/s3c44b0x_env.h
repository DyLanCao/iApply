
#ifndef __MACHINE_OPTION_H__
#define __MACHINE_OPTION_H__

// ************* OPTIONS **************
#define MCLK 60000000
//#define MCLK 42000000

#define WRBUFOPT (0x8)   //write_buf_on
//#define WRBUFOPT (0)   //write_buf_off

//#define STALLOPT (0x1) // stall enable
#define STALLOPT (0) // stall disable

#define SYSCFG_0KB (0x0 | WRBUFOPT | STALLOPT)
#define SYSCFG_4KB (0x2 | WRBUFOPT | STALLOPT)
#define SYSCFG_8KB (0x6 | WRBUFOPT | STALLOPT)

#define DRAM                   1            //In case DRAM is used
#define SDRAM                  2            //In case SDRAM is used
#define BDRAMTYPE              SDRAM 		//

//BUSWIDTH; 16,32
#define BUSWIDTH               (16)

#define CACHECFG               SYSCFG_4KB

#endif /*__MACHINE_OPTION_H__*/

