/* Define to prevent recursive inclusion -------------------------------------*/

#include "stdint.h"

/* Private macro -------------------------------------------------------------*/
// CIC filter setting
#define DECIMATION_M        20
#define PDM_SAMPLE_SIZE     4096

/* Exported types ------------------------------------------------------------*/
/* Exported enum tag ---------------------------------------------------------*/
/* Exported struct/union tag -------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

extern struct CicFilter_t PDM_st;
uint8_t PDM_Buff[PDM_SAMPLE_SIZE * DECIMATION_M / 8 * 2];     /* <Sample Size> * <Decimation of CIC filter> / <bits per 1Byte> * <for Double buffer> */
volatile uint8_t PDM_RawBits[PDM_SAMPLE_SIZE * DECIMATION_M / 8];
int32_t PDM_Filtered_int32[PDM_SAMPLE_SIZE];

/* Exported function prototypes ----------------------------------------------*/
void initializeCicFilterStruct(uint8_t CicFilterOrder, uint32_t CicFilterDecimation, struct CicFilter_t* st);
void executeCicFilter(uint8_t* pInBit, uint32_t pInBit_Num, int32_t* pOut_int32, struct CicFilter_t* st);
void test_func(void);


