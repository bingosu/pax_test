//----------------------------------------------------------------------------
//
// UtilFunc.cpp -	Utility APIs.
//              	November 2015
//
//----------------------------------------------------------------------------
/*****************************************************************************
	 Copyright (C) 2015 by VeriFone Inc. All rights reserved.

	 No part of this software may be used, stored, compiled, reproduced,
	 modified, transcribed, translated, transmitted, or transferred, in
	 any form or by any means  whether electronic, mechanical,  magnetic,
	 optical, or otherwise, without the express prior written permission
							  of VeriFone, Inc.
******************************************************************************/
#include <svc.h>
#include <svc_sec.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//=====================================================================================================================
void vdGetTime(unsigned char *buf, size_t bufLen)
{
   time_t t = time(0);
   struct tm *_tm = localtime(&t);
   if (!_tm) return;

   if (bufLen < 3) return;

   memset(buf, 0, bufLen);
   bcdfromulong(buf, 1, _tm->tm_hour);
   bcdfromulong(buf + 1, 1, _tm->tm_min);
   bcdfromulong(buf + 2, 1, _tm->tm_sec);
}

// ====================================================================================================================
void vdGetDate(unsigned char *buf, size_t bufLen)
{
   time_t t = time(0);
   struct tm *_tm = localtime(&t);
   if (!_tm) return;

   if (bufLen < 3) return;

   memset(buf, 0, bufLen);
   bcdfromulong(buf, 1, _tm->tm_year % 100);
   // tm_mon is 0-11, thus + 1
   bcdfromulong(buf + 1, 1, _tm->tm_mon + 1);
   bcdfromulong(buf + 2, 1, _tm->tm_mday);
}

// ====================================================================================================================
int bcdfromulong(unsigned char *bcd, int size, unsigned long value)
{
   memset(bcd,0,size);
   while(--size>=0) {
      bcd[size]= (unsigned char) (value%10);
      value/=10;
      bcd[size]+= (unsigned char) (value%10*16);
      value/=10;
   }
   if(value) return 1;
   return 0;
}

//===========================================================================================================
void sleepMilliSecond(int msec)
{
    SVC_WAIT(msec);
}

//===========================================================================================================
