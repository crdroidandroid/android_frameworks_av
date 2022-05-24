/******************************************************************************
 * This program is protected under international and U.S. copyright laws as
 * an unpublished work. This program is confidential and proprietary to the
 * copyright owners. Reproduction or disclosure, in whole or in part, or the
 * production of derivative works therefrom without the express permission of
 * the copyright owners is prohibited.
 *
 *              Copyright (C) 2017 by Dolby International AB.
 *                            All rights reserved.
 ******************************************************************************/

/**
 *  @file
 *
 *  @ingroup security_split
 *
 *  @brief A-SPX frequency scale calculation
 */

#include <math.h>

#include "generic_header.h"

#define LOG_TAG "generic_source"

#define DLB_ScF(a) ((float)(a))
static const float Q15 = 32768.0f;

union float_long
  {
    float f;
    long l;
  };

static long lmax(long a, long b) { return a < b ? b : a; }
static long lmin(long a, long b) { return a < b ? a : b; }
static long lclip(long a, long lo, long hi) { return lmin( lmax(a, lo), hi); }

static int16_t
DLB_16srndS(float a) {
    return (int16_t)lclip(lrintf(Q15 * a), INT16_MIN, INT16_MAX); }

static float
DLB_SsubSS(float a, float b) {
    return a - b; }

#define shru(a, shift) ldexpf((a), -(int)(shift))
static  float
DLB_SshrSU(float a, unsigned sh) {
    return shru(a, sh); }

static const float a_log_dualis[LOG_DUALIS_TABLE_SIZE] = {
    DLB_ScF(-1.00000000000),  /* actually, ld 0 is not defined */
    DLB_ScF(0.000000000000),  /*  ld(1) / 8  */
    DLB_ScF(0.125000000000),  /*  ld(2) / 8  */
    DLB_ScF(0.198120312590),  /*  ld(3) / 8  */
    DLB_ScF(0.250000000000),  /*  ld(4) / 8  */
    DLB_ScF(0.290241011861),  /*  ld(5) / 8  */
    DLB_ScF(0.323120312590),  /*  ld(6) / 8  */
    DLB_ScF(0.350919365257),  /*  ld(7) / 8  */
    DLB_ScF(0.375000000000),  /*  ld(8) / 8  */
    DLB_ScF(0.396240625180),  /*  ld(9) / 8  */
    DLB_ScF(0.415241011861),  /*  ld(10) / 8  */
    DLB_ScF(0.432428952330),  /*  ld(11) / 8  */
    DLB_ScF(0.448120312590),  /*  ld(12) / 8  */
    DLB_ScF(0.462554964768),  /*  ld(13) / 8  */
    DLB_ScF(0.475919365257),  /*  ld(14) / 8  */
    DLB_ScF(0.488361324451),  /*  ld(15) / 8  */
    DLB_ScF(0.500000000000),  /*  ld(16) / 8  */
    DLB_ScF(0.510932855156),  /*  ld(17) / 8  */
    DLB_ScF(0.521240625180),  /*  ld(18) / 8  */
    DLB_ScF(0.530990939180),  /*  ld(19) / 8  */
    DLB_ScF(0.540241011861),  /*  ld(20) / 8  */
    DLB_ScF(0.549039677847),  /*  ld(21) / 8  */
    DLB_ScF(0.557428952330),  /*  ld(22) / 8  */
    DLB_ScF(0.565445244507),  /*  ld(23) / 8  */
    DLB_ScF(0.573120312590),  /*  ld(24) / 8  */
    DLB_ScF(0.580482023722),  /*  ld(25) / 8  */
    DLB_ScF(0.587554964768),  /*  ld(26) / 8  */
    DLB_ScF(0.594360937770),  /*  ld(27) / 8  */
    DLB_ScF(0.600919365257),  /*  ld(28) / 8  */
    DLB_ScF(0.607247624391),  /*  ld(29) / 8  */
    DLB_ScF(0.613361324451),  /*  ld(30) / 8  */
    DLB_ScF(0.619274538798),  /*  ld(31) / 8  */
    DLB_ScF(0.625000000000),  /*  ld(32) / 8  */
    DLB_ScF(0.630549264920),  /*  ld(33) / 8  */
    DLB_ScF(0.635932855156),  /*  ld(34) / 8  */
    DLB_ScF(0.641160377118),  /*  ld(35) / 8  */
    DLB_ScF(0.646240625180),  /*  ld(36) / 8  */
    DLB_ScF(0.651181670704),  /*  ld(37) / 8  */
    DLB_ScF(0.655990939180),  /*  ld(38) / 8  */
    DLB_ScF(0.660675277358),  /*  ld(39) / 8  */
    DLB_ScF(0.665241011861),  /*  ld(40) / 8  */
    DLB_ScF(0.669694000577),  /*  ld(41) / 8  */
    DLB_ScF(0.674039677847),  /*  ld(42) / 8  */
    DLB_ScF(0.678283094338),  /*  ld(43) / 8  */
    DLB_ScF(0.682428952330),  /*  ld(44) / 8  */
    DLB_ScF(0.686481637041),  /*  ld(45) / 8  */
    DLB_ScF(0.690445244507),  /*  ld(46) / 8  */
    DLB_ScF(0.694323606460),  /*  ld(47) / 8  */
    DLB_ScF(0.698120312590),  /*  ld(48) / 8  */
    DLB_ScF(0.701838730514),  /*  ld(49) / 8  */
    DLB_ScF(0.705482023722),  /*  ld(50) / 8  */
    DLB_ScF(0.709053167746),  /*  ld(51) / 8  */
    DLB_ScF(0.712554964768),  /*  ld(52) / 8  */
    DLB_ScF(0.715990056820),  /*  ld(53) / 8  */
    DLB_ScF(0.719360937770),  /*  ld(54) / 8  */
    DLB_ScF(0.722669964191),  /*  ld(55) / 8  */
    DLB_ScF(0.725919365257),  /*  ld(56) / 8  */
    DLB_ScF(0.729111251771),  /*  ld(57) / 8  */
    DLB_ScF(0.732247624391),  /*  ld(58) / 8  */
    DLB_ScF(0.735330381170),  /*  ld(59) / 8  */
    DLB_ScF(0.738361324451),  /*  ld(60) / 8  */
    DLB_ScF(0.741342167195),  /*  ld(61) / 8  */
    DLB_ScF(0.744274538798),  /*  ld(62) / 8  */
    DLB_ScF(0.747159990437),  /*  ld(63) / 8  */
    DLB_ScF(0.750000000000)   /*  ld(64) / 8  */
};


