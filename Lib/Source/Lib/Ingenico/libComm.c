
#include "libMtiCommonApi.h"
#include "IP_.h"
#include "ExtraGPRS.h"
#include "dll_hwcnf.h"

#define MAX_COMM_COUNT		8

static tCommInfo g_tCommInfo[MAX_COMM_COUNT];
static LL_HANDLE g_lhndTable[MAX_COMM_COUNT];
static INT g_ucInitCommVar = FALSE;
static LL_HANDLE g_hDebugSession = NULL;
static INT g_ucGprsStart = FALSE;
static INT g_iConnectStatus = 0;
static INT g_iConnectResult = 0;
static INT g_iHangUp = FALSE;

CHAR GPRSIccId[20+1];	// @@WAY CHANGE PART, 20191001

typedef struct _tCellInfo {
	CHAR caGprsAPN[GPRS_APN_LENGTH];
	CHAR caGprsUserName[GPRS_USERNAME_LENGTH];
	CHAR caGprsPassword[GPRS_PASSWORD_LENGTH];
	INT iMNC;
	INT iCellID;
	INT iLAC;
} tCellInfo;

static tCellInfo g_tCellInfo;

#define CONN_CONTINUE			1
#define CONN_ERROR				2
#define CONN_CANCEL				3

char g_caDebugData[8192 + 1];
int g_iDebugLen;
long long g_lDebugTick;

enum {
	THREAD_CONN_READY,
	THREAD_TRY_CONN,
	THREAD_CONN_FAULT,
	THREAD_CONN_SUCCESS,
	THREAD_ERROR,
} THREAD_STATUS;

static VOID* threadConnection(VOID *pParam);
static VOID* threadPreConnection(VOID *pParam);
static T_OSL_HTHREAD g_hThreadPre = NULL;
static T_OSL_HTHREAD g_hThreadConn = NULL;
static T_OSL_HMUTEX g_hMutexPre = NULL;
static INT g_iStatus = THREAD_ERROR;

#define CHANGE_STATUS(X)								\
	if (g_hMutexPre != NULL) 	  {						\
	OSL_Mutex_Lock(g_hMutexPre, OSL_TIMEOUT_INFINITE);	\
	g_iStatus = X;										\
	OSL_Mutex_Unlock(g_hMutexPre); }

#define READ_STATUS(X)				X = g_iStatus;

INT MtiGetEDCModelName()
{

	UCHAR ucRange = 0;
	UCHAR ucProduct;

	ucRange = PSQ_Get_product_type(&ucProduct);

	switch(ucRange)
	{
		case TYPE_TERMINAL_IWL220:
			return EDC_MODEL_IWL220;
			break;
		case TYPE_TERMINAL_ICT220:
			return EDC_MODEL_ICT220;
			break;
		case TYPE_TERMINAL_ICT250:
			return EDC_MODEL_ICT250;
			break;
		case TYPE_TERMINAL_M2500:
			return EDC_MODEL_M2500;
			break;
		case TYPE_TERMINAL_M3500:
			return EDC_MODEL_M3500;
			break;
		default:
			return EDC_MODEL_UNKNOWN;
	}

	return EDC_MODEL_UNKNOWN;

}


INT mtiGetNetInfo(INT iNetType, tNetInfo *tpNetInfo)
{
	INT iRet = 0, iInterface = 0;
	ip_param_infos_s tHwInfo;
	CHAR *caNetStatMsg[3] = {"ERROR", "ONLINE", "OFFLINE"};

	dmsg("[FUNC] mtiGetNetInfo(%d, %p)", iNetType, tpNetInfo);

	if (tpNetInfo == NULL)
	{
		return RTN_ERROR;
	}

	mtiMemset(&tHwInfo, 0, sizeof(ip_param_infos_s));
	switch(iNetType)
	{
		case COMMTYPE_GPRS:
			iInterface = GPRS_INTERFACE;
			break;

		case COMMTYPE_WIFI:
			iInterface = WIFI_INTERFACE;
			break;

		case COMMTYPE_ETHERNET:
			iInterface = ETHERNET_INTERFACE;
			break;

		default:
			break;
	}

	if (iInterface > 0)
	{
		iRet = HWCNF_GetIPInfos(iInterface, &tHwInfo);

		dmsg("[%d] - HWCNF_GetIPInfos() Result = %d", iInterface, iRet);
		if (iRet == HWCNF_SETIP_OK)
		{
			dbuf("IP INFO", (UCHAR*)&tHwInfo, sizeof(ip_param_infos_s));

			sprintf(tpNetInfo->caAddr, "%d.%d.%d.%d",
					tHwInfo.addr[0],tHwInfo.addr[1],tHwInfo.addr[2],tHwInfo.addr[3]);

			sprintf(tpNetInfo->caGateway, "%d.%d.%d.%d",
					tHwInfo.gateway[0],tHwInfo.gateway[1],tHwInfo.gateway[2],tHwInfo.gateway[3]);

			sprintf(tpNetInfo->caNetmask, "%d.%d.%d.%d",
					tHwInfo.netmask[0],tHwInfo.netmask[1],tHwInfo.netmask[2],tHwInfo.netmask[3]);

			tpNetInfo->iNetStatus = COMM_NET_STAT_ONLINE;

			if (tHwInfo.boot_proto == HWCNF_GETIP_BOOT_PROTO_DHCP)
			{
				tpNetInfo->iDHCP_Enabled = TRUE;
			}

			if (iInterface == GPRS_INTERFACE)
			{
				tpNetInfo->iLAC = g_tCellInfo.iLAC;
				tpNetInfo->iCellID = g_tCellInfo.iCellID;
				tpNetInfo->iMNC = g_tCellInfo.iMNC;
			}

			dmsg("--------------------------------------------------");
			dmsg("                      IP INFO");
			dmsg("--------------------------------------------------");
			dmsg("NETWORK STATUS -> [%s]", caNetStatMsg[tpNetInfo->iNetStatus]);
			dmsg("DHCP ENABLED[%d]", tpNetInfo->iDHCP_Enabled);
			if(iNetType == COMMTYPE_ETHERNET)
			{
				dmsg("ETHERNET LINK -> [%d]", tpNetInfo->iEthLink);
			}
			dmsg("IP ADDR[%s]", tpNetInfo->caAddr);
			dmsg("NETMASK[%s]", tpNetInfo->caNetmask);
			dmsg("GATEWAY[%s]", tpNetInfo->caGateway);
			dmsg("LAC    [%d]", tpNetInfo->iLAC);
			dmsg("Cell ID[%d]", tpNetInfo->iCellID);
			dmsg("MNC    [%d]", tpNetInfo->iMNC);
			dmsg("--------------------------------------------------");
			return RTN_SUCCESS;
		}
		else if (iRet == HWCNF_SETIP_INTERFACE_GPRS_NOT_CONNECTED)
		{
			tpNetInfo->iNetStatus = COMM_NET_STAT_OFFLINE;
		}
		else
		{
			tpNetInfo->iNetStatus = COMM_NET_STAT_ERROR;
		}
	}
	return RTN_ERROR;
}

// to prevent serial port collision !!
#define DEBUG_PORT_NAME			LL_PHYSICAL_V_USB

void debugString(char *ucpString)
{
	int iLength = 0;

	if (g_hDebugSession != NULL)
	{
		while(ucpString[iLength] != 0)
		{
			iLength++;
		}

		LL_Send(g_hDebugSession, iLength, ucpString, LL_INFINITE);
	}
}

void debugSend(unsigned char *ucpData, int iLength)
{
	if (g_hDebugSession != NULL)
		LL_Send(g_hDebugSession, iLength, ucpData, LL_INFINITE);
}

void debugStop()
{
	if (g_hDebugSession != NULL)
	{
		LL_Disconnect(g_hDebugSession);
		g_hDebugSession = NULL;
	}
}

void debugStart(INT iPortName)
{
	UCHAR ucDatasize = 0;
	UCHAR ucParity = 0;
	UCHAR ucStopbits = 0;
	UINT  uiBps = 0;
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;

	static UCHAR ucComNoPre = DEBUG_PORT_NAME;
	UCHAR ucComNo = 0;
	INT iRet = 0;

	switch (iPortName)
	{
		case COMMCHANNEL_COM0:
			ucComNo = LL_PHYSICAL_V_COM0;
		break;

		case COMMCHANNEL_COM1:
			ucComNo = LL_PHYSICAL_V_COM1;
		break;

		case COMMCHANNEL_COM2:
			ucComNo = LL_PHYSICAL_V_COM2;
		break;

		case COMMCHANNEL_DEV_USB:
			ucComNo = LL_PHYSICAL_V_USB;
		break;

		default:
			ucComNo = DEBUG_PORT_NAME;
		break;
	}

	if (g_hDebugSession != NULL)
	{
		if(ucComNoPre!=ucComNo)
			debugStop();
	}

	ucComNoPre = ucComNo;

	/** N,8,1 **/
	ucDatasize = LL_PHYSICAL_V_8_BITS;
	ucParity = LL_PHYSICAL_V_NO_PARITY;
	ucStopbits = LL_PHYSICAL_V_1_STOP;

	/** BAUDRATE **/
	uiBps = LL_PHYSICAL_V_BAUDRATE_115200;

	// USB TEST
	//ucComNo = LL_PHYSICAL_V_USB;

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                 // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters (part1)
	// =================================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG, // TAG Physical layer parameters
										NULL,                         // VALUE (Null)
										0);                           // LENGTH 0

	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                       // TAG
							ucComNo,                                 // VALUE
							LL_PHYSICAL_L_LINK);                      // LENGTH 1 byte

	// Baud Rate
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BAUDRATE,                   // TAG
							uiBps,                                    // VALUE (Integer)
							LL_PHYSICAL_L_BAUDRATE);                  // LENGTH 4 bytes

	// Data bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BITS_PER_BYTE,              // TAG
							ucDatasize,                               // VALUE
							LL_PHYSICAL_L_BITS_PER_BYTE);             // LENGTH 1 byte

	// Stop bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_STOP_BITS,                  // TAG
							ucStopbits,                               // VALUE
							LL_PHYSICAL_L_STOP_BITS);                 // LENGTH 1 byte

	// Parity
	// ------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_PARITY,                     // TAG
							ucParity,                                 // VALUE
							LL_PHYSICAL_L_PARITY);                    // LENGTH 1 byte

	// Link Layer configuration
	iRet = LL_Configure(&g_hDebugSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
		return;

	if (piConfig)
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak

	iRet = LL_Connect(g_hDebugSession);
	switch (iRet)
	{
		case LL_ERROR_OK:
			iRet = RTN_SUCCESS;
			break;

		default:
			mtiSoundBeep(50, 2);
			break;
	}
}

