/*
 * bitmap.c - Load/Dispose Bitmaps
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "shapewin.h"

#include "osmulti2.h"
#include "osmulres.h"

/*
 * Bitmaps to Load
 */
 
HBITMAP     hbmMulNorm = NULLHANDLE ;
HBITMAP     hbmMulOhno = NULLHANDLE ;
HBITMAP     hbmMulPsyu = NULLHANDLE ;
HBITMAP     hbmMulUru1 = NULLHANDLE ;
HBITMAP     hbmMulUru2 = NULLHANDLE ;
HBITMAP     hbmMulClos = NULLHANDLE ;

/*
 * checkLoaded - check if bitmaps loaded
 */

static  BOOL    checkLoaded(void)
{
    if (hbmMulNorm == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMulOhno == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMulPsyu == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMulUru1 == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMulUru2 == NULLHANDLE) {
        return FALSE ;
    }
    if (hbmMulClos == NULLHANDLE) {
        return FALSE ;
    }
    return TRUE ;
}

/*
 * bitmapFree - free bitmaps if exists
 */

void    bitmapFree(void)
{
    if (hbmMulNorm != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulNorm) ;
	hbmMulNorm = NULLHANDLE ;
    }
    if (hbmMulOhno != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulOhno) ;
	hbmMulOhno = NULLHANDLE ;
    }
    if (hbmMulPsyu != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulPsyu) ;
	hbmMulPsyu = NULLHANDLE ;
    }
    if (hbmMulUru1 != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulUru1) ;
	hbmMulUru1 = NULLHANDLE ;
    }
    if (hbmMulUru2 != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulUru2) ;
	hbmMulUru2 = NULLHANDLE ;
    }
    if (hbmMulClos != NULLHANDLE) {
        GpiDeleteBitmap(hbmMulClos) ;
	hbmMulClos = NULLHANDLE ;
    }
}

/*
 * loadOne - load a Bitmap
 */

