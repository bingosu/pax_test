#include <stdio.h>
#include <com/libcom.h>
#include <svc.h>
#include "SVC_NET.H"
#include <ceif.h>

//#include "libMtiCommonApi.h"
#include "apMtiCommonApp.h"

#define MAX_COMM_COUNT		8

#define MTI_DATA_PATH	"F:15/MTI_DATA"

#define SERIAL_PROFILE		"raw_serial"
#define MODEM_PROFILE		"raw_dial"
#define	ETH_PROFILE			"eth"
#define GPRS_PROFILE		"gprs"
#define WIFI_PROFILE		"wifi"

#define CONN_PROFILE		"conn"

#define CONNECTED			1
#define CONNECTING			0
#define NOT_CONNECTED		-1

////////////////////////////////////////////////////////
// async connect

// conn type (PRE-CONN, NORMAL CONN)
#define TYPE_CONN_PRE				0
#define TYPE_CONN_NOR				1

#define MSG_CONN_ASNC_MSG			1000

#define USER_EVT_FROM_CONNECTION_CB		(1L<<25)

typedef struct
{
	INT iConnType;
	INT iTaskID;
	INT iMsg;
	INT *piStatus;
} tCommAsyncInfo;

extern networkStat vNetworkStat;

enum com_ErrorCodes connRetStat = COM_ERR_NONE;
enum com_ErrorCodes netRetStat = COM_ERR_NONE;

sem_t sem_ConnRetStat;
sem_t sem_ConnAsyncInfo;

static void setConnRet(enum com_ErrorCodes connRet)
{
	sem_wait(&sem_ConnRetStat);
	connRetStat = connRet;
	sem_post(&sem_ConnRetStat);
}

static VOID xmlFileDbg(CHAR *cpFileName);
static CHAR *getProfileName(INT iCommType);

static void connection_callback (enum com_ConnectionEvent event, enum com_ConnectionType type, const void *data, void *priv,
		enum com_ErrorCodes com_errno);

static tCommInfo g_tCommInfo[MAX_COMM_COUNT];
static struct com_ConnectHandle *g_lhndTable[MAX_COMM_COUNT];

static INT g_ucInitCommVar = FALSE;

static INT curNetworkType = 0;

CHAR *caBaudrateList[] = { "1200", "2400", "9600", "19200", "38400", "57600", "115200" };
CHAR *caModemModList[] = { "v22", "v22bis", "v32", "v34", "auto", "v90", "auto"};

INT iNetworkConnected = -1;

CHAR GPRSIccId[20+1]; // @@WAY CHANGE PART, 20191001

INT MtiGetEDCModelName()
{

	CHAR caModelNum[12+1] = {0,};
	INT iRet;

	iRet = SVC_INFO_MODELNO(caModelNum);
	if (iRet < 0) return EDC_MODEL_UNKNOWN;

	if(mtiMemcmp(caModelNum + 2, (CHAR *)"52", 2) == 0)
	{
		if(mtiMemcmp(caModelNum + 2, (CHAR *)"520C", 4) == 0) return EDC_MODEL_VX520C;
		else return EDC_MODEL_VX520;
	}
	else if(mtiMemcmp(caModelNum + 2, (CHAR *)"67", 2) == 0) return EDC_MODEL_VX675;
	else if(mtiMemcmp(caModelNum + 1, (CHAR *)"680", 3) == 0) return EDC_MODEL_C680;
	else return EDC_MODEL_UNKNOWN;


}


static updateMobileNetworkProvider(INT iNetType)
{
	switch(iNetType)
	{
		case COMMTYPE_GPRS:
			{
				INT iRssiPersent = 0;
				com_GetDevicePropertyInt(COM_PROP_GSM_SIGNAL_PERCENTAGE, &iRssiPersent, NULL);
				vNetworkStat.gprsRssiPersent = iRssiPersent;
				mtiMemset(vNetworkStat.caWifiSSID, 0x00, M_SIZEOF(networkStat, caWifiSSID));
				com_GetDevicePropertyString(COM_PROP_GSM_PROVIDER_NAME, vNetworkStat.caWifiSSID, M_SIZEOF(networkStat, caWifiSSID), NULL );

				//@@WAY BATTERY C680
				dmsg("NETWORK NAME [%s]", vNetworkStat.caWifiSSID);
				if(memcmp(vNetworkStat.caWifiSSID,"510 11", 6)==0)
				{
					mtiMemset(vNetworkStat.caWifiSSID, 0x00, M_SIZEOF(networkStat, caWifiSSID));
					strcpy(vNetworkStat.caWifiSSID, "IND XL");
				}
				//

			}
			break;
		case COMMTYPE_WIFI:
			{
				INT iRssiPersent = 0;

				com_GetDevicePropertyInt(COM_PROP_WLAN_SIGNAL_PERCENTAGE, &iRssiPersent, NULL);
				vNetworkStat.gprsRssiPersent = iRssiPersent;
				mtiMemset(vNetworkStat.caWifiSSID, 0x00, M_SIZEOF(networkStat, caWifiSSID));
				com_GetDevicePropertyString(COM_PROP_WLAN_SSID, vNetworkStat.caWifiSSID, M_SIZEOF(networkStat, caWifiSSID), NULL );
			}
			break;
	}
}

static VOID clearEvent()
{
	LONG lEvent = 0L;

	lEvent = read_event();

	if(lEvent != 0L)
		dmsg("EVENT [%x]", lEvent);
}

static INT mtiDetachNetwork(INT iCommType)
{
	enum com_ErrorCodes errCode;
	CHAR caFilePath[100] = {0,};
	INT iDetRet = 0;

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, getProfileName(iCommType));
	iDetRet = com_DetachNetwork(caFilePath, &errCode);
	dmsg("com_DetachNetwork iDetRet[%d], ErrCode[%d]",iDetRet, errCode);

	if(iDetRet < 0)
		return RTN_ERROR;
	else
		return RTN_SUCCESS;
}

BOOL mtiGetNetAttachStatus(INT iNetType, tNetInfo *tpNetInfo)
{
	enum com_ErrorCodes errCode = COM_ERR_NONE;
	CHAR caFilePath[100] = {0,};
	BOOL bRet = FALSE;
	INT CommNetStatus = 0;

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, getProfileName(iNetType));
	dmsg("CONN REQUEST NETWORK PROFILE PATH [%s]", caFilePath);
	mtiFileDbg(caFilePath);

	if(COM_NETWORK_STATUS_START == com_GetNetworkStatus(caFilePath, &errCode))
	{
		INT iEthLink = COMM_NET_STAT_ETH_UNLINK;

		switch(iNetType)
		{
			case COMMTYPE_ETHERNET:
				com_GetDevicePropertyInt(COM_PROP_ETH_0_LINK_STATUS, &iEthLink , &errCode);
				vNetworkStat.ethLink = (BOOL) iEthLink;
				break;
			case COMMTYPE_GPRS:
				{
					CHAR caBuff[100] = {0,};
					enum com_ErrorCodes comErr = COM_ERR_NONE;

					tpNetInfo->iMNC = tpNetInfo->iCellID = tpNetInfo->iLAC = 0;

					//MNC
					com_GetDevicePropertyString(COM_PROP_GSM_MNC, caBuff, sizeof(caBuff), &comErr);
					if(comErr == COM_ERR_NONE)
					{
						tpNetInfo->iMNC = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
					}
					mtiMemset(caBuff, 0x00, sizeof(caBuff));

					//CELL ID
					com_GetDevicePropertyString(COM_PROP_GSM_CELL_ID, caBuff, sizeof(caBuff), &comErr);
					if(comErr == COM_ERR_NONE)
					{
						tpNetInfo->iCellID = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
						dmsg("CELL ID[%s]", caBuff);
					}
					mtiMemset(caBuff, 0x00, sizeof(caBuff));

					//LAC
					com_GetDevicePropertyString(COM_PROP_GSM_LAC, caBuff, sizeof(caBuff), &comErr);
					if(comErr == COM_ERR_NONE)
					{
						tpNetInfo->iLAC = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
					}
					mtiMemset(caBuff, 0x00, sizeof(caBuff));

					dmsg("MNC[%d], CELLID[%d], LAC[%d]", tpNetInfo->iMNC, tpNetInfo->iCellID, tpNetInfo->iLAC);

					//ICC ID
					// @@WAY CHANGE PART, 20191001
					memset(GPRSIccId, 0, sizeof(GPRSIccId));
					com_GetDevicePropertyString(COM_PROP_GSM_SIM_ID, caBuff, sizeof(caBuff), &comErr);
					if(comErr == COM_ERR_NONE)
					{
						memcpy(GPRSIccId, caBuff, mtiStrlen(sizeof(caBuff), caBuff));
					}
					mtiMemset(caBuff, 0x00, sizeof(caBuff));
					//
				}
				updateMobileNetworkProvider(iNetType);
				break;
			case COMMTYPE_WIFI:
				updateMobileNetworkProvider(iNetType);
				break;
		}

		bRet = TRUE;
	}

	dmsg("com_GetNetworkStatus bRet[0x%.2x] errCode[%d]", bRet, errCode);
	return bRet;
}

