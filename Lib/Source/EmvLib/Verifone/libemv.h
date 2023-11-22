/*
 * libemv.h
 *
 *  Created on: 2016. 9. 20.
 *      Author: kjyoo
 */

#ifndef SRC_EMVLIB_VERIFONE_LIBEMV_H_
#define SRC_EMVLIB_VERIFONE_LIBEMV_H_

#include "libMtiEmvApi.h"


extern EMV_ADK_INFO SetCTApplicationData();
extern EMV_ADK_INFO SetCTTerminalData(void);

#ifdef __cplusplus
extern "C" {
#endif

extern void vdGetTime(unsigned char *buf, size_t bufLen);
extern void vdGetDate(unsigned char *buf, size_t bufLen);
extern int bcdfromulong(unsigned char *bcd, int size, unsigned long value);
extern void sleepMilliSecond(int msec);

extern void vdFormatCurrencyCode(unsigned char *pucHexCCode, char * szFormattedCCode);

#ifdef __cplusplus
}
#endif

#endif /* SRC_EMVLIB_VERIFONE_LIBEMV_H_ */