/**
   @brief   Scaled Logarithm dualis by table lookup

   The valid range for a is 1 to LOG_DUALIS_TABLE_SIZE.
   For a=0, the result will be -1 (should be -inf).

   @return   ld(a) / 8
*/
static inline float
log_dualis_div8
    (unsigned int a  /**< Index for logarithm table */
    )
{
    //DLB_assert(a<LOG_DUALIS_TABLE_SIZE, "ASPX", "Index for a_log_dualis[] out of range.");
    return a_log_dualis[a];
}


/**
   @brief   Get number of octaves between frequencies a and b

   The Result is scaled with 1/8.
   The valid range for a and b is 1 to LOG_DUALIS_TABLE_SIZE.

   @return   ld(a/b) / 8
*/
static float get_num_octaves_div8(unsigned int a  /**< lower band */
                                      ,unsigned int b  /**< upper band */
                                       )
{
    return DLB_SsubSS(log_dualis_div8(b), log_dualis_div8(a));
}

unsigned int function_a (unsigned int sbx, unsigned int sbz)
{
    return (unsigned)DLB_16srndS(DLB_SshrSU((get_num_octaves_div8(sbx, sbz)), 2));

}

void
function_b
    (unsigned int                       num_sbg_sig_highres
    ,unsigned int                       aspx_sbg_start
    ,signed int                         aspx_tsg_ptr
    ,unsigned long                      aspx_add_harmonic
    ,unsigned int                      *p_sine_start_env
    )
{
    unsigned int sbg;
    unsigned int aspx_sbg_stop = aspx_sbg_start + num_sbg_sig_highres;

    //DLB_assert(aspx_sbg_stop <= MAX_NUM_SBG_SIG_HIRES, "ASPX", "Calculated A-SPX stop subband group (aspx_sbg_stop) exceeds maximum number of A-SPX subband groups.");

    /* Reset all envelope values below aspx_sbg_start. */
    for (sbg = 0; sbg < aspx_sbg_start; sbg++)
    {
        p_sine_start_env[sbg] = MAX_NUM_ATSG_SIG;
    }

    /* Loop over subband groups within active A-SPX range */
    for (; sbg < aspx_sbg_stop; sbg++)
    {
        /* sine to be added in current subband group */
        if (ADD_HARMONIC(aspx_add_harmonic, sbg - aspx_sbg_start))
        {
            /* Sine starts at the first envelope when
               - there is no transient envelope
               - or a sine was present in previous frame's subband group */
            if (aspx_tsg_ptr == -1 || p_sine_start_env[sbg] != MAX_NUM_ATSG_SIG)
            {
                p_sine_start_env[sbg] = 0;
            }
            /* sine starts at envelope indicated by pointer */
            else
            {
                p_sine_start_env[sbg] = aspx_tsg_ptr;
            }
        }
        /* no sine present */
        else
        {
            p_sine_start_env[sbg] = MAX_NUM_ATSG_SIG;
        }
    }

    /* Reset all envelope values above num_sbg_sig_hires up to the max. */
    for (; sbg < MAX_NUM_SBG_SIG_HIRES; sbg++)
    {
        p_sine_start_env[sbg] = MAX_NUM_ATSG_SIG;
    }
}


