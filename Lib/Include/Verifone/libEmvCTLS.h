/*
 * libCTLS.h
 *
 *  Created on: Sep 25, 2016
 *      Author: share_pc
 */
#include "libMtiEmvApi.h"

#ifndef INC_LIB_LIBCTLS_H_
#define INC_LIB_LIBCTLS_H_

INT mtiRfEmvSetParam(BOOL bDoClear);

EMV_ADK_INFO v_SetCTLSApplicationData(void);
EMV_ADK_INFO v_SetCTLSTerminalData(void);
EMV_ADK_INFO RfEMVTransaction(int duration);
EMV_ADK_INFO v_InitFramework(void);
void v_EndTransaction(const char *szStatusMessage, int inExitWithError);

#endif /* INC_LIB_LIBCTLS_H_ */
