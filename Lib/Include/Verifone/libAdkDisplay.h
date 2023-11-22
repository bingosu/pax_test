/*
 * libAdkDisplay.h
 *
 *  Created on: 2017. 2. 9.
 *      Author: yakur
 */

#ifndef SRC_LIB_VERIFONE_LIBADKDISPLAY_H_
#define SRC_LIB_VERIFONE_LIBADKDISPLAY_H_

#include "apUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

void initADKGUI(networkStat *pNetworkStat);
int getIsADKGUI();
void startADKGUI();
void stopADKGUI();
void dispToGui(CHAR *msg);
void testADKGUI();
int dispQRC(CHAR *cpQrCode, INT iTimeOutByMs);

void dispGuiHomeScreen(INT *iMainTaskID);
int dispMainMenu(tMenuIdxVal *tpMenuIdxVal, INT iMenuCnt);
int dispMainMenuForC680(tMenuIdxVal *tpMenuIdxVal, INT iMenuCnt);
INT iAdkGuiShowDialogDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl);
INT iAdkGuiShowInputDisp(tDispContents *tpDispCont, INT iDispContCnt, tAddCtrlContents *tpAddCtrl);
INT iAdkGuiShowMenuDisp(tDispContents *tpDispCont, INT iDispContCnt, INT iDefaultSelect, tAddCtrlContents *tpAddCtrl);
INT iAdkGuiGetKeyPress(INT iTimeOutByMs);
INT InPutTest();


#ifdef __cplusplus
}
#endif


#endif /* SRC_LIB_VERIFONE_LIBADKDISPLAY_H_ */
