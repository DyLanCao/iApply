#ifndef SPEECH_EQ_H
#define SPEECH_EQ_H

#include <stdio.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "speech_eq.h"
typedef signed int                          iir_sample_24bits_t;
typedef signed short int                    iir_sample_16bits_t;

#define IIR_TRACE                           printf
#define TRACE                           printf

#define PI                                  3.14159265358979

int32_t __SSAT(int32_t val, uint32_t sat)
{
  if ((sat >= 1U) && (sat <= 32U))
  {
    const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
    const int32_t min = -1 - max ;
    if (val > max)
    {
      return max;
    }
    else if (val < min)
    {
      return min;
    }
  }
  return val;
}

//static IIR_RUN_CFG_T iir_run_cfg;

typedef struct {
    uint32_t sample_rate;
    int     num;
    float   gain0;      // left gain: it is usually negative, used to prevent saturation
    float   gain1;      // right gain: it is usually negative, used to prevent saturation
    float   coef[IIR_PARAM_NUM][6];
} IIR_CFG_TBL_T;

/*
IIR_CFG_TBL_T iir_cfg_tbl[] = {
    {AUD_SAMPRATE_44100,    0, 0, 0, {{0}} },
    {AUD_SAMPRATE_48000,    0, 0, 0, {{0}} },
    // {AUD_SAMPRATE_88200,    0, 0, 0, {{0}} },
    {AUD_SAMPRATE_96000,    0, 0, 0, {{0}} },
};


IIR_CFG_TBL_T* find_iir_cfg_by_sample_rate(enum AUD_SAMPRATE_T sample_rate)
{
    IIR_CFG_TBL_T* cfg = NULL;
    int i;
    for (i = 0; i < sizeof(iir_cfg_tbl) / sizeof(IIR_CFG_TBL_T); i++) {
        if (sample_rate == iir_cfg_tbl[i].sample_rate) {
            cfg = &iir_cfg_tbl[i];
            break;
        }
    }

    return cfg;
}

int dynamic_iir_set_cfg(IIR_RUN_CFG_T* iir_run_cfg)
{
    uint32_t sample_rate;
    uint16_t i;
    IIR_CFG_TBL_T* cfg;
    
    sample_rate = iir_run_cfg->sample_rate;
    cfg = find_iir_cfg_by_sample_rate(sample_rate);

    TRACE("[%s] sample_rate = %d", __func__, sample_rate);

    if (cfg) {
        iir_run_cfg->num = cfg->num;
        iir_run_cfg->gain0 = cfg->gain0;
        iir_run_cfg->gain1 = cfg->gain1;

        for (i = 0; i < iir_run_cfg->num; i++) {
            memcpy(iir_run_cfg->coef[i].coefs, cfg->coef[i], sizeof(cfg->coef[i]));
        }
    }

    return 0;
}
*/

// y = 20log(x)
static inline float convert_multiple_to_db(float multiple)
{
    return 20*(float)log10(multiple);
}

int aconvert_multiple_to_db(int multiple)
{
	int test = log(multiple);
    return test;
}
// x = 10^(y/20)
static inline float convert_db_to_multiple(float db)
{
    return (float)pow(10, db/20);
}

void iir_lp_coefs(float gain, float fn, float Q, float *coefs)
{
    float w0 = 2 * PI*fn;
    float alpha = (float)sin(w0) / (2 * Q);

    float a0 = 1 + alpha;
    float a1 = -2*cos(w0);
    float a2 = 1 - alpha;
    float b0 = (1 - cos(w0))/2;
    float b1 = 1 - cos(w0);
    float b2 = (1 - cos(w0))/2;

    coefs[0] = 1;
    coefs[1] = a1/a0;
    coefs[2] = a2/a0;
    coefs[3] = b0/a0;
    coefs[4] = b1/a0;
    coefs[5] = b2/a0;
}


