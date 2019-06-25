#include <stdio.h>
#include <stdlib.h>

typedef struct {
   float      lastinput;
   float      lastoutput;
   float      gain;
} MyObject;

void MessageClear(MyObject *mo) {
   mo->lastinput  = 0.0;
   mo->lastoutput = 0.0;
}

void dc_filter_base(MyObject *mo, short *input, short *output, int count)
{
   for (int i=0; i<count; i++) {
      output[i] = input[i] - mo->lastinput + mo->gain * mo->lastoutput;
      mo->lastinput  = input[i];
      mo->lastoutput = output[i];
   }

}

MyObject *mo = NULL;

void* create_object(void) 
{
   mo = (MyObject*)malloc(sizeof(MyObject));
   mo->gain = 0.6;
   MessageClear(mo);
}
#define FRAME_LEN 8
int main()
{
	short input[] = {1000,960,1023,1002,970,990,1034,1089};
	short output[FRAME_LEN];
	
	create_object();
	dc_filter_base(mo, input,output,FRAME_LEN);

	for(int cnt = 0; cnt < FRAME_LEN; cnt++)
	{
		printf("cnt:%d out:%d \n\t",cnt, output[cnt]);
	}

	return 0;
}
