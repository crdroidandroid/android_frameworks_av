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

#include <ctime>
#include <cstdlib>

#include "include/TableXInit.h"

namespace android {

TableXInit::TableXInit(uint8_t table_id, uint8_t mask_val)
     :_table_id(table_id),
      _mask_val(mask_val),
      _session_seed(rand()),
      _buffer_size(0),
      _isTableXInitialized(0)
{
    for (unsigned int i = 0; i < MAX_BUFFER_SIZE; i++) {
        _buffer[i] = 0;
    }
}

void
TableXInit::init()
{
    union SCRAMBLED_TABLE_PT
    {
        unsigned char  *ui8;
        unsigned int   *ui32;
    } scramble_table_pt;

    scramble_table_pt.ui32 = scrambled_table;

    if (   _table_id == AC4_TABLE_SEC_FRS_CODE
        && _isTableXInitialized == 0 )
    {

        _buffer_size = LUT_BUFFER_SIZE;

        for(unsigned int i = 0; i < _buffer_size; i++)
        {
            _buffer[i] = _table_a_u8[i] ^ (  scramble_table_pt.ui8[ ( i + _session_seed ) % LUT_BUFFER_SIZE]
                                            ^ scramble_table_pt.ui8[ ( i + _session_seed + _mask_val) % LUT_BUFFER_SIZE] );
        }
        _isTableXInitialized = 1;
    }
    else
    if (   _table_id == AC4_TABLE_SEC_MDD_MAX_FRAM
        && _isTableXInitialized == 0 )
    {
        _buffer_size = TABLE_B_C_U8_SZ;

        for(unsigned int i = 0; i < _buffer_size; i++)
        {
            _buffer[i] = _table_b_u8[i] ^ (  scramble_table_pt.ui8[ ( i + _session_seed ) % LUT_BUFFER_SIZE]
                                           ^ scramble_table_pt.ui8[ ( i + _session_seed + _mask_val ) % LUT_BUFFER_SIZE] );
        }
        _isTableXInitialized = 1;
    }
    else
    if (   _table_id == AC4_TABLE_SEC_MDD_MAX_INST
        && _isTableXInitialized == 0 )
    {

        _buffer_size = TABLE_B_C_U8_SZ;

        for(unsigned int i = 0; i < _buffer_size; i++)
        {
            _buffer[i] = _table_c_u8[i] ^ (  scramble_table_pt.ui8[ ( i + _session_seed ) % LUT_BUFFER_SIZE]
                                           ^ scramble_table_pt.ui8[ ( i + _session_seed + _mask_val ) % LUT_BUFFER_SIZE] );
        }
        _isTableXInitialized = 1;
    }
}

TableXInit::~TableXInit()
{
    _session_seed = 0;
    _table_id     = 0;
    _mask_val     = 0;
    _buffer_size  = 0;

    _isTableXInitialized = 0;

    for (int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        _buffer[i] = 0;
    }
}

}; // namespace

