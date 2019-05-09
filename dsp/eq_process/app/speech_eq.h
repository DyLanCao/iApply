#ifndef __SPEECH_EQ_H__
#define __SPEECH_EQ_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define IIR_PARAM_NUM                       (6)

enum AUD_BITS_T {
    AUD_BITS_8 = 8,
    AUD_BITS_12 = 12,
    AUD_BITS_16 = 16,
    AUD_BITS_20 = 20,
    AUD_BITS_24 = 24,
    AUD_BITS_32 = 32,

    AUD_BITS_NULL = 0xff,
};

enum AUD_SAMPRATE_T {
    AUD_SAMPRATE_7350 = 7350,
    AUD_SAMPRATE_8000 = 8000,
    AUD_SAMPRATE_8463 = 8463,       // 26M / 512 / 6
    AUD_SAMPRATE_14700 = 14700,
    AUD_SAMPRATE_16000 = 16000,
    AUD_SAMPRATE_16927 = 16927,     // 26M / 512 / 3
    AUD_SAMPRATE_22050 = 22050,
    AUD_SAMPRATE_24000 = 24000,
    AUD_SAMPRATE_32000 = 32000,
    AUD_SAMPRATE_44100 = 44100,
    AUD_SAMPRATE_48000 = 48000,
    AUD_SAMPRATE_50781 = 50781,     // 26M / 512
    AUD_SAMPRATE_88200 = 88200,
    AUD_SAMPRATE_96000 = 96000,
    AUD_SAMPRATE_128000 = 128000,
    AUD_SAMPRATE_176400 = 176400,
    AUD_SAMPRATE_192000 = 192000,
    AUD_SAMPRATE_352800 = 352800,
    AUD_SAMPRATE_384000 = 384000,
    AUD_SAMPRATE_768000 = 768000,

    AUD_SAMPRATE_NULL = 0,
};

typedef enum {
    SPEECH_LOW_SHELF = 0,
    SPEECH_PEAK,
    SPEECH_HIGH_SHELF,
    SPEECH_LOW_PASS,
    SPEECH_HIGH_PASS,
    SPEECH_NUM
} SPEECH_TYPE_EQ;


typedef struct {
    SPEECH_TYPE_EQ  type;
    float       gain;
    float       fc;
    float       Q;
} SPEECH_EQ_PARAM_T;

typedef struct {
    float   gain0;
    float   gain1;
    int     num;
    SPEECH_EQ_PARAM_T param[IIR_PARAM_NUM];
} EqConfig;

typedef struct {
    float coefs[6];
    float history[2][4];
} IIR_COEF_T;

enum AUD_CHANNEL_NUM_T {
    AUD_CHANNEL_NUM_1 = 1,
    AUD_CHANNEL_NUM_2 = 2,
    AUD_CHANNEL_NUM_3 = 3,
    AUD_CHANNEL_NUM_4 = 4,
    AUD_CHANNEL_NUM_5 = 5,
    AUD_CHANNEL_NUM_6 = 6,
    AUD_CHANNEL_NUM_7 = 7,
    AUD_CHANNEL_NUM_8 = 8,

    AUD_CHANNEL_NUM_NULL = 0xff,
};

typedef struct {
    enum AUD_BITS_T     sample_bits;
    enum AUD_SAMPRATE_T sample_rate;
    enum AUD_CHANNEL_NUM_T  chan_num;

    float   gain0;
    float   gain1;
    int     num;
    IIR_COEF_T coef[IIR_PARAM_NUM];
} IIR_RUN_CFG_T;



int speech_iir_open(IIR_RUN_CFG_T* iir_run_cfg, enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits, enum AUD_CHANNEL_NUM_T chan_num);
int speech_iir_set_cfg(IIR_RUN_CFG_T* iir_run_cfg, const EqConfig *cfg);
int speech_iir_run(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, uint32_t len);
int speech_iir_close(IIR_RUN_CFG_T* iir_run_cfg);

//int test(void);

//void update_iir_cfg_tbl(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, uint32_t  len);

#ifdef __cplusplus
}
#endif

#endif