void debugBuffer( char* ucpTitle, unsigned char* ucpData, int iSizeData, char* cpFile, int iLine )
{
	int i;
	int iNo = 0;
	int iY = 0;

	char caBuf[64];

	dmsg("[TRACE] %s : %d [File:%s, Line:%d]", ucpTitle, iSizeData, cpFile, iLine );
	dmsg("==========================================================================");
	dmsg("   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  |                   ");
	dmsg("--------------------------------------------------------------------------");
	dmsgnoln("%02d : ", iNo++ );
	memset( caBuf, 0, sizeof(caBuf) );
	for( i = 0; i < iSizeData; i++ )
	{
		dmsgnoln("%02X ", (UCHAR)ucpData[i] );

		caBuf[iY] = ucpData[i];

		if(caBuf[iY] == 0x00)
			caBuf[iY] = 0x20;
		else if(caBuf[iY] > 0x00 && caBuf[iY] <= 0x1F)
			caBuf[iY] = '^';

		iY++;

		if( (i+1)%16 == 0 && (i != (iSizeData - 1)))
		{
			caBuf[iY] = 0x00;

			dmsgnoln(" :  %s \r\n", caBuf);
			dmsgnoln("%02d : ", iNo++ );

			memset(caBuf, 0, sizeof(caBuf));
			iY = 0;
		}
	}

	if( iY > 0 )
	{
		char caDug[256];

		memset(caDug, 0, sizeof(caBuf) );
		memset(caDug, 0x20, sizeof(caBuf) - 1);

		sprintf(&caDug[3*(16-iY)], " :  %s \r\n", caBuf);
		dmsgnoln(caDug);
	}

	dmsgnoln("\r\n");
}

int debugIsUsingDebugPort(int nPortName)
{
	INT nDebugPortName = COMMCHANNEL_COM0;

	switch (nPortName)
	{
		case LL_PHYSICAL_V_COM0:
			nDebugPortName = COMMCHANNEL_COM0;
			break;

		case LL_PHYSICAL_V_COM1:
			nDebugPortName = COMMCHANNEL_COM1;
			break;

		case LL_PHYSICAL_V_COM2:
			nDebugPortName = COMMCHANNEL_COM2;
			break;

		case LL_PHYSICAL_V_USB:
			nDebugPortName = COMMCHANNEL_DEV_USB;
			break;

	}

	if (nPortName == nDebugPortName)
	{
		return TRUE;
	}

	return FALSE;
}

static int GprsReport()
{
	// Local variables
    // ***************
    Telium_File_t *hGprs = NULL;
    T_EGPRS_GET_INFORMATION xInfo;
    INT iRet;
    CHAR caTemp[16 + 1];

    // @@WAY CHANGE PART, 20200129
#if 1

#ifndef __TETRA__
    // Get state of GPRS driver
	// ************************
	hGprs = Telium_Stdperif((char*)"DGPRS", NULL);
	if (hGprs == NULL)
	{
		return RTN_ERROR;
	}
#endif

#else
	// Get state of GPRS driver
	// ************************
    hGprs = Telium_Stdperif((char*)"DGPRS", NULL);
	if (hGprs == NULL)
	{
		return RTN_ERROR;
	}
#endif
	//

	iRet = gprs_SetPinModeFree(hGprs, 1);
	if (iRet != 0)
	{
		return RTN_ERROR;
	}

	iRet = gprs_GetInformation(hGprs, &xInfo, sizeof(xInfo));
	if (iRet != 0)
	{
		return RTN_ERROR;
	}

	 g_tCellInfo.iCellID = xInfo.cell_id;
	 g_tCellInfo.iLAC = xInfo.location_area_code;
	 mtiMemset(caTemp, 0, sizeof(caTemp));
	 sprintf(caTemp, "%02X", (UCHAR)(xInfo.plmn & 0xFF));
	 g_tCellInfo.iMNC = mtiAtoi(2, (UCHAR*)caTemp);

	// @@WAY CHANGE PART, 20191001
	memset(GPRSIccId, 0, sizeof(GPRSIccId));
	// @@WAY CHANGE PART, 20200129
#if 1
#ifdef __TETRA__
	memcpy(GPRSIccId, xInfo.simIccId, mtiStrlen(19, xInfo.simIccId));
#else
	memcpy(GPRSIccId, xInfo.simIccId, mtiStrlen(20, xInfo.simIccId));
#endif
#else
	memcpy(GPRSIccId, xInfo.simIccId, mtiStrlen(20, xInfo.simIccId));
#endif
	//
	//

	// GPRS sim status
	// ===============
	switch (xInfo.sim_status)
	{
		/****
		case EGPRS_SIM_NOT_RECEIVED_YET:
			iRet = LL_GSM_Start(NULL);
			if (iRet != LL_ERROR_OK)
			{
				iRet = RTN_ERROR;
				CHK_ERR();
			}

			dmsg("**** LL_GSM_Start GOOD!");
			iRet = RTN_SUCCESS;
			break;
		****/

		case EGPRS_SIM_OK:
			iRet = LL_GPRS_Start(NULL, g_tCellInfo.caGprsAPN);
			if (iRet != LL_ERROR_OK)
			{
				return RTN_ERROR;
			}

			dmsg("**** LL_GPRS_Start GOOD!");
			iRet = RTN_SUCCESS;
			break;

		case EGPRS_SIM_KO:
		case EGPRS_SIM_NOT_INSERTED:
			iRet = RTN_COMM_GPRS_SIM_PROBLEM;
			break;

		case EGPRS_SIM_PIN_REQUIRED:
		case EGPRS_SIM_PIN2_REQUIRED:
		case EGPRS_SIM_PIN_ERRONEOUS:
			iRet = RTN_COMM_GPRS_REQ_PIN;
			break;

		case EGPRS_SIM_PUK_REQUIRED:
		case EGPRS_SIM_PUK2_REQUIRED:
			iRet = RTN_COMM_GPRS_REQ_PUK;
			break;
	}

	return iRet;
}

