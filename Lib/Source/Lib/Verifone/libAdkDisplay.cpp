#include <SVC.H>
#include <html/gui.h>
#include <html/gui_template.h>
#include <html/gui_error.h>
#include <html/jsobject.h>
#include <html/jsproc.h>
#include <html/scriptproc.h>

#define _USE_TIMEVAL_FROM_SVC_NET_H_
#include <sys/time.h>
#include <sysinfo/sysinfo.h>
#include <sysinfo/sysbar.h>
#include <sysinfo/sysbeep.h>

#include "libAdkDisplay.h"

/*
#include <html/jsobject.h>
#include <html/jsproc.h>
#include <html/scriptproc.h>
*/

extern int mtDisplay;

using namespace std;
using namespace vfigui;
using namespace vfisysinfo;
using namespace vfihtml;
using namespace js;

#define EVT_ADKGUI	(1L<<30)

#define USER_EVT_FROM_GUI_MAIN_MENU		(1L<<15)
#define USER_EVT_FROM_GUI_ADMIN_MENU	(1L<<16)
#define USER_EVT_FROM_GUI_INIT_MENU		(1L<<17)
#define USER_EVT_FROM_GUI_HOTKEY_MENU	(1L<<18)
#define USER_EVT_FROM_GUI_MAIN_MENU_ERR	(1L<<20)

static void statusbar_callback(void *data, int region_id, map<string,string> &values)
{
	networkStat *pNetworkStat = (networkStat *)data;
	// region ID is useful, if application uses one callback for multiple statusbars
	if(region_id==1)
	{
	static int signal_level=0;
	static int battery_level=11;
	static bool fDocked=FALSE;
	char buf[16];
	int iCommType = CMM_TYPE_DIALUP;
	string strCommType;
	signal_level = 5;	// 100%

	iCommType = mtiMapGetInt(MAP_EDC_PARAM, KEY_PARAM_COMM_TYPE);

	switch(iCommType)
	{
		case CMM_TYPE_DIALUP:
			strCommType = "DIAL";
			break;
		case CMM_TYPE_CELLULAR:
			if(mtiGetHwType() == VX675 || mtiGetHwType() == VX520C || mtiGetHwType() == C680)
				strCommType = "3G";
			else
				strCommType = "GPRS";
			if(mtiStrlen(M_SIZEOF(networkStat, caWifiSSID), pNetworkStat->caWifiSSID) > 0)
			{
				//dmsg("GPRS PROVIDER[%s]", pNetworkStat->caWifiSSID);
				strCommType.append("-");
				//@@WAY BATTERY C680 
				#if 1
				strCommType.append(string(pNetworkStat->caWifiSSID), 0, strlen(pNetworkStat->caWifiSSID));
				#else
				strCommType.append(string(pNetworkStat->caWifiSSID), 0, 10);
				#endif
			}
			break;
		case CMM_TYPE_WIFI:
			strCommType = "WIFI";
			if(mtiStrlen(M_SIZEOF(networkStat, caWifiSSID), pNetworkStat->caWifiSSID) > 0)
			{
				//dmsg("WIFI SSID[%s]", pNetworkStat->caWifiSSID);
				strCommType.append("-");
				//@@WAY BATTERY C680 
				#if 1
				strCommType.append(string(pNetworkStat->caWifiSSID), 0, strlen(pNetworkStat->caWifiSSID));
				#else
				strCommType.append(string(pNetworkStat->caWifiSSID, 0, 10));
				#endif
			}
			break;
		case CMM_TYPE_ETHERNET:
			strCommType = "ETHERNET";
			signal_level = pNetworkStat->ethLink?5:0;
			break;
	}
	
	//WIRELESS SIGNAL STRENGTH
	if(iCommType == CMM_TYPE_CELLULAR || iCommType == CMM_TYPE_WIFI)
	{
		signal_level = pNetworkStat->gprsRssiPersent/20;
	}

	sprintf(buf,"%d",signal_level);
	values["my_sys_mob_netw_signal_level"]=string(buf);
	values["my_sys_mob_netw_type"]=strCommType;

	sprintf(buf,"%d",battery_level);
	values["my_sys_battery_level"]=string(buf);

	switch (battery_level)
	{
		case 0:
		case 100:
			values["my_sys_battery_percentage"]=1;
			values["my_sys_battery_percentage_2"]="1%";
			break;
		case 1:
		case 101:
			values["my_sys_battery_percentage"]=5;
			values["my_sys_battery_percentage_2"]="5%";
			break;
		case 2:
		case 102:
			values["my_sys_battery_percentage"]=10;
			values["my_sys_battery_percentage_2"]="10%";
			break;
		case 3:
		case 103:
			values["my_sys_battery_percentage"]=20;
			values["my_sys_battery_percentage_2"]="20%";
			break;
		case 4:
		case 104:
			values["my_sys_battery_percentage"]=30;
			values["my_sys_battery_percentage_2"]="30%";
			break;
		case 5:
		case 105:
			values["my_sys_battery_percentage"]=40;
			values["my_sys_battery_percentage_2"]="40%";
			break;
		case 6:
		case 106:
			values["my_sys_battery_percentage"]=50;
			values["my_sys_battery_percentage_2"]="50%";
			break;
		case 7:
		case 107:
			values["my_sys_battery_percentage"]=60;
			values["my_sys_battery_percentage_2"]="60%";
			break;
		case 8:
		case 108:
			values["my_sys_battery_percentage"]=70;
			values["my_sys_battery_percentage_2"]="70%";
			break;
		case 9:
		case 109:
			values["my_sys_battery_percentage"]=80;
			values["my_sys_battery_percentage_2"]="80%";
			break;
		case 10:
		case 110:
			values["my_sys_battery_percentage"]=90;
			values["my_sys_battery_percentage_2"]="90%";
			break;
		default: // 11 / 111 or 100%
			values["my_sys_battery_percentage"]=100;
			values["my_sys_battery_percentage_2"]="100%";
			break;
	}

	if(battery_level==0 && !fDocked)
	{
		fDocked = TRUE;
		battery_level=100;
	}

	if(battery_level>=111 && fDocked)
	{
		fDocked = FALSE;
		battery_level=11;
	}
	if(fDocked)
		battery_level=((battery_level+1)%112); // 0..111, update level value for next refresh
	else
		battery_level=((battery_level-1)%12); // 11..0, update level value for next refresh
	}
}

int g_isADKGUI = FALSE;

void cbGuiHomeScr(void *data, UICBType type, UICBData &uidata)
{
	INT iKeyVal;
	INT *mainTaskId = (INT *)data;

	dmsg("TYPE: [%d]", type);

	if( type == UI_CB_RESULT )
	{
		iKeyVal = uidata.result();

		dmsg("Result[%d] mainTaskId[%d]", uidata.result(), *mainTaskId);

		switch(iKeyVal)
		{
			case 1:	//MENU
				post_user_event(*mainTaskId, USER_EVT_FROM_GUI_MAIN_MENU);
				dmsg("SENT USER_EVT_FROM_GUI_MAIN_MENU TO MAIN EVT LOOP");
				break;

			case 2:	//ADMIN
				post_user_event(*mainTaskId, USER_EVT_FROM_GUI_ADMIN_MENU);
				dmsg("SENT USER_EVT_FROM_GUI_ADMIN_MENU TO MAIN EVT LOOP");
				break;

			case 3: //INITIALIZE
				post_user_event(*mainTaskId, USER_EVT_FROM_GUI_INIT_MENU);
				dmsg("SENT USER_EVT_FROM_GUI_INIT_MENU TO MAIN EVT LOOP");
				break;

			case 4:	//HOT KEY
				post_user_event(*mainTaskId, USER_EVT_FROM_GUI_HOTKEY_MENU);
				dmsg("SENT USER_EVT_FROM_GUI_HOTKEY_MENU TO MAIN EVT LOOP");
				break;
			case UI_ERR_CANCELLED:
				dmsg("UI_ERR_CANCELLED");
				break;
			case UI_ERR_TIMEOUT:
				dmsg("MAIN SCR OCCURDDED TIMEOUT!");
			default:	//contained TIME_OUT
				post_user_event(*mainTaskId, USER_EVT_FROM_GUI_MAIN_MENU_ERR);
				dmsg("SENT USER_EVT_FROM_GUI_MAIN_MENU_ERR TO MAIN EVT LOOP");
				break;
		}

	}

	/*
	if(iMenuSel > 0)
	{
		ProcEvent(MTI_EVT_NORMAL_KEY, &iMenuSel, NULL);
		startADKGUI();
		dispGuiHomeScreen();
	}
	*/
	dmsg("cbGuiHomeScr end!");
}



