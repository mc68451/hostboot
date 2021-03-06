/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/fapi2/plat_vpd_access.C $                             */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2020                        */
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
/**
 *  @file plat_vpd_access.C
 *
 *  @brief Implements the GetVpd function
 */

#include <stdint.h>
#include <fapi2.H>
#include <vpd_access_defs.H>
#include <vpd_access.H>
#include <p9_get_mem_vpd_keyword.H>
#include <ddimm_get_efd.H>
#include <attribute_service.H>
#include <vpd/dvpdenums.H>
#include <errl/errlmanager.H>
#include <fapi2_spd_access.H>
//The following can be uncommented for unit testing
//#undef FAPI_DBG
//#define FAPI_DBG(args...) FAPI_INF(args)

namespace fapi2
{

fapi2::ReturnCode platGetVPD(
                        const fapi2::Target<fapi2::TARGET_TYPE_MCS>& i_target,
                        VPDInfo<fapi2::TARGET_TYPE_MCS>& io_vpd_info,
                        uint8_t* o_blob)
{
    fapi2::ReturnCode l_rc = fapi2::FAPI2_RC_SUCCESS;
    errlHndl_t l_errl = nullptr;
    keywordInfo_t l_keywordInfo;

    // Assume that all memory keywords (MR,MT,J0..JZ,X0...XZ) are all the
    // same size of 255. This avoids going through the decode and asking
    // the vpd DD the size of the keyword.
    const size_t VPD_KEYWORD_SIZE = 255;

    FAPI_DBG("platGetVPD: enter");

    do
    {
        // null blob pointer requests blob size
        if ( nullptr == o_blob) // just return size
        {
            // use a generic max blob size (DQ and CK need less)
            io_vpd_info.iv_size = VPD_KEYWORD_SIZE;
            FAPI_DBG("platGetVPD: return blob size of %d",
                 io_vpd_info.iv_size);
            break; //return success
        }

        //Make sure passed blob buffer is big enough
        if (VPD_KEYWORD_SIZE > io_vpd_info.iv_size)
        {
            FAPI_ERR("platGetVPD: blob size of %d too small, should be %d",
                     io_vpd_info.iv_size,
                     VPD_KEYWORD_SIZE);
            /*@
            * @errortype
            * @moduleid          fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS
            * @reasoncode        fapi2::RC_BUFFER_TOO_SMALL
            * @userdata1         Buffer size
            * @userdata2         Expected size
            * @devdesc           Passed buffer too small
            * @custdesc          Firmware Error
            */
            l_errl = new ERRORLOG::ErrlEntry(
                             ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                             fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS,
                             fapi2::RC_BUFFER_TOO_SMALL,
                             io_vpd_info.iv_size,
                             VPD_KEYWORD_SIZE,
                             true); //software callout
            break; //return with error
        }

        // Get targeting MCS target
        TARGETING::Target * l_pMcsTarget = nullptr;
        l_errl = fapi2::platAttrSvc::getTargetingTarget(i_target, l_pMcsTarget);
        if (l_errl)
        {
            FAPI_ERR("platGetVPD: Error from getTargetingTarget");
            break; //return with error
        }

        //Validate type and check for override
        uint64_t  l_mapKeyword  = 0;
        uint64_t  l_keywordEnum = 0; //set this now to be used later
        if ( fapi2::MT == io_vpd_info.iv_vpd_type )
        {
            if (1 ==
               l_pMcsTarget->getAttr<TARGETING::ATTR_VPD_OVERRIDE_MT_ENABLE>() )
            {
                uint8_t l_override[VPD_KEYWORD_SIZE]={0};
                assert(l_pMcsTarget->tryGetAttr<TARGETING::ATTR_VPD_OVERRIDE_MT>
                                         (l_override),
                       "platGetVPD: getAttr ATTR_VPD_OVERRIDE_MT failed");
                FAPI_DBG("platGetVPD: return MT override attr");
                memcpy(o_blob,l_override,VPD_KEYWORD_SIZE);
                break; //return with overriden keyword
            }

            // not overriden, continue
            l_mapKeyword = DVPD::MT;
            l_keywordEnum = DVPD::X0;
        }
        else if ( fapi2::MR == io_vpd_info.iv_vpd_type )
        {
            if (1==
               l_pMcsTarget->getAttr<TARGETING::ATTR_VPD_OVERRIDE_MR_ENABLE>() )
            {
                uint8_t l_override[VPD_KEYWORD_SIZE]={0};
                assert(l_pMcsTarget->tryGetAttr<TARGETING::ATTR_VPD_OVERRIDE_MR>
                                         (l_override),
                       "platGetVPD: getAttr ATTR_VPD_OVERRIDE_MR failed");
                FAPI_DBG("platGetVPD: return MR override attr");
                memcpy(o_blob,l_override,VPD_KEYWORD_SIZE);
                break; //return with overriden keyword
            }

            // not overriden, continue
            l_mapKeyword = DVPD::MR;
            l_keywordEnum = DVPD::J0;
        }
        else if ( fapi2::DQ == io_vpd_info.iv_vpd_type )
        {
            if (1==
               l_pMcsTarget->getAttr<TARGETING::ATTR_VPD_OVERRIDE_DQ_ENABLE>() )
            {
                ATTR_VPD_OVERRIDE_DQ_Type l_override={0};
                assert(l_pMcsTarget->tryGetAttr<TARGETING::ATTR_VPD_OVERRIDE_DQ>
                                         (l_override),
                       "platGetVPD: getAttr ATTR_VPD_OVERRIDE_DQ failed");
                FAPI_DBG("platGetVPD: return DQ override attr");
                memcpy(o_blob,l_override,sizeof(ATTR_VPD_OVERRIDE_DQ_Type));
                break; //return with overriden keyword
            }

            // not overriden, continue
            l_mapKeyword = DVPD::Q0;
            l_keywordEnum = DVPD::Q0;
        }
        else if ( fapi2::CK == io_vpd_info.iv_vpd_type )
        {
            if (1==
               l_pMcsTarget->getAttr<TARGETING::ATTR_VPD_OVERRIDE_CK_ENABLE>() )
            {
                ATTR_VPD_OVERRIDE_CK_Type l_override={0};
                assert(l_pMcsTarget->tryGetAttr<TARGETING::ATTR_VPD_OVERRIDE_CK>
                                         (l_override),
                       "platGetVPD: getAttr ATTR_VPD_OVERRIDE_CK failed");
                FAPI_DBG("platGetVPD: return CK override attr");
                memcpy(o_blob,l_override,sizeof(ATTR_VPD_OVERRIDE_CK_Type));
                break; //return with overriden keyword
            }
            l_mapKeyword = DVPD::CK;
            l_keywordEnum = DVPD::CK;
        }
        else
        {
            FAPI_ERR("platGetVPD: invalid type = %d",
                     io_vpd_info.iv_vpd_type);
            /*@
            * @errortype
            * @moduleid          fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS
            * @reasoncode        fapi2::RC_INVALID_TYPE
            * @userdata1         Vpd type
            * @userdata2         HUID of MCS target
            * @devdesc           MR and MT types supported
            * @custdesc          Firmware Error
            */
            l_errl = new ERRORLOG::ErrlEntry(
                             ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                             fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS,
                             fapi2::RC_INVALID_TYPE,
                             io_vpd_info.iv_vpd_type,
                             TARGETING::get_huid(l_pMcsTarget),
                             true); //software callout
            break; //return with error
        }

        //Read mapping keyword
        size_t l_buffSize = VPD_KEYWORD_SIZE;
        uint8_t * l_pMapping = new uint8_t[VPD_KEYWORD_SIZE]();

        l_errl = deviceRead((TARGETING::Target *)l_pMcsTarget,
                            l_pMapping,
                            l_buffSize,
                            DEVICE_DVPD_ADDRESS(DVPD::MEMD,
                                                l_mapKeyword));
        if( l_errl )
        {
            FAPI_ERR("platGetVPD: ERROR returned from deviceRead(MEMD,%d)",l_mapKeyword);
            break;
        }

        // Find vpd keyword name based on VPDInfo
        FAPI_EXEC_HWP(l_rc,
                  p9_get_mem_vpd_keyword,
                  i_target,
                  io_vpd_info,
                  l_pMapping,
                  VPD_KEYWORD_SIZE,
                  l_keywordInfo);

        if (l_rc)
        {
            delete l_pMapping;
            FAPI_ERR("platGetVPD: ERROR returned from p9_get_mem_vpd_keyword");
            break; //return with error
        }
        FAPI_DBG("platGetVPD: keyword name =  %s",
                 l_keywordInfo.kwName);

        // Skip grabbing CK keyword data again and just use the index provided
        // by the hwp to get the specific data for this particular mcs target
        if (l_mapKeyword == DVPD::CK )
        {
            l_buffSize = l_keywordInfo.kwBlobSize;

            // Just a safety check so you don't copy out of bounds
            if (l_buffSize <= VPD_KEYWORD_SIZE)
            {
                // o_blob was already checked for nullptr
                // copy blob of l_buffSize past the 4 byte header
                // (each indexed section is l_buffSize bytes)
                memcpy(o_blob, l_pMapping + l_keywordInfo.kwBlobIndex, l_buffSize);
            }
            else
            {
                memcpy(o_blob,
                    l_pMapping + l_keywordInfo.kwBlobIndex,
                    VPD_KEYWORD_SIZE);
            }
            delete l_pMapping;
        }
        else
        {
            delete l_pMapping;

            //Convert keyword name to keyword enumeration.
            //ascii 0..9 runs from 0x30 to 0x39.
            //The conversion assumes the input is valid (0..9,A..Z)
            //and that the enumeration is in order and consecutive.
            if ( '0' == (l_keywordInfo.kwName[1] & 0xf0)) //it is a digit (0..9)
            {
                l_keywordEnum += (l_keywordInfo.kwName[1] - '0');
            }
            else //it is a char (A..Z)
            {
                l_keywordEnum += (l_keywordInfo.kwName[1] - 'A') + 10;
            }

            //Read vpd blob
            l_buffSize = l_keywordInfo.kwBlobSize;
            l_errl = deviceRead((TARGETING::Target *)l_pMcsTarget,
                                o_blob,
                                l_buffSize,
                                DEVICE_DVPD_ADDRESS(DVPD::MEMD,
                                                    l_keywordEnum));

            if (l_errl)
            {
                FAPI_ERR("platGetVPD: ERROR reading keyword");
                break; //return with error
            }

            //Confirm all expected data was returned
            if (l_keywordInfo.kwBlobSize > l_buffSize)
            {
                FAPI_ERR("platGetVPD: insufficient vpd returned"
                         " for keyword %d;"
                         " %d returned, %d expected",
                         l_keywordEnum,
                         l_buffSize,
                         io_vpd_info.iv_size);
                /*@
                * @errortype
                * @moduleid          fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS
                * @reasoncode        fapi2::RC_RETURNED_VPD_TOO_SMALL
                * @userdata1[0:31]   Returned vpd in bytes
                * @userdata1[32:64]  Expected number of vpd bytes
                * @userdata2         Keyword
                * @devdesc           Less than expected number of bytes returned.
                * @custdesc          Firmware Error
                */
                l_errl = new ERRORLOG::ErrlEntry(
                                 ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                 fapi2::MOD_FAPI2_PLAT_GET_VPD_MCS,
                                 fapi2::RC_RETURNED_VPD_TOO_SMALL,
                                 TWO_UINT32_TO_UINT64(
                                                      l_buffSize,
                                                      l_keywordInfo.kwBlobSize),
                                 l_keywordEnum);
                l_errl->addHwCallout( l_pMcsTarget,
                                 HWAS::SRCI_PRIORITY_LOW,
                                 HWAS::NO_DECONFIG,
                                 HWAS::GARD_NULL );

                break; //return with error
            }
        }
    }
    while (0);

    if( l_errl )
    {
        // Add the error log pointer as data to the ReturnCode
        addErrlPtrToReturnCode(l_rc, l_errl);
    }
    FAPI_DBG("platGetVPD: exit");

    return l_rc;
}

fapi2::ReturnCode platGetVPD(
   const fapi2::Target<fapi2::TARGET_TYPE_OCMB_CHIP>& i_ocmbFapi2Target,
         VPDInfo<fapi2::TARGET_TYPE_OCMB_CHIP>&       io_vpdInfo,
         uint8_t* const o_blob)
{
    FAPI_DBG("platGetVPD(OCMB): enter");

    fapi2::ReturnCode l_rc{fapi2::FAPI2_RC_SUCCESS};

    errlHndl_t l_errl = nullptr;

    // Set up buffer we will read first 2KB of OCMB's eeprom to
    // 1st KB is SPD info, 2nd KB is EFD info. Both are needed.
    size_t   l_spdBufferSize = SPD::OCMB_SPD_EFD_COMBINED_SIZE;
    uint8_t* l_spdBuffer = nullptr;

    do
    {
        // Get targeting OCMB target
        TARGETING::Target * l_ocmbTarget = nullptr;
        l_errl = fapi2::platAttrSvc::getTargetingTarget(i_ocmbFapi2Target,
                                                        l_ocmbTarget);
        if (l_errl)
        {
            FAPI_ERR("platGetVPD(OCMB): Error from getTargetingTarget");
            break; //return with error
        }

        // Retrieve the EFD data or the EFD data size if o_blob is NULL
        if (fapi2::EFD == io_vpdInfo.iv_vpd_type)
        {
            // Allocate buffer to hold SPD and init to 0
            l_spdBuffer = new uint8_t[l_spdBufferSize];
            memset(l_spdBuffer, 0, l_spdBufferSize);

            // Get the SPD buffer, where the EFD data is to be extracted from
            // "ENTIRE_SPD" for OCMB target is first 2 KB of EEPROM
            l_errl = deviceRead(l_ocmbTarget,
                                l_spdBuffer,
                                l_spdBufferSize,
                                DEVICE_SPD_ADDRESS(SPD::ENTIRE_SPD));

            // If unable to retrieve the SPD buffer then can't
            // extract the EFD data, so return error.
            if (l_errl)
            {
                FAPI_ERR("platGetVPD(OCMB): Error from trying to read ENTIRE SPD from 0x%.08X ",
                         TARGETING::get_huid(l_ocmbTarget));
                break;
            }

            // Retrieve the EFD data from the given SPD buffer.  If o_blob is
            // nullptr then size will be returned in io_vpdInfo.iv_size
            FAPI_EXEC_HWP( l_rc,
                           ddimm_get_efd,
                           i_ocmbFapi2Target,
                           io_vpdInfo,
                           o_blob,
                           l_spdBuffer,
                           l_spdBufferSize );
            if (l_rc)
            {
                FAPI_ERR("platGetVPD(OCMB): Error returned from ddimm_get_efd called on target 0x%.08X",
                         TARGETING::get_huid(l_ocmbTarget));
            }
        }  // end if (fapi2::EFD == io_vpdInfo.iv_vpd_type)
        else
        {
            /*@
            * @errortype
            * @moduleid          fapi2::MOD_FAPI2_PLAT_GET_VPD_OCMB
            * @reasoncode        fapi2::RC_INVALID_TYPE
            * @userdata1         vpd_type attempted
            * @userdata2         HUID of OCMB target
            * @devdesc           Less than expected number of bytes returned.
            * @custdesc          Firmware Error
            */
            l_errl = new ERRORLOG::ErrlEntry(
                                 ERRORLOG::ERRL_SEV_UNRECOVERABLE,
                                 fapi2::MOD_FAPI2_PLAT_GET_VPD_OCMB,
                                 fapi2::RC_INVALID_TYPE,
                                 io_vpdInfo.iv_vpd_type,
                                 TARGETING::get_huid(l_ocmbTarget),
                                 ERRORLOG::ErrlEntry::ADD_SW_CALLOUT);
        }
    } while (0);

    // Caller is not interested in the SPD buffer, so delete it.
    if (l_spdBuffer)
    {
        delete []l_spdBuffer;
        l_spdBuffer = nullptr;
    }

    if ( l_errl )
    {
        // Add the error log pointer as data to the ReturnCode
        addErrlPtrToReturnCode(l_rc, l_errl);
    }

    FAPI_DBG("platGetVPD(OCMB): exiting with %s",
             ( (l_rc == fapi2::FAPI2_RC_SUCCESS) ?
               "no errors" : "errors" ));

    return l_rc;
}

} // namespace
