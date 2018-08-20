#ifndef _FRONT_H_
#define _FRONT_H_

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

extern int echo_init(void);
extern void echo_process(short* signal_in, short* signal_out, int in_length);
extern void resample_8k_up16k(short* pInAudioData, int nInAudioLen, short* pOutAudioData, int& nOutAudioLen);

/*
#ifdef __cplusplus
}
#endif
*/
#endif