void initADKGUI(networkStat *pNetworkStat)
{
	int iRet = 0;
	map<string,string> values;

	uiSetVerixNotification(get_task_id(), EVT_ADKGUI);

	htmlSetScriptProcessor("js",js::jsProcessorExt,0);
	jsSetHttpProxy("http://localhost:12345");

	uiLayout("layout");

	//status bar
	uiLayout("layout-status");

	iRet=sysStartStatusbarURL(1,"statusbar.html",values,statusbar_callback, pNetworkStat);
	dmsg("show up status bar done. [%d]", iRet);

	g_isADKGUI = TRUE;
}

int getIsADKGUI()
{
	return g_isADKGUI;
}

void startADKGUI()
{
	int iRet = 0;

	dmsg("!!!Start ADKGUI!!!");

	if(mtDisplay != 0)
	{
		close(mtDisplay);
		mtDisplay = 0;
	}
	SVC_WAIT(100);

	iRet = uiReleaseConsole();
	dmsg("uiReleaseConsole Ret[%d]", iRet);

	g_isADKGUI = TRUE;

}

void stopADKGUI()
{
	int iRet = 0;

	SVC_WAIT(100);
	dmsg("!!!Stop ADKGUI!!!");
	iRet = uiObtainConsole();
	dmsg("uiObtainConsole Ret[%d]", iRet);
	SVC_WAIT(100);
	if (mtDisplay == 0)
	{
		mtDisplay = open(DEV_CONSOLE, O_RDWR);
	}

	g_isADKGUI = FALSE;
}

void dispToGui(CHAR *msg)
{
	CHAR uiDispCts[100] = {0,};

	if(mtiGetHwType() == VX520)
		return;

	sprintf(uiDispCts, "<HTML><BODY><H4>%s</H4></BODY></HTML>", msg);

	//uiSetPropertyString(UI_PROP_RESOURCE_PATH, "I:1/WWW/VX675");

	uiDisplay(string(uiDispCts));
}
void testADKGUI()
{
	uiDisplay("simple","<h4>Display</h4>Hello world!");
	return;
}

void dispGuiHomeWarnScreen(INT *iMainTaskID)
{
	int iRet = 0;
	char caVer[12 + 1] = {0,};
	char caVerPost[12 + 1] = {0,};
	map<string,string> main_value;

	uiSetPropertyInt(UI_PROP_TIMEOUT, -1);

	mtiGetTerminalVersion(caVer);
	mtiStrcpy(caVerPost, caVer, 9);

	if (iTransRestrictionCheckSettle() == RTN_SUCCESS)
	{
		main_value["dispmsg1"] = string("SETTLEMENT FIRST  -  ") + string(caVerPost);
	}
	else
	{
		main_value["dispmsg1"] = string(caVerPost);
	}

	iRet = uiInvokeURLAsync(main_value, "homescr.html", cbGuiHomeScr, iMainTaskID);

	dmsg("uiInvokeURLAsync iRet[%d] mainTaskId[%d]",iRet, *iMainTaskID);
	if(iRet == UI_ERR_SCRIPT)
	{
		string errStr = "script Err[" + uiScriptError() + "]";
		dmsg("%s",errStr.c_str());
	}

	return;
}

void dispGuiHomeScreen(INT *iMainTaskID)
{
	int iRet = 0;
	char caVer[12 + 1] = {0,};
	char caVerPost[12 + 1] = {0,};
	map<string,string> main_value;

	uiSetPropertyInt(UI_PROP_TIMEOUT, -1);

	mtiGetTerminalVersion(caVer);
	mtiStrcpy(caVerPost, caVer, 9);

	//@@WAY BATTERY C680
#if 1
	if (mtiGetHwType() == C680 && mtiGetBatteryLevel() <= 10 && get_battery_value(CHARGERSTATUS) != 0x03)
	{
		main_value["dispmsg1"] = string("LOW BATTERY, PLEASE CHARGING!!!  -  ") + string(caVerPost);
	}
	else if(mtiGetHwType() == C680 && mtiGetBatteryLevel() <= 10 && get_battery_value(CHARGERSTATUS) == 0x03)
	{
		main_value["dispmsg1"] = string("ON CHARGING  -  ") + string(caVerPost) + string("\nPLEASE WAIT UNTIL MORE THAN 10%");
	}
#else
	if (mtiGetHwType() == C680 && mtiGetBatteryLevel() < 40 && get_battery_value(CHARGERSTATUS) != 0x03)
	{
		main_value["dispmsg1"] = string("CAUTION! BATTERY IS VERY LOW -  ") + string(caVerPost);
	}
	else if (mtiGetHwType() == C680 && mtiGetBatteryLevel() < 10 && get_battery_value(CHARGERSTATUS) == 0x03)
	{
		main_value["dispmsg1"] = string("CHARGING! PLEASE WAIT BATTERY 10% -  ") + string(caVerPost);
	}
#endif
	//
	else
	{
		if (iTransRestrictionCheckSettle() == RTN_SUCCESS)
		{
			main_value["dispmsg1"] = string("SETTLEMENT FIRST  -  ") + string(caVerPost);
		}
		else
		{
			main_value["dispmsg1"] = string(caVerPost);
		}
	}

	iRet = uiInvokeURLAsync(main_value, "homescr.html", cbGuiHomeScr, iMainTaskID);

	dmsg("uiInvokeURLAsync iRet[%d] mainTaskId[%d]",iRet, *iMainTaskID);
	if(iRet == UI_ERR_SCRIPT)
	{
		string errStr = "script Err[" + uiScriptError() + "]";
		dmsg("%s",errStr.c_str());
	}

	return;
}



#if 0
void dispGuiHomeScreen(INT *iMainTaskID)
{
	int iRet = 0;
	char caVer[12 + 1] = {0,};
	char caVerPost[12 + 1] = {0,};
	map<string,string> main_value;

	uiSetPropertyInt(UI_PROP_TIMEOUT, -1);

	mtiGetTerminalVersion(caVer);
	mtiStrcpy(caVerPost, caVer, 9);

	if (iTransRestrictionCheckSettle() == RTN_SUCCESS)
	{
		main_value["dispmsg1"] = string("SETTLEMENT FIRST  -  ") + string(caVerPost);
	}
	else
	{
		main_value["dispmsg1"] = string(caVerPost);
	}

	if (mtiGetHwType() == C680)
	{
		if (mtiGetBatteryLevel() < 40 && get_battery_value(CHARGERSTATUS) != 0x03)
		{
			iRet = uiInvokeURLAsync(main_value, "homescr_warn.html", cbGuiHomeScr, iMainTaskID);
		}
		else
		{
			iRet = uiInvokeURLAsync(main_value, "homescr.html", cbGuiHomeScr, iMainTaskID);
		}
	}
	else
	{
		iRet = uiInvokeURLAsync(main_value, "homescr.html", cbGuiHomeScr, iMainTaskID);
	}

	dmsg("uiInvokeURLAsync iRet[%d] mainTaskId[%d]",iRet, *iMainTaskID);
	if(iRet == UI_ERR_SCRIPT)
	{
		string errStr = "script Err[" + uiScriptError() + "]";
		dmsg("%s",errStr.c_str());
	}

	return;
}
#endif

int dispMainMenuForC680(tMenuIdxVal *tpMenuIdxVal, int iMenuCnt)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};


	char *caHtmlHead =
			"<html><body>"
			"<table cellpadding='0' cellspacing='5px' border='0' style='height: 60%; width: 60%;'><tr>";
	char *caHtmlTail = "</table>"
					"<button style='visibility: hidden' accesskey='&#129;' action='call focus.up()'></button>"
					"<button style='visibility: hidden' accesskey='&#132;' action='call focus.down()'></button>"
					"<button style='visibility: hidden' accesskey='&#130;' action='call focus.left()'></button>"
					"<button style='visibility: hidden' accesskey='&#131;' action='call focus.right()'></button>"
					"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
					"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
					"<button style='visibility: hidden' accesskey='&#13;' action='call active.action()'></button>"	//Enter
					"<button style='visibility: hidden' accesskey='&#135;' action='call active.action()'></button>"	//Enter
					"<input type='idletimeout' value='30' style='visibility: hidden' action='return -2'>"
					"</body></html>";

	iOffset += sprintf(caHtmlDocBuff,"%s", caHtmlHead);

	dmsg("iMenuCnt [%d]", iMenuCnt);

	for(int iIdx = 0; iIdx < iMenuCnt; iIdx++)
	{
		char caImgFileName[50] = {0,};
		
		{
			int loop;
			int iLoc = 0;
			char *caItemName = tpMenuIdxVal[iIdx].caMenuItemName;
			for(loop = 0; loop < mtiStrlen(40, caItemName); loop++)
			{
				if(caItemName[loop] == ' ')
					caImgFileName[iLoc++] = '-';
				else
					caImgFileName[iLoc++] = caItemName[loop];
			}
		}
		
		dmsg("iIdx[%d], itemname[%s]", iIdx, tpMenuIdxVal[iIdx].caMenuItemName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<td>");
		if(iIdx == 0)
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' autofocus>", iIdx+1, tpMenuIdxVal[iIdx].value);
		}
		else
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' >", iIdx+1, tpMenuIdxVal[iIdx].value);
		}

		iOffset += sprintf(caHtmlDocBuff + iOffset,
				"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 80px; width: 70px;'>\
				<tr><td align='center' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<img src='images/%s.png'></td></tr>", caImgFileName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,
						"<tr><td align='center' width='1px' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"%s</td></tr></table></button></td>", tpMenuIdxVal[iIdx].caMenuItemName);


		if((iIdx+1) % 4 == 0 && iIdx != 0)
			iOffset += sprintf(caHtmlDocBuff + iOffset, "</tr><tr>");
	}

	iOffset += sprintf(caHtmlDocBuff + iOffset,"</tr>%s",caHtmlTail);

	//dmsg("htmlDoc[%s]", htmlDoc);

	string *strHtmlDoc = new string(caHtmlDocBuff);

	iRet = uiInvoke(*strHtmlDoc);
	//iRet = uiInvoke(0, htmlDoc, NULL, NULL);
	if(iRet == UI_ERR_SCRIPT)
	{
		dmsg("uiInvokeURL Script Error [%s]", uiScriptError().c_str());
	}
	else
	{
		dmsg("uiInvokeURL Result[%d]", iRet);
	}
	delete strHtmlDoc;
	return iRet;
}


