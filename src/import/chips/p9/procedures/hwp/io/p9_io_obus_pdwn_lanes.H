/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/io/p9_io_obus_pdwn_lanes.H $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2018                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file p9_io_obus_pdwn_lanes.H
/// @brief Run Dccal on the link.
///-----------------------------------------------------------------------------
/// *HWP HWP Owner        : Chris Steffen <cwsteffen@us.ibm.com>
/// *HWP HWP Backup Owner : Gary Peterson <garyp@us.ibm.com>
/// *HWP FW Owner         : Jamie Knight <rjknight@us.ibm.com>
/// *HWP Team             : IO
/// *HWP Level            : 3
/// *HWP Consumed by      : FSP:HB
///-----------------------------------------------------------------------------

#ifndef _P9_IO_OBUS_PDWN_LANES_H
#define _P9_IO_OBUS_PDWN_LANES_H

//-----------------------------------------------------------------------------
//  fapi2 Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>

typedef fapi2::Target<fapi2::TARGET_TYPE_OBUS> OBUS_TGT;


typedef fapi2::ReturnCode (*p9_io_obus_pdwn_lanes_FP_t)( const OBUS_TGT, const uint32_t );

extern "C"
{

    /**
     * @brief A I/O Obus Procedure that Powers Down Lanes
     * on every group of the Obus.
     * @param[in] i_tgt          FAPI2 Target
     * @param[in] i_lane_vector  Lane Vector; all 24 lanes(0x00FFFFFF)
     * @retval ReturnCode
     */
    fapi2::ReturnCode p9_io_obus_pdwn_lanes(
        const OBUS_TGT      i_tgt         ,
        const uint32_t      i_lane_vector );

} // extern "C"

#endif // _P9_IO_OBUS_PDWN_LANES_H