static INT StartGPRS(ULONG ulTimeout, INT iReportCheck)
{
	// Local variables
    // ***************
    INT iExitFlag = TRUE, iKbdFlag = FALSE;
    INT iStatus, iRet = RTN_ERROR;
    ULONG ulTickMax = 0;
    INT iTimeout = 10 * 100; // @@WAY, 20190506 NETWORK
    // Connect the GPRS network
    // ************************
    if (!mtiIsValildKeyboard())
    {
    	iKbdFlag = TRUE;
    }
    else
    {
    	mtiInitializeKeyboard();
    }

    if (!g_ucGprsStart)
    {
    	dmsg("**** g_ucGprsStart = %d", g_ucGprsStart);
    	return iRet;
    }

    ulTickMax = mtiGetTickCount() + ulTimeout;
    do
    {
    	iRet = GprsReport();
    	if (iReportCheck)
    	{
			dmsg("GprsReport() = %d", iRet);
			if (iRet != RTN_SUCCESS)
			{
				return iRet;
			}
    	}
    	else
    	{
    		if (iRet == RTN_COMM_GPRS_SIM_PROBLEM)
    		{
    			return iRet;
    		}
    	}

		//@@WAY PREDIAL 20210607
		iRet = LL_Network_GetStatus(LL_PHYSICAL_V_GPRS, &iStatus);
		dmsg("LL_Network_GetStatus() Result = %d", iRet);
		dmsg("**** status value = %X", iStatus);
		if(iStatus == LL_STATUS_GPRS_CONNECTED)
		{
			iRet = RTN_SUCCESS;
			iExitFlag = TRUE;
			break;
		}
		//

#if 1   // @@WAY, 20190506 NETWORK
    	iRet = LL_GPRS_Connect(g_tCellInfo.caGprsAPN, g_tCellInfo.caGprsUserName, g_tCellInfo.caGprsPassword, iTimeout); // max timeout 10ms from lib
#else
    	iRet = LL_GPRS_Connect(g_tCellInfo.caGprsAPN, g_tCellInfo.caGprsUserName, g_tCellInfo.caGprsPassword, ulTimeout);
#endif
    	dmsg("LL_GPRS_Connect(%s, %s, %s) Result = %d", g_tCellInfo.caGprsAPN, g_tCellInfo.caGprsUserName, g_tCellInfo.caGprsPassword, iRet);
    	switch (iRet)
    	{
			case LL_ERROR_ALREADY_CONNECTED:
			case LL_ERROR_OK:
				iRet = RTN_SUCCESS;
				iExitFlag = TRUE;
				break;

			case LL_ERROR_NETWORK_NOT_SUPPORTED:
			case LL_ERROR_SERVICE_CALL_FAILURE:
				iExitFlag = TRUE;
				break;

			default:                                   // LL_ERROR_NETWORK_NOT_READY or LL_ERROR_NETWORK_ERROR
				iRet = LL_Network_GetStatus(LL_PHYSICAL_V_GPRS, &iStatus);
				dmsg("LL_Network_GetStatus() Result = %d", iRet);
				dmsg("**** status value = %X", iStatus);
				switch (iStatus)                       // Check network status
				{
					case LL_STATUS_GPRS_ERROR_SIM_LOCK:    // Pin locked => Wait Puk
					case LL_STATUS_GPRS_ERROR_BAD_PIN:     // Wrong Pin => Wait Pin
					case LL_STATUS_GPRS_ERROR_NO_PIN:      // Pin required => Wait Pin
					case LL_STATUS_GPRS_CONNECTING:        // Connection to GPRS Network in progress
					case LL_STATUS_GPRS_AVAILABLE:         // GPRS Network is available
					case LL_STATUS_GPRS_CONNECTING_PPP:    // PPP connection is in progress with the GPRS provider
					case LL_STATUS_GPRS_DISCONNECTED:      // Disconnection to GPRS Network, let's retry a connection
					case 0x2000600:                        // Wait until network ready
						iExitFlag = FALSE;
						Telium_Ttestall(0, 4*100);			// @@WAY, 201904 NETWORK
						break;

#ifdef LL_STATUS_GPRS_MODULE_OFF// @@ WIDYA - Network for EDC New Part
					case LL_STATUS_GPRS_MODULE_OFF:
						iExitFlag = FALSE;
						Telium_Ttestall(0, TIMEOUT_3S);
						break;
#endif

					case LL_STATUS_GPRS_CONNECTED:
						iRet = RTN_SUCCESS;
						iExitFlag = TRUE;
						break;

					case LL_STATUS_GPRS_ERROR_NO_SIM:      // No SIM card is inserted into the terminal
					case LL_STATUS_GSM_ERROR_NO_SIM:
						iRet = RTN_COMM_GPRS_SIM_PROBLEM;
						iExitFlag = TRUE;
						break;

					case LL_STATUS_GPRS_ERROR_PPP:         // Error occurred during the PPP link establishment.
					case LL_STATUS_GPRS_ERROR_UNKNOWN:     // Error status unknown
					default:
						iExitFlag = TRUE;
						break;
				}
				break;
     	}

		if (mtiGetKeyPress() == KEY_CANCEL)
		{
			iExitFlag = TRUE;
		}
    }
    while (!iExitFlag && (mtiGetTickCount() < ulTickMax));

    if (!iKbdFlag)
    {
    	mtiFinalizeKeyboard();
    }

	dmsg("StartGPRS() Result = %d", iRet);
	return iRet;
}

LL_HANDLE mtiSetGPRSCommConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	// Local variables
    // ***************
	// Tlv tree nodes
	// ==============
	TLV_TREE_NODE piConfig=NULL;
	TLV_TREE_NODE piPhysicalConfig=NULL;
	TLV_TREE_NODE piTransportConfig=NULL;
	LL_HANDLE hSession = NULL;
    INT iRet;

    // Create parameters tree
    // ======================
    piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                   // LinkLayer parameters Root tag of the configuration tree

    // Physical layer parameters
	// =========================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
		                                LL_TAG_PHYSICAL_LAYER_CONFIG,   // TAG Physical layer parameters
									    NULL,                           // VALUE (Null)
									    0);                             // LENGTH 0

	// GPRS
	// ----
    TlvTree_AddChildInteger(piPhysicalConfig,
		                    LL_PHYSICAL_T_LINK,                         // TAG
							LL_PHYSICAL_V_GPRS,                         // VALUE
							LL_PHYSICAL_L_LINK);                        // LENGTH 1

	// Transport and network layer parameters
	// ======================================
	piTransportConfig = TlvTree_AddChild(piConfig,
		                                 LL_TAG_TRANSPORT_LAYER_CONFIG, // TAG Transport layer parameters
										 NULL,                          // VALUE (Null)
										 0);                            // LENGTH 0

	// TCP/IP
	// ------
	TlvTree_AddChildInteger(piTransportConfig,
		                    LL_TRANSPORT_T_PROTOCOL,                    // TAG
							LL_TRANSPORT_V_TCPIP,                       // VALUE
							LL_TRANSPORT_L_PROTOCOL);                   // LENGTH 1 byte

	// Host Name
	// ---------
	TlvTree_AddChildString(piTransportConfig,
		                   LL_TCPIP_T_HOST_NAME,                        // TAG
						   tpCommInfo->cpRemoteIP);                                     // VALUE
                                                                        // LENGTH (strlen addr)

	// Port
	// ----
	TlvTree_AddChildInteger(piTransportConfig,
		                    LL_TCPIP_T_PORT,                            // TAG
							tpCommInfo->iRemotePort,                               // VALUE (Integer)
							LL_TCPIP_L_PORT);                           // LENGTH 4 bytes



	// Connection timeout
	// ------------------
    TlvTree_AddChildInteger(piTransportConfig,
                            LL_TCPIP_T_CONNECT_TIMEOUT,                 // TAG
							(UINT)ulTimeout,                            // Value (Integer)
                            LL_TCPIP_L_CONNECT_TIMEOUT);                // LENGTH 4 bytes

	// Link Layer configuration
	// ************************
    iRet = LL_Configure(&hSession, piConfig);                           // Initialize the handle of the session
    if (iRet != LL_ERROR_OK) //@@WAY, 20190628
	{
		dpt();
		return NULL;
	}

	// Errors treatment
    // ****************
    if (piConfig)
    {
    	TlvTree_Release(piConfig);                                      // Release tree to avoid memory leak
    }

	return hSession;                                                    // Return the handle of the session
}

LL_HANDLE mtiSetWifiCommConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;
	TLV_TREE_NODE piTransportConfig = NULL;
	LL_HANDLE hSession = NULL;                                          // Session handle
	int iRet;

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                   // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters
	// =========================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG,   // TAG Physical layer parameters
										NULL,                           // VALUE (Null)
										0);                             // LENGTH 0

	// Ethernet
	// --------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                         // TAG
							LL_PHYSICAL_V_WIFI,                     // VALUE
							LL_PHYSICAL_L_LINK);                        // LENGTH 1

	// Transport and network layer parameters
	// ======================================
	piTransportConfig = TlvTree_AddChild(piConfig,
										 LL_TAG_TRANSPORT_LAYER_CONFIG, // TAG Transport layer parameters
										 NULL,                          // VALUE (Null)
										 0);                            // LENGTH 0

	// TCP/IP
	// ------
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TRANSPORT_T_PROTOCOL,                    // TAG
							LL_TRANSPORT_V_TCPIP,                       // VALUE
							LL_TRANSPORT_L_PROTOCOL);                   // LENGTH 1 byte

	// Host Name
	// ---------
	TlvTree_AddChildString(piTransportConfig,
						   LL_TCPIP_T_HOST_NAME,                        // TAG
						   tpCommInfo->cpRemoteIP);                     // VALUE
																		// LENGTH (strlen addr)
	// Port
	// ----
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TCPIP_T_PORT,                            // TAG
							tpCommInfo->iRemotePort,                    // VALUE (Integer)
							LL_TCPIP_L_PORT);                           // LENGTH 4 bytes

	// Connection timeout
	// ------------------
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TCPIP_T_CONNECT_TIMEOUT,                 // TAG
							(UINT)ulTimeout,                            // Value (Integer)
							LL_TCPIP_L_CONNECT_TIMEOUT);                // LENGTH 4 bytes

	// Link Layer configuration
	// ************************
	iRet = LL_Configure(&hSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
	{
		dpt();
		return NULL;
	}

	if (piConfig)
	{
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak
	}

	return hSession;
}

INT mtiScanWiFi(tWifiScanAP *tpWifiScanList, INT *iScanCnt, INT iCurCommType)
{
	return RTN_ERROR;
}

LL_HANDLE mtiSetEthernetCommConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;
	TLV_TREE_NODE piTransportConfig = NULL;
	LL_HANDLE hSession = NULL;                                          // Session handle
	int iRet;

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                   // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters
	// =========================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG,   // TAG Physical layer parameters
										NULL,                           // VALUE (Null)
										0);                             // LENGTH 0

	// Ethernet
	// --------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                         // TAG
							LL_PHYSICAL_V_ETHERNET,                     // VALUE
							LL_PHYSICAL_L_LINK);                        // LENGTH 1

	// Transport and network layer parameters
	// ======================================
	piTransportConfig = TlvTree_AddChild(piConfig,
										 LL_TAG_TRANSPORT_LAYER_CONFIG, // TAG Transport layer parameters
										 NULL,                          // VALUE (Null)
										 0);                            // LENGTH 0

	// TCP/IP
	// ------
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TRANSPORT_T_PROTOCOL,                    // TAG
							LL_TRANSPORT_V_TCPIP,                       // VALUE
							LL_TRANSPORT_L_PROTOCOL);                   // LENGTH 1 byte

	// Host Name
	// ---------
	TlvTree_AddChildString(piTransportConfig,
						   LL_TCPIP_T_HOST_NAME,                        // TAG
						   tpCommInfo->cpRemoteIP);                     // VALUE
																		// LENGTH (strlen addr)
	// Port
	// ----
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TCPIP_T_PORT,                            // TAG
							tpCommInfo->iRemotePort,                    // VALUE (Integer)
							LL_TCPIP_L_PORT);                           // LENGTH 4 bytes

	// Connection timeout
	// ------------------
	TlvTree_AddChildInteger(piTransportConfig,
							LL_TCPIP_T_CONNECT_TIMEOUT,                 // TAG
							(UINT)ulTimeout,                            // Value (Integer)
							LL_TCPIP_L_CONNECT_TIMEOUT);                // LENGTH 4 bytes

	// Link Layer configuration
	// ************************
	iRet = LL_Configure(&hSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
	{
		dpt();
		return NULL;
	}

	if (piConfig)
	{
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak
	}

	return hSession;
}

