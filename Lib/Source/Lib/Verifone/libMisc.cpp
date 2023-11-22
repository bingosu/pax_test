
#include "libMtiCommonApi.h"

#include <sysinfo/sysinfo.h>
#include <sysinfo/syspm.h>
#include <ipc/ipc.h>
#include <ipc/jsobject.h>
#include <com/libcom.h>

#ifdef _VRXEVO
#   include <svc.h>
#   define sleep(x) SVC_WAIT(x*1000)
#   include <log/syslog.h>
#else
#   include <syslog.h>
#endif

#include <stdio.h>
#include <log/liblog.h>
#include <log/syslogcmd.h>


using namespace vfisysinfo;
using namespace vfiipc;
using namespace std;

INT mtiGetBatteryLevel()
{
	INT iRemainBattery =  get_battery_value(REMAININGCHARGE);
#if 1	//@@WAY BATTERY C680
	INT iFullCharge = get_battery_value(FULLCHARGE);
	INT iBattLimit = iFullCharge * 10 / 100;

	//dmsg("REMAININGCHARGE [%d]", iRemainBattery);
	dmsg("FULLCHARGE [%d]", iFullCharge);
	dmsg("LIMIT LOW BATTERY [%d]", iBattLimit);

	if(iRemainBattery <= iBattLimit)
	{
		dmsg("Remaining battery = %d", iRemainBattery * 100 / iFullCharge);
		iRemainBattery = iRemainBattery * 100 / iFullCharge;
	}
	else
	{
		dmsg("Remaining battery = %d", iRemainBattery * 100 / iFullCharge);
		iRemainBattery = iRemainBattery * 100 / iFullCharge;
	}
#else
	switch (iRemainBattery)
	{
		case 2450 :
			dmsg(">>>>>>>>>>>>>Remaining battery = 100%");
			iRemainBattery = 100;
			break;
		case 1715 :
			dmsg(">>>>>>>>>>>>>Remaining battery = 70%");
			iRemainBattery = 70;
			break;
		case 980 :
			dmsg(">>>>>>>>>>>>>Remaining battery = 40%");
			iRemainBattery = 40;
			break;

		case 245 :
			dmsg(">>>>>>>>>>>>>Remaining battery = 10%");
			iRemainBattery = 10;
			break;

		case 0 :
			dmsg(">>>>>>>>>>>>>Remaining battery = 0%");
			iRemainBattery = 0;
			break;

		default :
			dmsg(">>>>>>>>>>>>>Unknown battery value");
			iRemainBattery = -1;
			break;
	}
#endif
	return iRemainBattery;
}

INT mtiGetTerminalSerial(CHAR *cpOut)
{
	INT iRet = 0;

	iRet = SVC_INFO_SERLNO(cpOut);

	if (iRet < 0)
		return RTN_ERROR;

	return RTN_SUCCESS;
}

