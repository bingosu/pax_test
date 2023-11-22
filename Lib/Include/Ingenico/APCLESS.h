/**
 * \file
 * \brief Main header file for the Easy Path to Contactless Sample application.
 *
 * \author  Ingenico
 *
 * \author  Ingenico has intellectual property rights relating to the technology embodied \n
 *       in this software. In particular, and without limitation, these intellectual property rights may\n
 *       include one or more patents.\n
 *       This software is distributed under licenses restricting its use, copying, distribution, and\n
 *       and decompilation. No part of this software may be reproduced in any form by any means\n
 *       without prior written authorization of Ingenico.
 */

/**
 * \mainpage Overview
 * The Easy Path to Contact'less Sample application is not a full payment application but it demonstrates
 * how to use or call the Easy Path Contact'less kernels. This version of the sample complies with Telium+.
 *
 * The main characteristics of this sample are:
 *   - Two kernels are supported:
 *      - MasterCard PayPass 3:
 *         - With mobile authentication (double tap).
 *         - With Torn recovery.
 *         - With card balance.
 *         - Without Data Exchange & Data storage.
 *      - Visa payWave 2.1.1:
 *         - With Issuer Script processing.
 *         - With mobile authentication (double tap).
 *         - Without DRL.
 *   - Application selection:
 *      - Implicit.
 *      - Explicit.
 *   - Only debit transactions are supported (no refund or credit).
 *   - Simple user interface (the UI only manages the case of an iCT250 with internal contact'less reader).
 *   - Communication with Host not implemented (no authorization, no parameters downloading, no batch processing...).
 *   - No ticket.
 *   - No PIN online.
 *   - No black list.
 *   - No language management.
 *
 * To use this sample:
 *   - In implicit mode, just enter the amount and validate.
 *   - In explicit mode, select the CLess application in F menu, then 'Explicit Trn' item.
 *   - To activate/deactivate the kernel debug traces, in the CLess application menu select the corresponding 'Debug Trace' On or Off item.
 *
 */


#ifndef APCLESS_H_INCLUDED
#define APCLESS_H_INCLUDED


// Generic Tool Library includes
#include "GTL_SharedExchange.h"
#include "GTL_Convert.h"
#include "GTL_Assert.h"
#include "GTL_BerTlv.h"
#include "GTL_BerTlvDecode.h"
#include "GTL_TagsInfo.h"
#include "GTL_StdTimer.h"

#include "TlvTree.h"

// Cless includes
#include "oem_cless.h"
#include "TPass.h"

// EMV Lib includes
#include "EmvLib_Tags.h"

// Entry Point includes
#include "EntryPoint_Tags.h"

// Common cless kernel includes
#include "Common_Kernels_Tags.h"
#include "Common_Kernels_API.h"

// PayPass kernel includes
#include "PayPass_Tags.h"
#include "PayPass3_API.h"

// payWave kernel includes
#include "payWave_API.h"
#include "payWave_Tags.h"

//@@WAY JCB Cless
// JSpeedy kernel includes
#include "JCB_API.h"
#include "JCB_Tags.h"
//

// Sample of Easy Path to Contactless includes
#include "APCLESS_ParamTrn.h"
#include "APCLESS_Selection.h"
#include "APCLESS_Kernel.h"
#include "APCLESS_Gui.h"
#include "APCLESS_Tools.h"
#include "APCLESS_PayPass.h"
#include "APCLESS_payWave.h"

#include "libMtiCommonApi.h"
#include "libMtiEmvApi.h"
//@@WAY JCB Cless
#include "APCLESS_JSpeedy.h"
#include "APCLESS_ServicesKernel.h"
//
/////////////////////////////////////////////////////////////////
//// Macros & preprocessor definitions //////////////////////////

#define SERVICE_CUSTOM_KERNEL             0x0999   ///< Service number that payment kernels have to used to call the customization functions
#define SERVICE_DE_KERNEL                 0x0998   ///< Service number that payPass kernel use to call Data Exchange functions.

#define APCLESS_AID_PAYPASS_IDENTIFIER       0
#define APCLESS_AID_PAYWAVE_IDENTIFIER       1
#define APCLESS_AID_JSPEEDY_IDENTIFIER  	2 //@@WAY JCB Cless



/////////////////////////////////////////////////////////////////
//// Types //////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
//// Global variables ///////////////////////////////////////////

extern int APCLESS_DebugTrace;        ///< Global boolean variable to activate or not the kernel traces


/////////////////////////////////////////////////////////////////
//// Functions //////////////////////////////////////////////////


int APCLESS_Explicit_DoTransaction(tMtiMap *tpMap);

void APCLESS_Explicit_GetTrascationTag(tMtiMap *tpMap, T_SHARED_DATA_STRUCT* dataStruct);



#endif  //APCLESS_H_INCLUDED