INT mtiGetNetInfo(INT iNetType, tNetInfo *tpNetInfo)
{
	INT iRet = 0;
	enum com_Interfaces netIF = COM_INTERFACE_NONE;
	struct com_NetworkInterfaceInfo *NetIFInfo = NULL;
	enum com_ErrorCodes errCode;

	vNetworkStat.iCommType = iNetType;

	switch(iNetType)
	{
		case COMMTYPE_GPRS:
			{
				netIF = COM_INTERFACE_GPRS0;
				updateMobileNetworkProvider(iNetType);
			}
			break;
		case COMMTYPE_WIFI:
			{
				netIF = COM_INTERFACE_WLAN0;
				updateMobileNetworkProvider(iNetType);
			}
			break;
		case COMMTYPE_ETHERNET:
			netIF = COM_INTERFACE_ETH0;
			break;
		default:
			dmsg("iNetType [%d] is not under Network!");
			return RTN_ERROR;
	}

	iRet = com_GetNetworkInterfaceInfo(netIF, &NetIFInfo, &errCode);
	dmsg("NetIFInfo [%p] errCode[%d]", NetIFInfo, errCode);

	if(iRet < 0)
	{
		dmsg("com_GetNetworkInterfaceInfo ERROR!");
		return RTN_ERROR;
	}

	if(NetIFInfo != NULL || errCode == COM_ERR_NONE)
	{
		CHAR *netStatMsg[3] = {"ERROR", "START", "STOP"};

/*
		char caAddr[IPV4_ADDR_LENGTH];
		char caNetmask[IPV4_ADDR_LENGTH];
		char caGateway[IPV4_ADDR_LENGTH];
		int iDHCP_Enabled;
		int iNetStatus;
*/
		mtiMemset(tpNetInfo, 0x00, sizeof(tNetInfo));

		tpNetInfo->iNetStatus = NetIFInfo->status;
		tpNetInfo->iDHCP_Enabled = NetIFInfo->IPV4_DHCP_Enabled;
		mtiStrcpy(tpNetInfo->caAddr, NetIFInfo->IPV4_Addr, IPV4_ADDR_LENGTH);
		mtiStrcpy(tpNetInfo->caNetmask, NetIFInfo->IPV4_Netmask, IPV4_ADDR_LENGTH);
		mtiStrcpy(tpNetInfo->caGateway, NetIFInfo->IPV4_Gateway, IPV4_ADDR_LENGTH);

		if(iNetType == COMMTYPE_ETHERNET)
		{
			com_GetDevicePropertyInt(COM_PROP_ETH_0_LINK_STATUS, &tpNetInfo->iEthLink, &errCode);
			vNetworkStat.ethLink = tpNetInfo->iEthLink;
			dmsg("ETH LINK [%d] errCode[%d]", tpNetInfo->iEthLink, errCode);
		}

		dmsg("--------------------------------------------------");
		dmsg("NETWORK STATUS -> [%s]", netStatMsg[NetIFInfo->status]);
		if(iNetType == COMMTYPE_ETHERNET)
			dmsg("ETH LINK [%d]", tpNetInfo->iEthLink);
		dmsg("DHCP ENABLED[%d]", NetIFInfo->IPV4_DHCP_Enabled);
		dmsg("IP ADDR[%s]", NetIFInfo->IPV4_Addr);
		dmsg("NETMASK[%s]", NetIFInfo->IPV4_Netmask);
		dmsg("GATEWAY[%s]", NetIFInfo->IPV4_Gateway);
		dmsg("RX PACKET[%d]", NetIFInfo->RX_Packets);
		dmsg("TX PACKET[%d]", NetIFInfo->TX_Packets);
		dmsg("--------------------------------------------------");

		com_NetworkInterfaceInfo_Free(NetIFInfo);

		//3G or GPRS Location Info
		if(iNetType == COMMTYPE_GPRS)
		{
			CHAR caBuff[100] = {0,};
			enum com_ErrorCodes comErr = COM_ERR_NONE;

			tpNetInfo->iMNC = tpNetInfo->iCellID = tpNetInfo->iLAC = 0;

			//MNC
			com_GetDevicePropertyString(COM_PROP_GSM_MNC, caBuff, sizeof(caBuff), &comErr);
			if(comErr == COM_ERR_NONE)
			{
				tpNetInfo->iMNC = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
			}
			mtiMemset(caBuff, 0x00, sizeof(caBuff));

			//CELL ID
			com_GetDevicePropertyString(COM_PROP_GSM_CELL_ID, caBuff, sizeof(caBuff), &comErr);
			if(comErr == COM_ERR_NONE)
			{
				tpNetInfo->iCellID = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
				dmsg("CELL ID[%s]", caBuff);
			}
			mtiMemset(caBuff, 0x00, sizeof(caBuff));

			//LAC
			com_GetDevicePropertyString(COM_PROP_GSM_LAC, caBuff, sizeof(caBuff), &comErr);
			if(comErr == COM_ERR_NONE)
			{
				tpNetInfo->iLAC = mtiAtoUi(sizeof(caBuff), (UCHAR *)caBuff);
			}
			mtiMemset(caBuff, 0x00, sizeof(caBuff));

			dmsg("MNC[%d], CELLID[%d], LAC[%d]", tpNetInfo->iMNC, tpNetInfo->iCellID, tpNetInfo->iLAC);

		}
	}
	else
	{
		dmsg("NetIFInfo is NULL!");
		return RTN_ERROR;
	}

	return RTN_SUCCESS;
}

static CHAR *getProfileName(INT iCommType)
{
	switch(iCommType)
	{
		case COMMTYPE_GPRS:
			return GPRS_PROFILE;
		case COMMTYPE_ETHERNET:
			return ETH_PROFILE;
		case COMMTYPE_WIFI:
			return WIFI_PROFILE;
		default:
			dmsg("It's NOT NETWORK COMMTYPE[%d]", iCommType);
			return NULL;
	}
}

static void networkStatus_cb (enum com_NetworkEvent event, enum com_NetworkType type, const void *data, void* priv, enum com_ErrorCodes com_errno)
{
	dmsg("NET STAT CB -> NETWORK TYPE[%d] EVENT[%d] ERR_NO[%d]", type, event, com_errno);

	//LAN
	if(type == COM_NETWORK_TYPE_LAN || type == COM_NETWORK_TYPE_WLAN)
	{
		if(event == COM_EVENT_NETWORK_LINK_UP)
			vNetworkStat.ethLink = COMM_NET_STAT_ETH_LINK;
		else if(event == COM_EVENT_NETWORK_LINK_DOWN)
			vNetworkStat.ethLink = COMM_NET_STAT_ETH_UNLINK;
	}
	//GPRS/3G
	else if(type == COM_NETWORK_TYPE_PPP_GPRS)
	{
		if(event == COM_EVENT_NETWORK_SIGNAL && com_errno == COM_ERR_NONE )
		{
		INT iRssiPersent = 0;

		com_GetDevicePropertyInt(COM_PROP_GSM_SIGNAL_PERCENTAGE, &iRssiPersent, NULL);

		dmsg("GPRS SIGNAL STRENGTH [%d %%]", iRssiPersent);

		vNetworkStat.gprsRssiPersent = iRssiPersent;
		}
	}

	netRetStat = com_errno;
}

