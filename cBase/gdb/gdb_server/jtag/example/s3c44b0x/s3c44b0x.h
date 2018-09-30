/****************************************************
 * NAME                   : machine.h               *
 *                         for S3C44B0X             *
 ****************************************************/

#ifndef __S3C44B0X_H__
#define __S3C44B0X_H__

#include "s3c44b0x_env.h"

/* System */
#define rSYSCFG		(*(volatile uint32_t *)0x1c00000)

/* Cache */
#define rNCACHBE0	(*(volatile uint32_t *)0x1c00004)
#define rNCACHBE1	(*(volatile uint32_t *)0x1c00008)

/* Bus control */
#define rSBUSCON	(*(volatile uint32_t *)0x1c40000)

/* Memory control */
#define rBWSCON		(*(volatile uint32_t *)0x1c80000)
#define rBANKCON0	(*(volatile uint32_t *)0x1c80004)
#define rBANKCON1	(*(volatile uint32_t *)0x1c80008)
#define rBANKCON2	(*(volatile uint32_t *)0x1c8000c)
#define rBANKCON3	(*(volatile uint32_t *)0x1c80010)
#define rBANKCON4	(*(volatile uint32_t *)0x1c80014)
#define rBANKCON5	(*(volatile uint32_t *)0x1c80018)
#define rBANKCON6	(*(volatile uint32_t *)0x1c8001c)
#define rBANKCON7	(*(volatile uint32_t *)0x1c80020)
#define rREFRESH	(*(volatile uint32_t *)0x1c80024)
#define rBANKSIZE	(*(volatile uint32_t *)0x1c80028)
#define rMRSRB6		(*(volatile uint32_t *)0x1c8002c)
#define rMRSRB7		(*(volatile uint32_t *)0x1c80030)

/* UART */
#define rULCON0		(*(volatile uint32_t *)0x1d00000)
#define rULCON1		(*(volatile uint32_t *)0x1d04000)
#define rUCON0		(*(volatile uint32_t *)0x1d00004)
#define rUCON1		(*(volatile uint32_t *)0x1d04004)
#define rUFCON0		(*(volatile uint32_t *)0x1d00008)
#define rUFCON1		(*(volatile uint32_t *)0x1d04008)
#define rUMCON0		(*(volatile uint32_t *)0x1d0000c)
#define rUMCON1		(*(volatile uint32_t *)0x1d0400c)
#define rUTRSTAT0	(*(volatile uint32_t *)0x1d00010)
#define rUTRSTAT1	(*(volatile uint32_t *)0x1d04010)
#define rUERSTAT0	(*(volatile uint32_t *)0x1d00014)
#define rUERSTAT1	(*(volatile uint32_t *)0x1d04014)
#define rUFSTAT0	(*(volatile uint32_t *)0x1d00018)
#define rUFSTAT1	(*(volatile uint32_t *)0x1d04018)
#define rUMSTAT0	(*(volatile uint32_t *)0x1d0001c)
#define rUMSTAT1	(*(volatile uint32_t *)0x1d0401c)
#define rUBRDIV0	(*(volatile uint32_t *)0x1d00028)
#define rUBRDIV1	(*(volatile uint32_t *)0x1d04028)

#ifdef __BIG_ENDIAN
#define rUTXH0		(*(volatile uint8_t *)0x1d00023)
#define rUTXH1		(*(volatile uint8_t *)0x1d04023)
#define rURXH0		(*(volatile uint8_t *)0x1d00027)
#define rURXH1		(*(volatile uint8_t *)0x1d04027)
#define WrUTXH0(ch)	((*(volatile uint8_t *)(0x1d00023))=(uint8_t)((ch)))
#define WrUTXH1(ch)	((*(volatile uint8_t *)(0x1d04023))=(uint8_t)((ch)))
#define RdURXH0()	(*(volatile uint8_t *)(0x1d00027))
#define RdURXH1()	(*(volatile uint8_t *)(0x1d04027))
#define UTXH0		(0x1d00020+3)  //byte_access address by BDMA
#define UTXH1		(0x1d04020 +3)
#define URXH0		(0x1d00024+3)
#define URXH1		(0x1d04024+3)

