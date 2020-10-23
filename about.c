/*
 * about.c - about dialog
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "osmulti2.h"
#include "osmulres.h"

/*
 * loadDesc - load description
 */

void    loadDesc(HWND hwndDlg)
{
    HWND    hwnd = WinWindowFromID(hwndDlg, IDD_ADESC) ;
    APIRET  stat ;
    ULONG   idRes ;
    PVOID   pRes  ;
    PUCHAR  pDsc, pDst ;
    ULONG   len ;
    IPT     off ;
    UCHAR   error[64]  ;
    UCHAR   about[128] ;
    
    WinSendMsg(hwnd, MLM_DISABLEREFRESH, NULL, NULL) ;

    WinSendMsg(hwnd, MLM_FORMAT, MPFROMSHORT(MLFIE_CFTEXT), NULL) ;
    WinSendMsg(hwnd, MLM_SETTABSTOP, MPFROMSHORT(20), NULL) ;
  
    WinSendMsg(hwnd, MLM_SETIMPORTEXPORT, MPFROMP(about), MPFROMSHORT(128)) ;
    
    switch (ProgramLang) {
        case NLS_JA : idRes = IDD_ATXT_JP ; break ;
	case NLS_EN : idRes = IDD_ATXT_EN ; break ;
	default     : idRes = IDD_ATXT_EN ; break ;
    }

    if ((stat = DosGetResource(NULLHANDLE, IDD_ATYPE, idRes, &pRes)) != 0) {
        sprintf(about, "Error %d\n", stat) ;
	pDsc = error ;
    } else {
        pDsc = (PUCHAR) pRes ;
    }

    pDst = about, len = 0, off = 0 ;

    while (*pDsc != '\0') {
        if (len == 128) {
	    len = (ULONG) WinSendMsg(hwnd, 
	                MLM_IMPORT, MPFROMP(&off), MPFROMLONG(len)) ;
            off += len ;
            pDst = about, len = 0 ;
	}
	*pDst++ = *pDsc++ ;
	len += 1 ;
    }
    if (len > 0) {
        WinSendMsg(hwnd, MLM_IMPORT, MPFROMP(&off), MPFROMLONG(len)) ;
    }
    
    DosFreeResource(pRes) ;

    WinSendMsg(hwnd, MLM_ENABLEREFRESH, NULL, NULL) ;
}

/*
 * procAbout - dialog procedure for about dialog
 */
 
static MRESULT EXPENTRY procAbout(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg) {

    case WM_INITDLG :
        dialogAtMouse(hwnd) ;
	loadDesc(hwnd) ;
        return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
	case DID_OK :
	    WinDismissDlg(hwnd, DID_OK) ;
	    return (MRESULT) 0 ;
        case DID_CANCEL :
	    WinDismissDlg(hwnd, DID_CANCEL) ;
	    return (MRESULT) 0 ;    
        default :
	    return (MRESULT) 0 ;
	}
    }
    return WinDefDlgProc(hwnd, msg, mp1, mp2) ;
}

/*
 * aboutDialog - entry of about dialog
 */
 
void    aboutDialog(void)
{
    ULONG   idAbout ;
    
    switch (ProgramLang) {
        case NLS_JA : idAbout = IDD_ABOUT  ; break ;
	case NLS_EN : idAbout = IDD_ABOUTE ; break ;
	default     : idAbout = IDD_ABOUTE ; break ;
    }
    
    WinDlgBox(HWND_DESKTOP, NULLHANDLE, 
                procAbout, NULLHANDLE, idAbout, NULL) ;
}


