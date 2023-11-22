
#include "libMtiCommonApi.h"

static Telium_File_t *m_hMag13=NULL, *m_hMag2=NULL, *m_hMag3=NULL;


VOID mtiInitializeMagneticStripe(INT iAttribute)
{
	if (CHK_BIT(iAttribute, READ_TRACK1_SUCCESS) && CHK_BIT(iAttribute, READ_TRACK3_SUCCESS))
	{
		m_hMag13 = Telium_Fopen("SWIPE31", "r*");
	}

	if (CHK_BIT(iAttribute, READ_TRACK2_SUCCESS))
	{
		m_hMag2 = Telium_Fopen("SWIPE2", "r*");
	}

	if (CHK_BIT(iAttribute, READ_TRACK3_SUCCESS))
	{
		m_hMag3 = Telium_Fopen("SWIPE3", "r*");
	}

}

VOID mtiFlushMagneticStripe()
{
	if (m_hMag2 != NULL)
	{
		Telium_Reset_buf(m_hMag2, _receive_id);
	}

	if (m_hMag13 != NULL)
	{
		Telium_Reset_buf(m_hMag13, _receive_id);
	}

	if (m_hMag3 != NULL)
	{
		Telium_Reset_buf(m_hMag3, _receive_id);
	}
}

INT mtiReadMagneticStripe(tMagneticStripeContents *tpMSContents)
{
	INT iEvents = 0, iResult = 0, iRet = 0;
	UCHAR ucLen = 0;
	CHAR caTemp[128 + 1];
	CHAR *cpSrc = NULL, *cpDest = NULL;

	iEvents  = Telium_Ttestall(SWIPE31 | SWIPE2 | SWIPE3, 1);
	iEvents |= Telium_Ttestall(iEvents ^ (SWIPE31 | SWIPE2 | SWIPE3), 1);
	iEvents |= Telium_Ttestall(iEvents ^ (SWIPE31 | SWIPE2 | SWIPE3), 1);

	if(iEvents & SWIPE31)
	{
		mtiMemset(caTemp, 0, sizeof(caTemp));
		ucLen = 0;

		dpt();
		iRet = Telium_Is_iso1(m_hMag13, &ucLen, (byte*)caTemp);
		if (iRet == ISO_OK)
		{
			cpSrc = caTemp;
			cpDest = tpMSContents->caTrack1;
			tpMSContents->ucTrack1Length = ucLen;

			while(*cpSrc)
			{
				if(*cpSrc++ == '%')
				{
					break;
				}
			}

			while(*cpSrc)
			{
				if(*cpSrc == '?')
				{
					break;
				}
				*cpDest++ = *cpSrc++;
			}

			dmsg("tpMSContents->caTrack1 [%s]", tpMSContents->caTrack1);
			iResult |= READ_TRACK1_SUCCESS;
		}
		else
		{
			Telium_Fclose(m_hMag13);
			mtiSoundBeep(40, 2);
			m_hMag13 = Telium_Fopen("SWIPE31", "r*");

			iResult |= READ_TRACK_ERROR;
		}
	}

	if(iEvents & SWIPE2)
	{
		mtiMemset(caTemp, 0, sizeof(caTemp));
		ucLen = 0;

		iRet = Telium_Is_iso2(m_hMag2, &ucLen, (byte*)caTemp);
		dmsg("****** iRet = %d",iRet);
		if (iRet == ISO_OK || iRet == DEF_LUH)
		{
			cpSrc = caTemp;
			cpDest = tpMSContents->caTrack2;
			dmsg("caTemp [%s]", caTemp);
			dmsg("ucLen [%d]", ucLen);

			while(*cpSrc)
			{
				if(*cpSrc++ == 'B')
				{
					break;
				}
			}

			while(*cpSrc)
			{
				*cpDest++ = *cpSrc;
				if(*cpSrc == 'F')
				{
					break;
				}
				cpSrc++;
			}

			tpMSContents->ucTrack2Length = cpSrc - caTemp;
			iResult |= READ_TRACK2_SUCCESS;
		}
		else
		{
			Telium_Fclose(m_hMag2);
			mtiSoundBeep(40, 2);
			m_hMag2 = Telium_Fopen("SWIPE2", "r*");

			iResult |= READ_TRACK_ERROR;
		}
	}

	if(iEvents & SWIPE3)
	{
		mtiMemset(caTemp, 0, sizeof(caTemp));
		ucLen = 0;

		iRet = Telium_Is_iso3(m_hMag3, &ucLen, (byte*)caTemp);
		if (iRet == ISO_OK)
		{
			cpSrc = caTemp;
			cpDest = tpMSContents->caTrack3;
			tpMSContents->ucTrack3Length = ucLen;

			while(*cpSrc)
			{
				if(*cpSrc++ == 'B')
				{
					break;
				}
			}

			while(*cpSrc)
			{
				if(*cpSrc == 'F')
				{
					break;
				}

				if(*cpSrc == 'D')
				{
					*cpSrc = '=';
				}

				*cpDest++ = *cpSrc++;
			}

			iResult |= READ_TRACK3_SUCCESS;
		}
		else
		{
			Telium_Fclose(m_hMag3);
			mtiSoundBeep(40, 2);
			m_hMag3 = Telium_Fopen("SWIPE3", "r*");

			iResult |= READ_TRACK_ERROR;
		}
	}
	return iResult;
}

VOID mtiFinalizeMagneticStripe()
{
	if (m_hMag2 != NULL)
	{
		dpt();
		Telium_Fclose(m_hMag2);
		m_hMag2 = NULL;
	}

	if (m_hMag13 != NULL)
	{
		dpt();
		Telium_Fclose(m_hMag13);
		m_hMag13 = NULL;
	}

	if (m_hMag3 != NULL)
	{
		dpt();
		Telium_Fclose(m_hMag3);
		m_hMag3 = NULL;
	}

}
