/*
 * window.c - Create/Dispose Windows
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "shapewin.h"

#include "osmulti2.h"
#include "osmulres.h"

/*
 * Windows to Use
 */

HWND    hwndFrame = NULLHANDLE ;    /* Invisible Frame Window   */
HWND    hwndShape = NULLHANDLE ;    /* Shape Window for Bitmap  */
HWND    hwndTalk  = NULLHANDLE ;    /* Shape Window for Message */

/*
 * Mouse Pointers
 */

static  HPOINTER    pointerNade = NULLHANDLE ;
static  HPOINTER    pointerFuni = NULLHANDLE ;

static  BOOL        captureNade = FALSE ;
static  BOOL        captureFuni = FALSE ;

/*
 * doMouseDn/Up - process WM_BUTTON1DOWN/WM_BUTTON1UP
 */

static  void    doMouseDn(HWND hwnd)
{
    POINTL  pt ;
    
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;
    WinMapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1) ;

    if (captureNade || captureFuni) {
        return ;
    }
    if (bitmapAtNade(&pt)) {
	WinSetPointer(HWND_DESKTOP, pointerNade) ;
        WinSetCapture(HWND_DESKTOP, hwnd) ;
	captureNade = TRUE ;
	return ;
    }
    if (bitmapAtFuni(&pt)) {
	WinSetPointer(HWND_DESKTOP, pointerFuni) ;
	animeFuni() ;
        WinSetCapture(HWND_DESKTOP, hwnd) ;
	captureFuni = TRUE ;
	return ;
    }
}
 
static  void    doMouseUp(HWND hwnd)
{
    if (captureNade || captureFuni) {
        WinSetCapture(HWND_DESKTOP, NULLHANDLE) ;
	captureNade = captureFuni = FALSE ;
    }
}
 
/*
 * doMouseMove - process WM_MOUSEMOVE message
 */

static  BOOL    doMouseMove(HWND hwnd)
{
    POINTL  pt ;
    
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;
    WinMapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1) ;

    if (captureNade) {
        WinSetPointer(HWND_DESKTOP, pointerNade) ;
	animeNade() ;
	return TRUE ;
    }
    if (captureFuni) {
        WinSetPointer(HWND_DESKTOP, pointerFuni) ;
	return TRUE ;
    }
    if (bitmapAtNade(&pt) || bitmapAtFuni(&pt)) {
        WinSetPointer(HWND_DESKTOP, pointerNade) ;
	return TRUE ;
    }
    return FALSE ;
}

/*
 * contextMenu - menu with (right) mouse click
 */

static  HWND    hwndPopup = NULLHANDLE ;

static  void    contextMenu(void)
{
    POINTL  pt   ;
    SWP     swp  ;
    ULONG   opts ;
    ULONG   idMenu ;
    
    switch (ProgramLang) {
        case NLS_JA : idMenu = IDM_POPUP  ; break ;
	case NLS_EN : idMenu = IDM_POPUPE ; break ;
	default     : idMenu = IDM_POPUPE ; break ;
    }
    
    if (hwndPopup == NULLHANDLE) {
        hwndPopup = WinLoadMenu(hwndFrame, NULLHANDLE, idMenu) ;
    }
    if (hwndPopup == NULLHANDLE) {
        TRACE("failed to load popup menu\n") ;
        return ;
    }
    WinQueryPointerPos(HWND_DESKTOP, &pt) ;
    WinQueryWindowPos(hwndFrame, &swp) ;
    
    if (pt.x < swp.x || pt.x > (swp.x + swp.cx)) {
        return ;
    }
    if (pt.y < swp.y || pt.y > (swp.y + swp.cy)) {
        return ;
    }

    opts = PU_POSITIONONITEM | PU_HCONSTRAIN | PU_VCONSTRAIN |
                 PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 ;

    WinPopupMenu(HWND_DESKTOP, hwndFrame, hwndPopup,
                                pt.x, pt.y, IDM_SETUP, opts) ;
}

/*
 * procFrame - subclassed frame window procedure
 */
 
static  PFNWP   pfnFrame ;      /* original frame window procedure  */

static MRESULT EXPENTRY procFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSWP    pswp ;

    switch (msg) {

    case WM_ADJUSTWINDOWPOS :
        pswp = (PSWP) PVOIDFROMMP(mp1) ;
        WinSetWindowPos(hwndShape, pswp->hwndInsertBehind,
                    pswp->x, pswp->y, pswp->cx, pswp->cy, pswp->fl) ;
        pswp->fl &= ~SWP_SHOW ;
        pswp->fl |=  SWP_HIDE ;
        return (*pfnFrame) (hwnd, msg, mp1, mp2) ;

    case WM_BUTTON1DOWN :
        doMouseDn(hwnd) ;
	return (MRESULT) 0 ;

    case WM_BUTTON1UP :
        doMouseUp(hwnd) ;
	return (MRESULT) 0 ;

    case WM_MOUSEMOVE :
        if (doMouseMove(hwnd)) {
	    return (MRESULT) 0 ;
	}
        break ;     /* fall to default procedure */
	
    case WM_BUTTON2DOWN :
        contextMenu() ;
	return (MRESULT) 0 ;

    case WM_TIMER :
        trackFocus(WinQueryAnchorBlock(hwnd)) ;
	animeTimer() ;
        return (MRESULT) 0 ;

    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {

	case IDM_SETUP :
	    setupDialog() ;
	    return (MRESULT) 0 ;
	case IDM_ABOUT :
	    aboutDialog() ;
	    return (MRESULT) 0 ;

	case IDM_EXIT :
	    WinShowWindow(hwndShape, FALSE) ;
	    WinShowWindow(hwndTalk,  FALSE) ;
	    WinPostMsg(hwnd, WM_CLOSE, NULL, NULL) ;
	    return (MRESULT) 0 ;
        }
        return (MRESULT) 0 ;
    }

    return (*pfnFrame) (hwnd, msg, mp1, mp2) ;
}