INT mtiActiveNetwork(INT iCommType)
{
	enum com_ErrorCodes errCode;
	CHAR caFilePath[100] = {0,};

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, getProfileName(iCommType));
	dmsg("CONN REQUEST NETWORK PROFILE PATH [%s]", caFilePath);
	mtiFileDbg(caFilePath);

	//Detach PreConnected Network
	if(curNetworkType != 0 && iCommType != curNetworkType)
	{
		CHAR caNetProfilePath[100] = {0,};
		INT iDetRet = 0;
		sprintf(caNetProfilePath, "%s/%s.xml", MTI_DATA_PATH, getProfileName(curNetworkType));
		dmsg("PRE NETWORK PROFILE PATH [%s]", caNetProfilePath);
		iDetRet = com_DetachNetwork(caNetProfilePath, &errCode);
		dmsg("com_DetachNetwork iDetRet[%d], ErrCode[%d]",iDetRet, errCode);
		iNetworkConnected = -1;
	}

	if(iNetworkConnected == -1 && iCommType != COMMTYPE_WIFI)
	//if(iNetworkConnected == -1)
	{
		dmsg("*********** RESTART!!");
		com_NetworkSetCallback(networkStatus_cb, NULL, NULL);

		iNetworkConnected = com_AttachNetwork(caFilePath, &errCode);
		dmsg("ATTACH iNetworkConnected[%d], ErrCode[%d]",iNetworkConnected, errCode);
	}
	else
	{
		iNetworkConnected = com_NetworkRestart(caFilePath, &errCode);
		dmsg("com_RestartNetworkAsync iNetworkConnected[%d], ErrCode[%d]",iNetworkConnected, errCode);
	}

	if(iNetworkConnected == 0)	//ATTACH SUCEESS
	{
		curNetworkType = iCommType;
		return RTN_SUCCESS;
	}
	else	//ATTACH FAIL
	{
		switch(netRetStat)
		{
			case COM_ERR_DEV_BATTERY:
				return RTN_COMM_GPRS_LOW_BATTERY;
			case COM_ERR_DEV_NOSIM:
				return RTN_COMM_GPRS_SIM_PROBLEM;
			case COM_ERR_DEV_INTERNAL :
				return RTN_COMM_GPRS_INTERNAL_ERR;
			case COM_ERR_DEV_GSM_PWR :
				return RTN_COMM_GPRS_GSM_PWR_ERR;
			case COM_ERR_DEV_PIN_PUK_VERIFY :
				return RTN_COMM_GPRS_PIN_VERIFY_ERR;
			case COM_ERR_DEV_IN_USE :
				return RTN_COMM_GPRS_INUSE;
			case COM_ERR_DEV_DETACHED :
				return RTN_COMM_GPRS_DETATCH;
			default:
				return RTN_ERROR;
		}
	}
}

INT mtiSetEthIF(INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw)
{
	FILE* fpFile = NULL;
	CHAR caFilePath[100] = {0,};

	//Build Serial Connection Profile XML

	//EDC PARAM ex) 1-DHCP, 2-STATIC
	if(iDhcp == 2)
		iDhcp = 0;

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, ETH_PROFILE);
	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	if(fpFile == NULL)
		return RTN_ERROR;

	fprintf(fpFile, "<NETWORK_PROFILE>\n");
	fprintf(fpFile, "  <TYPE>LAN</TYPE>\n");
	fprintf(fpFile, "  <DEVICE_NAME>ETH0</DEVICE_NAME>\n");
	fprintf(fpFile, "  <STARTUP_MODE>AUTO</STARTUP_MODE>\n");
	fprintf(fpFile, "  <DHCP_ENABLED>%d</DHCP_ENABLED>\n", iDhcp);
	if(!iDhcp)
	{
		fprintf(fpFile, "  <IP_ADDRESS>%s</IP_ADDRESS>\n",cpIpAddr);
		fprintf(fpFile, "  <NETMASK>%s</NETMASK>\n", cpNetMask);
		fprintf(fpFile, "  <GATEWAY>%s</GATEWAY>\n", cpGw);

	}
	fprintf(fpFile, "  <TIMEOUT>10000</TIMEOUT>\n");
	fprintf(fpFile, "  <SPEED>AUTO</SPEED>\n");
	fprintf(fpFile, "</NETWORK_PROFILE>\n");
	fclose(fpFile);

	return RTN_SUCCESS;
}

CHAR *caCellularAuthType[] = {"none", "CHAP", "PAP", "CHAP_PAP"};

INT mtiSetCelluarIF(CHAR *cpAPN, INT iAuthType, CHAR *cpUserName, CHAR *cpPasswd)
{
	FILE* fpFile = NULL;
	CHAR caFilePath[100] = {0,};

	//Build Mobile Connection Profile XML

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, GPRS_PROFILE);
	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	if(fpFile == NULL)
		return RTN_ERROR;

	fprintf(fpFile, "<NETWORK_PROFILE>\n");
	fprintf(fpFile, "	<TYPE>GPRS</TYPE>\n");
	fprintf(fpFile, "	<DEVICE_NAME>GPRS0</DEVICE_NAME>\n");
	fprintf(fpFile, "	<STARTUP_MODE>AUTO</STARTUP_MODE>\n");
	fprintf(fpFile, "	<TIMEOUT>20000</TIMEOUT>\n");
	fprintf(fpFile, "	<APN>%s</APN>\n", cpAPN);
	fprintf(fpFile, "	<AUTHENTICATION>%s</AUTHENTICATION>\n", caCellularAuthType[iAuthType]);
	if(iAuthType != COMM_CELLULAR_AUTH_NONE)
	{
		fprintf(fpFile, "	<USERNAME>%s</USERNAME>\n", cpUserName);
		fprintf(fpFile, "	<PASSWORD>%s</PASSWORD>\n", cpPasswd);
	}
	fprintf(fpFile, "</NETWORK_PROFILE>\n");
	fclose(fpFile);

	return RTN_SUCCESS;
}

CHAR *caWifiAuthType[] = {"auto", "wpa", "wpa2", "wep"};
CHAR *caWifiKeyMgmt[] = {"psk", "eap", "none"};
CHAR *caWifiEapType[] = {"tls", "peap", "fast"};
CHAR *caWifiAuthAlg[] = {"open", "shared"};

INT mtiSetWifiIF(CHAR *cpSSID, INT iAuthType, INT iKeyMgmt, CHAR *cpUsername, CHAR *cpPasswd,
		INT iWifiEapType, INT iAuthAlg, INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw)
{
	FILE* fpFile = NULL;
	CHAR caFilePath[100] = {0,};
	CHAR caCurSSID[50] = {0,};
	enum com_ErrorCodes eComErrNum;
	INT iRet = 0;

	//Build Mobile Connection Profile XML
	/*dmsg("SSID[%s], AUTHTYPE[%d], KEY MGMT[%d], USERNAME [%s], PASSWORD[%s] EAP TYPE[%d]", cpSSID, iAuthType,
			iKeyMgmt, cpUsername, cpPasswd, iWifiEapType);
	*/
	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, WIFI_PROFILE);
	com_GetDevicePropertyString(COM_PROP_WLAN_SSID, caCurSSID, sizeof(caCurSSID), &eComErrNum );
	dmsg("Current Attached SSID[%s], eComErrNum[%d]", caCurSSID, eComErrNum);
	if(mtiMemcmp(caCurSSID, cpSSID, mtiStrlen(50, cpSSID)))
	{
		//Diffrent SSID each other -> Detatched current Network
		dmsg("OLD SSID [%s], NEW SSID[%s]", caCurSSID, cpSSID);
		iRet = com_DetachNetwork(caFilePath, &eComErrNum);
		dmsg("com_DetachNetwork iRet[%d], eComErrNum[%d]",iRet, eComErrNum);
	}

	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	
	if(fpFile == NULL) return RTN_ERROR;

	fprintf(fpFile, "<network_profile>\n");
	fprintf(fpFile, "	<type>wlan</type>\n");
	fprintf(fpFile, "	<device_name>wlan0</device_name>\n");
	//fprintf(fpFile, "	<startup_mode>on-demand</startup_mode>\n");
	fprintf(fpFile, "	<startup_mode>AUTO</startup_mode>\n");
	fprintf(fpFile, "	<timeout>30000</timeout>\n");
	if(iDhcp == 2)
	{
		fprintf(fpFile, "  <dhcp_enabled>0</dhcp_enabled>\n");
		fprintf(fpFile, "  <ip_address>%s</ip_address>\n",cpIpAddr);
		fprintf(fpFile, "  <netmask>%s</netmask>\n", cpNetMask);
		fprintf(fpFile, "  <gateway>%s</gateway>\n", cpGw);
	}
	else
	{
		fprintf(fpFile, "  <dhcp_enabled>1</dhcp_enabled>\n");
	}
	fprintf(fpFile, "	<wlan_node>\n");
	fprintf(fpFile, "		<ssid>%s</ssid>\n", (cpSSID==NULL)?"":cpSSID);

	if(iKeyMgmt == COMM_WIFI_KEY_MGMT_NONE)
	{
		fprintf(fpFile, "		<auth_alg>%s</auth_alg>\n", caWifiAuthAlg[iAuthAlg]);
		fprintf(fpFile, "		<key_mgmt>%s</key_mgmt>\n", caWifiKeyMgmt[iKeyMgmt]);
	}
	else if(iKeyMgmt == COMM_WIFI_KEY_MGMT_PSK)
	{
		if(iAuthType == COMM_WIFI_PROTO_WEP)
		{
			//WEP
			fprintf(fpFile, "		<auth_alg>%s</auth_alg>\n", caWifiAuthAlg[iAuthAlg]);
			fprintf(fpFile, "		<wep_index>0</wep_index>\n");
			fprintf(fpFile, "		<wep0>%s</wep0>\n", (cpPasswd==NULL)?"":cpPasswd);
		}
		else
		{
			//WPA, WPA2
			fprintf(fpFile, "		<auth_alg>%s</auth_alg>\n", caWifiAuthAlg[iAuthAlg]);
			fprintf(fpFile, "		<proto>%s</proto>\n", caWifiAuthType[iAuthType]);
			fprintf(fpFile, "		<key_mgmt>%s</key_mgmt>\n", caWifiKeyMgmt[iKeyMgmt]);
			if(cpPasswd != NULL)
				fprintf(fpFile, "		<psk>%s</psk>\n", (cpPasswd==NULL)?"":cpPasswd);
		}
	}
	else if(iKeyMgmt == COMM_WIFI_KEY_MGMT_EAP)
	{
		fprintf(fpFile, "		<bss_type>infra</bss_type>\n");
		fprintf(fpFile, "		<auth_alg>%s</auth_alg>\n", caWifiAuthAlg[iAuthAlg]);
		fprintf(fpFile, "		<group>auto</group>\n");
		fprintf(fpFile, "		<pairwise>auto</pairwise>\n");
		fprintf(fpFile, "		<eap_type>%s</eap_type>\n",caWifiEapType[iWifiEapType]);
		fprintf(fpFile, "		<eap_password>%s</eap_password>\n", (cpPasswd==NULL)?"":cpPasswd);
		fprintf(fpFile, "		<eap_identity>%s</eap_identity>\n", (cpUsername==NULL)?"":cpUsername);
		fprintf(fpFile, "		<proto>%s</proto>\n", caWifiAuthType[iAuthType]);
		fprintf(fpFile, "		<key_mgmt>%s</key_mgmt>\n", caWifiKeyMgmt[iKeyMgmt]);
	}
	
	fprintf(fpFile, "	<country_code>id</country_code>\n");
	fprintf(fpFile, "	</wlan_node>\n");
	fprintf(fpFile, "</network_profile>\n");
	fclose(fpFile);

	return RTN_SUCCESS;
}

