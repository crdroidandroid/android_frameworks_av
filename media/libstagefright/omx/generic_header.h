/******************************************************************************
 * This program is protected under international and U.S. copyright laws as
 * an unpublished work. This program is confidential and proprietary to the
 * copyright owners. Reproduction or disclosure, in whole or in part, or the
 * production of derivative works therefrom without the express permission of
 * the copyright owners is prohibited.
 *
 *                Copyright (C) 2017 by Dolby International AB.
 *                            All rights reserved.
 ******************************************************************************/

/**
 * @addtogroup ac4dec_lib_cch Configuration Change Handler (CCH)
 *
 * @brief This module implements Part 2 of the AC-4 Split Security Architecture
 *        This module implements the three functions that were identified as
 *        part of the original TEE Architecture. All function names have been
 *        renamed as generic_function_X
 */

/**
 *  @file
 *
 *  @ingroup generic_shared_library
 *
 *  @brief AC-4 Split Security Architecture Shared Library Implementation.
 *
 */

#ifndef GENERIC_HEADER_H
#define GENERIC_HEADER_H

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>

#define LOG_DUALIS_TABLE_SIZE   65   /**< Number of entries for ld(x) lookup table used by DLB_logDualisDiv8(x) */

static float get_num_octaves_div8(unsigned int a, unsigned int b);
unsigned int function_a (unsigned int sbx, unsigned int sbz);

#define MAX_NUM_ASPX_INST_CHANNELS            2 /**< Maximum number of A-SPX channels per instance. */
#define MAX_NUM_ATSG_SIG                      5 /**< Maximum number of signal time slot groups (signal envelopes) per A-SPX frame */
#define MAX_NUM_ATSG_NOISE                    2 /**< Maximum number of noise time slot groups (noise envelopes) per A-SPX frame */
#define MAX_NUM_SBG_SIG_HIRES                22 /**< Maximum number of signal QMF subband groups (scale factor bands) for high resolution */
#define MAX_NUM_SBG_SIG_LORES                11 /**< Maximum number of signal QMF subband groups (scale factor bands) for low resolution */
#define MAX_NUM_SBG_NOISE                     5 /**< Maximum number of noise QMF subband groups (scale factor bands) */
#define MAX_NUM_SB_ASPX                      44 /**< Maximum number of QMF subbands in the A-SPX range */
/** @brief Macro to retrieve the add harmonic flag for a specific A-SPX subband group from the respective bit field. */
#define ADD_HARMONIC(bitfield, sbg) ((bitfield) & (0x1UL << ((sizeof(unsigned long)*CHAR_BIT-1)-(sbg))))

/**
 * @brief  Calculate sine start envelopes.
 *
 * The values in p_sine_start_env indicate the envelope numbers where sines have to start (per subband group).
 * A value of MAX_NUM_ATSG_SIG indicates that no sine is present.
 *
 */
void
function_b
    (unsigned int                       num_sbg_sig_highres     /**< [in]     Number high resolution subband groups. */
    ,unsigned int                       aspx_sbg_start          /**< [in]     A-SPX start subband group. */
    ,signed int                         aspx_tsg_ptr            /**< [in]     Pointer to envelope border signalling sine start
                                                                              envelope */
    ,unsigned long                      aspx_add_harmonic       /**< [in]     Sinusoid insertion flags (one bit per sbg). */
    ,unsigned int                      *p_sine_start_env        /**< [in,out] An array indicating the sine start envelope per
                                                                              subband group. */
    );


#define GET_CONFIG_EVENT(config_change)               \
    (cch_config_change_t)((config_change) & 0x0F)

#define GET_FRAME_EVENT(config_change)                \
    (cch_config_change_t)((config_change) & 0xF0)
#define SET_CONFIG_EVENT(config_change, config_event) \
    (config_change = (cch_config_change_t)(((config_change) & 0xF0) | config_event))

#define SET_FRAME_EVENT(config_change, frame_event)   \
    (config_change = (cch_config_change_t)(((config_change) & 0x0F) | frame_event))

/**
 * @brief Configuration change information.
 *
 * Please note, that the order of the messages does matter.
 */
typedef enum
{
    UNDEFINED          = -1

    /* lower nibble reserved for configuration event messages */
   ,NO_CHANGE          = 0x00 /* Nothing changed, normal processing. */
   ,SEAMLESS           = 0x01 /* Perfect transition, only used in combination with frame rate switches between multiples. */
   ,GAPLESS            = 0x02 /* No silence gap in between. */
   ,CLEAN              = 0x03 /* Something better than GAPLESS but not as good as SEAMLESS, still needs to be defined. */
   ,SPLICE             = 0x04 /* A SPLICE always introduces silence gaps. */

    /* upper nibble reserved for frame event messages */
   ,FRAME_DROP         = 0x10
   ,FRAME_REPETITION   = 0x20
} cch_config_change_t;

/**
 * @brief Determines the slice index within an EHFR frame sequence.
 *
 * @return The slice index within an EHFR frame sequence.
 */
static inline unsigned int
cch_get_slice_index
    (unsigned int              sequence_counter               /**< [in] Sequence counter. */
    ,unsigned int              frame_rate_fraction            /**< [in] Frame rate fraction. */
    )
{
    return sequence_counter & (frame_rate_fraction - 1);
}

typedef struct cch_frame_data_s
{
    cch_config_change_t        config_change;                 /**< Configuration change.
                                                                   Can be of types: NO_CHANGE, SEAMLESS, GAPLESS,
                                                                   CLEAN, SPLICE, FRAME_DROP, FRAME_REPETITION */
    int                        b_frame_complete;              /**< Flag indicating if the frame is complete and can be decoded. */
    int                        b_collection_frame;            /**< Flag indicating a collection frame, i.e. the frame is not
                                                                   processed but collected. */
} cch_frame_data_t;

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
    );

#else

//#error "libstagefright: AC4 Split Security Architecture unsupported"

#endif // GENERIC_HEADER_H