#if 0
int dispMainMenuForC680(tMenuIdxVal *tpMenuIdxVal, int iMenuCnt)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};

	char *caHtmlHead =
			"<html><body>"
			//"<div id='mainHolder' style='overflow: auto; max-height: 310px;'>"
			"<table cellpadding='0' cellspacing='5px' border='0' style='height: 420px; width: 100%;'><tr>";
	char *caHtmlTail = 
					//"</div></table>"
					/*"</table>"
					"<table cellpadding='0' cellspacing='5' border='0' style='height: 20%; width: 100%;'><tr>"
					"<td>"
					"<button name='arrow_btn_1' action='call focus.left()'>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 70px; width: 69px;'><tr><td align='center' valign='middle'>"
					"<img src='images/left.png'></td></tr>"
					"<tr><td align='center' width='1px' valign='middle'></td></tr></table></button></td>"

					"<button name='arrow_btn_2' action='call focus.up()'>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 70px; width: 69px;'><tr><td align='center' valign='middle'>"
					"<img src='images/up.png'></td></tr>"
					"<tr><td align='center' width='1px' valign='middle'></td></tr></table></button></td>"

					"<button name='arrow_btn_3' action='call focus.down()'>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 70px; width: 69px;'><tr><td align='center' valign='middle'>"
					"<img src='images/down.png'></td></tr>"
					"<tr><td align='center' width='1px' valign='middle'></td></tr></table></button></td>"

					"<button name='arrow_btn_4' action='call focus.right()'>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 70px; width: 69px;'><tr><td align='center' valign='middle'>"
					"<img src='images/right.png'></td></tr>"
					"<tr><td align='center' width='1px' valign='middle'></td></tr></table></button></td>"
					"</tr></table>"*/
					
					/*"<button name='arrow_btn_2' action='call focus.down()'></button>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 90px; width: 70px;'><tr><td align='center' valign='middle'>"
					"<img src='images/up.png'></td></tr><tr><td align='center' width='1px' valign='middle'></td></tr></table></button>"
					
					"<button name='arrow_btn_3' action='call focus.left()'></button>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 90px; width: 70px;'><tr><td align='center' valign='middle'>"
					"<img src='images/down.png'></td></tr><tr><td align='center' width='1px' valign='middle'></td></tr></table></button>"
					
					"<button name='arrow_btn_4' action='call focus.right()'></button>"
					"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 90px; width: 70px;'><tr><td align='center' valign='middle'>"
					"<img src='images/right.png'></td></tr><tr><td align='center' width='1px' valign='middle'></td></tr></table></button></table></div>"*/
					
					"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
					"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
					"<button style='visibility: hidden' accesskey='&#13;' action='call active.action()'></button>"	//Enter
					"<button style='visibility: hidden' accesskey='&#135;' action='call active.action()'></button>"	//Enter
					"<input type='idletimeout' value='30' style='visibility: hidden' action='return -2'>"
					"</body></html>";

	iOffset += sprintf(caHtmlDocBuff,"%s", caHtmlHead);

	//dmsg("iMenuCnt C680 [%d]", iMenuCnt);

	for(int iIdx = 0; iIdx < iMenuCnt; iIdx++)
	{
		char caImgFileName[50] = {0,};
#if 1
		{
			int loop;
			int iLoc = 0;
			char *caItemName = tpMenuIdxVal[iIdx].caMenuItemName;
			for(loop = 0; loop < mtiStrlen(40, caItemName); loop++)
			{
				if(caItemName[loop] == ' ')
					caImgFileName[iLoc++] = '-';
				else
					caImgFileName[iLoc++] = caItemName[loop];
			}
		}
#endif
		dmsg("iIdx[%d], itemname[%s]", iIdx, tpMenuIdxVal[iIdx].caMenuItemName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<td>");
		if(iIdx == 0)
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' autofocus>", iIdx+1, tpMenuIdxVal[iIdx].value);
		}
		else
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' >", iIdx+1, tpMenuIdxVal[iIdx].value);
		}

		iOffset += sprintf(caHtmlDocBuff + iOffset,
				"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 80px; width: 70px;'>\
				<tr><td align='center' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<img src='images/%s.png'></td></tr>", caImgFileName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,
						"<tr><td align='center' width='1px' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"%s</td></tr></table></button></td>", tpMenuIdxVal[iIdx].caMenuItemName);


		if((iIdx+1) % 4 == 0 && iIdx != 0)
			iOffset += sprintf(caHtmlDocBuff + iOffset, "</tr><tr>");
	}

	iOffset += sprintf(caHtmlDocBuff + iOffset,"</tr>%s",caHtmlTail);


	string *strHtmlDoc = new string(caHtmlDocBuff);

	iRet = uiInvoke(*strHtmlDoc);

	switch (iRet)
	{
		case UI_ERR_OK :
			dmsg("uiInvokeURL Result[%s]", "No Error");
			break;
		case UI_ERR_ABORT :
			dmsg("uiInvokeURL Result[%s]", "abort by user");
			break;
		case UI_ERR_BACK :
			dmsg("uiInvokeURL Result[%s]", "user selected back button");
			break;
		case UI_ERR_TIMEOUT :
			dmsg("uiInvokeURL Result[%s]", "dialog timeout");
			break;
		case UI_ERR_PROPERTY :
			dmsg("uiInvokeURL Result[%s]", "the property does not exist");
			break;
		case UI_ERR_PARAMETER :
			dmsg("uiInvokeURL Result[%s]", "parameter error ");
			break;
		case UI_ERR_PERMISSION :
			dmsg("uiInvokeURL Result[%s]", "insufficient permissions");
			break;
		case UI_ERR_FAIL :
			dmsg("uiInvokeURL Result[%s]", "generic error");
			break;
		case UI_ERR_UNSUPPORTED :
			dmsg("uiInvokeURL Result[%s]", "the function requested is not supported on the current platform/device/...");
			break;
		case UI_ERR_CONNECTION_LOST :
			dmsg("uiInvokeURL Result[%s]", "lost connection to browser ");
			break;
		case UI_ERR_PROTOCOL :
			dmsg("uiInvokeURL Result[%s]", "data received violated the protocol");
			break;
		case UI_ERR_SCRIPT :
			dmsg("uiInvokeURL Result[%s]", "error occurred during script processing");
			break;
		case UI_ERR_FILE_READ :
			dmsg("uiInvokeURL Result[%s]", "the file was not found or could not be read");
			break;
		case UI_ERR_RESTRICTED :
			dmsg("uiInvokeURL Result[%s]", "the dialog was not shown due to security restrictions, e.g. using more than 3 buttons ");
			break;
		default :
			dmsg("uiInvokeURL Result[%d]", iRet);
			break;
	}

	/*
	if(iRet == UI_ERR_SCRIPT)
	{
		dmsg("uiInvokeURL Script Error [%s]", uiScriptError().c_str());
	}
	else
	{
		dmsg("uiInvokeURL Result[%d]", iRet);
	}*/
	
	delete strHtmlDoc;
	return iRet;
}
#endif