/*
 * createFrame - create frame window
 */

#ifndef WS_TOPMOST
#define WS_TOPMOST  0x00200000
#endif

static  void    createFrame(HAB hab)
{
    FRAMECDATA  fcd ;

    memset(&fcd, 0, sizeof(fcd)) ;
    fcd.cb = sizeof(fcd) ;
    fcd.flCreateFlags = (FCF_TASKLIST | FCF_ICON) ;
    fcd.hmodResources = NULLHANDLE ;
    fcd.idResources   = ID_OSMULTI ;

    hwndFrame = WinCreateWindow(
            HWND_DESKTOP,           /* Parent window handle     */
            WC_FRAME,               /* Frame Window Class       */
            ProgramName,            /* as Title                 */
            0,                      /* Window Style             */
            0, 0, 0, 0,             /* Position & size          */
            NULLHANDLE,             /* Owner Window             */
            HWND_TOP,               /* Z-Order                  */
            0,                      /* Window ID                */
            &fcd,                   /* Control Data             */
            NULL) ;                 /* Presentation Parameter   */

    if (hwndFrame == NULLHANDLE) {
        return ;
    }

    pfnFrame = WinSubclassWindow(hwndFrame, procFrame) ;

    WinSendMsg(hwndFrame, WM_SETICON,
        MPFROMP(WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_OSMULTI)), NULL) ;
}

/*
 * createShape - create shape window
 */

static  void    createShape(HAB hab)
{
    BITMAPINFOHEADER2   bmi ;
    SHAPEWIN    shpctrl ;

    if (hwndFrame == NULLHANDLE || 
            hpsBitmap == NULLHANDLE || hpsBalloon == NULLHANDLE) {
        return ;
    }

    /*
     * Register Window Class
     */

    WinRegisterClass(hab, ShapeWinName, ShapeWinProc, 0L, sizeof(PVOID)) ;

    /*
     * Create Shape Window for Bitmap
     */

    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmBitmap, &bmi) ;

    shpctrl.cx = bmi.cx ;
    shpctrl.cy = bmi.cy ;
    shpctrl.hpsDraw = hpsBitmap ;
    shpctrl.hpsMask = hpsBitmap ;

    hwndShape = WinCreateWindow(
            HWND_DESKTOP,           /* Parent Window    */
            ShapeWinName,           /* Window Class     */
            "",                     /* Window Text      */
            0,                      /* Window Style     */
            0, 0, 0, 0,             /* Pos & Size       */
            hwndFrame,              /* Owner Window     */
            HWND_TOP,               /* Z-Order          */
            0,                      /* Window ID        */
            &shpctrl,               /* Control Data     */
            NULL) ;                 /* Pres. Param.     */

    /*
     * Create Shape Window for Message Balloon
     */

    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmBalloon, &bmi) ;

    shpctrl.cx = bmi.cx ;
    shpctrl.cy = bmi.cy ;
    shpctrl.hpsDraw = hpsBalloon ;
    shpctrl.hpsMask = hpsBalloon ;

    hwndTalk  = WinCreateWindow(
            HWND_DESKTOP,           /* Parent Window    */
            ShapeWinName,           /* Window Class     */
            "",                     /* Window Text      */
            0,                      /* Window Style     */
            0, 0, 0, 0,             /* Pos & Size       */
            hwndFrame,              /* Owner Window     */
            HWND_TOP,               /* Z-Order          */
            1,                      /* Window ID        */
            &shpctrl,               /* Control Data     */
            NULL) ;                 /* Pres. Param.     */

}

/*
 * windowDispose - dispose window & related resources
 */

void    windowDispose(HAB hab)
{
    WinStopTimer(hab, hwndFrame, TIMER_ID) ;
    
    bitmapDispose()  ;
    balloonDispose() ;

    if (pointerNade != NULLHANDLE) {
        WinDestroyPointer(pointerNade) ;
	pointerNade = NULLHANDLE ;
    }
    if (pointerFuni != NULLHANDLE) {
        WinDestroyPointer(pointerFuni) ;
	pointerFuni = NULLHANDLE ;
    }

    if (hwndShape != NULLHANDLE) {
        WinDestroyWindow(hwndShape) ;
	hwndShape = NULLHANDLE ;
    }
    if (hwndTalk != NULLHANDLE) {
        WinDestroyWindow(hwndTalk) ;
	hwndTalk = NULLHANDLE ;
    }
    if (hwndFrame != NULLHANDLE) {
        WinDestroyWindow(hwndFrame) ;
	hwndFrame = NULLHANDLE ;
    }
}

/*
 * windowCreate - create windows
 */

BOOL    windowCreate(HAB hab)
{
    pointerNade = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_PTRNADE) ;
    pointerFuni = WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ID_PTRFUNI) ;
    
    if (bitmapCreate(hab) != TRUE) {
        return FALSE ;
    }
    if (balloonCreate(hab) != TRUE) {
        return FALSE ;
    }
    
    /*
     * Create Windows
     */

    createFrame(hab) ;
    createShape(hab) ;
    
    if (hwndFrame == NULLHANDLE ||
                hwndShape == NULLHANDLE || hwndTalk == NULLHANDLE) {
        windowDispose(hab) ;
	return FALSE ;
    }

    trackFocus(hab) ;
    
    if (WinStartTimer(hab, hwndFrame, TIMER_ID, TIMER_MS) == 0) {
        windowDispose(hab) ;
	return FALSE ;
    }
    return TRUE ;
}