#else //Little Endian
#define rUTXH0		(*(volatile uint8_t *)0x1d00020)
#define rUTXH1		(*(volatile uint8_t *)0x1d04020)
#define rURXH0		(*(volatile uint8_t *)0x1d00024)
#define rURXH1		(*(volatile uint8_t *)0x1d04024)
#define WrUTXH0(ch)	((*(volatile uint8_t *)0x1d00020)=(uint8_t)((ch)))
#define WrUTXH1(ch)	((*(volatile uint8_t *)0x1d04020)=(uint8_t)((ch)))
#define RdURXH0()	(*(volatile uint8_t *)0x1d00024)
#define RdURXH1()	(*(volatile uint8_t *)0x1d04024)
#define UTXH0		(0x1d00020)    //byte_access address by BDMA
#define UTXH1		(0x1d04020)
#define URXH0		(0x1d00024)
#define URXH1		(0x1d04024)
#endif

/* SIO */
#define rSIOCON		(*(volatile uint32_t *)0x1d14000)
#define rSIODAT		(*(volatile uint32_t *)0x1d14004)
#define rSBRDR		(*(volatile uint32_t *)0x1d14008)
#define rIVTCNT		(*(volatile uint32_t *)0x1d1400c)
#define rDCNTZ		(*(volatile uint32_t *)0x1d14010)

/* IIS */
#define rIISCON		(*(volatile uint32_t *)0x1d18000)
#define rIISMOD		(*(volatile uint32_t *)0x1d18004)
#define rIISPSR		(*(volatile uint32_t *)0x1d18008)
#define rIISFCON	(*(volatile uint32_t *)0x1d1800c)

#ifdef __BIG_ENDIAN
#define rIISFIF		((volatile uint16_t *)0x1d18012)

#else //Little Endian
#define rIISFIF		((volatile uint16_t *)0x1d18010)
#endif

/* I/O PORT */
#define rPCONA		(*(volatile uint32_t *)0x1d20000)
#define rPDATA		(*(volatile uint32_t *)0x1d20004)

#define rPCONB		(*(volatile uint32_t *)0x1d20008)
#define rPDATB		(*(volatile uint32_t *)0x1d2000c)

#define rPCONC		(*(volatile uint32_t *)0x1d20010)
#define rPDATC		(*(volatile uint32_t *)0x1d20014)
#define rPUPC		(*(volatile uint32_t *)0x1d20018)

#define rPCOND		(*(volatile uint32_t *)0x1d2001c)
#define rPDATD		(*(volatile uint32_t *)0x1d20020)
#define rPUPD		(*(volatile uint32_t *)0x1d20024)

#define rPCONE		(*(volatile uint32_t *)0x1d20028)
#define rPDATE		(*(volatile uint32_t *)0x1d2002c)
#define rPUPE		(*(volatile uint32_t *)0x1d20030)

#define rPCONF		(*(volatile uint32_t *)0x1d20034)
#define rPDATF		(*(volatile uint32_t *)0x1d20038)
#define rPUPF		(*(volatile uint32_t *)0x1d2003c)

#define rPCONG		(*(volatile uint32_t *)0x1d20040)
#define rPDATG		(*(volatile uint32_t *)0x1d20044)
#define rPUPG		(*(volatile uint32_t *)0x1d20048)

#define rSPUCR		(*(volatile uint32_t *)0x1d2004c)
#define rEXTINT		(*(volatile uint32_t *)0x1d20050)
#define rEXTINPND	(*(volatile uint32_t *)0x1d20054)

/* WATCHDOG */
#define rWTCON		(*(volatile uint32_t *)0x1d30000)
#define rWTDAT		(*(volatile uint32_t *)0x1d30004)
#define rWTCNT		(*(volatile uint32_t *)0x1d30008)

/* ADC */
#define rADCCON		(*(volatile uint32_t *)0x1d40000)
#define rADCPSR		(*(volatile uint32_t *)0x1d40004)
#define rADCDAT		(*(volatile uint32_t *)0x1d40008)

/* Timer */
#define rTCFG0		(*(volatile uint32_t *)0x1d50000)
#define rTCFG1		(*(volatile uint32_t *)0x1d50004)
#define rTCON		(*(volatile uint32_t *)0x1d50008)

#define rTCNTB0		(*(volatile uint32_t *)0x1d5000c)
#define rTCMPB0		(*(volatile uint32_t *)0x1d50010)
#define rTCNTO0		(*(volatile uint32_t *)0x1d50014)