int dispMainMenu(tMenuIdxVal *tpMenuIdxVal, int iMenuCnt)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};

	char *caHtmlHead =
			"<html><body>"
			"<table cellpadding='0' cellspacing='5px' border='0' style='height: 60%; width: 60%;'><tr>";
	char *caHtmlTail = "</table>"
					"<button style='visibility: hidden' accesskey='&#129;' action='call focus.up()'></button>"
					"<button style='visibility: hidden' accesskey='&#132;' action='call focus.down()'></button>"
					"<button style='visibility: hidden' accesskey='&#130;' action='call focus.left()'></button>"
					"<button style='visibility: hidden' accesskey='&#131;' action='call focus.right()'></button>"
					"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
					"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
					"<button style='visibility: hidden' accesskey='&#13;' action='call active.action()'></button>"	//Enter
					"<button style='visibility: hidden' accesskey='&#135;' action='call active.action()'></button>"	//Enter
					"<input type='idletimeout' value='30' style='visibility: hidden' action='return -2'>"
					"</body></html>";

	iOffset += sprintf(caHtmlDocBuff,"%s", caHtmlHead);

	dmsg("iMenuCnt [%d]", iMenuCnt);

	for(int iIdx = 0; iIdx < iMenuCnt; iIdx++)
	{
		char caImgFileName[50] = {0,};
#if 1
		{
			int loop;
			int iLoc = 0;
			char *caItemName = tpMenuIdxVal[iIdx].caMenuItemName;
			for(loop = 0; loop < mtiStrlen(40, caItemName); loop++)
			{
				if(caItemName[loop] == ' ')
					caImgFileName[iLoc++] = '-';
				else
					caImgFileName[iLoc++] = caItemName[loop];
			}
		}
#endif
		dmsg("iIdx[%d], itemname[%s]", iIdx, tpMenuIdxVal[iIdx].caMenuItemName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<td>");
		if(iIdx == 0)
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' autofocus>", iIdx+1, tpMenuIdxVal[iIdx].value);
		}
		else
		{
			iOffset += sprintf(caHtmlDocBuff + iOffset,
					"<button name='menu_btn_%d' action='return %d' >", iIdx+1, tpMenuIdxVal[iIdx].value);
		}

		iOffset += sprintf(caHtmlDocBuff + iOffset,
				"<table cellpadding='0px' cellspacing='0px' border='0' style='height: 90px; width: 70px;'>\
				<tr><td align='center' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"<img src='images/%s.png'></td></tr>", caImgFileName);

		iOffset += sprintf(caHtmlDocBuff + iOffset,
						"<tr><td align='center' width='1px' valign='middle'>");

		iOffset += sprintf(caHtmlDocBuff + iOffset,"%s</td></tr></table></button></td>", tpMenuIdxVal[iIdx].caMenuItemName);


		if((iIdx+1) % 4 == 0 && iIdx != 0)
			iOffset += sprintf(caHtmlDocBuff + iOffset, "</tr><tr>");
	}

	iOffset += sprintf(caHtmlDocBuff + iOffset,"</tr>%s",caHtmlTail);

	//dmsg("htmlDoc[%s]", htmlDoc);

	string *strHtmlDoc = new string(caHtmlDocBuff);

	iRet = uiInvoke(*strHtmlDoc);
	//iRet = uiInvoke(0, htmlDoc, NULL, NULL);
	if(iRet == UI_ERR_SCRIPT)
	{
		dmsg("uiInvokeURL Script Error [%s]", uiScriptError().c_str());
	}
	else
	{
		dmsg("uiInvokeURL Result[%d]", iRet);
	}
	delete strHtmlDoc;
	return iRet;
}

INT InPutTest()
{
	char caHtmlDocBuff[1024*10] = {0,};
	int iOffset = 0;
	INT iRet = 0;

	startADKGUI();
	

	char *caHtmlHead =
				"<html><body>";

	iOffset += sprintf(caHtmlDocBuff, caHtmlHead);
	iOffset += sprintf(caHtmlDocBuff + iOffset, 
		"<form name='inputcode'><input type='number' name='code' size='10' precision='0'\></form>\
		 <table border='0' style='height:100%;width:100%'>\
		 <tr><td style='height:1px'>\
		 <table style='width:100%'>\
		 <tr><td style='width:33%'>\
		 <button accesskey='&#27;' style='width:100%;background-image:linear-gradient(to bottom, #ffd1d1 0%,#ff7f7f 50%,#e90000 51%,#ff0000 75%,#c80000 100%)' action='return -1'>CANCEL</button>\
		 </td><td style='width:34%'>\
		 </td><td  style='width:33%'>\
		 <button accesskey='&#13;' style='width:100%;background-image:linear-gradient(to bottom, #d1ffd1 0%,#7fff7f 50%,#00e900 51%,#00ff00 75%,#00c800 100%)' action='return 0'>OK</button>\
		 </td></tr></table>\
		 </td></tr></table>\
		 <button accesskey='&#138;' action='call focus.previous()'></button>\
		 <button accesskey='&#139;' action='call focus.next()'></button></body></html>");
		 
	string *strHtmlDoc = new string(caHtmlDocBuff);

	//map<string,string> value;
	iRet = uiInvoke(*strHtmlDoc);

	//dmsg("strHtmlDoc[%s]",strHtmlDoc);

	delete strHtmlDoc;

	stopADKGUI();

	return iRet;
}	

#if 1
INT iAdkGuiShowInputDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)