/*
 * Determines if the frame is complete and can be decoded.
 * This information is essential for EHFR modes, where one frame consists of multiple slices. For the non-EHFR mode the return
 * value (b_frame_complete) is always 1.
 */
static inline int
function_c_aux
    (unsigned int              sequence_counter               /* [in] Sequence counter. */
    ,unsigned int              frame_rate_fraction            /* [in] Frame rate fraction. */
    ,cch_config_change_t       config_change                  /* [in] Configuration change. */
    ,unsigned int             *p_num_slices_available         /* [in,out] Slice counter holding the number of available slices.
                                                                          This variable is updated within this function. */
    ,int                      *pb_dropped_first_slice         /* [out] Flag indicating that the first slice of an EHFR frame
                                                                       has been dropped. */
    )
{
    const unsigned int max_slice_index = frame_rate_fraction - 1;

    *pb_dropped_first_slice = 0;

    /* reset number of slices */
    if (    ((sequence_counter & max_slice_index) == 0)
         || (SPLICE == GET_CONFIG_EVENT(config_change))
       )
    {
        *p_num_slices_available = 0;
    }
    else if (FRAME_DROP == GET_FRAME_EVENT(config_change))
    {
        /* check if the start frame was dropped */
        if (((sequence_counter - 1) & max_slice_index) == 0)
        {
            *p_num_slices_available = 0;

            if (frame_rate_fraction > 1)
            {
                /* For frame rate fraction values greater than one, the decoder shall process an output frame.
                   Therefore, we count the dropped frames as well. The decoder is supposed to conceal for an incomplete frame,
                   which results in silence at the output. */
                *p_num_slices_available = 1;
                *pb_dropped_first_slice = 1;
            }
        }
        else
        {
            ++(*p_num_slices_available);
        }
    }

    return ++(*p_num_slices_available) == frame_rate_fraction;
}

/*
 * Determines and assigns data to the frame data structure which exists for every single frame and is stored in the internal FIFO
 * buffer. Moreover, the config_change message is complemented with a frame event (FRAME_DROP or FRAME_REPETITION).
 * Note that short frame equivalents - as used for frame length information - means, that e.g. a long frame has the length of 4.
 */