INT mtiScanWiFi(tWifiScanAP *tpWifiScanList, INT *iScanCnt, INT iCurCommType)
{
	enum com_ErrorCodes com_errno;
	CHAR caScanRawdata[5120] = {0,};

	if(iCurCommType == COMMTYPE_WIFI)
		mtiDetachNetwork(COMMTYPE_WIFI);

	if(com_WirelessScan(COM_WIRELESS_TYPE_WLAN, caScanRawdata, sizeof(caScanRawdata), &com_errno) < 0)
	{
		 dmsg( "WLAN Site Survey could not be done [errno %d: %s]", (int)com_errno,
				 com_GetErrorString(com_errno));
		 return RTN_ERROR;
	}
	else
	{
		//WIFI Scan Success

		dmsg("WIFI SCAN RAW DATA[%s]", caScanRawdata);
		return iWiFiScanRawDataParser(caScanRawdata, tpWifiScanList, iScanCnt);
	}
}

static INT mtiSetEthernetCommConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	FILE* fpFile = NULL;
	CHAR caFilePath[100] = {0,};
	CHAR caNetwork[100] = {0,};

	//Build Serial Connection Profile XML
	if(tpCommInfo->cpRemoteIP == NULL || tpCommInfo->iRemotePort < 1)
	{
		dmsg("IP is NULL or Port is smaller than 0!");
		return RTN_ERROR;
	}

	if(mtiStrlen(20, tpCommInfo->cpRemoteIP) < 7)
	{
		dmsg("IP Address Length was wrong! IP ADDR[%s]",tpCommInfo->cpRemoteIP);
		return RTN_ERROR;
	}

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, CONN_PROFILE);

	if(tpCommInfo->iCommType == COMMTYPE_ETHERNET)
		sprintf(caNetwork, "%s/%s", MTI_DATA_PATH, ETH_PROFILE);
	else if(tpCommInfo->iCommType == COMMTYPE_GPRS)
		sprintf(caNetwork, "%s/%s", MTI_DATA_PATH, GPRS_PROFILE);
	else if(tpCommInfo->iCommType == COMMTYPE_WIFI)
		sprintf(caNetwork, "%s/%s", MTI_DATA_PATH, WIFI_PROFILE);


	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	if(fpFile == NULL)
		return RTN_ERROR;

	fprintf(fpFile, "<CONNECTION_PROFILE>\n");
	fprintf(fpFile, "  <CONNECTION>\n");
	fprintf(fpFile, "    <TYPE>TCP</TYPE>\n");
	fprintf(fpFile, "    <NETWORK>%s</NETWORK>\n", caNetwork);

	//IP ADDRESS, PORT NUMBER
	fprintf(fpFile, "    <ADDRESS>%s</ADDRESS>\n", tpCommInfo->cpRemoteIP);
	fprintf(fpFile, "    <PORT>%d</PORT>\n", tpCommInfo->iRemotePort);

	fprintf(fpFile, "  </CONNECTION>\n");
	fprintf(fpFile, "</CONNECTION_PROFILE>\n");

	fclose(fpFile);

	return RTN_SUCCESS;
}

#if 0
static INT mtiSetSerialCommConfigure(tCommInfo *tpCommInfo)
{
	INT iRet = 0;
	FILE* fpFile = NULL;
	CHAR caFilePath[512];
	CHAR caPort[10] = {0,};
	CHAR *cpBaudRate = NULL;
	INT nPortNum = 0;


	// VX675 (Credle - COM8 !!!)
	//Build Serial Connection Profile XML
	dmsg("mtiSetSerialCommConfigure - CHANNEL : %d", tpCommInfo->iCommChannel);

	nPortNum = tpCommInfo->iCommChannel - COMMCHANNEL_COM0;

	if (nPortNum < 0)
		nPortNum = 0;

	sprintf(caPort, "COM%d", nPortNum);

	//BAUDRATE
	cpBaudRate = caBaudrateList[tpCommInfo->iCommBaudrate];

	memset(caFilePath, 0x00, sizeof(caFilePath));
	sprintf(caFilePath, "%s/%s%d.xml", MTI_DATA_PATH, SERIAL_PROFILE, nPortNum);

#if 0
	dpt();
	xmlFileDbg(caFilePath);
#endif

	/*
	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	*/

	fpFile = fopen(caFilePath, "w");

	if (fpFile == NULL)
	{
		dmsg("mtiSetSerialCommConfigure - CHANNEL : %d, FILE OPEN FAIL !!!!!", nPortNum);
		return RTN_ERROR;
	}

#if 0
	fprintf(fpFile, "<CONNECTION_PROFILE>\n");
	fprintf(fpFile, "  <CONNECTION>\n");
	fprintf(fpFile, "    <TYPE>RAW_SERIAL</TYPE>\n");
	fprintf(fpFile, "    <SERIAL_PORT>%s</SERIAL_PORT>\n", caPort);
	fprintf(fpFile, "    <BAUDRATE>%s</BAUDRATE>\n", cpBaudRate);

	/** N,8,1 **/
	fprintf(fpFile, "    <PARITY>none</PARITY>\n");
	fprintf(fpFile, "    <DATABITS>8</DATABITS>\n");
	fprintf(fpFile, "    <STOPBITS>1</STOPBITS>\n");
	fprintf(fpFile, "    <FLOWCONTROL>none</FLOWCONTROL>\n");
	fprintf(fpFile, "  </CONNECTION>\n");
	fprintf(fpFile, "</CONNECTION_PROFILE>\n");
	fclose(fpFile);
#else
	memset(caFilePath, 0x00, sizeof(caFilePath));
	sprintf(caFilePath,
	"<CONNECTION_PROFILE>\n"
	"<CONNECTION>\n"
	"<TYPE>RAW_SERIAL</TYPE>\n"
	"<SERIAL_PORT>%s</SERIAL_PORT>\n"
	"<BAUDRATE>%s</BAUDRATE>\n"
	"<PARITY>none</PARITY>\n"
	"<DATABITS>8</DATABITS>\n"
	"<STOPBITS>1</STOPBITS>\n"
	"<FLOWCONTROL>none</FLOWCONTROL>\n"
	"</CONNECTION>\n"
	"</CONNECTION_PROFILE>\n", caPort, cpBaudRate);

	iRet = fprintf(fpFile, caFilePath);

	dmsg("mtiSetSerialCommConfigure - FILE WRITE result - [%d][%d]", strlen(caFilePath), iRet);
	fclose(fpFile);

	if (iRet != strlen(caFilePath))
		return RTN_ERROR;
#endif

	return RTN_SUCCESS;
}
#else
static INT mtiSetSerialCommConfigure(tCommInfo *tpCommInfo)
{
	return RTN_SUCCESS;
}
#endif