{

#if 0
	INT iRet = 0, i = 0, iLen = 0, iCnt = 0, iIndex = 0, iKey = 0, iRow = 0, iTid = 0, iAfterShow = 0;
	INT iSelIndex = 0, ii = 0, iInPos = 0, iMaxInput = 0, iLastIndex = 0;
	INT iIsUpdate = FALSE, iIsShown = FALSE;
	INT iLineIndex[2] = { 0 }, iLineOrg[2] = { 0 };
	INT iSkipFlag = FALSE, iDefaultFlag = FALSE, iFreeInput = FALSE, iCurrencyFlag = FALSE;
	INT iAlphaInput = FALSE;

	int iOffset = 0;

	INT iAddLine = 0;
	
	CHAR *cpCaption = NULL;
	CHAR *caLine = NULL;
	CHAR *caInput = NULL;
	CHAR *caOutput = NULL;

	CHAR cFmt = 0;
	INT iNumCnt = 0, iCurCnt = 0, iANSCnt = 0;

	INT iInputTimer = -1;
	INT iAlphaIdx = 0;
	INT nDiableTimout = 0;

	char caHtmlDocBuff[1024*10] = {0,};
	char caLineAgainTag[30] = {0,};


	caLine = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caLine, 0x00, DISP_MAX_COULUM + 1);

	caInput = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caInput, 0x00, DISP_MAX_COULUM + 1);

	caOutput = (CHAR *)mtiMalloc(DISP_MAX_COULUM + 1);
	mtiMemset(caOutput, 0x00, DISP_MAX_COULUM + 1);

	char *caHtmlHead =
				"<html><body>"
				"<div style='height:190px;'>"
				"<table cellpadding='0' cellspacing='0px' border='0' style='height:20%;width:100%;'>";

	char *caHtmlTail = "</body></html>";

	char *leftAlign = "text-align:left;";

	if (tpDispCont == NULL)
	{
		return RTN_ERROR;
	}

	sprintf(caLineAgainTag, leftAlign);

	iOffset += sprintf(caHtmlDocBuff, caHtmlHead);

	dmsg("iDispContCnt [%d]",iDispContCnt);

	for (i = 0; i < iDispContCnt; i++)
	{
		cpCaption = tpDispCont[i].caContents;
		iLen = SLEN(DISP_MAX_COULUM, cpCaption);
		if (iLen > 0)
		{
			switch(tpDispCont[i].iAlignment)
			{
				case DISP_ALIGN_CENTER:
					mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
					sprintf(caLineAgainTag, "text-align:center;");
					break;
				case DISP_ALIGN_RIGHT:
					mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
					sprintf(caLineAgainTag, "text-align:right;");
					break;
			}
			

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_TITLE))
			{
				dmsg("iDispContCnt DISP_OPT_TITLE[%s]",cpCaption);
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='background-color:blue;'><td>\
						<div style='width:320px;%sfont-size:20px'><b>%s</b></div></td></tr></table>"
						, caLineAgainTag, cpCaption);
			}
			else if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_SUBTITLE))
			{
				dmsg("iDispContCnt DISP_OPT_SUBTITLE[%s]",cpCaption);
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td><div style='%s font-size:18px'><b>%s</b></div></td></tr></table>"
						, caLineAgainTag, cpCaption);
			}
			

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
			{

				iLineIndex[0] = i + 1;
				iLineOrg[0] = i;
				if (tpDispCont[i].caDefValue[0] > 0)
				{
					dmsg("iDispContCnt DISP_OPT_INPUT[%s]",cpCaption);
					//write_at(tpDispCont[i].caDefValue, mtiStrlen(DISP_MAX_COULUM, tpDispCont[i].caDefValue),1 , i + iAddLine + 2);
					iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
						caLineAgainTag, tpDispCont[i].caDefValue);
				}
				iAddLine++;
					
				iCnt++;
			}

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NORMAL))
			{
				dmsg("iDispContCnt DISP_OPT_NORMAL[%s]",cpCaption);
				//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , i + iAddLine+ 1);
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
						caLineAgainTag, caLine);

				// Check Max  
				if (2 > i)
				{
					iRow = i + 1;
				}
			}

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_AFTER_CHECK))
			{
				iAfterShow = 1;
			}

			if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_NOTUSED_TIMEOUT))
			{
				nDiableTimout = 1;
			}
		}


		if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_ALPHA))
		{
			iAlphaInput = TRUE;
		}

	}


	if (iCnt > 1)
	{
		dmsg("iAdkGuiShowInputDisp iCnt [%d]",iCnt );
		
		for (i = 0; i < iDispContCnt; i++)
		{
			cpCaption = tpDispCont[i].caContents;
			iLen = SLEN(DISP_MAX_COULUM, cpCaption);
			if (iLen > 0)
			{
				switch(tpDispCont[i].iAlignment)
				{
					case DISP_ALIGN_CENTER:
						mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
						sprintf(caLineAgainTag, "text-align:center;");
						break;
					case DISP_ALIGN_RIGHT:
						mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
						sprintf(caLineAgainTag, "text-align:right;");
						break;
				}
				

				if (CHK_BIT(tpDispCont[i].iAttribute, DISP_OPT_INPUT))
				{
					if (iRow == 1)
					{
						if (i == 1)
						{
							//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 ,1 + 1 );
							iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
							caLineAgainTag, caLine);
							iLineOrg[0] = i;
							iLineIndex[0] = 2;
						}
						else if (i == 2)
						{
							//dmsg("caLine TIP[%s]",caLine);
							//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 3 + 1 );
							iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
							caLineAgainTag, caLine);
							iLineOrg[1] = i;
							iLineIndex[1] = 4;
						}
					}
					else if (iRow == 2)
					{
						if (i == 2)
						{
							//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 2 + 1 );
							iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
							caLineAgainTag, caLine);
							iLineOrg[0] = i;
							iLineIndex[0] = 3;
						}
						else if (i == 3)
						{
							//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , 4 + 1 );
							iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
							caLineAgainTag, caLine);
							iLineOrg[1] = i;
							iLineIndex[1] = 5;
						}
					}
				}
			}

		}
	}
	else
	{
		dmsg("caLine[%s]", caLine);
		//write_at(caLine, mtiStrlen(DISP_MAX_COULUM, caLine),1 , iLineOrg[0] + 1 );
		iOffset += sprintf(caHtmlDocBuff + iOffset, 
		"<table cellpadding='0' cellspacing='0' border='0' style='height: 20px; width: 100%;'> \
		<tr><td %s valign='middle'><font color='blue' size='10'>%s</font></td></tr></table>",
		caLineAgainTag, caLine);
	}

	
	iOffset += sprintf(caHtmlDocBuff + iOffset, "<form name='inputcode'><input type='number' name='code' size='10' precision='0'\
				></form>");
	

	if (tpAddCtrl)
	{
		if(mtiStrlen(100, tpAddCtrl->tBtnLeft.caContents)< 1)
		{
			sprintf(tpAddCtrl->tBtnLeft.caContents, "NO");
		}
		if(mtiStrlen(100, tpAddCtrl->tBtnRight.caContents)< 1)
		{
			sprintf(tpAddCtrl->tBtnRight.caContents, "YES");
		}
		dmsg("LEFT BTN[%s], RIGHT BTN[%s]", tpAddCtrl->tBtnLeft.caContents, tpAddCtrl->tBtnRight.caContents);
		//Cancel Key
		iOffset += sprintf(caHtmlDocBuff + iOffset, "</table></div><table cellpadding='0' cellspacing='0' \
				border='0' style='height: 30px; width: 320px;'><tr><td align='left'><button style='height: 30px;width: 100%; \
				background-color:red;' accesskey='&#27;' action='return %d'><b>%s</b></button></td>",
				RTN_SELECT_LEFT, tpAddCtrl->tBtnLeft.caContents);

		iOffset += sprintf(caHtmlDocBuff + iOffset, "<td align='right'><button style='height: 30px;width: 115%;background-color:green;' \
				accesskey='&#13;' action='return %d'><b>%s</b></button></td></tr></table>",
				RTN_SELECT_RIGHT, tpAddCtrl->tBtnRight.caContents);
		//Timer
		iOffset += sprintf(caHtmlDocBuff + iOffset, "<input type='timeout' value='%dms'\
				style='visibility: hidden' action='return -2'>", tpAddCtrl->ulTimeout);
	}
	else
	{
		iOffset += sprintf(caHtmlDocBuff + iOffset, "<input type='timeout' value='%dms'\
				style='visibility: hidden' action='return -2'>", 30000);
	}

	
	iOffset += sprintf(caHtmlDocBuff + iOffset, caHtmlTail);


	dmsg("caHtmlDocBuff [%s]",caHtmlDocBuff);

	string *strHtmlDoc = new string(caHtmlDocBuff);
	if (tpAddCtrl)
	{
		iRet = uiInvoke(*strHtmlDoc);
	}
	else
	{
		iRet = uiDisplay(*strHtmlDoc);
	}
	delete strHtmlDoc;

	return iRet;


	MZERO(caInput, DISP_MAX_COULUM + 1);
	iIndex = 0;
	iLen = 0;

	if (tpAddCtrl != NULL)
	{
		if (!iAfterShow)
		{
			iIsShown = TRUE;
			mtiProcAddCtrl(tpAddCtrl);
		}
		else
		{
			iIsShown = FALSE;
		}

		iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
	}
	else
	{
		iTid = mtiStartTimer(TIMEOUT_30S);
	}

	iSelIndex = -1;
	iIsUpdate = TRUE;
	while(TRUE)
	{
		if (nDiableTimout)
		{
		}
		else
		{
			if (mtiIsTimeout(iTid))
			{
				mtiStopTimer(iTid);
				if(iAlphaInput == TRUE && iInputTimer != -1)
					mtiStopTimer(iInputTimer);
				if(caLine)
					mtiFree(caLine);
				if(caInput)
					mtiFree(caInput);
				if(caOutput)
					mtiFree(caOutput);
				if(taFrmEle)
					mtiFree(taFrmEle);
				if(taCurEle)
					mtiFree(taCurEle);
				if(taNumEle)
					mtiFree(taNumEle);
				if(taAnsEle)
					mtiFree(taAnsEle);
				return RTN_TIMEOUT;
			}
		}

		iKey = mtiGetKeyPress();

		if (iKey != KEY_NONE)
		{
			mtiResetTimer(iTid);
		}

		if (iSelIndex != iIndex)
		{
			mtiMemset(taFrmEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
			mtiMemset(taCurEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
			mtiMemset(taNumEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));
			mtiMemset(taAnsEle, 0x00, (DISP_MAX_COULUM + 1) * sizeof(tFmtEle));

			iInPos = 0;
			iCurCnt = 0;
			iNumCnt = 0;
			iANSCnt = 0;
			iMaxInput = 0;

			if (tpDispCont[iLineOrg[iIndex]].caFormat[0] != 0x00)
			{
				for (ii = 0; ii < DISP_INPUT_MAX_COULUM; ii++)
				{
					cFmt = tpDispCont[iLineOrg[iIndex]].caFormat[ii];
					if (cFmt == '@')
					{
						taFrmEle[ii].cpVal = ' ';
						taFrmEle[ii].iIsInput = IN_TYPE_CURRENCY;
						taFrmEle[ii].iRelPos = ii;

						taCurEle[iCurCnt].iIsInput = IN_TYPE_CURRENCY;
						taCurEle[iCurCnt].iRelPos = ii;
						iCurCnt++;

						iCurrencyFlag = TRUE;
						iMaxInput++;
					}
						else if (cFmt == '#')
						{
							taFrmEle[ii].cpVal = ' ';
							taFrmEle[ii].iIsInput = IN_TYPE_NUM;
							taFrmEle[ii].iRelPos = ii;

							taNumEle[iNumCnt].iIsInput = IN_TYPE_NUM;
							taNumEle[iNumCnt].iRelPos = ii;
							iNumCnt++;

							iMaxInput++;
						}
						else if (cFmt == '*')
						{
							taFrmEle[ii].cpVal = '_';
							taFrmEle[ii].iIsInput = IN_TYPE_ANS;
							taFrmEle[ii].iRelPos = ii;

							taAnsEle[iANSCnt].iIsInput = IN_TYPE_ANS;
							taAnsEle[iANSCnt].iRelPos = ii;
							iANSCnt++;

							iMaxInput++;
						}
						else if (cFmt == ',' || cFmt == '-' || cFmt == '.' || cFmt == '/')
						{
							taFrmEle[ii].cpVal = cFmt;
							taFrmEle[ii].iRelPos = -1;
						}
						else
						{
							taFrmEle[ii].cpVal = cFmt;
							taFrmEle[ii].iRelPos = -2;
						}
					}
				}
				else
				{
					iFreeInput = TRUE;
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_SMALL_FONT))
					{
						iMaxInput = g_DISP_INPUT_MAX_COULUM;
					}
					else
					{
						iMaxInput = g_DISP_INPUT_MAX_COULUM;
					}
				}

				iSelIndex = iIndex;

				if (tpDispCont[iLineOrg[iIndex]].caDefValue[0] > 0)
				{
					iLen = mtiStrcpy(caInput, tpDispCont[iLineOrg[iIndex]].caDefValue, DISP_INPUT_MAX_COULUM);
					iDefaultFlag = TRUE;
				}
				else
				{
					iDefaultFlag = FALSE;
				}

				iIsUpdate = TRUE;
				//dmsg("iMaxInput = %d", iMaxInput);
			}

			if (tpAddCtrl != NULL && iIsShown)
			{
				INT iRtn = 0;
				//dmsg("iKey[%d], tpAddCtrl->tBtnRight.cRegKey[%d]", iKey, tpAddCtrl->tBtnRight.cRegKey);
				if (iKey == tpAddCtrl->tBtnLeft.cRegKey)
				{
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP();
					mtiStopTimer(iTid);
					iRtn = RTN_SELECT_LEFT;
				}
				else if (iKey == tpAddCtrl->tBtnRight.cRegKey)
				{
					//dmsg("iIndex [%d], iLineOrg[iIndex] [%d], caInput[%s] iLen[%d]",iIndex, iLineOrg[iIndex], caInput, iLen );
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					FINAL_DISP();
					mtiStopTimer(iTid);
					iRtn = RTN_SELECT_RIGHT;
				}
				else if (iKey == tpAddCtrl->ucCancelKey)
				{
					if (tpAddCtrl->iCancelKeyResultValue)
					{
						iRet = tpAddCtrl->iCancelKeyResultValue;
					}
					else
					{
						iRet = RTN_CANCEL;
					}
					break;
				}
				else if (iKey == tpAddCtrl->ucEnterKey)
				{
					if(iLen > 0)
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

					if (tpAddCtrl->iEnterKeyResultValue)
					{
						iRet = tpAddCtrl->iEnterKeyResultValue;
					}
					else
					{
						iRet = RTN_SUCCESS;
					}
					break;
				}
				if(iAlphaInput == TRUE && iInputTimer != -1)
					mtiStopTimer(iInputTimer);

				if(iRtn != 0)
				{
					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					if(caOutput)
						mtiFree(caOutput);
					if(taFrmEle)
						mtiFree(taFrmEle);
					if(taCurEle)
						mtiFree(taCurEle);
					if(taNumEle)
						mtiFree(taNumEle);
					if(taAnsEle)
						mtiFree(taAnsEle);
					return iRtn;
				}
			}
			if(iKey != KEY_NONE)
			{
				dmsg("iKey[%d]",iKey);
				dmsg("iIsShown[%d]",iIsShown);
			}

			if (iIsShown)
			{
				continue;
			}
			switch (iKey)
			{
				case KEY_ENTER:
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_CORRECT_MAX_LENGTH))
					{
						if (iLen != iMaxInput)
						{
							continue;
						}
					}

					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_CHECK_NO_INPUT))
					{
						dmsg("************* DISP_OPT_CHECK_NO_INPUT : [%d]", iLen);
					
						if (0 < iLen)
						{							
						}
						else 
						{
							// NO INPUT
							break;
						}
					}
					
					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_ENABLE_SKIP))
					{
						iSkipFlag = TRUE;
					}
					else
					{
						iSkipFlag = FALSE;
					}

					if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_AFTER_CHECK) && iAfterShow)
					{
						if (!iIsShown)
						{
							mtiProcAddCtrl(tpAddCtrl);

							if (iTid > 0)
							{
								mtiStopTimer(iTid);
							}
							iTid = mtiStartTimer(tpAddCtrl->ulTimeout);
							iIsShown = TRUE;
							break;
						}
					}
					if (0 < iLen)
					{
						MCPY(tpDispCont[iLineOrg[iIndex]].caInout, caInput, iLen);

						if (iCnt == (iIndex + 1))
						{
							FINAL_DISP();
							mtiStopTimer(iTid);

							if(iAlphaInput == TRUE && iInputTimer != -1)
								mtiStopTimer(iInputTimer);

							if(caLine)
								mtiFree(caLine);
							if(caInput)
								mtiFree(caInput);
							if(caOutput)
								mtiFree(caOutput);
							if(taFrmEle)
								mtiFree(taFrmEle);
							if(taCurEle)
								mtiFree(taCurEle);
							if(taNumEle)
								mtiFree(taNumEle);
							if(taAnsEle)
								mtiFree(taAnsEle);
							return RTN_SUCCESS;
						}
						else
						{
							MZERO(caInput, DISP_MAX_COULUM + 1);
							iLen = 0;
							iIndex++;
						}
					}
					else
					{
						if (iSkipFlag)
						{
							FINAL_DISP_KEY();
							mtiStopTimer(iTid);

							if(iAlphaInput == TRUE && iInputTimer != -1)
								mtiStopTimer(iInputTimer);

							if(caLine)
								mtiFree(caLine);
							if(caInput)
								mtiFree(caInput);
							if(caOutput)
								mtiFree(caOutput);
							if(taFrmEle)
								mtiFree(taFrmEle);
							if(taCurEle)
								mtiFree(taCurEle);
							if(taNumEle)
								mtiFree(taNumEle);
							if(taAnsEle)
								mtiFree(taAnsEle);
							return RTN_SKIP;
						}

						if (CHK_BIT(tpDispCont[iLineOrg[iIndex]].iAttribute, DISP_OPT_NO_CHECK_NO_INPUT))
						{
							MZERO(caInput, sizeof(caInput));
							iLen = 0;
							iIndex++;
							break;
						}

						if (tpDispCont[iLineOrg[iIndex]].caDefValue[0] > 0)
						{
							//SCPY(tpDispCont[iLineOrg[iIndex]].caInout, tpDispCont[iLineOrg[iIndex]].caDefValue, DISP_INPUT_MAX_COULUM);

							if (iCnt == (iIndex + 1))
							{
								FINAL_DISP_KEY();
								mtiStopTimer(iTid);

								if(iAlphaInput == TRUE && iInputTimer != -1)
									mtiStopTimer(iInputTimer);

								if(caLine)
									mtiFree(caLine);
								if(caInput)
									mtiFree(caInput);
								if(caOutput)
									mtiFree(caOutput);
								if(taFrmEle)
									mtiFree(taFrmEle);
								if(taCurEle)
									mtiFree(taCurEle);
								if(taNumEle)
									mtiFree(taNumEle);
								if(taAnsEle)
									mtiFree(taAnsEle);
								return RTN_SUCCESS;
							}
							else
							{
								MZERO(caInput, DISP_MAX_COULUM + 1);
								iLen = 0;
								iIndex++;
							}
						}
					}
					break;

				case KEY_CANCEL:
					FINAL_DISP();
					mtiStopTimer(iTid);

					if(iAlphaInput == TRUE && iInputTimer != -1)
						mtiStopTimer(iInputTimer);

					if(caLine)
						mtiFree(caLine);
					if(caInput)
						mtiFree(caInput);
					if(caOutput)
						mtiFree(caOutput);
					if(taFrmEle)
						mtiFree(taFrmEle);
					if(taCurEle)
						mtiFree(taCurEle);
					if(taNumEle)
						mtiFree(taNumEle);
					if(taAnsEle)
						mtiFree(taAnsEle);
					return RTN_CANCEL;

				case KEY_0:
				case KEY_1:
				case KEY_2:
				case KEY_3:
				case KEY_4:
				case KEY_5:
				case KEY_6:
				case KEY_7:
				case KEY_8:
				case KEY_9:
					if (iDefaultFlag)
					{
						mtiMemset(caInput, 0, DISP_MAX_COULUM + 1);
						iLen = 0;
						iDefaultFlag = FALSE;
					}

					if (iLen < iMaxInput)
					{
						if (iLen == 0 && iKey == KEY_0)
						{
							if (iCurrencyFlag != TRUE)
							{
								caInput[iLen++] = (CHAR)iKey;
								iIsUpdate = TRUE;
							}
						}
						else
						{
							caInput[iLen++] = (CHAR)iKey;
							iIsUpdate = TRUE;
						}
					}
					if(iAlphaInput == TRUE)
						iInputTimer = mtiStartTimer(TIMEOUT_1S);

					break;

				case KEY_DOT:
					if (iANSCnt > 0 || iFreeInput == TRUE)
					{
						caInput[iLen++] = '.';
						iIsUpdate = TRUE;
					}
					if(iAlphaInput == TRUE)
						iInputTimer = mtiStartTimer(TIMEOUT_1S);
					break;
				case KEY_FUNC:
					if (iANSCnt > 0 || iFreeInput == TRUE)
					{
						caInput[iLen++] = '*';
						iIsUpdate = TRUE;
					}
					if(iAlphaInput == TRUE)
						iInputTimer = mtiStartTimer(TIMEOUT_1S);
					break;
				case KEY_BACK:
					if (0 < iLen)
					{
						caInput[iLen--] = 0x00;
						dmsg("caInput[%s]",caInput);
					}
					iIsUpdate = TRUE;
					break;
			}

			while(1)
			{
				if (iIsUpdate)
				{
					mtiMemset(caOutput, 0, DISP_MAX_COULUM + 1);
					if (iFreeInput == FALSE)
					{
						iInPos = 0;

						for (i = iInPos; i < DISP_INPUT_MAX_COULUM; i++)
						{
							if (taFrmEle[i].iRelPos == -2)
							{
								caOutput[i] = taFrmEle[i].cpVal;
							}
							else
							{
								caOutput[i] = ' ';
							}
						}

						if (0 < iNumCnt)
						{
							iInPos = 0;
							iLastIndex = 0;
							for (i = 0; i < iNumCnt; i++)
							{
								if (iInPos < iLen)
								{
									caOutput[taNumEle[i].iRelPos] = caInput[iInPos++];
									iLastIndex = taNumEle[i].iRelPos;
								}
							}

							if (iLen > 0)
							{
								for (i = 0; i < iLastIndex ; i++)
								{
									if (taFrmEle[i].iRelPos == -1)
									{
										caOutput[i] = taFrmEle[i].cpVal;
									}
								}
							}
						}

						if (0 < iCurCnt)
						{
							iInPos = iLen;
							iLastIndex = 0;
							for (i = iCurCnt - 1; 0 <= i; i--)
							{
								if (0 < iInPos)
								{
									caOutput[taCurEle[i].iRelPos] = caInput[--iInPos];
									iLastIndex = taCurEle[i].iRelPos;
								}
							}

							if (iLen > 0)
							{
								for (i = DISP_INPUT_MAX_COULUM; iLastIndex < i; i--)
								{
									if (taFrmEle[i].iRelPos == -1)
									{
										caOutput[i] = taFrmEle[i].cpVal;
									}
								}
							}
							else
							{
								caOutput[taCurEle[iCurCnt - 1].iRelPos] = '0';
							}
						}
					}
					else
					{
						mtiMemset(caOutput, 0, DISP_INPUT_MAX_COULUM);
						mtiMemset(caOutput, 0x20, DISP_INPUT_MAX_COULUM);
						mtiMemcpy(caOutput, caInput, iLen);
					}
					dmsg("caOutput[%s]",caOutput);
					write_at(caOutput, mtiStrlen(DISP_MAX_COULUM, caOutput),1 , iLineIndex[iIndex] + 1 );
					iIsUpdate = FALSE;

					//ALPHA INPUT
					if(iAlphaInput == TRUE && ((iKey >= KEY_0 && iKey <= KEY_9)
							|| iKey == KEY_FUNC || iKey == KEY_DOT ))
					{
						INT iAfterKey = KEY_NONE;

						while(1)
						{
							iAfterKey = mtiGetKeyPress();

							if(mtiIsTimeout(iInputTimer) == TRUE)
							{
								dmsg("Next Key...");
								mtiStopTimer(iInputTimer);
								iAlphaIdx = 0;
								break;
							}

							if(iAfterKey != KEY_NONE)
								break;
						}

						if(iKey == iAfterKey)
						{
							INT iIdx = 0;
							dmsg("ALPHA INPUT key[%c]", iAfterKey);
							for(iIdx = 0; iIdx < 12; iIdx++)
							{
								if(tAlphaKeyMapList[iIdx].iKeyVal == iAfterKey)
								{
									if(iAlphaIdx >= mtiStrlen(DISP_MAX_COULUM, tAlphaKeyMapList[iIdx].caAlphaVal))
									{
										iAlphaIdx = 0;
										caInput[iLen - 1] = iAfterKey;
									}
									else
									{
										//dmsg("iLen[%d], iAlphaIdx[%d]", iLen, iAlphaIdx);
										caInput[iLen - 1] = tAlphaKeyMapList[iIdx].caAlphaVal[iAlphaIdx];
										iAlphaIdx++;
									}
								}
							}
							mtiResetTimer(iInputTimer);
							iIsUpdate = TRUE;
							continue;
						}


					}
				}
				break;
			}
			if(iAlphaInput == TRUE && iInputTimer != -1)
				mtiStopTimer(iInputTimer);
		}

	FINAL_DISP();
	mtiStopTimer(iTid);

	if(iAlphaInput == TRUE && iInputTimer != -1)
		mtiStopTimer(iInputTimer);

	if(caLine)
		mtiFree(caLine);
	if(caInput)
		mtiFree(caInput);
	if(caOutput)
		mtiFree(caOutput);
	if(taFrmEle)
		mtiFree(taFrmEle);
	if(taCurEle)
		mtiFree(taCurEle);
	if(taNumEle)
		mtiFree(taNumEle);
	if(taAnsEle)
		mtiFree(taAnsEle);

	return iRet;