#define rTCNTB1		(*(volatile uint32_t *)0x1d50018)
#define rTCMPB1		(*(volatile uint32_t *)0x1d5001c)
#define rTCNTO1		(*(volatile uint32_t *)0x1d50020)

#define rTCNTB2		(*(volatile uint32_t *)0x1d50024)
#define rTCMPB2		(*(volatile uint32_t *)0x1d50028)
#define rTCNTO2		(*(volatile uint32_t *)0x1d5002c)

#define rTCNTB3		(*(volatile uint32_t *)0x1d50030)
#define rTCMPB3		(*(volatile uint32_t *)0x1d50034)
#define rTCNTO3		(*(volatile uint32_t *)0x1d50038)

#define rTCNTB4		(*(volatile uint32_t *)0x1d5003c)
#define rTCMPB4		(*(volatile uint32_t *)0x1d50040)
#define rTCNTO4		(*(volatile uint32_t *)0x1d50044)

#define rTCNTB5		(*(volatile uint32_t *)0x1d50048)
#define rTCNTO5		(*(volatile uint32_t *)0x1d5004c)

/* IIC */
#define rIICCON		(*(volatile uint32_t *)0x1d60000)
#define rIICSTAT	(*(volatile uint32_t *)0x1d60004)
#define rIICADD		(*(volatile uint32_t *)0x1d60008)
#define rIICDS		(*(volatile uint32_t *)0x1d6000c)

/* RTC */
#ifdef __BIG_ENDIAN
#define rRTCCON		(*(volatile uint8_t *)0x1d70043)
#define rRTCALM		(*(volatile uint8_t *)0x1d70053)
#define rALMSEC		(*(volatile uint8_t *)0x1d70057)
#define rALMMIN		(*(volatile uint8_t *)0x1d7005b)
#define rALMHOUR	(*(volatile uint8_t *)0x1d7005f)
#define rALMDAY		(*(volatile uint8_t *)0x1d70063)
#define rALMMON		(*(volatile uint8_t *)0x1d70067)
#define rALMYEAR	(*(volatile uint8_t *)0x1d7006b)
#define rRTCRST		(*(volatile uint8_t *)0x1d7006f)
#define rBCDSEC		(*(volatile uint8_t *)0x1d70073)
#define rBCDMIN		(*(volatile uint8_t *)0x1d70077)
#define rBCDHOUR	(*(volatile uint8_t *)0x1d7007b)
#define rBCDDAY		(*(volatile uint8_t *)0x1d7007f)
#define rBCDDATE	(*(volatile uint8_t *)0x1d70083)
#define rBCDMON		(*(volatile uint8_t *)0x1d70087)
#define rBCDYEAR	(*(volatile uint8_t *)0x1d7008b)
#define rTICINT		(*(volatile uint8_t *)0x1d7008e)
#else
#define rRTCCON		(*(volatile uint8_t *)0x1d70040)
#define rRTCALM		(*(volatile uint8_t *)0x1d70050)
#define rALMSEC		(*(volatile uint8_t *)0x1d70054)
#define rALMMIN		(*(volatile uint8_t *)0x1d70058)
#define rALMHOUR	(*(volatile uint8_t *)0x1d7005c)
#define rALMDAY		(*(volatile uint8_t *)0x1d70060)
#define rALMMON		(*(volatile uint8_t *)0x1d70064)
#define rALMYEAR	(*(volatile uint8_t *)0x1d70068)
#define rRTCRST		(*(volatile uint8_t *)0x1d7006c)
#define rBCDSEC		(*(volatile uint8_t *)0x1d70070)
#define rBCDMIN		(*(volatile uint8_t *)0x1d70074)
#define rBCDHOUR	(*(volatile uint8_t *)0x1d70078)
#define rBCDDAY		(*(volatile uint8_t *)0x1d7007c)
#define rBCDDATE	(*(volatile uint8_t *)0x1d70080)
#define rBCDMON		(*(volatile uint8_t *)0x1d70084)
#define rBCDYEAR	(*(volatile uint8_t *)0x1d70088)
#define rTICINT		(*(volatile uint8_t *)0x1d7008c)
#endif

/* Clock & Power management */
#define rPLLCON		(*(volatile uint32_t *)0x1d80000)
#define rCLKCON		(*(volatile uint32_t *)0x1d80004)
#define rCLKSLOW	(*(volatile uint32_t *)0x1d80008)
#define rLOCKTIME	(*(volatile uint32_t *)0x1d8000c)

