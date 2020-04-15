#ifndef _FRONT_H_
#define _FRONT_H_

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

const unsigned char CN_POWER_ON [] = {
//#include "SAM16K.txt"
#include "SOUND_ANSWER.txt"
};


extern int ibuff_init(void);
extern void ibuff_process(short* signal_in, short* signal_out, int in_length);
extern void exit_ibuff(void);
/*
#ifdef __cplusplus
}
#endif
*/
#endif