CHAR *monthList[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"};

INT mtiGetTerminalVersion(CHAR *cpOut)
{
	INT iRet;
	CHAR caModelNum[12+1] = {0,};
	CHAR *cpVer = cpOut;

	cpVer += mtiStrcpy(cpVer, (CHAR *)"VE", 2);

	iRet = SVC_INFO_MODELNO(caModelNum);

	if (iRet < 0)
		return RTN_ERROR;

	if(mtiMemcmp(caModelNum + 2, (CHAR *)"52", 2) == 0)
	{
		if(mtiMemcmp(caModelNum + 2, (CHAR *)"520C", 4) == 0) cpVer += mtiStrcpy(cpVer, (CHAR *)"05", 2);
		else cpVer += mtiStrcpy(cpVer, (CHAR *)"00", 2);
	}
	else if(mtiMemcmp(caModelNum + 2, (CHAR *)"67", 2) == 0) cpVer += mtiStrcpy(cpVer, (CHAR *)"01", 2);
	else if(mtiMemcmp(caModelNum + 1, (CHAR *)"680", 3) == 0) cpVer += mtiStrcpy(cpVer, (CHAR *)"08", 2);
	else return RTN_ERROR;

	{
		INT iIdx, iMonth = 1;
		CHAR caDATE[20] = {0,};
		CHAR caMonth[10] = {0,};
		CHAR caDay[10] = {0,};
		CHAR caYear[10] = {0,};

		sprintf(caDATE, "%s", __DATE__);

		sscanf(caDATE, "%s %s %s", caMonth, caDay, caYear);

		for(iIdx = 0; iIdx < 12; iIdx++)
			if(!mtiMemcmp(monthList[iIdx], caMonth, 3))
				iMonth = iIdx + 1;

		iMonth = 8;	// @@EB MARK
		cpVer += sprintf(cpVer, "%s%02d%s", "21", iMonth, REL_NUM);
	}

//	dmsg("TERM VER[%s]", cpOut);
	return RTN_SUCCESS;
}

INT mtiSysSleep(INT iOnOff)
{
	INT iCurDispBL = 0;
	INT iCurKeyBL = 0;


	sysGetPropertyInt(SYS_PROP_DISP_BACKLIGHT, &iCurDispBL);
	sysGetPropertyInt(SYS_PROP_KEYB_BACKLIGHT, &iCurKeyBL);

	//dmsg("Current DISP BL [%d], KEY BL [%d]", iCurDispBL, iCurKeyBL);


	if(iOnOff == TRUE)
	{
		//SLEEP

		//LCD Back light Off
		sysSetPropertyInt(SYS_PROP_DISP_BACKLIGHT, 0);
		//KEY Back light Off
		sysSetPropertyInt(SYS_PROP_KEYB_BACKLIGHT, 0);
	}
	else
	{
		//WAKE UP
		//LCD Back light Off
		sysSetPropertyInt(SYS_PROP_DISP_BACKLIGHT, 100);
		//KEY Back light Off
		sysSetPropertyInt(SYS_PROP_KEYB_BACKLIGHT, 100);

	}

	return RTN_SUCCESS;
}

INT mtiGetHwType()
{
	INT iRet = 0;

	CHAR caModelNum[12+1] = {0,};

	iRet = SVC_INFO_MODELNO(caModelNum);

	//dmsg("mtiGetHwType: [%s]", caModelNum);

	if(mtiMemcmp(caModelNum + 2, (CHAR *)"52", 2) == 0)
	{
		if(mtiMemcmp(caModelNum + 2, (CHAR *)"520C", 4) == 0) iRet = VX520C;
		else iRet = VX520;
	}
	else if(mtiMemcmp(caModelNum + 2, (CHAR *)"67", 2) == 0) iRet = VX675;
	else if(mtiMemcmp(caModelNum + 1, (CHAR *)"680", 3) == 0) iRet = C680;

	return iRet;
}

INT mtiGetHwSupport()
{
	INT iHwPresent = 0;
	INT iSupportInfo = 0;

	//Check Support HW
	iHwPresent = SVC_INFO_PRESENT();

	dmsg("HW SUPPORT PRESENT: [0x%.8x]", iHwPresent);

	if(iHwPresent & INFO_PRES_CTLS)
	{
		dmsg("Support - [HW_SUPPORT_CTLS]");
		iSupportInfo |= HW_SUPPORT_CTLS;
	}

	return iSupportInfo;
}


VOID mtiSysReboot()
{
	SVC_RESTART("");
}

INT iWiFiScanRawDataParser(CHAR *cpWiFiScanRawData, tWifiScanAP *tpWifiScanList, INT *iWifiScanCnt)
{
	JSObject jsobj;
	INT iScanApCnt = 0;

	if(cpWiFiScanRawData == NULL)
	{
		dmsg("cpWiFiScanRawData is NULL!!!");
		return RTN_ERROR;
	}

	dmsg("MAX WIFI Scan List Cnt[%d]", *iWifiScanCnt);

	if(jsobj.load(std::string(cpWiFiScanRawData))){
		dmsg("WIFI Site Survey Parsed successfully");

		CHAR surveyNumber[3] = {0,};

		if (!jsobj.exists(COM_WLAN_COUNT)) {
			dmsg("NO WLAN_COUNT found");
			*iWifiScanCnt = 0;
			return RTN_ERROR;
		}
		else
		{
			INT iApIdx = 0;
			INT iGetApCnt = 0;

			iScanApCnt = jsobj(COM_WLAN_COUNT).getInt();
			dmsg("** Found WIFI APs [%d] **", iScanApCnt);
			if(iScanApCnt > *iWifiScanCnt)
				iScanApCnt = *iWifiScanCnt;

			for(iApIdx = 0; iApIdx < iScanApCnt; iApIdx++)
			{
				//SSID
				string ssid;
				snprintf(surveyNumber,sizeof(surveyNumber), "%u", iApIdx);
				surveyNumber[2] = '\0';

				string name("WLAN");
				string entry;
				name.append(surveyNumber);

				entry = name;
				entry.append("_SSID");

				if (jsobj.exists(entry))
					ssid = jsobj(entry).getString();

				//RSSI - PERSENTAGE
				int percentage = 0;
				entry = name;
				entry.append("_PERCENTAGE");
				percentage = jsobj(entry).getInt();

				//PROTOCOL
				enum com_WLANProto protocol;
				entry = name;
				entry.append("_PROTO");
				protocol = (enum com_WLANProto)jsobj(entry).getInt();

				//KEY MGMT
				enum com_WLANKeyMgmt key_mgmt;
				entry = name;
				entry.append("_KEY_MGMT");
				key_mgmt = (enum com_WLANKeyMgmt)jsobj(entry).getInt();

				//AUTH ALG
				enum com_WLANAuthAlg auth_alg;
				entry = name;
				entry.append("_AUTH_ALG");
				auth_alg = (enum com_WLANAuthAlg)jsobj(entry).getInt();

				if(ssid.c_str() == NULL)
				{
					dmsg("SSID is NULL!!!");
					continue;
				}

				dmsg("SSID[%s] - [%d]%%, PROTO[%d], KEY MGMT[%d], AUTH_ALG[%d]", (ssid.c_str()==NULL)?"":ssid.c_str(),
						percentage, protocol, key_mgmt, auth_alg);

				switch(protocol)
				{
					case COM_WLAN_PROTO_WPA_WPA2:
						tpWifiScanList[iGetApCnt].iAuthType = COMM_WIFI_PROTO_WPA_WPA2;
						break;
					case COM_WLAN_PROTO_WPA:
						tpWifiScanList[iGetApCnt].iAuthType = COMM_WIFI_PROTO_WPA;
						break;
					case COM_WLAN_PROTO_WPA2:
						tpWifiScanList[iGetApCnt].iAuthType = COMM_WIFI_PROTO_WPA2;
						break;
					case COM_WLAN_PROTO_WEP:
						tpWifiScanList[iGetApCnt].iAuthType = COMM_WIFI_PROTO_WEP;
						break;
					default:
						continue;

				}

				switch(key_mgmt)
				{
					case COM_WLAN_KEY_MGMT_PSK:
						tpWifiScanList[iGetApCnt].iKeyMgmt = COMM_WIFI_KEY_MGMT_PSK;
						break;
					case COM_WLAN_KEY_MGMT_EAP:
						tpWifiScanList[iGetApCnt].iKeyMgmt = COMM_WIFI_KEY_MGMT_EAP;
						break;
					case COM_WLAN_KEY_MGMT_NONE:
						tpWifiScanList[iGetApCnt].iKeyMgmt = COMM_WIFI_KEY_MGMT_NONE;
						break;
					default:
						continue;

				}

				switch(auth_alg)
				{
					case COM_WLAN_AUTH_OPEN:
						tpWifiScanList[iGetApCnt].iAuthAlg = COMM_WIFI_AUTH_OPEN;
						break;
					case COM_WLAN_AUTH_SHARED:
						tpWifiScanList[iGetApCnt].iAuthAlg = COMM_WIFI_AUTH_SHARED;
						break;
					default:
						continue;
				}
				mtiStrcpy(tpWifiScanList[iGetApCnt].caSSID, (VOID *)ssid.c_str(), (INT)ssid.length());
				tpWifiScanList[iGetApCnt].iRssiPersentage = percentage;

				iGetApCnt++;

			}
			*iWifiScanCnt = iGetApCnt;

		}
	}
	else
	{
		dmsg("WIFI Site Survey Parsed failed...");
		return RTN_ERROR;
	}

	return RTN_SUCCESS;
}

VOID mtiGetRamStatus(UINT *uiMaxBytes, UINT *uiFreeBytes)
{
	*uiMaxBytes = SVC_RAM_SIZE()*1024;
	*uiFreeBytes = *uiMaxBytes - ((UINT)(_stack_max() + _heap_current()));
	dmsg("stack[%ld] heap[%ld]",_stack_max(), _heap_current());
}

VOID mtiDebugStart(CHAR *pszIP, INT nPort)
{
#if 0
	LOGAPI_SETLEVEL(LOGAPI_TRACE);
#else
	// # Set log output destionation. Available values: 0-serial, 1-udp, 2-file, 3-USB
	// # Serial communication port: 1-COM1 (default), 2-COM2, 6-COM6

	INT iDest, iPreDest;
	INT iPort, iPrePort;
	INT iEnable;

	if(NULL==pszIP && 0==nPort)
	{
		LOGAPI_SETLEVEL(LOGAPI_TRACE);
		return;
	}

	if(NULL==pszIP)
	{
		switch (nPort)
		{
			case COMMCHANNEL_COM1:
				iDest = 0;
				iPort = 1;
				break;

			case COMMCHANNEL_COM6:
				iDest = 0;
				iPort = 6;
				break;

			case COMMCHANNEL_DEV_USB:
				iDest = 3;
				break;
		}

		if(0==iDest)
		{
			syslcmd_set_cfg_int(SERIAL_COM, iPort);
		}
	}
	else
	{
		iDest = 1;
		syslcmd_set_cfg_str(UDP_HOST, pszIP);
		syslcmd_set_cfg_int(UDP_PORT, nPort);
	}

	syslcmd_set_cfg_int(DESTINATION, iDest);
	syslcmd_get_cfg_int(ENABLED, &iEnable);

	if(0==iEnable)
	{
		syslcmd_set_cfg_int(ENABLED, 1);
	}

	syslcmd_apply_config();

	LOGAPI_SETLEVEL(LOGAPI_TRACE);

#endif
}

VOID mitDebugStop()
{
#if 1
	LOGAPI_SETLEVEL(LOGAPI_OFF);
#else
	INT iEnable;

	syslcmd_get_cfg_int(ENABLED, &iEnable);

	if(1==iEnable)
		syslcmd_set_cfg_int(ENABLED, 0);

	syslcmd_apply_config();

	LOGAPI_SETLEVEL(LOGAPI_OFF);
#endif
}