static void iir_hp_coefs(float gain, float fn, float Q, float *coefs)
{
    float w0 = 2 * PI*fn;
    float alpha = (float)sin(w0) / (2 * Q);

    float a0 = 1 + alpha;
    float a1 = -2*cos(w0);
    float a2 = 1 - alpha;
    float b0 = (1 + cos(w0))/2;
    float b1 = -(1 + cos(w0));
    float b2 = (1 + cos(w0))/2;

    coefs[0] = 1;
    coefs[1] = a1/a0;
    coefs[2] = a2/a0;
    coefs[3] = b0/a0;
    coefs[4] = b1/a0;
    coefs[5] = b2/a0;
}

static void iir_ls_coefs(float gain, float fn, float Q, float *coefs)
{
    float A = (float)sqrt(pow(10, gain / 20));
    float w0 = 2 * PI*fn;
    float alpha = (float)sin(w0)/(2*Q);

    float a0 = (A + 1) + (A - 1)*(float)cos(w0) + 2 * (float)sqrt(A)*alpha;
    float a1 = (-2 * ((A - 1) + (A + 1)*(float)cos(w0)));
    float a2 = ((A + 1) + (A - 1)*(float)cos(w0) - 2 * (float)sqrt(A)*alpha);
    float b0 = (A*((A + 1) - (A - 1)*(float)cos(w0) + 2 *(float)sqrt(A)*alpha));
    float b1 = (2 * A*((A - 1) - (A + 1)*(float)cos(w0)));
    float b2 = (A*((A + 1) - (A - 1)*(float)cos(w0) - 2 * (float)sqrt(A)*alpha));

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;
    
//    IIR_TRACE("fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
    IIR_TRACE("[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

static void iir_hs_coefs(float gain, float fn, float Q, float *coefs)
{
    float A = (float)sqrt(pow(10, gain / 20));
    float w0 = 2 * PI*fn;
    float alpha = (float)sin(w0) / (2 * Q);

    float a0 = (A + 1) - (A - 1)*(float)cos(w0) + 2 * (float)sqrt(A)*alpha;  //  a0
    float a1 = (2 * ((A - 1) - (A + 1)*(float)cos(w0)));  // a1
    float a2 = ((A + 1) - (A - 1)*(float)cos(w0) - 2 * (float)sqrt(A)*alpha);  //a2
    float b0 = (A*((A + 1) + (A - 1)*(float)cos(w0) + 2 * (float)sqrt(A)*alpha));  //b0
    float b1 = (-2 * A*((A - 1) + (A + 1)*(float)cos(w0)));   // b1
    float b2 = (A*((A + 1) + (A - 1)*(float)cos(w0) - 2 * (float)sqrt(A)*alpha));  // b2

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;
    
//    IIR_TRACE("fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
    IIR_TRACE("[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

// fn: Normalized frequency
static void iir_pk_coefs(float gain, float fn, float Q, float *coefs)
{
//    float Fs = 1000.0 / 48000.0;
//    float gain = -5.0;
//    float fo = 1000.0;
//    float Q = 0.7;
    float A = (float)sqrt(pow(10, gain/20));
    float w0 = 2*PI*fn; 
    float alpha = (float)sin(w0)/(2*Q); 

    float a0 = 1 + alpha/A; 
    float a1 = -2*(float)cos(w0); 
    float a2 = 1 - alpha/A;
    float b0 = 1 + alpha*A; 
    float b1 = -2*(float)cos(w0); 
    float b2 = 1 - alpha*A; 

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;
    
//    IIR_TRACE("fn = %f, gain = %f, Q = %f, A = %f, w0 = %f, alpha = %f", fn, gain, Q, A, w0, alpha);
//    IIR_TRACE("[%s] gain[%f], fn[%f], Q[%f]", __func__, (double)gain, (double)fn, (double)Q);
    IIR_TRACE("[%s] %f, %f, %f, %f, %f, %f", __func__, (double)coefs[0], (double)coefs[1], (double)coefs[2], (double)coefs[3], (double)coefs[4], (double)coefs[5]);
}

static inline iir_sample_16bits_t iir_ssat_16bits(float in)
{
    int res = 0;
    iir_sample_16bits_t out;

    res = (int)in;
    out = __SSAT(res,16);

    return out;
}

static inline iir_sample_24bits_t iir_ssat_24bits(float in)
{
    int res = 0;
    iir_sample_24bits_t out;

    res = (int)in;
    out = __SSAT(res,24);

    return out;
}
#if 0
// Optimize iir eq:
// use fixed point
// use arm function
// delete value exchange(repeat 3 times, care first and last)
// iir should be done in hw fir
// history store local 
int iir_run_16bits(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, int len)
{
    iir_sample_16bits_t *iir_buf;
    int len_mono;
    int num;
    float gain0 = 0, gain1 = 0;
    float *coefs = NULL;
    float *history = NULL;

    // Left
    float x0, x1, x2;
    float y0, y1, y2;

    // Right
    float X0, X1, X2;
    float Y0, Y1, Y2;

    // Coefs
    float a0, a1, a2;
    float b0, b1, b2;

    len_mono = len>>1;

    gain0 = iir_run_cfg->gain0;
    gain1 = iir_run_cfg->gain1;
    num = iir_run_cfg->num;
    iir_buf = (iir_sample_16bits_t *)buf;

    if(num==0)
    {
        iir_buf = (iir_sample_16bits_t *)buf;

        for(int j=0; j<len_mono; j++)
        {
            x0 = *iir_buf*gain0;
            *iir_buf++ = iir_ssat_16bits(x0);

            x0 = *iir_buf*gain1;
            *iir_buf++ = iir_ssat_16bits(x0);
        }

        return 0;
    }

    for(int i=0; i<num; i++)
    {
        // Coef
        coefs = iir_run_cfg->coef[i].coefs;
        a0 = *coefs++;
        a1 = *coefs++;
        a2 = *coefs++;
        b0 = *coefs++;
        b1 = *coefs++;
        b2 = *coefs;

//        TRACE("[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

        // Left
        history = iir_run_cfg->coef[i].history[0];
        x1 = *history++;
        x2 = *history++;
        y1 = *history++;
        y2 = *history;

        // Right
        history = iir_run_cfg->coef[i].history[1];
        X1 = *history++;
        X2 = *history++;
        Y1 = *history++;
        Y2 = *history;
      
        iir_buf = (iir_sample_16bits_t *)buf;
        if(i==0)
        {
            for(int j=0; j<len_mono; j++)
            {
                // Left channel
                // x0 = *iir_buf>>1;   // 0.5: -6dB, 0.63: -4dB
                x0 = *iir_buf*gain0;
                y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                y2 = y1;
                y1 = y0;
                x2 = x1;
                x1 = x0;

                *iir_buf++ = iir_ssat_16bits(y0);

                // Righy channel
                // X0 = *iir_buf>>1;
                X0 = *iir_buf*gain1;
                Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
                Y2 = Y1;
                Y1 = Y0;
                X2 = X1;
                X1 = X0;

                *iir_buf++ = iir_ssat_16bits(Y0);
            }
        }
        else
        {
            for(int j=0; j<len_mono; j++)
            {
                // Left channel
                x0 = *iir_buf;
                y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                y2 = y1;
                y1 = y0;
                x2 = x1;
                x1 = x0;

                *iir_buf++ = iir_ssat_16bits(y0);

                // Righy channel
                X0 = *iir_buf;
                Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
                Y2 = Y1;
                Y1 = Y0;
                X2 = X1;
                X1 = X0;

                *iir_buf++ = iir_ssat_16bits(Y0);
            }
        }

        // Left
        history = iir_run_cfg->coef[i].history[0];
        *history++ = x1;
        *history++ = x2;
        *history++ = y1;
        *history = y2;

        // Right
        history = iir_run_cfg->coef[i].history[1];
        *history++ = X1;
        *history++ = X2;
        *history++ = Y1;
        *history = Y2;
    }

    return 0;    
}

int iir_run_24bits(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, int len)
{
    iir_sample_24bits_t *iir_buf;
    int len_mono;
    int num;
    float gain0 = 0, gain1 = 0;
    float *coefs = NULL;
    float *history = NULL;

    // Left
    float x0, x1, x2;
    float y0, y1, y2;

    // Right
    float X0, X1, X2;
    float Y0, Y1, Y2;

    // Coefs
    float a0, a1, a2;
    float b0, b1, b2;

    len_mono = len>>1;

    gain0 = iir_run_cfg->gain0;
    gain1 = iir_run_cfg->gain1;
    num = iir_run_cfg->num;
    iir_buf = (iir_sample_24bits_t *)buf;

    if(num==0)
    {
        iir_buf = (iir_sample_24bits_t *)buf;

        for(int j=0; j<len_mono; j++)
        {
            x0 = *iir_buf*gain0;
            *iir_buf++ = iir_ssat_24bits(x0);

            x0 = *iir_buf*gain1;
            *iir_buf++ = iir_ssat_24bits(x0);
        }

        return 0;
    }

    for(int i=0; i<num; i++)
    {
        // Coef
        coefs = iir_run_cfg->coef[i].coefs;
        a0 = *coefs++;
        a1 = *coefs++;
        a2 = *coefs++;
        b0 = *coefs++;
        b1 = *coefs++;
        b2 = *coefs;

//        TRACE("[%d] %f, %f, %f, %f, %f, %f", i, a0, a1, a2, b0, b1, b2);

        // Left
        history = iir_run_cfg->coef[i].history[0];
        x1 = *history++;
        x2 = *history++;
        y1 = *history++;
        y2 = *history;

        // Right
        history = iir_run_cfg->coef[i].history[1];
        X1 = *history++;
        X2 = *history++;
        Y1 = *history++;
        Y2 = *history;

        iir_buf = (iir_sample_24bits_t *)buf;
        if(i==0)
        {
            for(int j=0; j<len_mono; j++)
            {
                // Left channel
                // x0 = *iir_buf>>1;   // 0.5: -6dB, 0.63: -4dB
                x0 = *iir_buf*gain0;
                y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                y2 = y1;
                y1 = y0;
                x2 = x1;
                x1 = x0;

                *iir_buf++ = iir_ssat_24bits(y0);

                // Righy channel
                // X0 = *iir_buf>>1;
                X0 = *iir_buf*gain1;
                Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
                Y2 = Y1;
                Y1 = Y0;
                X2 = X1;
                X1 = X0;

                *iir_buf++ = iir_ssat_24bits(Y0);
            }
        }
        else
        {
            for(int j=0; j<len_mono; j++)
            {
                // Left channel
                x0 = *iir_buf;
                y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
                y2 = y1;
                y1 = y0;
                x2 = x1;
                x1 = x0;

                *iir_buf++ = iir_ssat_24bits(y0);

                // Righy channel
                X0 = *iir_buf;
                Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
                Y2 = Y1;
                Y1 = Y0;
                X2 = X1;
                X1 = X0;

                *iir_buf++ = iir_ssat_24bits(Y0);
            }
        }

        // Left
        history = iir_run_cfg->coef[i].history[0];
        *history++ = x1;
        *history++ = x2;
        *history++ = y1;
        *history = y2;

        // Right
        history = iir_run_cfg->coef[i].history[1];
        *history++ = X1;
        *history++ = X2;
        *history++ = Y1;
        *history = Y2;
    }

    return 0;    
}

#else

int iir_run_16bits(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, int len)
{
    iir_sample_16bits_t *iir_buf;
    int len_mono;
    int num;
    float gain0 = 0, gain1 = 0;
    float *coefs = NULL;
    float *history = NULL;

    // Left
    float x0, x1, x2;
    float y0, y1, y2;

    // Right
    float X0, X1, X2;
    float Y0, Y1, Y2;

    // Coefs
    float a0, a1, a2;
    float b0, b1, b2;

    len_mono = len>>1;

    gain0 = iir_run_cfg->gain0;
    gain1 = iir_run_cfg->gain1;
    num = iir_run_cfg->num;
    iir_buf = (iir_sample_16bits_t *)buf;

    if(num==0)
    {
        iir_buf = (iir_sample_16bits_t *)buf;

        for(int j=0; j<len_mono; j++)
        {
            x0 = *iir_buf*gain0;
            *iir_buf++ = iir_ssat_16bits(x0);

            x0 = *iir_buf*gain1;
            *iir_buf++ = iir_ssat_16bits(x0);
        }

        return 0;
    }

    for (int j = 0; j < len_mono; j++) {
        
        // left channel
        x0 = iir_buf[0];
        // right channel
        X0 = iir_buf[1];
        
        for (int i = 0; i < num; i++) {
            // to get coef
            coefs = iir_run_cfg->coef[i].coefs;
            a0 = *coefs++;
            a1 = *coefs++;
            a2 = *coefs++;
            b0 = *coefs++;
            b1 = *coefs++;
            b2 = *coefs;

            // to get left history
            history = iir_run_cfg->coef[i].history[0];
            x1 = *history++;
            x2 = *history++;
            y1 = *history++;
            y2 = *history;

            // to get right history
            history = iir_run_cfg->coef[i].history[1];
            X1 = *history++;
            X2 = *history++;
            Y1 = *history++;
            Y2 = *history;

            // left channel calc
            y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
            y2 = y1;
            y1 = y0;
            x2 = x1;
            x1 = x0;
            
            x0 = y0;

            // right channel calc
            Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
            Y2 = Y1;
            Y1 = Y0;
            X2 = X1;
            X1 = X0;

            X0 = Y0;

            // to store left history
            history = iir_run_cfg->coef[i].history[0];
            *history++ = x1;
            *history++ = x2;
            *history++ = y1;
            *history = y2;

            // to store right history
            history = iir_run_cfg->coef[i].history[1];
            *history++ = X1;
            *history++ = X2;
            *history++ = Y1;
            *history = Y2;
        }

        *iir_buf++ = iir_ssat_16bits(x0*gain0);
        *iir_buf++ = iir_ssat_16bits(X0*gain1);
    }

    return 0;    
}
int iir_run_24bits(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, int len)
{
    iir_sample_24bits_t *iir_buf;
    int len_mono;
    int num;
    float gain0 = 0, gain1 = 0;
    float *coefs = NULL;
    float *history = NULL;

    // Left
    float x0, x1, x2;
    float y0, y1, y2;

    // Right
    float X0, X1, X2;
    float Y0, Y1, Y2;

    // Coefs
    float a0, a1, a2;
    float b0, b1, b2;

    len_mono = len>>1;

    gain0 = iir_run_cfg->gain0;
    gain1 = iir_run_cfg->gain1;
    num = iir_run_cfg->num;
    iir_buf = (iir_sample_24bits_t *)buf;

    if(num==0)
    {
        iir_buf = (iir_sample_24bits_t *)buf;

        for(int j=0; j<len_mono; j++)
        {
            x0 = *iir_buf*gain0;
            *iir_buf++ = iir_ssat_24bits(x0);

            x0 = *iir_buf*gain1;
            *iir_buf++ = iir_ssat_24bits(x0);
        }

        return 0;
    }

    for (int j = 0; j < len_mono; j++) {
        
        // left channel
        x0 = iir_buf[0];
        // right channel
        X0 = iir_buf[1];
        
        for (int i = 0; i < num; i++) {
            
            // to get coef
            coefs = iir_run_cfg->coef[i].coefs;
            a0 = *coefs++;
            a1 = *coefs++;
            a2 = *coefs++;
            b0 = *coefs++;
            b1 = *coefs++;
            b2 = *coefs;

            // to get left history
            history = iir_run_cfg->coef[i].history[0];
            x1 = *history++;
            x2 = *history++;
            y1 = *history++;
            y2 = *history;

            // to get right history
            history = iir_run_cfg->coef[i].history[1];
            X1 = *history++;
            X2 = *history++;
            Y1 = *history++;
            Y2 = *history;

            // left channel calc
            y0 = x0*b0 + x1*b1 + x2*b2 - y1*a1 - y2*a2;
            y2 = y1;
            y1 = y0;
            x2 = x1;
            x1 = x0;
            
            x0 = y0;

            // right channel calc
            Y0 = X0*b0 + X1*b1 + X2*b2 - Y1*a1 - Y2*a2;
            Y2 = Y1;
            Y1 = Y0;
            X2 = X1;
            X1 = X0;

            X0 = Y0;

            // to store left history
            history = iir_run_cfg->coef[i].history[0];
            *history++ = x1;
            *history++ = x2;
            *history++ = y1;
            *history = y2;

            // to store right history
            history = iir_run_cfg->coef[i].history[1];
            *history++ = X1;
            *history++ = X2;
            *history++ = Y1;
            *history = Y2;            
        }

        *iir_buf++ = iir_ssat_24bits(x0*gain0);
        *iir_buf++ = iir_ssat_24bits(X0*gain1);
    }

    return 0;    
}

#endif

int speech_iir_run(IIR_RUN_CFG_T* iir_run_cfg, uint8_t *buf, uint32_t len)
{
    int ret;
    
    if(iir_run_cfg->sample_bits == AUD_BITS_16)
    {
        ret = iir_run_16bits(iir_run_cfg, buf, len);
    }
    else if(iir_run_cfg->sample_bits == AUD_BITS_24)
    {
        ret = iir_run_24bits(iir_run_cfg, buf, len);
    }
    else
    {
        //ASSERT(0, "[%s] bits(%d) is invalid", __func__, iir_run_cfg->sample_bits);
    }

    return ret;
}

int speech_iir_open(IIR_RUN_CFG_T* iir_run_cfg, enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits, enum AUD_CHANNEL_NUM_T chan_num)
{
    // float *history0 = NULL;
    // float *history1 = NULL;

    // Check parameter
    iir_run_cfg->sample_rate = sample_rate;
    iir_run_cfg->sample_bits = sample_bits;
    iir_run_cfg->chan_num = chan_num;

    for(int i=0; i<IIR_PARAM_NUM; i++)
    {
        memset(iir_run_cfg->coef[i].history, 0, sizeof(iir_run_cfg->coef[i].history));
    }

    TRACE("[%s] len = %d, gain0 = %f, gain1 = %f", __func__, iir_run_cfg->num, (double)iir_run_cfg->gain0, (double)iir_run_cfg->gain1);

    // TRACE("[%s] float = %d", __func__, iir_run_cfg->coef[0].history[0][0]);
    // history0 = iir_run_cfg->coef[i].history[0];
    // history1 = iir_run_cfg->coef[i].history[1];
    // for(int i=0; i<4; i++)
    // {
    //     *history0++ = 0.0;
    //     *history1++ = 0.0;
    // }

    return 0;
}

int speech_iir_set_cfg(IIR_RUN_CFG_T* iir_run_cfg, const EqConfig *cfg)
{
    // Check parameter
    //ASSERT(cfg->num < IIR_PARAM_NUM, "[%s] num(%d) is too large", __func__, cfg->num);

    iir_run_cfg->num = cfg->num;
    iir_run_cfg->gain0 = convert_db_to_multiple(cfg->gain0);
    iir_run_cfg->gain1 = convert_db_to_multiple(cfg->gain1);

    IIR_TRACE("[%s] gain0 = %f, gain1 = %f", __func__, (double)iir_run_cfg->gain0, (double)iir_run_cfg->gain1);
    IIR_TRACE("[%s] len = %d", __func__, iir_run_cfg->num);



    for(int i=0; i<iir_run_cfg->num; i++)
    {
        IIR_TRACE("cfg->param[i].type:%d ", cfg->param[i].type);

        if (cfg->param[i].type == SPEECH_LOW_SHELF) {  
            // ASSERT(0, "[%s] IIR_TYPE_LOW_SHELF is not supported", __func__);          
            iir_ls_coefs(cfg->param[i].gain,
                            cfg->param[i].fc / iir_run_cfg->sample_rate, 
                            cfg->param[i].Q, 
                            iir_run_cfg->coef[i].coefs);
        } else if (cfg->param[i].type == SPEECH_PEAK) {        // pk
            iir_pk_coefs(cfg->param[i].gain,
                            cfg->param[i].fc / iir_run_cfg->sample_rate, 
                            cfg->param[i].Q, 
                            iir_run_cfg->coef[i].coefs);
            IIR_TRACE("sample_rate = %d fc:%f Q:%f ", iir_run_cfg->sample_rate,cfg->param[i].fc,cfg->param[i].Q);
        
        } else if (cfg->param[i].type == SPEECH_HIGH_SHELF) {
            // ASSERT(0, "[%s] IIR_TYPE_HIGH_SHELF is not supported", __func__);
            iir_hs_coefs(cfg->param[i].gain,
                            cfg->param[i].fc / iir_run_cfg->sample_rate, 
                            cfg->param[i].Q, 
                            iir_run_cfg->coef[i].coefs);
        } else if (cfg->param[i].type == SPEECH_LOW_PASS) {            
            iir_lp_coefs(cfg->param[i].gain,                                // low pass filter
                            cfg->param[i].fc / iir_run_cfg->sample_rate, 
                            cfg->param[i].Q, 
                            iir_run_cfg->coef[i].coefs);
        } else if (cfg->param[i].type == SPEECH_HIGH_PASS) {      // high pass filter
            iir_hp_coefs(cfg->param[i].gain,
                            cfg->param[i].fc / iir_run_cfg->sample_rate, 
                            cfg->param[i].Q, 
                            iir_run_cfg->coef[i].coefs);
        } else {
            //ASSERT(0, "[%s] %d is not supported", __func__, cfg->param[i].type);
        }
    }

    return 0;
}

int speech_iir_close(IIR_RUN_CFG_T* iir_run_cfg)
{
    for(int i=0; i<IIR_PARAM_NUM; i++)
    {
        memset(iir_run_cfg->coef[i].history, 0, sizeof(iir_run_cfg->coef[i].history));
    }
    TRACE("[%s] len = %f ", __func__, sizeof(iir_run_cfg->coef[0].history)/sizeof(float));

    TRACE("[%s] float = %f", __func__, (double)iir_run_cfg->coef[0].history[0][0]);
    TRACE("[%s] float = %f", __func__, (double)iir_run_cfg->coef[0].history[1][3]);
    return 0;
}



/*
EqState *eq_init(int32_t sample_rate, int32_t frame_size, const EqConfig *cfg)
{
   // speech_iir_open();
}
*/
#if 0
void filter_iir_test (void)
{
    float fo[5] = {50.0, 250.0, 1000.0, 4000.0, 8000.0};
    float Q[5] = {0.7, 0.7, 0.7, 0.7, 0.7};
    float gain[5] = {1.0, 1.0, 1.0, 1.0, 1.0};

    filter_iir_set_cfg(5, fo, Q, gain, 96000);

    filter_iir_update_coefs();

    SAFE_PROGRAM_STOP();

/*
    -1.991169, 0.991212, 1.000536, -1.991169, 0.990676
    -1.955778, 0.956826, 1.002634, -1.955778, 0.954192
    -1.822479, 0.838206, 1.009871, -1.822479, 0.828335
    -1.295316, 0.495702, 1.030767, -1.295316, 0.464935
    -0.631319, 0.262637, 1.044986, -0.631319, 0.217651
*/
}
#endif


#endif
