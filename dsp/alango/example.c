//-----------------------------------------------------------------------------
// onlyvoice package using example
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "onlyvoice-api.h"

typedef short   pcm_t;

#define FRAME_LENGTH		120
#define INPUTS_NUMBER		2

static short mic[FRAME_LENGTH*INPUTS_NUMBER], spk[FRAME_LENGTH], mout[FRAME_LENGTH], sout[FRAME_LENGTH];

void
error_exit(int err)
{
    exit(-1);
}


//-----------------------------------------------------------------------------

mem_reg_t   mem_regs[NUM_OBJECTS][NUM_MEM_REGIONS];


//-----------------------------------------------------------------------------
// load sample
// mic is mic1[FRAME_LENGTH], mic2[FRAME_LENGTH]
// spk[FRAME_LENGTH]
//-----------------------------------------------------------------------------
void load_samples( pcm_t* mic, pcm_t* spk )
{
    ...
}

//-----------------------------------------------------------------------------
// mout[FRAME_LENGTH]
//-----------------------------------------------------------------------------
void save_samples( pcm_t* mout)
{
    ....
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(void)
{
    err_t err;
    int i, i_prof, smem, size, fr_len;
    mem_reg_t *regs[NUM_OBJECTS];
    profiles_t p;
    void *mem, *prof_bin_ext, *prof_bin_int, *prof_bin_vmx;

    // read profiles
    pf1 = fopen("prof_1.bin","rb");
    fseek(pf1, 0, SEEK_END);
    size = ftell(pf1 )
    prof_bin_ext = malloc( size );
    fseek(pf1, 0, SEEK_SET);
    fread(prof_bin_ext, 1, size, pf1);

    pf2 = fopen("prof_2.bin","rb");
    fseek(pf2, 0, SEEK_END);
    size = ftell(pf2);
    prof_bin_int = malloc( size );
    fseek(pf2, 0, SEEK_SET);
    fread(prof_bin_int, 1, size, pf2);

    pf3 = fopen("prof_3.bin","rb");
    fseek(pf3, 0, SEEK_END);
    size = ftell(pf3);
    prof_bin_mix = malloc( size );
    fseek(pf3, 0, SEEK_SET);
    fread(prof_bin_mix, 1, size, pf3);


    p.p_ext = prof_bin_ext;
    p.p_int = prof_bin_int;
    p.p_vmx = profi_bin_vmx;    

    // ------


    for (i_prof = 0; i_prof < NUM_OBJECTS; i_prof++)
    {
        regs[i_prof] = mem_regs[i_prof];
    }

    smem = onlyvoice_get_hook_size();
    if (smem< sizeof(smem_0))
    {
        message("it needs %d bytes to start initialization\n", smem);
    }
    else
    {
        message("no memory %d (%d only) to start initialization\n", i, sizeof(smem_0));
        error_exit(-3);
    }

    mem = malloc(smem);
    if (mem == 0)
        error_exit(-1);

    onlyvoice_get_mem_size( &p, regs, mem );

    for (i_prof = 0; i_prof < NUM_OBJECTS; i_prof++)
    {
        for (i = 0; i < NUM_MEM_REGIONS; i++)
        {
            mem_reg_t* reg = regs[i_prof];
            reg[i].mem = (void*)malloc(reg[i].size);
            message("I need %d bytes in memory region %d (%d)\n", reg[i].size, i + 1, reg[i].size);
        }
    }

    err.err = onlyvoice_init(mem, regs);

    if (err.err != ERR_NO_ERROR)
    {
        message("initialization error - pid %d!\n", err.pid);
        error_exit(-3);
    }

    while (1)
    {
        load_samples( mic, spk );

        onlyvoice_process_rx( regs, spk, sout );
        onlyvoice_process_tx( regs, mic, sout, mout );

        save_samples(mout);
    }

    free( mem );
    free( prof_bin_ext );
    free( prof_bin_int );
    free( profi_bin_vmx );

    return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