/* INTERRUPT */
#define rINTCON		(*(volatile uint32_t *)0x1e00000)
#define rINTPND		(*(volatile uint32_t *)0x1e00004)
#define rINTMOD		(*(volatile uint32_t *)0x1e00008)
#define rINTMSK		(*(volatile uint32_t *)0x1e0000c)
#define rI_PSLV		(*(volatile uint32_t *)0x1e00010)
#define rI_PMST		(*(volatile uint32_t *)0x1e00014)
#define rI_CSLV		(*(volatile uint32_t *)0x1e00018)
#define rI_CMST		(*(volatile uint32_t *)0x1e0001c)
#define rI_ISPR		(*(volatile uint32_t *)0x1e00020)
#define rI_ISPC		(*(volatile uint32_t *)0x1e00024)
#define rF_ISPR		(*(volatile uint32_t *)0x1e00038)
#define rF_ISPC		(*(volatile uint32_t *)0x1e0003c)

/* LCD */
#define rLCDCON1	(*(volatile uint32_t *)0x1f00000)
#define rLCDCON2	(*(volatile uint32_t *)0x1f00004)
#define rLCDCON3	(*(volatile uint32_t *)0x1f00040)

#define rLCDSADDR1	(*(volatile uint32_t *)0x1f00008)
#define rLCDSADDR2	(*(volatile uint32_t *)0x1f0000c)
#define rLCDSADDR3	(*(volatile uint32_t *)0x1f00010)

#define rREDLUT		(*(volatile uint32_t *)0x1f00014)
#define rGREENLUT	(*(volatile uint32_t *)0x1f00018)
#define rBLUELUT	(*(volatile uint32_t *)0x1f0001c)

#define rDP1_2		(*(volatile uint32_t *)0x1f00020)
#define rDP4_7		(*(volatile uint32_t *)0x1f00024)
#define rDP3_5		(*(volatile uint32_t *)0x1f00028)
#define rDP2_3		(*(volatile uint32_t *)0x1f0002c)
#define rDP5_7		(*(volatile uint32_t *)0x1f00030)
#define rDP3_4		(*(volatile uint32_t *)0x1f00034)
#define rDP4_5		(*(volatile uint32_t *)0x1f00038)
#define rDP6_7		(*(volatile uint32_t *)0x1f0003c)
#define rDITHMODE	(*(volatile uint32_t *)0x1f00044)

/* ZDMA0 */
#define rZDCON0		(*(volatile uint32_t *)0x1e80000)
#define rZDISRC0	(*(volatile uint32_t *)0x1e80004)
#define rZDIDES0	(*(volatile uint32_t *)0x1e80008)
#define rZDICNT0	(*(volatile uint32_t *)0x1e8000c)
#define rZDCSRC0	(*(volatile uint32_t *)0x1e80010)
#define rZDCDES0	(*(volatile uint32_t *)0x1e80014)
#define rZDCCNT0	(*(volatile uint32_t *)0x1e80018)

/* ZDMA1 */
#define rZDCON1		(*(volatile uint32_t *)0x1e80020)
#define rZDISRC1	(*(volatile uint32_t *)0x1e80024)
#define rZDIDES1	(*(volatile uint32_t *)0x1e80028)
#define rZDICNT1	(*(volatile uint32_t *)0x1e8002c)
#define rZDCSRC1	(*(volatile uint32_t *)0x1e80030)
#define rZDCDES1	(*(volatile uint32_t *)0x1e80034)
#define rZDCCNT1	(*(volatile uint32_t *)0x1e80038)

/* BDMA0 */
#define rBDCON0		(*(volatile uint32_t *)0x1f80000)
#define rBDISRC0	(*(volatile uint32_t *)0x1f80004)
#define rBDIDES0	(*(volatile uint32_t *)0x1f80008)
#define rBDICNT0	(*(volatile uint32_t *)0x1f8000c)
#define rBDCSRC0	(*(volatile uint32_t *)0x1f80010)
#define rBDCDES0	(*(volatile uint32_t *)0x1f80014)
#define rBDCCNT0	(*(volatile uint32_t *)0x1f80018)