LL_HANDLE mtiSetSerialCommConfigure(tCommInfo *tpCommInfo)
{
	UCHAR ucDatasize = 0;
	UCHAR ucParity = 0;
	UCHAR ucStopbits = 0;
	UINT  uiBps = 0;
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;
	LL_HANDLE hSession = NULL;
	UCHAR ucComNo = 0;
	INT iRet = 0;

	if (NULL == tpCommInfo)
		return NULL;

	/** N,8,1 **/
	ucDatasize = LL_PHYSICAL_V_8_BITS;
	ucParity = LL_PHYSICAL_V_NO_PARITY;
	ucStopbits = LL_PHYSICAL_V_1_STOP;

	/** BAUDRATE **/
	uiBps = LL_PHYSICAL_V_BAUDRATE_9600;
	switch (tpCommInfo->iCommBaudrate)
	{
		case COMMBAUD_19200:
			uiBps = LL_PHYSICAL_V_BAUDRATE_19200;
			break;

		case COMMBAUD_38400:
			uiBps = LL_PHYSICAL_V_BAUDRATE_38400;
			break;

		case COMMBAUD_57600:
			uiBps = LL_PHYSICAL_V_BAUDRATE_57600;
			break;

		case COMMBAUD_115200:
			uiBps = LL_PHYSICAL_V_BAUDRATE_115200;
			break;
	}

	ucComNo = LL_PHYSICAL_V_COM0;
	switch (tpCommInfo->iCommChannel)
	{
		case COMMCHANNEL_COM1:
			ucComNo = LL_PHYSICAL_V_COM1;
			break;

		case COMMCHANNEL_COM2:
			ucComNo = LL_PHYSICAL_V_COM2;
			break;

		case COMMCHANNEL_DEV_USB:
			ucComNo = LL_PHYSICAL_V_USB;
			break;
	}
	dmsg("ucComNo = %d", ucComNo);

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                 // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters (part1)
	// =================================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG, // TAG Physical layer parameters
										NULL,                         // VALUE (Null)
										0);                           // LENGTH 0

	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                       // TAG
							ucComNo,                                 // VALUE
							LL_PHYSICAL_L_LINK);                      // LENGTH 1 byte

	// Baud Rate
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BAUDRATE,                   // TAG
							uiBps,                                    // VALUE (Integer)
							LL_PHYSICAL_L_BAUDRATE);                  // LENGTH 4 bytes

	// Data bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BITS_PER_BYTE,              // TAG
							ucDatasize,                               // VALUE
							LL_PHYSICAL_L_BITS_PER_BYTE);             // LENGTH 1 byte

	// Stop bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_STOP_BITS,                  // TAG
							ucStopbits,                               // VALUE
							LL_PHYSICAL_L_STOP_BITS);                 // LENGTH 1 byte

	// Parity
	// ------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_PARITY,                     // TAG
							ucParity,                                 // VALUE
							LL_PHYSICAL_L_PARITY);                    // LENGTH 1 byte

	// Link Layer configuration
	iRet = LL_Configure(&hSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
	{
		return NULL;
	}

	if (piConfig)
	{
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak
	}

	return hSession;

}

#if 0
LL_HANDLE mtiSetFaxModemConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	UCHAR ucDatasize = 0;
	UCHAR ucParity = 0;
	UCHAR ucStopbits = 0;
	UCHAR ucBlindDial = 0;
	UCHAR ucFast = 0;
	UCHAR ucCompression = 0;;
	UCHAR ucCorrection = 0;
	UCHAR ucHangUpDtr = 0;
	UCHAR ucIsSync = 0;
	UINT  uiBps = 0;
	//UCHAR ucProduct;

	/** UCHAR ucProduct; **/
	CHAR caInitString[100 + 1];
	CHAR caCnxInitString[100 + 1];
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;
	TLV_TREE_NODE piDataLinkConfig = NULL;
	LL_HANDLE hSession = NULL;
	INT iRet = 0;

	CHAR caTemp[100];

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_DATA_COMPRESSION))
	{
		ucCompression = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_CORRECTION))
	{
		ucCorrection = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_HANGUP_DTR))
	{
		ucHangUpDtr = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_BLIND_DIAL))
	{
		ucBlindDial = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_ASYNC_MODEM))
	{
		ucIsSync = 0;
	}
	else
	{
		ucIsSync = 1;
	}

	/** N,8,1 **/
	ucDatasize = LL_PHYSICAL_V_8_BITS;
	ucParity = LL_PHYSICAL_V_NO_PARITY;
	ucStopbits = LL_PHYSICAL_V_1_STOP;

	/** BAUDRATE **/
	switch (tpCommInfo->iCommBaudrate)
	{
		case COMMBAUD_1200:
		case COMMBAUD_2400:
			ucFast = 1;
			break;

		default:
			ucFast = 0;
			break;
	}
	uiBps = LL_PHYSICAL_V_BAUDRATE_115200;

	/** TIMEOUT **/
	if (!ulTimeout)
	{
		ulTimeout = 60 * 100;
	}
	dmsg("----- MODEM TIME OUT : %lu", ulTimeout);

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                 // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters (part1)
	// =================================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG, // TAG Physical layer parameters
										NULL,                         // VALUE (Null)
										0);                           // LENGTH 0
	// Modem
	// -----
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                       // TAG
							LL_PHYSICAL_V_MODEM,                      // VALUE
							LL_PHYSICAL_L_LINK);                      // LENGTH 1 byte

	// Baud Rate
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BAUDRATE,                   // TAG
							uiBps,                                    // VALUE (Integer)
							LL_PHYSICAL_L_BAUDRATE);                  // LENGTH 4 bytes

	// Data bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BITS_PER_BYTE,              // TAG
							ucDatasize,                               // VALUE
							LL_PHYSICAL_L_BITS_PER_BYTE);             // LENGTH 1 byte

	// Stop bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_STOP_BITS,                  // TAG
							ucStopbits,                               // VALUE
							LL_PHYSICAL_L_STOP_BITS);                 // LENGTH 1 byte

	// Parity
	// ------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_PARITY,                     // TAG
							ucParity,                                 // VALUE
							LL_PHYSICAL_L_PARITY);                    // LENGTH 1 byte

	// Modem type
	// ----------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_TYPE,                          // TAG
							LL_MODEM_V_TYPE_STANDARD,                 // VALUE
							LL_MODEM_L_TYPE);                         // LENGTH 1 byte

	// Dial Timeout
	// ------------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_DIAL_TIMEOUT,                  // TAG
							(UINT)ulTimeout,                          // VALUE (Integer)
							LL_MODEM_L_DIAL_TIMEOUT);                 // LENGTH 4 bytes

	// Command Line Terminator
	// -----------------------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_CMD_TERMINATOR,                // TAG
							LL_MODEM_V_CMD_TERMINATOR_CR,             // VALUE
							LL_MODEM_L_CMD_TERMINATOR);               // LENGTH 1 byte

	dmsg("ucIsSync = %d", ucIsSync);


	// Modem initialization string
	// ---------------------------
	if (ucIsSync)
	{
		/** sprintf(caInitString, "ATE0%sS6=1$M251F4S144=16", ucBlindDial ? "X1":"X4"); **/
		    //sprintf(caInitString, "ATE0X1S6=1$M249$M251F4S144=16");
		if (ucFast)
		{
			sprintf(caInitString, "ATE0%s$M249$M251F4S144=16", ucBlindDial ? "X1":"X4");
		}
		else
		{
			sprintf(caInitString, "ATE0%s$M250F4S144=16", ucBlindDial ? "X1":"X4");
		}
		//                     ATE0X1$M249$M251F4S144=16 //MANDIRI 250
		//                     ATE0X1S6=1$M249$M251F4S144=16 //MANDIRI 250

		//sprintf(caInitString, "ATE0X1S144=16");
		/** sprintf(caCnxInitString, "ATE0%sS6=1;+MS=V22B;+ES=6,,8;S17=11", ucBlindDial ? "X1":"X4"); **/
		//sprintf(caCnxInitString, "ATE0X1S6=1;+MS=V22B;+ES=6,,8;S17=11");

		/*	1-12-2017, By Roh.
		please try change LL_MODEM_T_INIT_STRING_CONEXANT  (conexant value) with this :
		"AT&FX0&K0%C0\\N0M2L3;+ES=6,,8;+ESA=0,0,,,1,0;$F2S17=13"
		*/
		//ATE0+ES=6,,8;+ESA=0,0,,,1,0$F2S17=13

		//AT&F0%%C0    \\N0M2L3;+A8E=,,,0+ES=6,,8;+ESA=0,0,,,1,0$F2S17=13
		//AT&FX0&K0%%C0\\N0M2L3;+ES=6,,8;+ESA=0,0,,,1,0;$F2S17=13S6=0
		//sprintf(caCnxInitString, "AT&FX0&K0%%C0\\N0M2L3;+ES=6,,8;+ESA=0,0,,,1,0;$F2S17=13");
		                        //AT&F0%%C0    \\N0M2L3;+A8E=,,,0+ES=6,,8;+ESA=0,0,,,1,0$F2S17=13
								//AT&FX0&K0%%C0\\N0M2L3;+ES=6,,8;+ESA=0,0,,,1,0;$F2S17=13S

		if (ucBlindDial)
		{
			sprintf(caCnxInitString, "AT&F0X1%%C0\\N0M2L3;+A8E=,,,0\r\nAT+ES=6,,8;+ESA=0,0,,,1,0\r\nAT$F2S17=13");
		}
		else
		{
			sprintf(caCnxInitString, "AT&F0%%C0\\N0M2L3;+A8E=,,,0\r\nAT+ES=6,,8;+ESA=0,0,,,1,0\r\nAT$F2S17=13");
		}
	}
	else
	{
		sprintf(caInitString, "ATE0\\N%c%%C%c%s", ucCorrection ? '3':'0', ucCompression ? '1':'0', ucBlindDial ? "X1":"X4");
	}

	TlvTree_AddChildString(piPhysicalConfig,
							LL_MODEM_T_INIT_STRING,                    // TAG+
							caInitString);                             // VALUE

	TlvTree_AddChildString(piPhysicalConfig,
							LL_MODEM_T_INIT_STRING_CONEXANT,		   // TAG
							caCnxInitString);                          // VALUE
															  	  	   // LENGTH (strlen initString)

	mtiMemset(caTemp, 0, sizeof(caTemp));
	//sprintf(caTemp, "T%s", tpCommInfo->cpRemoteTelNo);
	sprintf(caTemp, "%s", tpCommInfo->cpRemoteTelNo);

	// Phone number
	// ------------
	TlvTree_AddChildString(piPhysicalConfig,
						   LL_MODEM_T_PHONE_NUMBER,                   // TAG
						   caTemp);                // VALUE
																	  // LENGTH (strlen buf)
	if (ucIsSync)
	{
		// Data link layer parameters
		// ==========================
		piDataLinkConfig = TlvTree_AddChild(piConfig,
											LL_TAG_DATA_LINK_LAYER_CONFIG, // TAG Data link layer parameters
											NULL,                     // VALUE (Null)
											0);                       // LENGTH 0

		// HDLC
		// ----
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_DATA_LINK_T_PROTOCOL,              // TAG
								LL_DATA_LINK_V_HDLC,                  // VALUE
								LL_DATA_LINK_L_PROTOCOL);             // LENGTH 1 byte

		// Connection timeout
		// ------------------
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_HDLC_T_CONNECT_TIMEOUT,            // TAG
								LL_HDLC_V_CONNECT_TIMEOUT_DEFAULT,	  // Value (Integer)
								LL_HDLC_L_CONNECT_TIMEOUT);           // LENGTH 4 bytes

		// Minimum tries for sending
		// -------------------------
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_HDLC_T_MIN_RESEND_REQUESTS,        // TAG
								2,                                    // VALUE
								LL_HDLC_L_MIN_RESEND_REQUESTS);       // LENGTH 1 byte
	}

	dmsg("MODEM INIT [%s]", caInitString);
	dmsg("MODEM INIT CNX [%s]", caCnxInitString);
	dmsg("tpCommInfo->cpRemoteTelNo [%s]", tpCommInfo->cpRemoteTelNo);

	// Link Layer configuration
	iRet = LL_Configure(&hSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
	{
		return NULL;
	}

	if (piConfig)
	{
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak
	}

	return hSession;
}
#else