#endif
}
#endif

INT iAdkGuiShowDialogDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	int iIdx;
	int iLen;

	char *cpCaption = NULL;

	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};
	char caLineAgainTag[30] = {0,};

	int isTitle = 0;

	dmsg("@@@@@@@@@@@@@@@@Enter iAdkGuiShowDialogDisp");

	char *caHtmlHead =
				"<html><body>"
				"<div style='height:190px; overflow:hidden;'>"
				"<table cellpadding='0' cellspacing='0px' border='0' style='height:100%;width:100%;'>";

	char *caHtmlTail = "</body></html>";

	char *leftAlign = "text-align:left;";

	if (tpDispCont == NULL)
	{
		return RTN_ERROR;
	}

	sprintf(caLineAgainTag, leftAlign);

	iOffset += sprintf(caHtmlDocBuff, caHtmlHead);

	for (iIdx = 0; iIdx < iDispContCnt; iIdx++)
	{
		cpCaption = tpDispCont[iIdx].caContents;
		iLen = SLEN(100, cpCaption);
		if (iLen > 0)
		{
			switch(tpDispCont[iIdx].iAlignment)
			{
				case DISP_ALIGN_CENTER:
					mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
					sprintf(caLineAgainTag, "text-align:center;");
					break;
				case DISP_ALIGN_RIGHT:
					mtiMemset(caLineAgainTag, 0x00, sizeof(caLineAgainTag));
					sprintf(caLineAgainTag, "text-align:right;");
					break;
			}

			//TITLE
			if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_TITLE))
			{
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='background-color:yellow;'><td>\
						<div style='width:320px;%sfont-size:22px'><b>%s</b></div></td></tr>"
						, caLineAgainTag, cpCaption);
			}

			//SUB TITLE
			else if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_SUBTITLE))
			{
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td><div style='%s font-size:18px'><b>%s</b></div></td></tr>"
						, caLineAgainTag, cpCaption);
			}
