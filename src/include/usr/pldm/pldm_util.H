/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/pldm/pldm_util.H $                            */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020                             */
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

/** @file pldm_util.H
 *
 *  @brief PLDM utility functions and macros.
 */

#ifndef PLDM_UTIL_H
#define PLDM_UTIL_H

#ifdef __HOSTBOOT_RUNTIME
// For Hostboot Runtime we just use nullptr as the message queue parameter to
// the various send/receive functions.
#define MSG_Q_RESOLVE(FUNC, QNAME) nullptr
#else
#define MSG_Q_RESOLVE(FUNC, QNAME)                                      \
    ({                                                                  \
    const msg_q_t msgQ = msg_q_resolve(QNAME);                          \
    assert(msgQ != nullptr,                                             \
           FUNC ": PLDM Req Out Message queue did not resolve properly!"); \
    msgQ;                                                               \
    })
#endif

#endif
