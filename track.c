/*
 * track.c - track focus window
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "osmulti2.h"

/*
 * Adjustment on Position
 *
 *      posRatio may be changed with in 0..100
 */

#define     XPOSRATIO       20      /* 0..100       */
#define     XPOSADJUST      64      /* Bitmap Width */
#define     YPOSADJUST      28

int     posRatio   = XPOSRATIO  ;
int     posAdjustX = XPOSADJUST ;
int     posAdjustY = YPOSADJUST ;

/*
 * last position
 */

static  HWND    lastTarget = NULLHANDLE ;
static  HWND    lastActive = NULLHANDLE ;
static  BOOL    lastShow = FALSE ;
static  SWP     lastSwp  = { 0 } ;

/*
 * queryPos - calc. position of bitmap
 */

static  void    queryPos(HWND hwnd, PSWP swp)
{
    SWP     pos ;
    
    WinQueryWindowPos(hwnd, &pos) ;
    
    swp->y = pos.y + pos.cy - posAdjustY ;
    if (pos.cx <= posAdjustX) {
        swp->x = pos.x ;
    } else {
        swp->x = pos.x + ((pos.cx - posAdjustX) * posRatio / 100) ;
    }
}

/*
 * queryMax - check if maximized window
 */

static  BOOL    queryMax(HWND hwnd)
{
    SWP     swpMax ;
    SWP     swpChk ;
    
    WinQueryWindowPos(HWND_DESKTOP, &swpMax) ;
    WinQueryWindowPos(hwnd, &swpChk) ;
    
    if (swpChk.x > swpMax.x) {
        return FALSE ;
    }
    if ((swpChk.x + swpChk.cx) < swpMax.cx) {
        return FALSE ;
    }
    if (swpChk.y > swpMax.y) {
        return FALSE ;
    }
    if ((swpChk.y + swpChk.cy) < swpMax.cy) {
        return FALSE ;
    }
    return TRUE ;   /* checking window covers all desktop */
}

/*
 * queryOob - check if out of boundary
 */

static  BOOL    queryOob(HWND hwnd)
{
    SWP     swpMax ;
    SWP     swpChk ;
    BITMAPINFOHEADER2   bmi ;
    int     more ;
    
    WinQueryWindowPos(HWND_DESKTOP, &swpMax) ;
    WinQueryWindowPos(hwnd, &swpChk) ;
    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmMulNorm, &bmi) ;
    more = bmi.cy - posAdjustY ;
    
    if (swpChk.x >= swpMax.cx) {
        return TRUE ;
    }
    if ((swpChk.x + swpChk.cx) <= 0) {
        return TRUE ;
    }
    if ((swpChk.y + swpChk.cy + more) >= swpMax.cy) {
        return TRUE ;
    }
    if ((swpChk.y + swpChk.cy) <= 0) {
        return TRUE ;
    }
    return FALSE ;
}

/*
 * trackFocus - track Focus Window and Move/Draw
 */

void    trackFocus(HAB hab)
{
    HWND    hwndTarget, hwndActive ;
    SWP     pos  ;
    BOOL    show, draw ;
    BITMAPINFOHEADER2   bmi ;
    
    /*
     * Query Active Window & Position
     */
     
    hwndActive  = WinQueryActiveWindow(HWND_DESKTOP) ;

    /*
     * if cannot attach to active window, then try old one
     */
     
    if (hwndActive == NULLHANDLE) {
        hwndTarget = lastTarget ;
        pos.x = lastSwp.x ;
	pos.y = lastSwp.y ;
    } else if (hwndActive == hwndFrame || hwndActive == hwndShape) {
        hwndTarget = lastTarget ;
        pos.x = lastSwp.x ;
	pos.y = lastSwp.y ;
    } else if (WinIsWindowShowing(hwndActive) != TRUE) {
        hwndTarget = lastTarget ;
        pos.x = lastSwp.x ;
	pos.y = lastSwp.y ;
    } else if (queryMax(hwndActive)) {
        hwndTarget = lastTarget ;
        pos.x = lastSwp.x ;
	pos.y = lastSwp.y ;
    } else if (queryOob(hwndActive)) {
        hwndTarget = lastTarget ;
        pos.x = lastSwp.x ;
	pos.y = lastSwp.y ;
    } else {
        hwndTarget = hwndActive ;
        queryPos(hwndTarget, &pos) ;
    }
    
    /*
     * check if visible
     */

    if (hwndTarget == NULLHANDLE) {
        show = FALSE ;
    } else if (hwndTarget == hwndFrame || hwndTarget == hwndShape) {
        show = FALSE ;
    } else if (! WinIsWindowShowing(hwndTarget)) {
        show = FALSE ;
    } else if (queryMax(hwndTarget)) {
        show = FALSE ;
    } else if (queryOob(hwndTarget)) {
        show = FALSE ;
    } else {
        show = TRUE ;
    }


    /*
     * If cannot attach then hide
     */
     
    if (! show) {
        if (lastShow) {
            WinSetWindowPos(hwndFrame, NULLHANDLE, 0, 0, 0, 0, SWP_HIDE) ;
	}
	lastTarget = NULLHANDLE ;
	lastActive = hwndActive ;
	lastShow = FALSE ;
	lastSwp.x = lastSwp.y = 0 ;
	return ;
    }
    
    /*
     * check waht to do
     */
     
    draw = FALSE ;
    
    if (lastShow == FALSE || hwndTarget != lastTarget) {
        draw = TRUE ;
    }
    if (pos.x != lastSwp.x || pos.y != lastSwp.y) {
        draw = TRUE  ;
    }
    if (lastActive != hwndActive) {
        draw = TRUE ;
    }
    if (! draw) {
	lastTarget = hwndTarget ;
	lastActive = hwndActive ;
	lastShow = TRUE ;
	lastSwp.x = pos.x ;
	lastSwp.y = pos.y ;
	return ;
    }
    
    /*
     * re-position
     */
     
    bmi.cbFix = sizeof(bmi) ;
    GpiQueryBitmapInfoHeader(hbmMulNorm, &bmi) ;
    WinSetWindowPos(hwndFrame, HWND_TOP, pos.x, pos.y,
            bmi.cx, bmi.cy, (SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_SHOW)) ;
    balloonMove() ;

    lastTarget = hwndTarget ;
    lastActive = hwndActive ;
    lastShow = TRUE ;
    lastSwp.x = pos.x ;
    lastSwp.y = pos.y ;
}