static INT mtiSetFaxModemConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	FILE* fpFile = NULL;
	CHAR caFilePath[100];
	CHAR *cpModulation = NULL;

	//Build Serial Connection Profile XML

	//BAUDRATE
	//cpModulation = caModemModList[tpCommInfo->iCommBaudrate];
	//cpModulation = "auto";
	//cpModulation = "v22";
	cpModulation = "v90";

	dmsg("tpCommInfo->iCommBaudrate[%d], tpCommInfo->cpRemoteTelNo[%s]",tpCommInfo->iCommBaudrate, tpCommInfo->cpRemoteTelNo);

	sprintf(caFilePath, "%s/%s.xml", MTI_DATA_PATH, MODEM_PROFILE);
	remove(caFilePath);
	fpFile = fopen(caFilePath, "w+");
	if(fpFile == NULL)
		return RTN_ERROR;

	fprintf(fpFile, "<CONNECTION_PROFILE>\n");
	fprintf(fpFile, "\t<NAME>RAW_DIAL</NAME>\n");
	fprintf(fpFile, "\t<CONNECTION>\n");
	fprintf(fpFile, "\t\t<TYPE>RAW_MODEM</TYPE>\n");
	//fprintf(fpFile, "    <DEVICE_NAME>MDM_EXT</DEVICE_NAME>\n");
	//<PABX_CODE>8</PABX_CODE>
	//fprintf(fpFile, "\t\t<PABX_CODE>9</PABX_CODE>\n");
	fprintf(fpFile, "\t\t<ADDRESS>%s</ADDRESS>\n", tpCommInfo->cpRemoteTelNo);
	//fprintf(fpFile, "    <TIMEOUT>%d</TIMEOUT>\n", ulTimeout);
	fprintf(fpFile, "\t\t<DEVICE_NAME>MDM_INT</DEVICE_NAME>\n");
	//fprintf(fpFile, "\t\t<DEVICE_NAME>MDM_EXT</DEVICE_NAME>\n");

	//ETC
	//COMMOPT_ASYNC_MODEM
	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_ASYNC_MODEM))
		fprintf(fpFile, "\t\t<DIAL_MODE>async</DIAL_MODE>\n");
	else
		fprintf(fpFile, "\t\t<DIAL_MODE>sync</DIAL_MODE>\n");
	fprintf(fpFile, "\t\t<DIAL_TYPE>tone</DIAL_TYPE>\n");
	fprintf(fpFile, "\t\t<MODULATION>%s</MODULATION>\n", cpModulation);
	//fprintf(fpFile, "    <MODULATION>auto</MODULATION>\n");
	fprintf(fpFile, "\t\t<BLIND_DIALING>yes</BLIND_DIALING>\n");
	/*
	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_ASYNC_MODEM))
		fprintf(fpFile, "\t\t<FAST_CONNECT>no</FAST_CONNECT>\n");
	else
		fprintf(fpFile, "\t\t<FAST_CONNECT>yes</FAST_CONNECT>\n");
	*/
	fprintf(fpFile, "\t\t<FAST_CONNECT>no</FAST_CONNECT>\n");

	fprintf(fpFile, "\t\t<ERROR_CORRECTION>none</ERROR_CORRECTION>\n");
	fprintf(fpFile, "\t\t<COMPRESSION>none</COMPRESSION>\n");
/*
	fprintf(fpFile, "\t\t<BAUDRATE>115200</BAUDRATE>\n");
	fprintf(fpFile, "\t\t<PARITY>none</PARITY>\n");
	fprintf(fpFile, "\t\t<DATABITS>8</DATABITS>\n");
	fprintf(fpFile, "\t\t<STOPBITS>1</STOPBITS>\n");
*/
	fprintf(fpFile, "\t</CONNECTION>\n");
	fprintf(fpFile, "</CONNECTION_PROFILE>\n");

	fclose(fpFile);

	return RTN_SUCCESS;
}

tCommInfo *mtiInitializeComm()
{
	INT i = 0;

	sem_init(&sem_ConnRetStat, 1);
	sem_init(&sem_ConnAsyncInfo, 1);

	if (!g_ucInitCommVar)
	{
		g_ucInitCommVar = TRUE;

		mtiMemset(g_tCommInfo, 0, sizeof(g_tCommInfo));
		mtiMemset(g_lhndTable, 0, sizeof(g_lhndTable));
	}

	for (i = 0; i < MAX_COMM_COUNT; i++)
	{
		if (!g_tCommInfo[i].iCommID)
		{
			g_tCommInfo[i].iCommID = i + 1;
			return &g_tCommInfo[i];
		}
	}
	return NULL;
}

INT mtiFlushComm(tCommInfo *tpCommInfo)
{
	//Receive Buffer Clear

	return RTN_SUCCESS;
}

INT mtiPreOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	INT iRet = RTN_ERROR;
	CHAR *cpProfileXml = NULL;
	CHAR caCommXmlFile[100] = {0,};
	struct com_ConnectHandle *hSession;

	dpt();
	if ((tpCommInfo->iCommType == COMMTYPE_GPRS) || (tpCommInfo->iCommType == COMMTYPE_FAXMODEM))
	{
		dpt();
		/** CONNECTED CHECK **/
		if (tpCommInfo->iCommStatus >= COMMSTAT_CONNECTION && tpCommInfo->iCommStatus != COMMSTAT_DISCONNECT)
		{
			dpt();
			return RTN_SUCCESS;
		}

		/** COMMUNICATION SETTING **/
		switch (tpCommInfo->iCommType)
		{
			case COMMTYPE_GPRS:
				iRet = mtiSetEthernetCommConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = CONN_PROFILE;
				break;

			case COMMTYPE_FAXMODEM:
				dpt();
				iRet = mtiSetFaxModemConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = MODEM_PROFILE;
				break;
		}

		sprintf(caCommXmlFile, "%s/%s.xml", MTI_DATA_PATH, cpProfileXml);

		//XML DEBUG
		xmlFileDbg(caCommXmlFile);
#if 0
		dmsg("VXCEHW.INI");
		xmlFileDbg("VXCEHW.INI");
		dmsg("VXCESYS.INI");
		xmlFileDbg("VXCESYS.INI");
#endif


		dpt();
		/** COMMUNICATION START **/
		if (iRet == RTN_SUCCESS)
		{
			enum com_ErrorCodes iErrCode;

#if 0
			hSession = com_ConnectAsync(caCommXmlFile, connection_callback, &inConnStatus, &iErrCode);
#else
			hSession = com_ConnectAsync(caCommXmlFile, connection_callback, NULL, &iErrCode);
#endif

			if(hSession == NULL)
			{
				dmsg("mtiPreOpenComm Fail - [%d]", (int)iErrCode);
				return RTN_ERROR;
			}
			g_lhndTable[tpCommInfo->iCommID - 1] = hSession;
			tpCommInfo->iCommStatus = COMMSTAT_PRECONNECTION;
		}
	}

	return iRet;
}

static INT g_hThreadConnComm = 0;
static struct com_ConnectHandle *g_hThreadSession;
INT fnThreadConnComm(CHAR *cpXMLFile, int timeout_msec)
{
	enum com_ErrorCodes iErrCode;

	dpt();
	dpt();
	dmsg("***** cpXMLFile [%s]", cpXMLFile);
	g_hThreadSession = com_Connect(cpXMLFile, connection_callback, NULL, timeout_msec, &iErrCode);
	dpt();
	dpt();

	dmsg("######################### iErrCode = %d", iErrCode);
	if (NULL != g_hThreadSession)
	{
		dpt();
	}
}

