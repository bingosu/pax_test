/*
  ICL_inicrypto.h
  librsadlx

  Created by DaeHyun Kim on 2015. 2. 28..
  Copyright (c) 2015??INITECH. All rights reserved.
*/

#ifndef librsadlx_ICL_log_h
#define librsadlx_ICL_log_h

#define ICL_OFF			0
#define ICL_FATAL		2
#define ICL_ERROR		3
#define ICL_NORMAL		5
#define ICL_INFORM		7
#define ICL_DEBUG		8

void    ICL_Log(int level, char *file, int line, char *format, ...);
void	ICL_Log_HEXA(int level, char *file, int line, char *msgname, unsigned char *content, int len);

#endif
