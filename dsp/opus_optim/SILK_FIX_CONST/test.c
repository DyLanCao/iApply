#include <stdio.h>

typedef int			int32_t;
typedef int32_t opus_int32;
#define opus_int64       long long
#define MAX_PREDICTION_POWER_GAIN_AFTER_RESET   1e2f

#define SILK_FIX_CONST_FLOAT( C, Q )              ((opus_int32)((C) * ((opus_int64)1 << (Q)) + 0.5))
#define SILK_FIX_CONST( C, Q )              ((opus_int32)(2*(C) * ((opus_int64)1 << (Q)) + 1)>>1)

int main()
{
   float C = 0.99;
   //float C = 100.0;
   int Q = 16;
	
   printf("c:%f",C);
   printf("float:%d fixed:%d",SILK_FIX_CONST_FLOAT(C,Q),SILK_FIX_CONST(C,Q));

   return 0;
}