INT mtiOpenCommWithPOS(tCommInfo *tpCommInfo, ULONG ulTimeout, BOOL bPOSUse)
{
	INT iRet = RTN_ERROR;
	struct com_ConnectHandle *hSession;
	CHAR caCommXmlFile[100] = {0,};
	CHAR *cpProfileXml = NULL;
	INT nPortNumber = 0;
	tCommAsyncInfo tCommAsync;
	LONG lEvent;
	INT nSubMsg, nPOSMSg;
	BOOL bUserStop = FALSE;
	INT hTimer = -1;
	enum com_ErrorCodes errCode = COM_ERR_NONE;
	enum com_ErrorCodes closeErrCode = COM_ERR_NONE;

	nSubMsg = nPOSMSg = 0;
	setConnRet(errCode);

	dmsg("[FUNC] mtiOpenComm()");
	if (tpCommInfo != NULL)
	{
		dmsg("tpCommInfo->iCommID = %d ", tpCommInfo->iCommID);

		/** PRECONNECTION SYNC **/
		switch (tpCommInfo->iCommType)
		{
			case COMMTYPE_FAXMODEM:
			case COMMTYPE_GPRS: //@@WAY PREDIAL 20210607
				if (tpCommInfo->iCommStatus == COMMSTAT_PRECONNECTION)
				{
					hSession = g_lhndTable[tpCommInfo->iCommID - 1];

					//TODO: Preconnection Thread
					dmsg("Waiting Predialing...");
					if(com_ConnectWait(hSession, ulTimeout, &errCode) == COM_CONNECTION_STATUS_CONNECT)
					{
						tpCommInfo->iCommStatus = COMMSTAT_CONNECTION;
						dpt();
						return RTN_SUCCESS;
					}
					else
					{
						//FAIL, ERROR, TIMEOUT
						dmsg("Predialing is failed...");
						iRet = com_ConnectClose(hSession, &closeErrCode);
						g_tCommInfo[tpCommInfo->iCommID - 1].iCommID = 0;
						dmsg("com_ConnectClose() iRet[%d] closeErrCode [%d]", iRet, closeErrCode);

						switch(errCode)
						{
							case COM_ERR_DEV_NO_DIAL_TONE:
							case COM_ERR_DEV_NO_CARRIER:
								iRet = RTN_COMM_NO_DIALTONE;
								break;
							case COM_ERR_DEV_IN_USE:
							case COM_ERR_DEV_LINE_IN_USE:
							case COM_ERR_DEV_BUSY:
								iRet = RTN_COMM_LINE_BUSY;
								break;
							case COM_ERR_DEV_NO_CABLE:
								iRet = RTN_COMM_PSTN_CABLE_DISCONN;
								break;
							
							//@@WAY PREDIAL 20210607
							case COM_ERR_CON_TIMEOUT:
								iRet = RTN_TIMEOUT;
								break;
							//
							default:
								iRet = RTN_ERROR;
						}
						tpCommInfo->iCommStatus = COMMSTAT_DISCONNECT;
						return iRet;

					}
				}
				break;
		}

		/** CONNECTED CHECK **/
		if (tpCommInfo->iCommStatus >= COMMSTAT_CONNECTION && tpCommInfo->iCommStatus != COMMSTAT_DISCONNECT)
		{
			return RTN_SUCCESS;
		}

		/** COMMUNICATION SETTING **/
		switch (tpCommInfo->iCommType)
		{
			case COMMTYPE_GPRS:
				iRet = mtiSetEthernetCommConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = CONN_PROFILE;
				//cpProfileXml = GPRS_PROFILE;
				break;

			case COMMTYPE_FAXMODEM:
				dpt();
				iRet = mtiSetFaxModemConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = MODEM_PROFILE;
				break;

			case COMMTYPE_SERIAL:
				dpt();
				iRet = mtiSetSerialCommConfigure(tpCommInfo);
				nPortNumber = tpCommInfo->iCommChannel - COMMCHANNEL_COM0;

				sprintf(caCommXmlFile, "%s/%s%d.xml", MTI_DATA_PATH, SERIAL_PROFILE, nPortNumber);

				break;

			case COMMTYPE_WIFI:
				iRet = mtiSetEthernetCommConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = CONN_PROFILE;
				//cpProfileXml = WIFI_PROFILE;
				break;

			case COMMTYPE_ETHERNET:
				iRet = mtiSetEthernetCommConfigure(tpCommInfo, ulTimeout);
				cpProfileXml = CONN_PROFILE;
				//cpProfileXml = ETH_PROFILE;
				break;
		}

		if (COMMTYPE_SERIAL!=tpCommInfo->iCommType)
			sprintf(caCommXmlFile, "%s/%s.xml", MTI_DATA_PATH, cpProfileXml);

		//XML DEBUG
		dpt();
		//xmlFileDbg(caCommXmlFile);

		/** COMMUNICATION START **/
		if (iRet == RTN_SUCCESS)
		{
			enum com_ErrorCodes iErrCode;
			INT iStatus = NOT_CONNECTED;

			//Clear Pending Event
			clearEvent();

#if 0
			if (0 == ulTimeout)
				hSession = com_Connect(caCommXmlFile, connection_callback, NULL, 0, &iErrCode);
			else
				hSession = com_Connect(caCommXmlFile, connection_callback, NULL, ulTimeout, &iErrCode);
#else
			dmsg("--- connect start ----");

			dpt();
			if (COMMTYPE_SERIAL == tpCommInfo->iCommType )
			{
				dpt();
				dpt();
				dpt();
				dpt();
				hSession = com_Connect(caCommXmlFile, connection_callback, NULL, ulTimeout, &iErrCode);
			}
			else
			{
				mtiMemset(&tCommAsync, 0x00, sizeof(tCommAsync));

				tCommAsync.iConnType = TYPE_CONN_NOR;
				tCommAsync.iTaskID = get_task_id();
				tCommAsync.iMsg = USER_EVT_FROM_CONNECTION_CB;
				tCommAsync.piStatus = &iStatus;

				dmsg("---- ASYNC TASK : %d, MSG : %d -----", get_task_id(), MSG_CONN_ASNC_MSG);

#if 0
				if (0 != ulTimeout)
				{
					hTimer = set_timer(ulTimeout, EVT_TIMER);
				}
#endif

				sem_wait(&sem_ConnAsyncInfo);
				hSession = com_ConnectAsync(caCommXmlFile, connection_callback, &tCommAsync, &iErrCode);

				if (NULL != hSession)
				{
					// POS Integration 
					if (1==bPOSUse)
						iGetAppTypeCode(NULL, NULL, &nPOSMSg);

					mtiInitializeKeyboard();

					while (TRUE)
					{
						//dmsg("--- wait connect event ----");

						sem_post(&sem_ConnAsyncInfo);
#if 0
						if (0 <= hTimer)
							lEvent = wait_evt(EVT_KBD | EVT_USER | EVT_TIMER);
						else
							lEvent = wait_evt(EVT_KBD | EVT_USER);
#endif
						lEvent = read_event();

						if (lEvent != 0L)
						{
							dmsg("--- release connect event [x%08x] ----", lEvent);
						}

						if (EVT_KBD == (EVT_KBD & lEvent))
						{
							nSubMsg = mtiForceReadKeyData();

							dmsg("--- key event [%d] ----", nSubMsg);

							if (KEY_CANCEL == nSubMsg)
							{
								iErrCode = COM_ERR_CON_TIMEOUT;
								bUserStop = TRUE;
								break;
							}
						}
						else if (EVT_USER == (EVT_USER & lEvent))
						{
							LONG lUserEvt = 0L;
							lUserEvt = read_user_event();

							dmsg("--- user event [%x] ----", lUserEvt);

							if (lUserEvt & USER_EVT_FROM_CONNECTION_CB)
							{
								dmsg("iStatus[%d]", iStatus);
								if (CONNECTED==iStatus)
								{
									dmsg("GOT CONNECTED!!!");
									iErrCode = COM_ERR_NONE;
								}
								else if(CONNECTING == iStatus)
								{
									dmsg("Connecting...");
									continue;
								}
								else
								{
									sem_wait(&sem_ConnRetStat);
									iErrCode = connRetStat;
									dmsg("connRetStat [%d]", connRetStat);
									sem_post(&sem_ConnRetStat);
								}
								break;
							}
							
							if (TRUE == bPOSUse && 0 != nPOSMSg && nSubMsg == nPOSMSg)
							{
								iErrCode = COM_ERR_CON_TIMEOUT;
								bUserStop = TRUE;
								break;
							}
						}
						else if (0 <= hTimer && EVT_TIMER == (EVT_TIMER & lEvent))
						{
							dmsg("--- timer event [%d] ----", nSubMsg);

							iErrCode = COM_ERR_CON_TIMEOUT;
							//bUserStop = TRUE;
							break;
						}
					}

					{
						//com_ConnectAsync must be couple with com_ConnectWait func.
						enum com_WaitStatus waitStat = COM_CONNECTION_STATUS_FAIL;
						enum com_ErrorCodes waitErrCode = COM_ERR_NONE;

						waitStat = com_ConnectWait(hSession, 200L, &waitErrCode);
						dmsg("com_ConnectWait WaitStatus[%d] waitErrCode[%d]", waitStat, waitErrCode);
						if(COM_CONNECTION_STATUS_TIMEOUT == waitStat)
							iErrCode = COM_ERR_CON_TIMEOUT;
					}

					mtiFinalizeKeyboard();

					if (0 <= hTimer)
					{
						clr_timer(hTimer);
					}

					dmsg("--- connect result [%d] ----", iErrCode);

					if (COM_ERR_NONE != iErrCode)
					{
						enum com_ErrorCodes disConnErrCode = COM_ERR_NONE;
						com_ConnectClose(hSession, &disConnErrCode);
						dmsg("com_ConnectClose Error Code [%d]", disConnErrCode);
						hSession = NULL;
					}
				}
			}
#endif

			g_lhndTable[tpCommInfo->iCommID - 1] = hSession;

			if (TRUE == bUserStop)
			{
				iRet = RTN_COMM_USER_CANCEL;
			}
			else
			{
				dmsg("com_Connect() Error Code [%d]", iErrCode);
				{
					switch (iErrCode)
					{
					case COM_ERR_NONE:
						iRet = RTN_SUCCESS;
						tpCommInfo->iCommStatus = COMMSTAT_CONNECTION;
						g_lhndTable[tpCommInfo->iCommID - 1] = hSession;
						break;
					case COM_ERR_CON_TIMEOUT:
						iRet = RTN_TIMEOUT;
						break;
					case COM_ERR_DEV_NO_DIAL_TONE:
					case COM_ERR_DEV_NO_CARRIER:
						iRet = RTN_COMM_NO_DIALTONE;
						break;
					case COM_ERR_DEV_IN_USE:
					case COM_ERR_DEV_BUSY:
					case COM_ERR_DEV_LINE_IN_USE:
						iRet = RTN_COMM_LINE_BUSY;
						break;
					case COM_ERR_DEV_NO_CABLE:
						iRet = RTN_COMM_PSTN_CABLE_DISCONN;
						break;
					default:
						iRet = RTN_ERROR;
						break;
					}
				}
			}
		}
		if (RTN_SUCCESS != iRet)
		{
			g_lhndTable[tpCommInfo->iCommID - 1] = NULL;
			g_tCommInfo[tpCommInfo->iCommID - 1].iCommID = 0;
		}
	}
	return iRet;
}