static  HBITMAP loadOne(HAB hab, int id)
{
    HDC     hdc ;
    HPS     hps ;
    SIZEL   siz ;
    HBITMAP hbm ;
    
    siz.cx = siz.cy = 0 ;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    hps = GpiCreatePS(hab, hdc, &siz,
                PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;
    if (hdc == NULLHANDLE || hps == NULLHANDLE) {
        if (hps != NULLHANDLE) GpiDestroyPS(hps) ;
        if (hdc != NULLHANDLE) DevCloseDC(hdc)   ;
        return NULLHANDLE ;
    }

    hbm = GpiLoadBitmap(hps, NULLHANDLE, id, 0, 0) ;
    
    GpiDestroyPS(hps) ;
    DevCloseDC(hdc) ;
    
    TRACE("load bitmap ID %d, HBITMAP %08x\n", id, hbm) ;
    
    return hbm ;
}

/*
 * bitmapLoad - load bitmaps
 */

BOOL    bitmapLoad(HAB hab)
{
    hbmMulNorm = loadOne(hab, ID_MULNORM) ;
    hbmMulOhno = loadOne(hab, ID_MULOHNO) ;
    hbmMulPsyu = loadOne(hab, ID_MULPSYU) ;
    hbmMulUru1 = loadOne(hab, ID_MULURU1) ;
    hbmMulUru2 = loadOne(hab, ID_MULURU2) ;
    hbmMulClos = loadOne(hab, ID_MULCLOS) ;
    
    if (checkLoaded() != TRUE) {
        bitmapFree() ;
	return FALSE ;
    }
    return TRUE ;
}

/*
 * Dimensions for Bitmap
 */

#define BITMAP_CX       64      /* bitmap dimension */
#define BITMAP_CY      120

#define FACE_X1         12      /* face area        */
#define FACE_Y1         65
#define FACE_X2         52
#define FACE_Y2         96

#define NADE_Y1         92      /* area of 'nadenade'   */
#define NADE_Y2         120
#define FUNI_Y1         45      /* area of 'funifuni'   */
#define FUNI_Y2         60

/*
 * Memory DC/PS & Bitmap for Multi Bitmap
 */

HDC     hdcBitmap = NULLHANDLE ;
HPS     hpsBitmap = NULLHANDLE ;
HBITMAP hbmBitmap = NULLHANDLE ;

/*
 * bitmapDispose - dispose resources related to working Bitmap
 */

void    bitmapDispose(void)
{
    if (hbmBitmap != NULLHANDLE) {
        GpiDeleteBitmap(hbmBitmap) ;
	hbmBitmap = NULLHANDLE ;
    }
    if (hpsBitmap != NULLHANDLE) {
        GpiSetBitmap(hpsBitmap, NULLHANDLE) ;
        GpiDestroyPS(hpsBitmap) ;
	hpsBitmap == NULLHANDLE ;
    }
    if (hdcBitmap != NULLHANDLE) {
        DevCloseDC(hdcBitmap) ;
	hdcBitmap = NULLHANDLE ;
    }
}

/*
 * bitmapCreate - create Memory DC/PS & Bitmap Area for Bitmap
 */

BOOL    bitmapCreate(HAB hab)
{
    SIZEL   siz ;
    POINTL  apt[4] ;
    BITMAPINFOHEADER2   bmi ;

    /*
     * Create Memory DC/PS for play with Bitmap
     */

    hdcBitmap =DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    hpsBitmap = GpiCreatePS(hab, hdcBitmap, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;
    if (hdcBitmap == NULLHANDLE || hpsBitmap == NULLHANDLE) {
        return FALSE ;
    }
    
    /*
     * create and associate bitmap for Image Bitmap
     */

    memset(&bmi, 0, sizeof(bmi)) ;
    bmi.cbFix = sizeof(bmi) ;
    bmi.cx = BITMAP_CX ;
    bmi.cy = BITMAP_CY ;
    bmi.cPlanes       = 1  ;
    bmi.cBitCount     = 24 ;
    bmi.ulCompression = 0  ;
    bmi.cclrUsed      = 0  ;
    bmi.cclrImportant = 0  ;

    hbmBitmap = GpiCreateBitmap(hpsBitmap, &bmi, 0, NULL, NULL) ;
    
    if (hbmBitmap == NULLHANDLE) {
        return FALSE ;
    }
    GpiSetBitmap(hpsBitmap, hbmBitmap) ;
    
    /*
     * draw Initial Image
     */

    apt[0].x = 0 ;
    apt[0].y = 0 ;
    apt[1].x = BITMAP_CX ;
    apt[1].y = BITMAP_CY ;
    apt[2].x = 0 ;
    apt[2].y = 0 ;
    apt[3].x = BITMAP_CX ;
    apt[3].y = BITMAP_CY ;

    GpiWCBitBlt(hpsBitmap, hbmMulNorm, 4, apt, ROP_SRCCOPY, 0) ;

    return TRUE ;
}

/*
 * Checks Mouse in Nade/Funi Area
 */

BOOL    bitmapAtNade(PPOINTL pt)
{
    if (pt->y > NADE_Y1 && pt->y < NADE_Y2) {
        return TRUE ;
    }
    return FALSE ;
}

BOOL    bitmapAtFuni(PPOINTL pt)
{
    if (pt->y > FUNI_Y1 && pt->y < FUNI_Y2) {
        return TRUE ;
    }
    return FALSE ;
}

/*
 * bitmapDrawBody - draw entire body
 */

void    bitmapDrawBody(HBITMAP hbm)
{
    POINTL  apt[4] ;
    RECTL   rect   ;
    
    apt[0].x = 0 ;
    apt[0].y = 0 ;
    apt[1].x = BITMAP_CX ;
    apt[1].y = BITMAP_CY ;
    apt[2].x = 0 ;
    apt[2].y = 0 ;
    apt[3].x = BITMAP_CX ;
    apt[3].y = BITMAP_CY ;
    
    rect.xLeft   = 0 ;
    rect.xRight  = BITMAP_CX ;
    rect.yBottom = 0 ;
    rect.yTop    = BITMAP_CY ;

    GpiWCBitBlt(hpsBitmap, hbm, 4, apt, ROP_SRCCOPY, 0) ;
    WinSendMsg(hwndShape, SHAPEWIN_MSG_UPDATE, MPFROMP(&rect), NULL) ;
}

/*
 * bitmapDrawFace - draw face only
 */

void    bitmapDrawFace(HBITMAP hbm)
{
    POINTL  apt[4] ;
    RECTL   rect   ;
 
    if (ProgramFull == FALSE) {
        apt[0].x =  FACE_X1 ;
        apt[0].y =  FACE_Y1 ;
        apt[1].x =  FACE_X2 ;
        apt[1].y =  FACE_Y2 ;
        apt[2].x =  FACE_X1 ;
        apt[2].y =  FACE_Y1 ;
        apt[3].x =  FACE_X2 ;
        apt[3].y =  FACE_Y2 ;

        rect.xLeft   =  FACE_X1 ;
        rect.xRight  =  FACE_Y1 ;
        rect.yBottom =  FACE_X2 ;
        rect.yTop    =  FACE_Y2 ;
    } else {
        apt[0].x = 0 ;
        apt[0].y = 0 ;
        apt[1].x = BITMAP_CX ;
        apt[1].y = BITMAP_CY ;
        apt[2].x = 0 ;
        apt[2].y = 0 ;
        apt[3].x = BITMAP_CX ;
        apt[3].y = BITMAP_CY ;
    
        rect.xLeft   = 0 ;
        rect.xRight  = BITMAP_CX ;
        rect.yBottom = 0 ;
        rect.yTop    = BITMAP_CY ;
    }
    
    GpiWCBitBlt(hpsBitmap, hbm, 4, apt, ROP_SRCCOPY, 0) ;
    WinSendMsg(hwndShape, SHAPEWIN_MSG_UPDATE, MPFROMP(&rect), NULL) ;
}


