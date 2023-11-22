#ifndef __INC_LIBDEBUG_H__
#define __INC_LIBDEBUG_H__

#ifdef __INGENICO_SRC__
//#include <sdk_tplus.h>

extern char g_caDebugData[8192 + 1];
extern int  g_iDebugLen;
extern long long g_lDebugTick;

extern void debugSend(unsigned char *ucpData, int iLength);
extern void debugStop();
extern void debugStart(int nPortName);
extern void debugBuffer( char* cpTitle, unsigned char* ucpData, int iSizeData, char* cpFile, int iLine );
extern int debugIsUsingDebugPort(int nPortName);

#ifdef __DEBUG_ON__
#define dstr(X) 		debugSend((unsigned char*)X, strlen((char*)X));
#define dstrln(X) 		debugSend((unsigned char*)X, strlen((char*)X)); debugSend((unsigned char*)"\n\r", 2);
#define dpt()			sprintf(g_caDebugData, "FILE[%s], LINE[%d]", __FILE__, __LINE__); dstrln(g_caDebugData);// mtiSleep(100);
#define dmsg(...)		sprintf(g_caDebugData, __VA_ARGS__); dstrln(g_caDebugData);
#define dmsgnoln(...)	sprintf(g_caDebugData, __VA_ARGS__); dstr(g_caDebugData);
#define dbuf(X,Y,Z)		debugBuffer(X, Y, Z, __FILE__, __LINE__);
#define dtickstart()	g_lDebugTick = OSL_TimeStp_Now();
#define dtick()			sprintf(g_caDebugData, "FILE[%s], LINE[%d], TICK [%ldms]", __FILE__, __LINE__, (long)((OSL_TimeStp_Now() - g_lDebugTick) / 1000)); dstrln(g_caDebugData);
#define dpt_n_dmsg(...)		sprintf(g_caDebugData, "----- FILE[%s], LINE[%d] -----\n\r", __FILE__, __LINE__);\
							sprintf(g_caDebugData + strlen(g_caDebugData), __VA_ARGS__);\
							dstrln(g_caDebugData);

#define dpt_e			dpt
#define dmsg_e			dsmg
#define dbuf_e			dbuf
#define dpt_n_dmsg_e	dpt_n_dmsg

#define dmsg_ll(title, value)	 { CHAR buffer[64] = { '\0', }; mtiLLongToA(value, buffer); dmsg("[%s]: [%s] = [%s]", title, #value, buffer); }			

#else
#define dstr(X)
#define dstrln(X)
#define dpt()
#define dmsg(...)
#define dmsgnoln(...)
#define dbuf(X,Y,Z)
#define dtickstart()
#define dtick()
#define dpt_n_dmsg(...)	

#define dpt_e()
#define dmsg_e(...)
#define dbuf_e(X,Y,Z)
#define dpt_n_dmsg_e(...)
#define dmsg_ll(buffer, value)

#endif
#elif __VERIFONE_SRC__
#ifdef __DEBUG_ON__

		#define dinit()	
		#define dstr(X)			LOGF_TRACE(X);
		#define dpt()			LOGF_TRACE("FUNC[%s]", __func__);
		#define dmsg(...)		LOGF_TRACE(__VA_ARGS__);
		#define dbuf(X,Y,Z)		do {\
									if(X) { \
										LOGF_TRACE(X); \
									}\
									LOGAPI_HEXDUMP_RAW_TRACE((void *)Y, Z);\
								}while(0);

		#define dtickstart()
		#define dtick()
		#define dtickend()
		#define dpt_n_dmsg(...)	LOGF_TRACE(__VA_ARGS__);


		#define dmsg_e(...)		LOGF_ERROR(__VA_ARGS__);
		#define dbuf_e(X,Y,Z)		do {\
		if(X) { \
		LOGF_ERROR(X); \
		}\
		LOGAPI_HEXDUMP_RAW_ERROR((void *)Y, Z); \
								}while(0);
		#define dpt_e()			LOGF_ERROR("FUNC[%s]", __func__);
		#define dpt_n_dmsg_e(...)	LOGF_ERROR(__VA_ARGS__);
		#define dmsg_ll(title, value)	 { CHAR buffer[64] = { '\0', }; mtiLLongToA(value, buffer); dmsg("[%s]: [%s] = [%s]", title, #value, buffer); }

#else
#define dinit()
#define dstr(X)
#define dpt()
#define dmsg(...)
#define dbuf(X,Y,Z)
#define dtickstart()
#define dtick()
#define dtickend()
#define dpt_n_dmsg(...)		

#define dpt_e()
#define dmsg_e(...)
#define dbuf_e(X,Y,Z)
#define dpt_n_dmsg_e(...)
#define dmsg_ll(buffer, value)

#endif	// __DEBUG_ON__

#endif // __VERIFONE_SRC__

#endif //__INC_LIBDEBUG_H__