INT mtiOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	return mtiOpenCommWithPOS(tpCommInfo, ulTimeout, FALSE);
}

INT mtiGetCommHandle(tCommInfo *tpCommInfo)
{
	struct com_ConnectHandle *hSession;
	INT hFile;
	enum com_ErrorCodes com_errno;

	hSession = g_lhndTable[tpCommInfo->iCommID - 1];

	hFile = com_ConnectGetFD(hSession, &com_errno);

	if (-1 == hFile)
	{
		//dmsg("mtiGetCommHandle - Error [%d]", com_errno);
		return 0;
	}

	return hFile;
}

//Callback function to inform the status of the connection process.
void connection_callback (enum com_ConnectionEvent event, enum com_ConnectionType type, const void *data, void *priv,
		enum com_ErrorCodes com_errno)
{
	INT inStatus;
	tCommAsyncInfo *ptCommAsync = (tCommAsyncInfo *)priv;
	INT iResult;

	if(type == COM_CONNECTION_TYPE_RAW_MODEM)
	{
		dmsg("--- connection_callback : MODEM EVENT -[%d] com_errno[%d]----", (int)event, com_errno);
	}
	else
	{
		dmsg("--- connection_callback : event -[%d], type - [%d] ----", (int)event, (int)type);
	}
	if (NULL != ptCommAsync)
	{
		dmsg("ptCommAsync->iConnType = %d, ptCommAsync->iTaskID = %d, ptCommAsync->iMsg = %d, ptCommAsync->piStatus = %p", 
			ptCommAsync->iConnType, ptCommAsync->iTaskID, ptCommAsync->iMsg, ptCommAsync->piStatus);
	}

	setConnRet(com_errno);

	switch(event)
	{
		case COM_EVENT_CONNECTION_ESTABLISHED:
			dmsg("Connection established!");
			inStatus = CONNECTED;
			break;
		case COM_EVENT_CONNECTION_NEXT:
			dmsg("Trying new connection");
			inStatus = CONNECTING;
			break;
		case COM_EVENT_CONNECTION_FAILED:
			dmsg("Connection failed %d: %s", com_errno, com_GetErrorString(com_errno));
			inStatus = NOT_CONNECTED;
			break;
		case COM_EVENT_PROFILE_FAILED:
			dmsg("Entire profile failed %d: %s", com_errno, com_GetErrorString(com_errno));
			inStatus = NOT_CONNECTED;
			break;
		default:
			dmsg("COM EVENT %d - errno %d, %s", event, com_errno, com_GetErrorString(com_errno));
			inStatus = NOT_CONNECTED;
			break;
	}

	if (NULL!=ptCommAsync)
	{
		//If priv parameter is not NULL, it means that the cb function was called from
		//com_ConnectAsync, so priv is updated with the status of the connection

		if (NULL!=ptCommAsync->piStatus)
			*ptCommAsync->piStatus = inStatus;

		if (0 != ptCommAsync->iTaskID && 0 != ptCommAsync->iMsg)
		{
			if (TYPE_CONN_NOR == ptCommAsync->iConnType)
			{
				if (CONNECTED == inStatus || NOT_CONNECTED == inStatus)
				{
					sem_wait(&sem_ConnAsyncInfo);
					iResult = post_user_event(ptCommAsync->iTaskID, USER_EVT_FROM_CONNECTION_CB);
					dmsg("connection_callback : post_user_event - [%d]", iResult);
					sem_post(&sem_ConnAsyncInfo);
				}
			}
		}
	}
}

INT mtiWriteCommWithTimeOut(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength, INT nTimeOut)
{
	struct com_ConnectHandle *hSession = NULL;
	INT iRet = RTN_ERROR;
	enum com_ErrorCodes iErrCode;

	dmsg("[FUNC] mtiWriteComm()");
	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			/** COMM SEND **/
			dbuf("WRITE", ucpWrite, iLength);
			iRet = com_Send(hSession, ucpWrite, iLength, &iErrCode);
			dmsg("com_Send() Result = %d", iRet);
			if (iRet == iLength)
			{
				return RTN_SUCCESS;
			}
			else
			{
				iRet = RTN_ERROR;
			}
		}
	}

	return iRet;
}

INT mtiWriteComm(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength)
{
	return mtiWriteCommWithTimeOut(tpCommInfo, ucpWrite, iLength, 0);
}

#if 1
INT mtiReadComm_With_Timeout(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength, INT nMillisecond)
{
	struct com_ConnectHandle *hSession = NULL;
	INT iRet = RTN_ERROR;
	enum com_ErrorCodes iErrCode;
	UINT uiFlagOut;
	INT nTimeOut = -1;


	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			/** COMM RECEIVE **/
			if (0 != nMillisecond)
				nTimeOut = nMillisecond;

			iRet = com_Select(hSession, COM_SELECT_READ, nTimeOut, &uiFlagOut, &iErrCode);
			if (iRet == -1)
			{
				dmsg("com_Select [%d] [0x%x]", iErrCode, iErrCode);
				return RTN_ERROR;
			}
			else if (iRet == 0)
			{
				dmsg("com_Select timeout. iLength[%d]", iLength);
				return RTN_ERROR;
			}

			if (!(uiFlagOut & COM_SELECT_READ))
			{
				dmsg("wrong flag detect!! [%u] ", uiFlagOut);
				return RTN_ERROR;
			}

			iRet = com_Receive(hSession, ucpRead, iLength, &iErrCode);
			if (0 < iRet && iErrCode == COM_ERR_NONE)
			{
				dmsg("[FUNC] mtiReadComm() - SUCCESS");
				dbuf("RECV", ucpRead, iRet);
				*iReadLength = iRet;
				return RTN_SUCCESS;
			}
			else
			{
				if (iErrCode != COM_ERR_NONE)
				{
					dmsg("[FUNC] mtiReadComm() ERROR, iErrCode[%d]", iErrCode);
				}
				iRet = RTN_ERROR;
			}
		}
	}

	return iRet;

}


