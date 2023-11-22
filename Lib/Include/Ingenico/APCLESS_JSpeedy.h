/*
 * APCLESS_JSpeedy.h
 *
 *  Created on: March 5 2021
 *      Author by: Wilada Achmad
 *	
 *
 */

#ifndef APCLESS_JSPEEDY_H
#define APCLESS_JSPEEDY_H

#include "libMtiCommonApi.h"
#include "sdk_tplus.h"
#include "APCLESS.h"
#include "apMtiCommonApp.h"

#define CLESS_SAMPLE_JCB_END_APPLI                          1
#define CLESS_SAMPLE_JCB_END_APPLI_RESTART_COMM_ERROR       2
#define CLESS_SAMPLE_JCB_END_APPLI_RESTART_ON_DEVICE_CVM    3

int APCLESS_JCB_PerformTransaction (T_SHARED_DATA_STRUCT * dataStruct);
int APCLESS_JCB_CustomizeStep (T_SHARED_DATA_STRUCT * pSharedData, const unsigned char ucCustomizationStep);

#endif