LL_HANDLE mtiSetFaxModemConfigure(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	UCHAR ucDatasize = 0;
	UCHAR ucParity = 0;
	UCHAR ucStopbits = 0;
	UCHAR ucBlindDial = 0;
#ifndef __TETRA__
	UCHAR ucFast = 0;
#endif
	UCHAR ucCompression = 0;;
	UCHAR ucCorrection = 0;
	//UCHAR ucHangUpDtr = 0; //@@WAY, 201812 not used
	UCHAR ucIsSync = 0;
	UINT  uiBps = 0;
	//UCHAR ucProduct;

	/** UCHAR ucProduct; **/
	CHAR caInitString[100 + 1];
#ifndef __TETRA__
	CHAR caCnxInitString[100 + 1];
#endif
	TLV_TREE_NODE piConfig = NULL;
	TLV_TREE_NODE piPhysicalConfig = NULL;
	TLV_TREE_NODE piDataLinkConfig = NULL;
	LL_HANDLE hSession = NULL;
	INT iRet = 0;

	CHAR caTemp[100];

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_DATA_COMPRESSION))
	{
		ucCompression = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_CORRECTION))
	{
		ucCorrection = 1;
	}

	/* //@@WAY, 201812 not used
	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_HANGUP_DTR))
	{
		ucHangUpDtr = 1;
	}
	*/

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_BLIND_DIAL))
	{
		ucBlindDial = 1;
	}

	if (CHK_BIT(tpCommInfo->iCommOption, COMMOPT_ASYNC_MODEM))
	{
		ucIsSync = 0;
	}
	else
	{
		ucIsSync = 1;
	}

	/** N,8,1 **/
	ucDatasize = LL_PHYSICAL_V_8_BITS;
	ucParity = LL_PHYSICAL_V_NO_PARITY;
	ucStopbits = LL_PHYSICAL_V_1_STOP;

	// @@WAY, 20190506 NETWORK
#ifndef __TETRA__
	/** BAUDRATE **/
	switch (tpCommInfo->iCommBaudrate)
	{
		case COMMBAUD_1200:
			uiBps = LL_PHYSICAL_V_BAUDRATE_1200;
			ucFast = 1;
			break;
		case COMMBAUD_2400:
			uiBps = LL_PHYSICAL_V_BAUDRATE_2400;
			ucFast = 1;
			break;
		default:
			uiBps = LL_PHYSICAL_V_BAUDRATE_115200;
			ucFast = 0;
			break;
	}     
#else
	switch (tpCommInfo->iCommBaudrate)
	{
		case COMMBAUD_1200:
			uiBps = LL_PHYSICAL_V_BAUDRATE_1200;
			//ucFast = 1;
			break;
		case COMMBAUD_2400:
			uiBps = LL_PHYSICAL_V_BAUDRATE_2400;
			//ucFast = 1;
			break;
		default:
			uiBps = LL_PHYSICAL_V_BAUDRATE_115200;
			//ucFast = 0;
			break;
	}
#endif

	/** TIMEOUT **/
	if (!ulTimeout)
	{
		ulTimeout = 60 * 100;
	}
	dmsg("----- MODEM TIME OUT : %lu", ulTimeout);

	// Create parameters tree
	// ======================
	piConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);                 // LinkLayer parameters Root tag of the configuration tree

	// Physical layer parameters (part1)
	// =================================
	piPhysicalConfig = TlvTree_AddChild(piConfig,
										LL_TAG_PHYSICAL_LAYER_CONFIG, // TAG Physical layer parameters
										NULL,                         // VALUE (Null)
										0);                           // LENGTH 0
	// Modem
	// -----
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_LINK,                       // TAG
							LL_PHYSICAL_V_MODEM,                      // VALUE
							LL_PHYSICAL_L_LINK);                      // LENGTH 1 byte

	// Baud Rate
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BAUDRATE,                   // TAG
							uiBps,                                    // VALUE (Integer)
							LL_PHYSICAL_L_BAUDRATE);                  // LENGTH 4 bytes

	// Data bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_BITS_PER_BYTE,              // TAG
							ucDatasize,                               // VALUE
							LL_PHYSICAL_L_BITS_PER_BYTE);             // LENGTH 1 byte

	// Stop bits
	// ---------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_STOP_BITS,                  // TAG
							ucStopbits,                               // VALUE
							LL_PHYSICAL_L_STOP_BITS);                 // LENGTH 1 byte

	// Parity
	// ------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_PHYSICAL_T_PARITY,                     // TAG
							ucParity,                                 // VALUE
							LL_PHYSICAL_L_PARITY);                    // LENGTH 1 byte

	// Modem type
	// ----------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_TYPE,                          // TAG
							LL_MODEM_V_TYPE_STANDARD,                 // VALUE
							LL_MODEM_L_TYPE);                         // LENGTH 1 byte

	// Dial Timeout
	// ------------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_DIAL_TIMEOUT,                  // TAG
							(UINT)ulTimeout,                          // VALUE (Integer)
							LL_MODEM_L_DIAL_TIMEOUT);                 // LENGTH 4 bytes

#ifndef __TETRA__
	// Command Line Terminator
	// -----------------------
	TlvTree_AddChildInteger(piPhysicalConfig,
							LL_MODEM_T_CMD_TERMINATOR,                // TAG
							LL_MODEM_V_CMD_TERMINATOR_CR,             // VALUE
							LL_MODEM_L_CMD_TERMINATOR);               // LENGTH 1 byte
#else
	if (ucIsSync)
	{
		// Disconnection type
	    // ------------------
		TlvTree_AddChildInteger(piPhysicalConfig,
								LL_MODEM_T_DISCONNECTION_TYPE,        // TAG
								LL_MODEM_V_DISCONNECTION_TYPE_ATZ,    // LENGTH 1 byte
								LL_MODEM_L_DISCONNECTION_TYPE);       // LENGTH 1 byte
	}
	else
	{
		// Command Line Terminator
		// -----------------------
		TlvTree_AddChildInteger(piPhysicalConfig,
								LL_MODEM_T_CMD_TERMINATOR,                // TAG
								LL_MODEM_V_CMD_TERMINATOR_CR,             // VALUE
								LL_MODEM_L_CMD_TERMINATOR);               // LENGTH 1 byte
	}
#endif
	
	dmsg("ucIsSync = %d", ucIsSync);

	// Modem initialization string
	// ---------------------------