INT mtiReadComm(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength)
{
	return mtiReadComm_With_Timeout(tpCommInfo, ucpRead, iLength, iReadLength, 1000);
}
#else
INT mtiReadComm(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength)
{
	struct com_ConnectHandle *hSession = NULL;
	INT iRet = RTN_ERROR;
	enum com_ErrorCodes iErrCode;
	UINT uiFlagOut;
	fd_set read_set;
	int socket;
	int status;
	struct timeval tv;
	char buf[4]={0,};
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			/** COMM RECEIVE **/
			socket = com_ConnectGetFD(hSession, NULL);
			dmsg("get socket[%d]",socket);
			dmsg("get_port_status RET[%d], buf dbg...",get_port_status(socket, buf));
			dbuf(NULL, buf, sizeof(buf));

			FD_ZERO(&read_set);
			FD_SET(socket, &read_set);

			status = select(socket + 1, &read_set,NULL,NULL, &tv);

			switch(status)
			{
				case -1:
					dmsg("Select Error");
					return RTN_ERROR;
				case 0:
					dmsg("Select Timeout");
					return RTN_ERROR;
			}

			dmsg("Select Catch Read");
			iRet = read(socket, ucpRead, iLength);
			if(iRet > 0 )
			{
				dmsg("[FUNC] mtiReadComm() - SUCCESS");
				dbuf("RECV", ucpRead, iRet);
				*iReadLength = iRet;
				return RTN_SUCCESS;
			}
			else
			{
				iRet = RTN_ERROR;
			}
		}
	}

	return iRet;
}
#endif
INT mtiCloseComm(tCommInfo *tpCommInfo)
{
	struct com_ConnectHandle *hSession = NULL;
	INT iRet = RTN_ERROR;
	enum com_ErrorCodes iErrCode;

	dmsg("[FUNC] mtiCloseComm()");
	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			dmsg("tpCommInfo->iCommStatus [%d]", tpCommInfo->iCommStatus);
			/**
			if (tpCommInfo->iCommStatus == COMMSTAT_PRECONNECTION)
			{
				iRet = com_ConnectWait(hSession, 200L, &iErrCode);
				dmsg("com_ConnectWait iRet[%d] Error Code[%d] ", iRet, iErrCode);
			}
			**/

			/** COMM DISCONNECT **/
			iRet = com_ConnectClose(hSession, &iErrCode);

			g_lhndTable[tpCommInfo->iCommID - 1] = NULL;
			g_tCommInfo[tpCommInfo->iCommID - 1].iCommID = 0;
			dmsg("com_ConnectClose() iErrCode [%d]", iErrCode);

			//MODEM HANGUP SLEEP
			/**
			if (tpCommInfo->iCommType == COMMTYPE_FAXMODEM)
			{
				mtiCheckHangUp();
			}
			**/

			/*if (tpCommInfo->iCommType == COMMTYPE_WIFI)
			{
				dmsg("Recheck WIFI status");
				//updateMobileNetworkProvider(COMMTYPE_WIFI);
				iNetworkWatcher(FALSE);
			}*/

			return RTN_SUCCESS;
		}

		return RTN_COMM_ALREADY_DISCONNECT;
	}
	return iRet;
}

VOID mtiFinalizeComm(tCommInfo *tpCommInfo)
{
	dmsg("[FUNC] mtiFinalizeComm()");
	if (tpCommInfo != NULL)
	{
		mtiMemset(tpCommInfo, 0, sizeof(tCommInfo));
	}
}

static VOID xmlFileDbg(CHAR *cpFileName)
{
#ifdef __DEBUG_ON__

	FILE *file;
	CHAR readBuff[1024] = { '\0', };

	dmsg("------------------------------");

	file = fopen(cpFileName, "r");
	if (NULL== file)
	{
		dmsg("FILE[%s] Contents.. Open Fail !!", cpFileName);
		return;
	}

	dmsg("FILE[%s] Contents...", cpFileName);

	while (0 < fread(readBuff, 1, sizeof(readBuff) - 1, file))
	{
		dmsg("%s", readBuff);
		memset(readBuff, 0x00, sizeof(readBuff));
	}
	
	dmsg("------------------------------");
	fclose(file);

#endif
}

INT mtiCheckHangUp()
{
	enum com_ErrorCodes errCode = COM_ERR_NONE;
	INT iRet = 0, iRetVal = TRUE;
	UINT infCnt = 0, foundInfCnt = 0;
	stNIInfo niInfo[20] = {0,};
	INT iIdx = 0, modemIdx=0;
	INT checkingTry = 0;
	dpt();

	iRet = com_purgeCache(&errCode);
	dmsg("com_purgeCache iRet [%d] Error Code[%d]", iRet, errCode);

	iRet = com_Destroy(&errCode);
	SVC_WAIT(100);

	ceRegister();


/*	while(checkingTry < 20)
	{
		memset(niInfo, 0x00, sizeof(niInfo));
		infCnt = ceGetNWIFCount();
		dmsg("ceGetNWIFCount infCnt[%d]",infCnt);

		if(infCnt > 0)
		{
			ceGetNWIFInfo(niInfo, infCnt, &foundInfCnt );
			dmsg("foundInfCnt [%d]",foundInfCnt);
			for(iIdx = 0; iIdx < foundInfCnt; iIdx++)
			{
				dmsg("IDX[%d] - INTERFACE", iIdx);
				dmsg("niHandle [%d]", niInfo[iIdx].niHandle);
				dmsg("niCommTech [%s]", niInfo[iIdx].niCommTech);
				dmsg("niDeviceName [%s]", niInfo[iIdx].niDeviceName);
				dmsg("niRunState [%d]", niInfo[iIdx].niRunState);
				dmsg("niDeviceDriverName [%s]", niInfo[iIdx].niDeviceDriverName);
				dmsg("niDeviceDriverType [%d]", niInfo[iIdx].niDeviceDriverType);
				if(niInfo[iIdx].niDeviceDriverType == 3)
				{
					modemIdx = iIdx;
				}
			}
		}

		if(niInfo[modemIdx].niRunState == 2)	//Ready
		{
			dmsg("Dialing is Ready.")
			iRetVal = FALSE;
			break;
		}
		checkingTry++;
		dmsg("checkingTry[%d]", checkingTry);
		SVC_WAIT(1000);
	}
*/
	ceUnregister();
	SVC_WAIT(100);


	iRet = com_Init(&errCode);
	dmsg("com_Init iRet [%d] Error Code[%d]", iRet, errCode);
	return iRetVal;
}

//@@WAY CHANGED, 20190801
//@@WAY UPDATED, 20190901
int checkCommIsAvailable()
{
	enum com_ErrorCodes com_errno;
	int rc = 0;
	int inSuppInterfaces;

	//CHECK COMM IS READY....
	while(1)
	{
		rc = com_GetDevicePropertyInt(COM_PROP_SUPP_INTERFACES, &inSuppInterfaces, &com_errno);
		if(rc < 0)
		{
			//Error getting the property
			dmsg("com_GetDevicePropertyInt failed with com_errno [%d], %s", com_errno, com_GetErrorString(com_errno));
			inSuppInterfaces=0;
			break;
		}
		else
		{
			if(inSuppInterfaces == 1)
			{
				SVC_WAIT(1000);
				continue;
			}
			else
				break;
		}


	}

	return inSuppInterfaces;
}

int IsMODEM(void)
{
	int iRetComm = 0;
	int iRet = 0;

	iRetComm = checkCommIsAvailable();
	if(iRetComm & COM_MODEM)
	{
		dmsg("DIAL UP IS AVAIL");
		iRet = 1;
	}
	else
	{
		dmsg("DIAL UP IS NOT AVAIL");
		iRet = 0;
	}

	return iRet;
}

int IsRadioGPRS(void)
{
	int iRetComm = 0;
	int iRet = 0;

	iRetComm = checkCommIsAvailable();
	if(iRetComm & COM_GPRS || iRetComm & COM_UMTS)
	{
		dmsg("GPRS/3G IS AVAIL");
		iRet = 1;
	}
	else
	{
		dmsg("GPRS/3G IS NOT AVAIL");
		iRet = 0;
	}

	return iRet;
}

int IsWifi(void)
{
	int iRetComm = 0;
	int iRet = 0;

	iRetComm = checkCommIsAvailable();
	if(iRetComm & COM_WIFI)
	{
		dmsg("WIFI IS AVAIL");
		iRet = 1;
	}
	else
	{
		dmsg("WIFI IS NOT AVAIL");
		iRet = 0;
	}

	return iRet;
}

int IsETHERNET(void)
{
	int iRetComm = 0;
	int iRet = 0;

	iRetComm = checkCommIsAvailable();
	if(iRetComm & COM_LAN_1 || iRetComm & COM_LAN_2)
	{
		dmsg("ETHERNET IS AVAIL");
		iRet = 1;
	}
	else
	{
		dmsg("ETHERNET IS NOT AVAIL");
		iRet = 0;
	}

	return iRet;
}
//@@WAY UPDATED, 20190901
//@@WAY CHANGED, 20190801