/*
			//Boundary
			iOffset += sprintf(caHtmlDocBuff, "<tr><td><br></td></tr>");
*/
			//NORMAL CONTENTS
			else
			{
				iOffset += sprintf(caHtmlDocBuff + iOffset, "<tr style='width:320px;'><td style='vertical-align:top'><div style='width:320px; %s font-size:16px'>%s</div></td></tr>",
						caLineAgainTag, cpCaption);
			}
		}
	}

	if (tpAddCtrl)
	{
		if(mtiStrlen(100, tpAddCtrl->tBtnLeft.caContents)< 1)
		{
			sprintf(tpAddCtrl->tBtnLeft.caContents, "CANCEL");
		}
		if(mtiStrlen(100, tpAddCtrl->tBtnRight.caContents)< 1)
		{
			sprintf(tpAddCtrl->tBtnRight.caContents, "ENTER");
		}
		dmsg("LEFT BTN[%s], RIGHT BTN[%s]", tpAddCtrl->tBtnLeft.caContents, tpAddCtrl->tBtnRight.caContents);
		//Cancel Key
		iOffset += sprintf(caHtmlDocBuff + iOffset, "</table></div><div style='height:30px; overflow:hidden;'><table cellpadding='0' cellspacing='0' \
				border='0' style='height: 30px; width: 320px;'><tr><td align='left'><button style='height: 30px;width: 100%; \
				background-color:red;' accesskey='&#27;' action='return %d'><b>%s</b></button></td>",
				RTN_SELECT_LEFT, tpAddCtrl->tBtnLeft.caContents);

		iOffset += sprintf(caHtmlDocBuff + iOffset, "<td align='right'><button style='height: 30px;width: 115%;background-color:green;' \
				accesskey='&#13;' action='return %d'><b>%s</b></button></td></tr></table>",
				RTN_SELECT_RIGHT, tpAddCtrl->tBtnRight.caContents);
		//Timer
		iOffset += sprintf(caHtmlDocBuff + iOffset, "<input type='timeout' value='%dms'\
				style='visibility: hidden' action='return -2'>", tpAddCtrl->ulTimeout);
	}
	iOffset += sprintf(caHtmlDocBuff + iOffset, caHtmlTail);

	string *strHtmlDoc = new string(caHtmlDocBuff);
	if (tpAddCtrl)
	{
		iRet = uiInvoke(*strHtmlDoc);
	}
	else
	{
		iRet = uiDisplay(*strHtmlDoc);
	}
	delete strHtmlDoc;

	return iRet;
}

