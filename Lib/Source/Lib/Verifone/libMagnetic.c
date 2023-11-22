#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "libMtiCommonApi.h"

#define MAX_ERR		10

static INT iMagDev = 0;
static INT iReadRange;

static const char cardErrTbl[MAX_ERR][22] = {
    "NO ERR",                        //  0
    "NO DATA",                       //  1
    "NO STX",                        //  2
    "NO ETX",                        //  3
    "BAD LRC",                       //  4
    "B PARITY",                      //  5
    "REV ETX",                       //  6
    "BAD JIIS",                      //  7
    "BAD TRK",                       //  8
    "BUF OVR"                        //  9
};

extern char isActiveEVTchk;

VOID mtiInitializeMagneticStripe(INT iAttribute)
{
	//if (CHK_BIT(iAttribute, READ_TRACK1_SUCCESS) && CHK_BIT(iAttribute, READ_TRACK3_SUCCESS))
	if(iMagDev == 0)
	{
		iMagDev = open(DEV_CARD, O_RDWR);
		if (iMagDev < 1) {
			dmsg("!!!Could not Open Magnetic Swipe Device!!!");
			return;
		}
		dmsg("Magnetic Swipe Open Success [%d]",iMagDev);
	}
	//else
	//	dmsg("Magnetic Swipe Device is already opened. [%d]", iMagDev);

	iReadRange = iAttribute;
}

VOID mtiFlushMagneticStripe()
{
	//This Function is NOT Necessary to VERIFONE TERMINAL.
}

INT mtiReadMagneticStripe(tMagneticStripeContents *tpMSContents)
{
	INT iRet = 0;
	INT iReadSize = 0;
	INT iTrackIdx, iIdx;
	CHAR caCardRawData[500] = {0,};
	INT iLen, iErrNo;
	LONG lEvent = 0L;

	if(isActiveEVTchk)
		lEvent = read_evt(EVT_MAG);
	else
		lEvent = 1L;

	if(lEvent)
	{
		dmsg("Card Detected!!! Event[0x%.8x]", lEvent);

		if(card_pending())
		{
			iReadSize = read(iMagDev, caCardRawData, sizeof(caCardRawData));
			if(iReadSize < 1){
				dpt();
				dmsg("Mag Data Read Error [%s]", strerror(errno));
				return RTN_ERROR;
			}

			dmsg("caCardRawData[%dB]-[%s]",iReadSize, caCardRawData);
			dbuf("CARD RAW DATA", caCardRawData, iReadSize);

			for (iTrackIdx = 0, iIdx = 0; iTrackIdx < 3; iTrackIdx++)
			{
				//Track Data Length
				iLen = ((caCardRawData[iIdx] > 0) ? (caCardRawData[iIdx] - 2) : 0);
				//Track Data Error
				iErrNo = caCardRawData[iIdx + 1];

				//Error Check
				if(iErrNo != 0){
					dpt();
					dmsg("Track-%d Read Error [%s]", iTrackIdx + 1, cardErrTbl[iErrNo]);
				}

				if(iTrackIdx == 0 && iErrNo == 0 && CHK_BIT(iReadRange, READ_TRACK1_SUCCESS))
				{
					//Get Track1 Data
					mtiMemcpy(tpMSContents->caTrack1, caCardRawData+(iIdx+2), iLen);
					tpMSContents->ucTrack1Length = iLen;
					iRet |= READ_TRACK1_SUCCESS;
				}

				if(iTrackIdx == 1 && iErrNo == 0 && CHK_BIT(iReadRange, READ_TRACK2_SUCCESS))
				{
					//Get Track2 Data
					mtiMemcpy(tpMSContents->caTrack2, caCardRawData+(iIdx+2), iLen);
					tpMSContents->ucTrack2Length = iLen;
					tpMSContents->caTrack2[tpMSContents->ucTrack2Length++] = 'F';
					iRet |= READ_TRACK2_SUCCESS;

					//Replace Token Letter '=' -> 'D'
					{
						INT iLoop;
						for(iLoop = 0; iLoop < iLen; iLoop++)
							if(tpMSContents->caTrack2[iLoop] == '=')
								tpMSContents->caTrack2[iLoop] = 'D';
					}

					dmsg("Track2 Data[%s] Len[%d]",tpMSContents->caTrack2, tpMSContents->ucTrack2Length);
					dbuf("TRACK2 DATA",tpMSContents->caTrack2, iLen);
				}

				if(iTrackIdx == 2 && iErrNo == 0 && CHK_BIT(iReadRange, READ_TRACK3_SUCCESS))
				{
					//Get Track3 Data
					mtiMemcpy(tpMSContents->caTrack3, caCardRawData+(iIdx+2), iLen);
					tpMSContents->ucTrack3Length = iLen;
					iRet |= READ_TRACK3_SUCCESS;
				}

				//Jump to Next Track Data
				iIdx += caCardRawData[iIdx];
			}
		}
		else
		{
			dpt();
			dmsg("Card Pending is Failed!!!");
		}
	}
	if(iRet == 0)
		iRet = RTN_ERROR;
	return iRet;
}

VOID mtiFinalizeMagneticStripe()
{
	if(iMagDev != 0)
	{
		close(iMagDev);
		iMagDev = 0;
	}
}