/* BDMA1 */
#define rBDCON1		(*(volatile uint32_t *)0x1f80020)
#define rBDISRC1	(*(volatile uint32_t *)0x1f80024)
#define rBDIDES1	(*(volatile uint32_t *)0x1f80028)
#define rBDICNT1	(*(volatile uint32_t *)0x1f8002c)
#define rBDCSRC1	(*(volatile uint32_t *)0x1f80030)
#define rBDCDES1	(*(volatile uint32_t *)0x1f80034)
#define rBDCCNT1	(*(volatile uint32_t *)0x1f80038)

/* ISR */
extern void no_isr(void);

typedef void vf(void);

extern volatile vf *HandleADC;
extern volatile vf *HandleRTC; 
extern volatile vf *HandleUTXD1;
extern volatile vf *HandleUTXD0;
extern volatile vf *HandleSIO;
extern volatile vf *HandleIIC;
extern volatile vf *HandleURXD1;
extern volatile vf *HandleURXD0;
extern volatile vf *HandleTIMER5;
extern volatile vf *HandleTIMER4;
extern volatile vf *HandleTIMER3;
extern volatile vf *HandleTIMER2;
extern volatile vf *HandleTIMER1;
extern volatile vf *HandleTIMER0;
extern volatile vf *HandleUERR01;
extern volatile vf *HandleWDT;
extern volatile vf *HandleBDMA1;
extern volatile vf *HandleBDMA0;
extern volatile vf *HandleZDMA1;
extern volatile vf *HandleZDMA0;
extern volatile vf *HandleTICK;
extern volatile vf *HandleEINT4567;
extern volatile vf *HandleEINT3;
extern volatile vf *HandleEINT2;
extern volatile vf *HandleEINT1;
extern volatile vf *HandleEINT0;

typedef void (*vfentry)(void)__attribute__((long_call));

extern vfentry interruptVectorTable[] /*__attribute__ ((section (".isr_bss")))*/;

#define pISR_ADC	HandleADC
#define pISR_RTC	HandleRTC
#define pISR_UTXD1	HandleUTXD1
#define pISR_UTXD0	HandleUTXD0
#define pISR_SIO	HandleSIO
#define pISR_IIC	HandleIIC
#define pISR_URXD1	HandleURXD1
#define pISR_URXD0	HandleURXD0
#define pISR_TIMER5	HandleTIMER5
#define pISR_TIMER4	HandleTIMER4
#define pISR_TIMER3	HandleTIMER3
#define pISR_TIMER2	HandleTIMER2
#define pISR_TIMER1	HandleTIMER1
#define pISR_TIMER0	HandleTIMER0
#define pISR_UERR01	HandleUERR01
#define pISR_WDT	HandleWDT
#define pISR_BDMA1	HandleBDMA1
#define pISR_BDMA0	HandleBDMA0
#define pISR_ZDMA1	HandleZDMA1
#define pISR_ZDMA0	HandleZDMA0
#define pISR_TICK	HandleTICK
#define pISR_EINT4567	HandleEINT4567
#define pISR_EINT3	HandleEINT3
#define pISR_EINT2	HandleEINT2
#define pISR_EINT1	HandleEINT1
#define pISR_EINT0	HandleEINT0


/* PENDING BIT */

#define BIT_ADC		(0x1)
#define BIT_RTC		(0x1<<1)
#define BIT_UTXD1	(0x1<<2)
#define BIT_UTXD0	(0x1<<3)
#define BIT_SIO		(0x1<<4)
#define BIT_IIC		(0x1<<5)
#define BIT_URXD1	(0x1<<6)
#define BIT_URXD0	(0x1<<7)
#define BIT_TIMER5	(0x1<<8)
#define BIT_TIMER4	(0x1<<9)
#define BIT_TIMER3	(0x1<<10)
#define BIT_TIMER2	(0x1<<11)
#define BIT_TIMER1	(0x1<<12)
#define BIT_TIMER0	(0x1<<13)
#define BIT_UERR01	(0x1<<14)
#define BIT_WDT		(0x1<<15)
#define BIT_BDMA1	(0x1<<16)
#define BIT_BDMA0	(0x1<<17)
#define BIT_ZDMA1	(0x1<<18)
#define BIT_ZDMA0	(0x1<<19)
#define BIT_TICK	(0x1<<20)
#define BIT_EINT4567	(0x1<<21)
#define BIT_EINT3	(0x1<<22)
#define BIT_EINT2	(0x1<<23)
#define BIT_EINT1	(0x1<<24)
#define BIT_EINT0	(0x1<<25)
#define BIT_GLOBAL	(0x1<<26)

#endif 