INT iAdkGuiShowMenuDisp(tDispContents *tpDispCont, INT iDispContCnt, INT iDefaultSelect, tAddCtrlContents *tpAddCtrl)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	int iIdx;
	int iLen;

	char *cpCaption = NULL;

	char caTimeout[10] = {0,};

	int isTitle = 0;

	dmsg("@@@@@@@@@@@@@@@@Enter iAdkGuiShowMenuDisp");

	struct UIMenuEntry *tpMenuList;
	int menuAddingCnt = 0;
	string strTitle;

	if (tpDispCont == NULL)
	{
		return RTN_ERROR;
	}

	tpMenuList = new UIMenuEntry[iDispContCnt];

	for (iIdx = 0; iIdx < iDispContCnt; iIdx++)
	{
		cpCaption = tpDispCont[iIdx].caContents;
		iLen = SLEN(100, cpCaption);
		if (iLen > 0)
		{

			//TITLE
			if (CHK_BIT(tpDispCont[iIdx].iAttribute, DISP_OPT_TITLE))
			{
				strTitle = string(cpCaption);
			}

			else	//Building Menu List
			{
				tpMenuList[menuAddingCnt].text = string(cpCaption);
				tpMenuList[menuAddingCnt++].value = menuAddingCnt;
			}
		}
	}

	//Menu Listing
	dmsg("iDispContCnt [%] dmenuAddingCnt [%d]", iDispContCnt, menuAddingCnt);
	iRet =uiMenu("newmenu", strTitle, tpMenuList, iDispContCnt, iDefaultSelect);

	if(iRet==UI_ERR_CONNECTION_LOST)
	{
		dmsg("UI Connection is Lost");
		iRet = RTN_ERROR;
	}

	dmsg("uiMenu iRet[%d]", iRet);

	delete tpMenuList;

	return iRet;

}

INT iAdkGuiGetKeyPress(INT iTimeOutByMs)
{
	INT iKeyVal = 0, iOffset = 0;
	CHAR caHTML[3*1024] = {0,};

	uiSetPropertyInt(UI_PROP_KEEP_DISPLAY, 1);
	iOffset += sprintf(caHTML + iOffset, "<html><body>");

	iOffset += sprintf(caHTML + iOffset,
			"<button style='visibility: hidden' accesskey='&#13;' action='return 141'></button>");
	iOffset += sprintf(caHTML + iOffset,
			"<button style='visibility: hidden' accesskey='&#27;' action='return 155'></button>");
	iOffset += sprintf(caHTML + iOffset,
			"<button style='visibility: hidden' accesskey='&#8;' action='return 136'></button>");
	//Timer
	iOffset += sprintf(caHTML + iOffset, "<input type='timeout' value='%dms'\
			style='visibility: hidden' action='return -2'></body></html>", iTimeOutByMs);

	string *strHtmlDoc = new string(caHTML);

	iKeyVal = uiDisplay(*strHtmlDoc);
	dmsg("iKeyVal[%d]", iKeyVal);

	delete strHtmlDoc;
	uiSetPropertyInt(UI_PROP_KEEP_DISPLAY, 0);

	return iKeyVal;
}

typedef struct _tAsciiToHtmlCode
{
	char cAsciiLetter;
	char *caHtmlCode;
}tAsciiToHtmlCode;

tAsciiToHtmlCode tAsciiToHtmlCodeListQR2[] =
{
		{'<',	"%3C"},
		{'>',	"%3E"},
		{' ',   "%20"},
		{0x00,	 NULL},
};

INT ConverHtml(CHAR *cpContents, CHAR *dest)
{
	CHAR caData[1024];
	CHAR *cpData = caData;
	INT iOffset = 0;
	INT iIdx = 0;
	INT iDestIdx = 0;
	INT iFoundFlag = 0;

	mtiMemset(caData, 0, sizeof(caData));

	for(iIdx = 0; iIdx < 512; iIdx++)
	{
		iOffset = 0;
		iFoundFlag = 0;
		while(tAsciiToHtmlCodeListQR2[iOffset].cAsciiLetter != 0x00)
		{
			if(cpContents[iIdx] == tAsciiToHtmlCodeListQR2[iOffset].cAsciiLetter)
			{
				iFoundFlag = 1;
				iDestIdx += mtiStrcpy(cpData + iDestIdx, tAsciiToHtmlCodeListQR2[iOffset].caHtmlCode, 10);
			}
			iOffset++;
		}
		if(iFoundFlag == 0)
		{
			cpData[iDestIdx] = cpContents[iIdx];
			iDestIdx++;
		}
	}

	mtiMemcpy(dest, cpData, 1024);
	return RTN_SUCCESS;
}


int dispQRC(CHAR *cpQrCode, INT iTimeOutByMs)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[2048];
	CHAR caData[2048];

	char *caHtmlHead = "<html><body>";

	// @@EB QR 20190314
#if 1
	char *caHtmlTail =  "<input type='idletimeout' value='10' style='visibility: hidden' action='return -2'>"
							"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
							"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
							"<button style='visibility: hidden' accesskey='&#13;' action='return -1'></button>"	//Enter
							"</body></html>";
#else
	char *caHtmlTail =  "<input type='idletimeout' value='90' style='visibility: hidden' action='return -2'>"
						"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
						"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
						"<button style='visibility: hidden' accesskey='&#13;' action='return -1'></button>"	//Enter
						"</body></html>";
#endif
	//

	startADKGUI();

	mtiMemset(caData, 0, sizeof(caData));
	ConverHtml(cpQrCode, caData);

	memset(caHtmlDocBuff, 0x00, sizeof(caHtmlDocBuff));

	iOffset += sprintf(caHtmlDocBuff,"%s", caHtmlHead);

	iOffset += sprintf(caHtmlDocBuff + iOffset,
				"<center><table border='0' style='height:100%;width:100%;background-color:yellow'><tr><td style='height:100%;vertical-align:middle;align:center'>");

	if (mtiGetHwType() == C680)
	{
		iOffset += sprintf(caHtmlDocBuff + iOffset,"<?barcode qr-h %s 320 320?>", caData);
	}
	else
	{
		iOffset += sprintf(caHtmlDocBuff + iOffset,"<?barcode qr-h %s 200 200?>", caData);
	}

	iOffset += sprintf(caHtmlDocBuff + iOffset,"</td></tr></table></center>");

	iOffset += sprintf(caHtmlDocBuff + iOffset,"%s",caHtmlTail);

	iOffset += sprintf(caHtmlDocBuff + iOffset,"<input type='timeout' value='%dms' style='visibility: hidden' action='return -2'></body></html>", iTimeOutByMs);

	dmsg("====dispQRC caHtmlDocBuff[%s]", caHtmlDocBuff);

	string *strHtmlDoc = new string(caHtmlDocBuff);

	iRet = uiInvoke(*strHtmlDoc);
	if(iRet == UI_ERR_SCRIPT)
	{
		dmsg("uiInvokeURL Script Error [%s]", uiScriptError().c_str());
	}
	else
	{
		switch (iRet)
		{
			case UI_ERR_ABORT :
				dmsg("====uiDisplay Result[abort by user %d]", iRet);
				break;
			case UI_ERR_BACK :
				dmsg("====uiDisplay Result[user selected back button %d]", iRet);
				break;
			case UI_ERR_TIMEOUT :
				dmsg("====uiDisplay Result[dialog timeout  %d]", iRet);
				break;
			case UI_ERR_PROPERTY :
				dmsg("====uiDisplay Result[the property does not exist   %d]", iRet);
				break;
			case UI_ERR_WRONG_PIN :
				dmsg("====uiDisplay Result[wrong PIN was entered   %d]", iRet);
				break;
			case UI_ERR_PARAMETER :
				dmsg("====uiDisplay Result[parameter error  %d]", iRet);
				break;
			case UI_ERR_PERMISSION :
				dmsg("====uiDisplay Result[insufficient permissions   %d]", iRet);
				break;
			case UI_ERR_CANCELLED :
				dmsg("====uiDisplay Result[the dialog has been cancelled by callback or by displaying another dialog %d]", iRet);
				break;
			default :
				dmsg("====uiDisplay Result[%d]", iRet);
				break;
		}

	}

	delete strHtmlDoc;

	stopADKGUI();
	return iRet;
}