#ifndef __TETRA__
	if (ucIsSync)
	{
		if (ucFast)
		{
			sprintf(caInitString, "ATE0%s$M249$M251F4S144=16", ucBlindDial ? "X1":"X4"); 
		}
		else
		{
			sprintf(caInitString, "ATE0%s$M250F4S144=16", ucBlindDial ? "X1":"X4");
		}

		if (ucBlindDial)
		{
			sprintf(caCnxInitString, "AT&F0X1%%C0\\N0M2L3;+A8E=,,,0\r\nAT+ES=6,,8;+ESA=0,0,,,1,0\r\nAT$F2S17=13");
		}
		else
		{
			sprintf(caCnxInitString, "AT&F0%%C0\\N0M2L3;+A8E=,,,0\r\nAT+ES=6,,8;+ESA=0,0,,,1,0\r\nAT$F2S17=13");
		}
	}
	else
	{
		sprintf(caInitString, "ATE0\\N%c%%C%c%s", ucCorrection ? '3':'0', ucCompression ? '1':'0', ucBlindDial ? "X1":"X4");
	}
#else
	if(ucIsSync)
	{
		//@@CACING PENTEST strcpy(caInitString, "AT\\N0%C0;+A8E=,,,0\rAT+ES=6,,8;+ESA=0,0,,,1,0\rAT$F2;S17=13\r");
		sprintf(caInitString, "AT\\N0%C0;+A8E=,,,0\rAT+ES=6,,8;+ESA=0,0,,,1,0\rAT$F2;S17=13\r");
	}
	else
	{
        sprintf(caInitString, "ATE0\\N%c%%C%cW2;%s", ucCorrection ? '3':'0', ucCompression ? '1':'0', ucBlindDial ? "X1":"X4");
	}
#endif

	TlvTree_AddChildString(piPhysicalConfig,
							LL_MODEM_T_INIT_STRING,                    // TAG+
							caInitString);                             // VALUE

	dmsg("MODEM INIT [%s]", caInitString);

#ifndef __TETRA__
	if (ucIsSync)
	{
		TlvTree_AddChildString(piPhysicalConfig,
								LL_MODEM_T_INIT_STRING_CONEXANT,		   // TAG
								caCnxInitString);                          // VALUE
		dmsg("MODEM INIT CNX [%s]", caCnxInitString);
	}
#endif

	mtiMemset(caTemp, 0, sizeof(caTemp));
	sprintf(caTemp, "%s", tpCommInfo->cpRemoteTelNo);

	// Phone number
	// ------------
	TlvTree_AddChildString(piPhysicalConfig,
						   LL_MODEM_T_PHONE_NUMBER,                   // TAG
						   caTemp);                					  // VALUE
																	  // LENGTH (strlen buf)
	if (ucIsSync)
	{
		// Data link layer parameters
		// ==========================
		piDataLinkConfig = TlvTree_AddChild(piConfig,
											LL_TAG_DATA_LINK_LAYER_CONFIG, // TAG Data link layer parameters
											NULL,                     // VALUE (Null)
											0);                       // LENGTH 0

		// HDLC
		// ----
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_DATA_LINK_T_PROTOCOL,              // TAG
								LL_DATA_LINK_V_HDLC,                  // VALUE
								LL_DATA_LINK_L_PROTOCOL);             // LENGTH 1 byte

		// Connection timeout
		// ------------------
		// @@EB MODIFY uint uiTimeOut = 6 * LL_HDLC_V_CONNECT_TIMEOUT_DEFAULT;
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_HDLC_T_CONNECT_TIMEOUT,            // TAG
// @@EB MODIFY
#if 1
LL_HDLC_V_CONNECT_TIMEOUT_DEFAULT,	  		     // Value (Integer)
#else
uiTimeOut,	  		     // Value (Integer)
#endif
								LL_HDLC_L_CONNECT_TIMEOUT);           // LENGTH 4 bytes

		// Minimum tries for sending
		// -------------------------
		TlvTree_AddChildInteger(piDataLinkConfig,
								LL_HDLC_T_MIN_RESEND_REQUESTS,        // TAG
								2,                                    // VALUE
								LL_HDLC_L_MIN_RESEND_REQUESTS);       // LENGTH 1 byte
#ifdef __TETRA__
		//@@WAY, 201812
	    // To enable the fast connection mode
	    // ----------------------------------
	    TlvTree_AddChildInteger(piDataLinkConfig,
	    						LL_HDLC_T_FAST_UA,                    // TAG
	    						1,                                    // VALUE
	    						LL_HDLC_L_FAST_UA );                  // LENGTH 1 byte
#endif
	}

	dmsg("tpCommInfo->cpRemoteTelNo [%s]", tpCommInfo->cpRemoteTelNo);

	// Link Layer configuration
	iRet = LL_Configure(&hSession, piConfig);                         // Initialize the handle of the session
	if (iRet != LL_ERROR_OK)
	{
		return NULL;
	}

	if (piConfig)
	{
		TlvTree_Release(piConfig);                                    // Release tree to avoid memory leak
	}

	return hSession;
}
#endif

tCommInfo *mtiInitializeComm()
{
	INT i = 0;

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
	LL_HANDLE hSession = NULL;

	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			LL_ClearReceiveBuffer(hSession);
		}
	}

	return RTN_SUCCESS;
}

INT mtiPreOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	INT iRet = RTN_ERROR;
	dpt();
	dmsg("g_hThreadPre = %p", g_hThreadPre);
	if (g_hThreadPre == NULL)
	{
		dpt();

		g_hThreadPre = OSL_Thread_Create(threadPreConnection);
		g_hMutexPre = OSL_Mutex_Create (0, OSL_SECURITY_LOCAL);

		dpt();
		if (g_hThreadPre != NULL)
		{
			dpt();
			CHANGE_STATUS(THREAD_CONN_READY);

			if (ulTimeout > 0)
			{
				tpCommInfo->ulTimeoutMsec = ulTimeout / 10;
				dmsg("****PREDIAL tpCommInfo->ulTimeoutMsec : %lu", tpCommInfo->ulTimeoutMsec);
			}

			iRet = OSL_Thread_Start(g_hThreadPre, tpCommInfo);
			dmsg("OSL_Thread_Start() Result = %d", iRet);
			if (iRet == OSL_SUCCESS)
			{
				return RTN_SUCCESS;
			}
			else
			{
				return RTN_ERROR;
			}
		}
		else
		{
			return RTN_ERROR;
		}
	}
	return iRet;
}

