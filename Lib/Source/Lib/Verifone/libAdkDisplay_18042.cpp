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
			if(mtiGetHwType() == VX675)
				strCommType = "3G";
			else
				strCommType = "GPRS";
			if(mtiStrlen(M_SIZEOF(networkStat, caWifiSSID), pNetworkStat->caWifiSSID) > 0)
			{
				//dmsg("GPRS PROVIDER[%s]", pNetworkStat->caWifiSSID);
				strCommType.append("-");
				strCommType.append(string(pNetworkStat->caWifiSSID), 0, 10);
			}
			break;
		case CMM_TYPE_WIFI:
			strCommType = "WIFI";
			if(mtiStrlen(M_SIZEOF(networkStat, caWifiSSID), pNetworkStat->caWifiSSID) > 0)
			{
				//dmsg("WIFI SSID[%s]", pNetworkStat->caWifiSSID);
				strCommType.append("-");
				strCommType.append(string(pNetworkStat->caWifiSSID, 0, 10));
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

void dispGuiHomeScreen(INT *iMainTaskID)
{
	int iRet = 0;
	char caVer[12 + 1] = {0,};
	char caVerPost[12 + 1] = {0,};

	uiSetPropertyInt(UI_PROP_TIMEOUT, -1);

	mtiGetTerminalVersion(caVer);
	mtiStrcpy(caVerPost, caVer, 9);

	map<string,string> value;
	if (iTransRestrictionCheckSettle() == RTN_SUCCESS)
	{
		value["dispmsg1"] = string("SETTLEMENT FIRST  -  ") + string(caVerPost);
	}
	else
	{
		value["dispmsg1"] = string(caVerPost);
	}

	iRet = uiInvokeURLAsync(value, "homescr.html", cbGuiHomeScr, iMainTaskID);

	dmsg("uiInvokeURLAsync iRet[%d] mainTaskId[%d]",iRet, *iMainTaskID);
	if(iRet == UI_ERR_SCRIPT)
	{
		string errStr = "script Err[" + uiScriptError() + "]";
		dmsg("%s",errStr.c_str());
	}

	return;
}

int dispMainMenu(tMenuIdxVal *tpMenuIdxVal, int iMenuCnt)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};


	//string menuList[7] = {"SALE","VOID","REFUND","TRANSFER","TIP-ADJUST","TARIK-TUNAI","SETTLE"};
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

int dispQRC(CHAR *cpQrCode, INT iTimeOutByMs)
{
	int iRet = UI_ERR_OK;
	int iOffset = 0;
	char caTimeout[10] = {0,};
	char caHtmlDocBuff[1024*10] = {0,};

	char *caHtmlHead = "<html><body>";

	char *caHtmlTail =  "<input type='idletimeout' value='90' style='visibility: hidden' action='return -2'>"
						"<button style='visibility: hidden' accesskey='&#27;' action='return -1'></button>"	//Cancel Key
						"<button style='visibility: hidden' accesskey='&#8;' action='return -3'></button>"	//Clear Key
						"<button style='visibility: hidden' accesskey='&#13;' action='return -1'></button>"	//Enter
						"</body></html>";

	startADKGUI();

	iOffset += sprintf(caHtmlDocBuff,"%s", caHtmlHead);

	iOffset += sprintf(caHtmlDocBuff + iOffset,
				"<center><table border='0' style='height:100%;width:100%;background-color:yellow'><tr><td style='height:100%;vertical-align:middle;align:center'>");

	iOffset += sprintf(caHtmlDocBuff + iOffset,"<?barcode qr-h %s 200 200?>", cpQrCode);

	iOffset += sprintf(caHtmlDocBuff + iOffset,"</td></tr></table></center>");

	iOffset += sprintf(caHtmlDocBuff + iOffset,"%s",caHtmlTail);

	iOffset += sprintf(caHtmlDocBuff + iOffset, "<input type='timeout' value='%dms'style='visibility: hidden' action='return -2'></body></html>", iTimeOutByMs);

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
