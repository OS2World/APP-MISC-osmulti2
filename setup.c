/*
 * setup.c - setup dialog
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "osmulti2.h"
#include "osmulres.h"

/*
 * Data to Setup Here
 */

extern  int     posRatio ;      /* track.c      */
extern  BOOL    useMsg   ;      /* balloon.c    */
extern  BOOL    useSnd   ;      /* balloon.c    */

/*
 * dialogAtMouse - place dialog near mouse position
 */

void    dialogAtMouse(HWND hwndDialog)
{
    POINTL  pt ;
    SWP     posDlg ;
    SWP     posScr ;

    WinQueryPointerPos(HWND_DESKTOP, &pt)    ;
    WinQueryWindowPos(HWND_DESKTOP, &posScr) ;
    WinQueryWindowPos(hwndDialog,   &posDlg) ;

    posDlg.x = pt.x - (posDlg.cx / 10)    ;
    posDlg.y = pt.y - (posDlg.cy * 4 / 5) ;

    if (posDlg.x < 0) {
        posDlg.x = 0 ;
    }
    if (posDlg.y < 0) {
        posDlg.y = 0 ;
    }
    if ((posDlg.x + posDlg.cx) > posScr.cx) {
        posDlg.x = posScr.cx - posDlg.cx ;
    }
    if ((posDlg.y + posDlg.cy) > posScr.cy) {
        posDlg.y = posScr.cy - posDlg.cy ;
    }
    WinSetWindowPos(hwndDialog, NULLHANDLE,
                    posDlg.x, posDlg.y, 0, 0, SWP_MOVE) ;
}

/*
 * sliderIni - initialize Slider
 *
 *      set tick marks on slider
 */

static  void    sliderIni(HWND hwndDlg)
{
    HWND    hwnd = WinWindowFromID(hwndDlg, IDD_SPOS) ;
    int     i ;
    
    for (i = 0 ; i <= 100 ; i++) {
        if ((i % 10) == 0) {
            WinSendMsg(hwnd, SLM_SETTICKSIZE, MPFROM2SHORT(i, 5), NULL) ;
        } else if ((i % 5) == 0) {
	    WinSendMsg(hwnd, SLM_SETTICKSIZE, MPFROM2SHORT(i, 2), NULL) ;
	}
    }
}

/*
 * sliderSet/Get - position Slider
 */

static  void    sliderSet(HWND hwndDlg, int pos)
{
    HWND    hwndPos = WinWindowFromID(hwndDlg, IDD_SPOS) ;
    HWND    hwndNum = WinWindowFromID(hwndDlg, IDD_SNUM) ;
    UCHAR   buff[16] ;

    sprintf(buff, "%d%%", pos) ;
    WinSetWindowText(hwndNum, buff) ;

    WinSendMsg(hwndPos, SLM_SETSLIDERINFO,
        MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE), 
	MPFROMLONG(pos)) ;
}

static  int     sliderGet(HWND hwndDlg)
{
    HWND    hwndPos = WinWindowFromID(hwndDlg, IDD_SPOS) ;
    int     pos ;
    
    pos = (int) WinSendMsg(hwndPos, SLM_QUERYSLIDERINFO, 
        MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE), NULL) ;
    return pos ;    
}

static  void    sliderUpd(HWND hwndDlg)
{
    HWND    hwndPos = WinWindowFromID(hwndDlg, IDD_SPOS) ;
    HWND    hwndNum = WinWindowFromID(hwndDlg, IDD_SNUM) ;
    UCHAR   buff[16] ;
    int     pos ;
    
    pos = (int) WinSendMsg(hwndPos, SLM_QUERYSLIDERINFO, 
        MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE), NULL) ;

    sprintf(buff, "%d%%", pos) ;
    WinSetWindowText(hwndNum, buff) ;
}

/*
 * setData - set option data to dialog
 */

static  void    setData(HWND hwndDlg)
{
    HWND    hwndMsg = WinWindowFromID(hwndDlg, IDD_SMSG) ;
    HWND    hwndSnd = WinWindowFromID(hwndDlg, IDD_SSND) ;
    
    WinSendMsg(hwndMsg, BM_SETCHECK, MPFROMSHORT(useMsg), NULL) ;
    WinSendMsg(hwndSnd, BM_SETCHECK, MPFROMSHORT(useSnd), NULL) ;
    WinEnableWindow(hwndSnd, FALSE) ;       /* not use yet */
    sliderSet(hwndDlg, posRatio) ;
}

/*
 * getData - get option data from dialog
 */

static  void    getData(HWND hwndDlg)
{
    HWND    hwndMsg = WinWindowFromID(hwndDlg, IDD_SMSG) ;
    HWND    hwndSnd = WinWindowFromID(hwndDlg, IDD_SSND) ;

    posRatio = sliderGet(hwndDlg) ;
    useMsg = (BOOL) WinSendMsg(hwndMsg, BM_QUERYCHECK, NULL, NULL) ;
    useSnd = (BOOL) WinSendMsg(hwndSnd, BM_QUERYCHECK, NULL, NULL) ;
}

/*
 * procSetup - dialog procedure for setup dialog
 */
 
static MRESULT EXPENTRY procSetup(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {

    case WM_INITDLG :
        dialogAtMouse(hwnd) ;
	sliderIni(hwnd) ;
	setData(hwnd) ;
        return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
	case DID_OK :
	    getData(hwnd) ;
	    profileSave(WinQueryAnchorBlock(hwnd)) ;
	    WinDismissDlg(hwnd, DID_OK) ;
	    return (MRESULT) 0 ;
        case DID_CANCEL :
	    WinDismissDlg(hwnd, DID_CANCEL) ;
	    return (MRESULT) 0 ;    
        default :
	    return (MRESULT) 0 ;
	}

    case WM_CONTROL :
        if (SHORT1FROMMP(mp1) == IDD_SPOS) {
	    if (SHORT2FROMMP(mp1) == SLN_CHANGE) {
	        sliderUpd(hwnd) ;
	    } else if (SHORT2FROMMP(mp1) == SLN_SLIDERTRACK) {
                sliderUpd(hwnd) ;
            }
        }
	return (MRESULT) 0 ;
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
}

/*
 * setupDialog - entry of setup dialog
 */
 
void    setupDialog(void)
{
    ULONG   idSetup ;

    switch (ProgramLang) {
        case NLS_JA : idSetup = IDD_SETUP  ; break ;
	case NLS_EN : idSetup = IDD_SETUPE ; break ;
	default     : idSetup = IDD_SETUPE ; break ;
    }
    
    WinDlgBox(HWND_DESKTOP, NULLHANDLE, 
                procSetup, NULLHANDLE, idSetup, NULL) ;

    trackFocus(WinQueryAnchorBlock(HWND_DESKTOP)) ;     /* update multi */
}