INT mtiOpenCommWithPOS(tCommInfo *tpCommInfo, ULONG ulTimeout, BOOL bPOSUse)
{
	LL_HANDLE hSession = NULL;
	INT iRet = RTN_ERROR;
	ULONG ulTimeoutMsec = 0L;
	INT iKeyFlag = TRUE, iKey = 0;

#define COMM_EXCEPTION()						\
		if (!iKeyFlag) {						\
		mtiFinalizeKeyboard(); }				\
		return iRet;

	dmsg("[FUNC] mtiOpenComm()");
	if (tpCommInfo != NULL)
	{
		dmsg("tpCommInfo->iCommID = %d ", tpCommInfo->iCommID - 1);
		dpt();
		/** PRECONNECTION SYNC **/
		switch (tpCommInfo->iCommType)
		{
			case COMMTYPE_GPRS:
			case COMMTYPE_FAXMODEM:
				if (g_hThreadPre != NULL)
				{
					READ_STATUS(iRet);
					if (iRet == THREAD_TRY_CONN)
					{
						dmsg("Join a Preconnection Thread... OPEN");
						dtickstart();
						OSL_Thread_Join(g_hThreadPre);
						OSL_Thread_Destroy(g_hThreadPre);
						dtick();

						READ_STATUS(iRet);
						if (iRet == THREAD_CONN_SUCCESS)
						{
							tpCommInfo->iCommStatus = COMMSTAT_CONNECTION;
							g_hThreadPre = NULL;

							iRet = RTN_SUCCESS;
							COMM_EXCEPTION();
						}
						else
						{
							tpCommInfo->iCommStatus = COMMSTAT_CMMERR_CONN_FAULT;
							dpt();

							g_hThreadPre = NULL;

							iRet = RTN_ERROR;
							COMM_EXCEPTION();
						}
					}
				}
				break;
		}

		/** CONNECTED CHECK **/
		if (tpCommInfo->iCommStatus >= COMMSTAT_CONNECTION && tpCommInfo->iCommStatus != COMMSTAT_DISCONNECT)
		{
			return RTN_SUCCESS;
		}

		/** TIMEOUT REDUCE **/
		if (ulTimeout > 0)
		{
			ulTimeoutMsec = ulTimeout / 10;
			dmsg("Reduce timeout : %lu", ulTimeoutMsec);
		}

		/** COMMUNICATION SETTING **/
		dmsg("tpCommInfo->iCommType = %d", tpCommInfo->iCommType);
		switch (tpCommInfo->iCommType)
		{
			case COMMTYPE_GPRS:
				hSession = mtiSetGPRSCommConfigure(tpCommInfo, ulTimeoutMsec);
				break;

			case COMMTYPE_FAXMODEM:
				hSession = mtiSetFaxModemConfigure(tpCommInfo, ulTimeoutMsec);
				break;

			case COMMTYPE_SERIAL:
				hSession = mtiSetSerialCommConfigure(tpCommInfo);
				break;

			case COMMTYPE_WIFI:
				hSession = mtiSetWifiCommConfigure(tpCommInfo, ulTimeoutMsec);
				break;

			case COMMTYPE_ETHERNET:
				hSession = mtiSetEthernetCommConfigure(tpCommInfo, ulTimeoutMsec);
				break;
		}

		/** COMMUNICATION START **/
		if (hSession != NULL)
		{
			if (tpCommInfo->iCommType == COMMTYPE_GPRS)
			{
				iRet = StartGPRS(ulTimeoutMsec, TRUE);
			}

			g_iConnectStatus = 0;
			g_iConnectResult = 1000;	// @@EB REBOOT 20190314
			g_lhndTable[tpCommInfo->iCommID - 1] = hSession;
			g_hThreadConn = OSL_Thread_Create(threadConnection);

			iRet = OSL_Thread_Start(g_hThreadConn, tpCommInfo);
			dmsg("OSL_Thread_Start() Result = %d", iRet);
			if (iRet != OSL_SUCCESS)
			{
				iRet = RTN_ERROR;
				COMM_EXCEPTION();
			}

			if (!mtiIsValildKeyboard())
			{
				mtiInitializeKeyboard();
				iKeyFlag = FALSE;
			}

			while(1)
			{
				iKey = mtiGetKeyPressWithPOS(TRUE);
				if (iKey == KEY_CANCEL)
				{
					g_iConnectStatus = CONN_CANCEL;
					iRet = RTN_COMM_USER_CANCEL;
					break;
				}

				if (g_iConnectResult <= 0)
				{
					iRet = g_iConnectResult;
					break;
				}
			}

			dpt(); dmsg("******************** Result =%d", iRet);
			if (iRet != LL_ERROR_OK)
			{
				if (tpCommInfo->iCommType == COMMTYPE_FAXMODEM)
				{
					LL_Disconnect(hSession);
					dmsg("LL_Disconnect() Result = %d", iRet);

					LL_Configure(&hSession, NULL);
					dmsg("LL_Configure() Result = %d", iRet);

					dmsg("Join a Preconnection Thread... OPEN");
					OSL_Thread_Join(g_hThreadConn);
				}
			}

			if (iRet != RTN_COMM_USER_CANCEL)
			{
				switch (iRet)
				{
					case LL_ERROR_OK:
						tpCommInfo->iCommStatus = COMMSTAT_CONNECTION;
						iRet = RTN_SUCCESS;
						break;

					case LL_ERROR_TIMEOUT:
						iRet = RTN_TIMEOUT;
						break;

					case LL_ERROR_BUSY:
					case LL_MODEM_ERROR_RESPONSE_BUSY:
						if (tpCommInfo->iCommType == COMMTYPE_FAXMODEM)
						{
							iRet = RTN_COMM_LINE_BUSY;
						}
						else
						{
							iRet = RTN_ERROR;
						}
						break;

					case LL_ERROR_NETWORK_NOT_READY:
						iRet = RTN_ERROR;
						break;

					case LL_MODEM_ERROR_RESPONSE_NO_CARRIER:
					case LL_MODEM_ERROR_RESPONSE_NO_DIALTONE:
						if (tpCommInfo->iCommType == COMMTYPE_FAXMODEM)
						{
							iRet = RTN_COMM_NO_DIALTONE;
						}
						else
						{
							iRet = RTN_ERROR;
						}
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

	if (!iKeyFlag)
	{
		mtiFinalizeKeyboard();
	}
	return iRet;
}

INT mtiOpenComm(tCommInfo *tpCommInfo, ULONG ulTimeout)
{
	return mtiOpenCommWithPOS(tpCommInfo, ulTimeout, 0);
}

INT mtiGetCommHandle(tCommInfo *tpCommInfo)
{
	return (INT)g_lhndTable[tpCommInfo->iCommID - 1];
}


INT mtiWriteCommWithTimeOut(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength, INT nTimeOut)
{
	LL_HANDLE hSession = NULL;
	INT iRet = RTN_ERROR;
	INT iTry = 2;
	INT nTime = LL_INFINITE;
	INT nOffset = 0;
	INT iLoopCount = 10;
	//char cTestBuff[10];

	dmsg("[FUNC] mtiWriteComm()");
	if (tpCommInfo != NULL)
	{

		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		
		if (hSession != NULL)
		{
			while (iTry-- > 0)
			{
				/** COMM SEND **/
				if(0!=nTimeOut) nTime = 1000 * nTimeOut;

				// serial device busy problem
				if(COMMTYPE_SERIAL==tpCommInfo->iCommType)
				{
					while(nOffset < iLength && iLoopCount > 0)
					{
						iRet = LL_Send(hSession, iLength - nOffset, ucpWrite + nOffset, nTime);

						if(0 >= iRet)
						{
							dmsg("Try[%d] LL_Send() Result = %d, GetLastError = %d", iLoopCount, iRet, LL_GetLastError(hSession));
							LL_ClearSendBuffer(hSession);
							dmsg("LL_ClearSendBuffer GetLastError = %d", LL_GetLastError(hSession));
						}
						else
						{
							dbuf("WRITE", ucpWrite + nOffset, iRet);
						}
						
						nOffset += iRet;
						iLoopCount--;
					}
				}
				else
				{
					//dbuf("WRITE", ucpWrite, iLength);
					if (nTime == LL_INFINITE) 
					{
						dmsg("LL_Send >>> length=[%d] LL_INFINITE", iLength);
					}
					else 
					{
						dmsg("LL_Send >>> length=[%d] timeout=[%d]", iLength, nTime);
					}
					
					iRet = LL_Send(hSession, iLength, ucpWrite, nTime);

					if(0 >= iRet)
					{
						dmsg("LL_Send() Result = %d, GetLastError = %d", iRet, LL_GetLastError(hSession));
						LL_ClearSendBuffer(hSession);
						dmsg("LL_ClearSendBuffer GetLastError = %d", LL_GetLastError(hSession));
					}
					else
					{
						dbuf("WRITE", ucpWrite, iLength);
					}
				}

				if (iRet == iLength)
				{
					iRet = RTN_SUCCESS;
					break;
				}
				else
				{
					if (iRet < 0)
					{
						iRet = RTN_ERROR;
						break;
					}
#ifdef __TETRA__ 
				Telium_Ttestall(0, 100);
#endif

				}
			}
		}
	}

	return iRet;
}


// for testing serial
INT mtiWriteComm(tCommInfo *tpCommInfo, UCHAR *ucpWrite, INT iLength)
{
	return mtiWriteCommWithTimeOut(tpCommInfo, ucpWrite, iLength, 0);
}

INT mtiReadComm_With_Timeout(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength, INT nMillisecond)
{
	LL_HANDLE hSession = NULL;
	INT iRet = RTN_ERROR;
	INT nTimeout = LL_INFINITE;

	if (0 != nMillisecond)
		nTimeout = nMillisecond;

	if (tpCommInfo != NULL)
	{
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		if (hSession != NULL)
		{
			/** COMM RECEIVE **/
			if (iLength > 0)
			{
				iRet = LL_Receive(hSession, iLength, ucpRead, nTimeout);
				if (0 < iRet)
				{
					dmsg("[FUNC] mtiReadComm() - SUCCESS");
					dbuf("RECV", ucpRead, iRet);
					*iReadLength = iRet;
					return RTN_SUCCESS;
				}
				else
				{
					if (iRet == 0)
					{
						//dmsgnoln(".");
					}
					else
					{
						dmsgnoln("X");
					}
					iRet = RTN_ERROR;
				}
			}
			else
			{
				iRet = RTN_ERROR;
			}
		}
	}

	return iRet;
}

INT mtiReadComm(tCommInfo *tpCommInfo, UCHAR *ucpRead, INT iLength, INT *iReadLength)
{
	return mtiReadComm_With_Timeout(tpCommInfo, ucpRead, iLength, iReadLength, 10);
}

INT mtiCloseComm(tCommInfo *tpCommInfo)
{
	LL_HANDLE hSession = NULL;
	INT iRet = RTN_ERROR;

	dmsg("[FUNC] mtiCloseComm()");
	if (tpCommInfo != NULL)
	{
		dmsg("*** tpCommInfo->iCommID = %d", tpCommInfo->iCommID - 1);
		hSession = g_lhndTable[tpCommInfo->iCommID - 1];
		dmsg("*** hSession [%p]", hSession);

		if (hSession != NULL)
		{
			/** COMM DISCONNECT **/
			iRet = LL_Disconnect(hSession);

			dpt();
			if (g_hThreadPre != NULL)
			{
				READ_STATUS(iRet);
				if (iRet == THREAD_TRY_CONN)
				{
					dmsg("Join a Preconnection Thread... CLOSE");
					dtickstart();
					OSL_Thread_Join(g_hThreadPre);
					OSL_Thread_Destroy(g_hThreadPre);
					dtick();
				}
				dpt();
				g_hThreadPre = NULL;
			}

			if (g_hMutexPre != NULL)
			{
				dpt();
				OSL_Mutex_Destroy(g_hMutexPre);
				g_hMutexPre = NULL;
			}

			g_lhndTable[tpCommInfo->iCommID - 1] = NULL;
			g_tCommInfo[tpCommInfo->iCommID - 1].iCommID = 0;
			dmsg("LL_Disconnect() Result = %d", iRet);

			iRet = LL_Configure(&hSession, NULL);
			dmsg("LL_Configure() Result = %d", iRet);

			if (LL_ERROR_OK == iRet)
			{
				iRet = RTN_SUCCESS;
			}
		}
		else
		{
			iRet = RTN_COMM_ALREADY_DISCONNECT;
		}
	}

	dmsg("***** mtiCloseComm() Result = %d", iRet);
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

INT mtiActiveNetwork(INT iCommType)
{
	INT iRet = RTN_ERROR;
	tNetInfo netInfo;

	mtiMemset(&netInfo, 0x00, sizeof(tNetInfo));

	iRet = mtiGetNetInfo(iCommType, &netInfo);
	if (iCommType == COMMTYPE_GPRS && iRet != RTN_SUCCESS)
	{
// @@EB MODIFY
#if 1
		iRet = StartGPRS(TIMEOUT_120S, FALSE);
#else
		iRet = StartGPRS(TIMEOUT_30S, FALSE);
#endif
		dmsg("**** [] StartGPRS = %X", iRet);
	}
	return iRet;
}

INT mtiSetEthIF(INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw)
{
	ip_param_infos_s tIpInfo;
	UCHAR ucaAddr[5];
	CHAR *cpAddr = NULL;
	CHAR caTemp[3 + 1];
	INT i = 0, iTemp = 0, iAddr = 0, iRet = 0;

	mtiMemset(&tIpInfo, 0, sizeof(tIpInfo));

	tIpInfo.uiInterface = ETHERNET_INTERFACE;

#if 0 //@@WAY, 20190624 LAN
	//1-DHCP, 2-STATIC
	if (iDhcp == 2)
	{
		tIpInfo.boot_proto = HWCNF_GETIP_BOOT_PROTO_STATIC;
	}
	else
	{
		tIpInfo.boot_proto = HWCNF_GETIP_BOOT_PROTO_DHCP;
	}
#endif

#define INPUT_IP_ADDR(X, Y)													\
	mtiMemset(ucaAddr, 0, sizeof(ucaAddr));									\
	mtiMemset(caTemp, 0, sizeof(caTemp));									\
	iTemp = 0; iAddr = 0;													\
	cpAddr = Y;																\
	for (i = 0; i < 15; i++) {												\
		if (cpAddr[i] == 0) { break;}										\
		else {																\
			if (cpAddr[i] == '.' && (iTemp > 0)) {							\
				X[iAddr++] = (UINT)mtiAtoi(iTemp, (UCHAR*)caTemp);			\
				mtiMemset(caTemp, 0, sizeof(caTemp));						\
				iTemp = 0;													\
			}																\
			else {															\
				caTemp[iTemp++] = cpAddr[i];								\
				dmsg("caTemp(%d) [%s]", iTemp, caTemp);						\
			}																\
		}																	\
	}																		\
	if (iTemp > 0) {														\
		dmsg("last caTemp(%d) [%s]", iTemp, caTemp);						\
		X[iAddr++] = (UINT)mtiAtoi(iTemp, (UCHAR*)caTemp);					\
		mtiMemset(caTemp, 0, sizeof(caTemp));								\
		iTemp = 0;															\
	}																		\
	dmsg("BEFORE ADDR [%s]", Y);											\
	dbuf("RESULT ADDR", (UCHAR*)X, 16);

//@@WAY, 20190624 LAN
#if 1
	//1-DHCP, 2-STATIC
	if (iDhcp == 2)
	{
		tIpInfo.boot_proto = HWCNF_GETIP_BOOT_PROTO_STATIC;
		INPUT_IP_ADDR(tIpInfo.addr, cpIpAddr);
		INPUT_IP_ADDR(tIpInfo.netmask, cpNetMask);
		INPUT_IP_ADDR(tIpInfo.gateway, cpGw);

		iRet = HWCNF_SetIPInfos(ETHERNET_INTERFACE, &tIpInfo);
		dmsg("HWCNF_SetIPInfos() Result = %d", iRet);

	}
	else
	{
		tIpInfo.boot_proto = HWCNF_GETIP_BOOT_PROTO_DHCP;
		iRet = HWCNF_GetIPInfos(ETHERNET_INTERFACE, &tIpInfo);
		dmsg("HWCNF_GetIPInfos() Result = %d", iRet);
	}
#else
	INPUT_IP_ADDR(tIpInfo.addr, cpIpAddr);
	INPUT_IP_ADDR(tIpInfo.netmask, cpNetMask);
	INPUT_IP_ADDR(tIpInfo.gateway, cpGw);

	dmsg("tIpInfo.boot_proto = %d", tIpInfo.boot_proto);
	iRet = HWCNF_SetIPInfos(ETHERNET_INTERFACE, &tIpInfo);
	dmsg("HWCNF_SetIPInfos() Result = %d", iRet);
#endif

	if (iRet == HWCNF_SETIP_OK)
	{
		iRet = RTN_SUCCESS;
	}
	else
	{
		iRet = RTN_ERROR;
	}
	return iRet;
}

INT mtiSetCelluarIF(CHAR *cpAPN, INT iAuthType, CHAR *cpUserName, CHAR *cpPasswd)
{
	g_ucGprsStart = TRUE;
	mtiMemset(g_tCellInfo.caGprsAPN, 0, sizeof(g_tCellInfo.caGprsAPN));
	mtiStrcpy(g_tCellInfo.caGprsAPN, cpAPN, sizeof(g_tCellInfo.caGprsAPN));

	mtiMemset(g_tCellInfo.caGprsUserName, 0, sizeof(g_tCellInfo.caGprsUserName));
	mtiStrcpy(g_tCellInfo.caGprsUserName, cpUserName, sizeof(g_tCellInfo.caGprsUserName));

	mtiMemset(g_tCellInfo.caGprsPassword, 0, sizeof(g_tCellInfo.caGprsPassword));
	mtiStrcpy(g_tCellInfo.caGprsPassword, cpPasswd, sizeof(g_tCellInfo.caGprsPassword));

	gprs_enable(cpAPN);
	return RTN_SUCCESS;
}

INT mtiSetWifiIF(CHAR *cpSSID, INT iAuthType, INT iKeyMgmt, CHAR *cpUsername, CHAR *cpPasswd,
		INT iWifiEapType, INT iAuthAlg, INT iDhcp, CHAR *cpIpAddr, CHAR *cpNetMask, CHAR *cpGw)
{
	return RTN_SUCCESS;
}

INT mtiCheckHangUp()
{
	return g_iHangUp;
}

static VOID* threadConnection(VOID *pParam)
{
	tCommInfo *tpCommInfo = (tCommInfo*)pParam;
	LL_HANDLE hSession = g_lhndTable[tpCommInfo->iCommID - 1];
	INT iRet = 0;
	INT iCommType = tpCommInfo->iCommType;

	dmsg("[FUNC] threadConnection()");
	// @@EB REBOOT 20190314 g_iConnectResult = 1;
	g_iConnectResult = 1000;	// @@EB REBOOT 20190314
	if (hSession != NULL)
	{
		if (iCommType == COMMTYPE_FAXMODEM)
		{
			g_iHangUp = TRUE;
		}

		g_iConnectResult = LL_Connect(hSession);
		dmsg("** threadConnection ** LL_Connect() Result = %d", g_iConnectResult);
		dmsg("** g_iConnectStatus = %d **", g_iConnectStatus);

		if (iCommType != COMMTYPE_FAXMODEM)
		{
			dpt();
			if (g_iConnectStatus == CONN_CANCEL)
			{
				dpt();
				iRet = LL_Disconnect(hSession);
				dmsg("LL_Disconnect() Result = %d", iRet);

				iRet = LL_Configure(&hSession, NULL);
				dmsg("LL_Configure() Result = %d", iRet);
			}
		}
		else
		{
			g_iHangUp = FALSE;
		}
	}
	return NULL;
}

static VOID* threadPreConnection(VOID *pParam)
{
	LL_HANDLE hSession = NULL;
	INT iRet = RTN_ERROR;
	tCommInfo *tpCommInfo = (tCommInfo*)pParam;

#define TH_PRECONN_EXCEPTION()						\
		READ_STATUS(iRet);							\
		return g_hThreadPre;

	dmsg("[FUNC] threadPreConnection()");
	if (tpCommInfo != NULL)
	{
		dmsg("tpCommInfo->iCommID = %d ", tpCommInfo->iCommID - 1);

		/** CONNECTED CHECK **/
		if (tpCommInfo->iCommStatus >= COMMSTAT_CONNECTION && tpCommInfo->iCommStatus != COMMSTAT_DISCONNECT)
		{
			CHANGE_STATUS(THREAD_CONN_SUCCESS);
			TH_PRECONN_EXCEPTION();
		}
		else
		{
			tpCommInfo->iCommStatus = COMMSTAT_PRECONNECTION;
		}

		/** COMMUNICATION SETTING **/
		if (tpCommInfo->iCommType == COMMTYPE_FAXMODEM)
		{
			hSession = mtiSetFaxModemConfigure(tpCommInfo, tpCommInfo->ulTimeoutMsec);
		}
		//@@WAY PREDIAL 20210607		
		else if(tpCommInfo->iCommType == COMMTYPE_GPRS)
		{
			hSession = mtiSetGPRSCommConfigure(tpCommInfo, tpCommInfo->ulTimeoutMsec);
		}
		//
		else
		{
			CHANGE_STATUS(THREAD_ERROR);
			TH_PRECONN_EXCEPTION();
		}

		/** COMMUNICATION START **/
		if (hSession != NULL)
		{
			CHANGE_STATUS(THREAD_TRY_CONN);

			g_lhndTable[tpCommInfo->iCommID - 1] = hSession;
			iRet = LL_Connect(hSession);
			dmsg("** threadPreConnection ** LL_Connect() Result = %d", iRet);
			switch (iRet)
			{
				case LL_ERROR_OK:
					tpCommInfo->iCommStatus = COMMSTAT_CONNECTION;
					CHANGE_STATUS(THREAD_CONN_SUCCESS);
					break;

				default:
					CHANGE_STATUS(THREAD_CONN_FAULT);
					/**
					LL_Disconnect(hSession);
					LL_Configure(&hSession, NULL);
					**/
					break;
			}
		}
	}

	READ_STATUS(iRet);
	return g_hThreadPre;
}