int
function_c
    (int                       b_collection_frame_previous    /* [in] Flag indicating if the previous frame was a collection
                                                                      frame. */
    ,unsigned int              sequence_counter_current       /* [in] Sequence counter of current frame. */
    ,unsigned int              frame_rate_fraction_previous   /* [in] Frame rate fraction of previous frame. */
    ,unsigned int              frame_rate_fraction_current    /* [in] Frame rate fraction of current frame. */
    ,unsigned int              length_frame_delayed           /* [in] Length of the delayed frame in units of short frame
                                                                      equivalents. */
    ,unsigned int              length_frame_current           /* [in] Length of the current frame in units of short frame
                                                                      equivalents. */
    ,cch_config_change_t      *p_config_change                /* [in,out] Configuration change. */
    ,unsigned int             *p_length_frames_collected      /* [in,out] Stores the overall length of collected frames in units of
                                                                          short frame equivalents.
                                                                          This variable is updated within this function. */
    ,unsigned int             *p_num_frames_collected         /* [in,out] Stores the number of collected frames during a
                                                                          collection phase.
                                                                          This variable is updated within this function. */
    ,unsigned int             *p_num_slices_available         /* [in,out] Slice counter holding the number of available slices.
                                                                          This variable is updated within this function. */
    ,cch_config_change_t      *p_config_change_previous       /* [out] Config change of previous frame. */
    ,cch_frame_data_t         *p_frame_data_current           /* [out] Frame data structure to be filled. */
    )
{
    const unsigned int max_slice_index_previous  = frame_rate_fraction_previous - 1;
    const unsigned int max_slice_index_current   = frame_rate_fraction_current - 1;
    const unsigned int sequence_counter_previous = sequence_counter_current - 1;
    const unsigned int slice_index_previous      = cch_get_slice_index(sequence_counter_previous, frame_rate_fraction_previous);
    const unsigned int slice_index_current       = cch_get_slice_index(sequence_counter_current, frame_rate_fraction_current);
    int                b_dropped_last_slice      = 0;
    int                b_dropped_first_slice;

    /* reset frame length collector variable in case of splice */
    if (    (SPLICE == GET_CONFIG_EVENT(*p_config_change))
         || (    (FRAME_DROP == GET_FRAME_EVENT(*p_config_change))
              && (!b_collection_frame_previous)
              && (0 == (sequence_counter_current & max_slice_index_current))
            ) /* frame drop at last slice of previous frame */
       )
    {
        *p_length_frames_collected = 0;
    }

    /* reset collection frame counter variable */
    if (0 == *p_length_frames_collected)
    {
        *p_num_frames_collected = 0;
    }

    if (FRAME_REPETITION == GET_FRAME_EVENT(*p_config_change))
    {
        p_frame_data_current->b_frame_complete   = frame_rate_fraction_current == 1;
        p_frame_data_current->b_collection_frame = (length_frame_delayed > length_frame_current) ? 1 : b_collection_frame_previous;

        p_frame_data_current->config_change = *p_config_change; /* assign config change message */

        return 0;
    }

    /* determine and assign frame completion flag */
    p_frame_data_current->b_frame_complete = function_c_aux
        (sequence_counter_current
        ,frame_rate_fraction_current
        ,*p_config_change
        ,p_num_slices_available
        ,&b_dropped_first_slice
        );

    /* determine collection frame flag */
    if (1 == (*p_num_slices_available - b_dropped_first_slice))
    {
        /* increase the frame length counter only if there is a decodable frame available */
        *p_length_frames_collected += length_frame_current;
        (*p_num_frames_collected)++;
    }

    /* determine completion flag */
    p_frame_data_current->b_collection_frame = *p_length_frames_collected < length_frame_delayed;

    if (    (!p_frame_data_current->b_collection_frame)
         && (p_frame_data_current->b_frame_complete)
       )
    {
        /* On exit of the collection phase, the overall collection frame length is reset.
           Note, that the number of collected frames (num_frames_collected) is needed later on. Hence this variable is reset when
           entering this function. */
        *p_length_frames_collected = 0;
    }

    if (FRAME_DROP == GET_FRAME_EVENT(*p_config_change))
    {
        /* mark an EHFR frame as dropped */
        if (    (frame_rate_fraction_current > 1)
             && (frame_rate_fraction_previous > 1)
           ) /* previous and current frame are EHFR frames */
        {
            SET_FRAME_EVENT(*p_config_change, FRAME_DROP);

            if (slice_index_previous == max_slice_index_previous)
            {
                b_dropped_last_slice = 1;
            }
        }
        else if (    (1 == frame_rate_fraction_current)
                  && (frame_rate_fraction_previous > 1)
                  && (slice_index_previous < max_slice_index_previous)
                ) /* current frame is self-contained */
        {
            SET_FRAME_EVENT(*p_config_change_previous, FRAME_DROP);
        }
        else if (    (frame_rate_fraction_current > 1)
                  && (1 == frame_rate_fraction_previous)
                  && (slice_index_current > 0)
                ) /* previous frame was self-contained */
        {
            SET_FRAME_EVENT(*p_config_change, FRAME_DROP);
        }
    }

    p_frame_data_current->config_change = *p_config_change; /* assign config change message */
    return b_dropped_last_slice;
}
