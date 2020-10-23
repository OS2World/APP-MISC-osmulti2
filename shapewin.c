/*
 * shapewin.c - Shape Window Control
 *
 *  version 1.02 - include speedup patch from "Ulrich Moeller (ulrich.moeller@rz.hu-berlin.de)"
 *  version 1.01 - include speedup patch from "Takayuki Suwa (jjsuwa@ibm.net)"
 *                 add some mouse events send to owner
 */

#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shapewin.h"

#ifdef  SHPDEBUG
#define TRACE       printf
#else
#define TRACE
#endif

#ifdef  SHPDEBUG
#define DUMPSWP(p)      \
        printf("SWP %08x, flag %08x, size %d %d, pos %d %d, Z-Order %08x\n",\
        (p), (p)->fl, (p)->cx, (p)->cy, (p)->x, (p)->y, (p)->hwndInsertBehind)
#else
#define DUMPSWP(p)
#endif

/*
 * forward decalations for Pointers
 */
 
typedef struct  _SHPCTLREC  *SHPCTLPTR ;    /* Control Data     */
typedef struct  _SHPRGNREC  *SHPRGNPTR ;    /* Drawing Region   */

/*
 * Internal Record to Manage Shape Window (as Window Pointer)
 */

typedef struct _SHPCTLREC {
    /*
     * Common Window Parameters (CREATESTRUCT)
     */
    HWND        hwndSelf   ;    /* Window Handle of Self    */
    HWND        hwndParent ;    /* Parent of this Control   */
    HWND        hwndOwner  ;    /* Owner  of this Control   */
    USHORT      id         ;    /* Window ID                */
    SHORT       x          ;    /* x - horz. position       */
    SHORT       y          ;    /* y - vert. position       */
    SHORT       cx         ;    /* horz. size               */
    SHORT       cy         ;    /* vert. size               */
    PSZ         pszText    ;    /* Window Text              */
    
    /*
     * Shape Window Parameters
     */

    HPS         hpsDraw ;       /* Data to Display          */
    ULONG       nRegion ;       /* Number of Rectangles     */
    SHPRGNPTR   aRegion ;       /* Array of Rectangles      */
    
} SHPCTLREC ;

/*
 * Drawing Regions represent Window Shape
 */

typedef struct _SHPRGNREC {
    SHPCTLPTR   pCtrl ;         /* Link to Parent       */
    HWND        hwnd  ;         /* Drawing Window       */
    RECTL       rect  ;         /* Rectangle of Bitmap  */
} SHPRGNREC ;

/*
 * Parse Mask Pattern to Rectangles
 */

#define STEP    1024

typedef struct _SPAN {
    ULONG   nSize    ;
    ULONG   nUsed    ;
    RECTL   aSpan[1] ;
} SPANREC, *SPANPTR ;

static  void    freeSpan(SPANPTR pSpan)
{
    if (pSpan != NULL) {
        free(pSpan) ;
    }
}

static  SPANPTR makeSpan(void)
{
    SPANPTR pSpan ;
    int     len   ;
    
    len = sizeof(SPANREC) + sizeof(RECTL) * STEP ;
    
    if ((pSpan = (SPANPTR) malloc(len)) == NULL) {
        return NULL ;
    }
    memset(pSpan, 0, len) ;
    pSpan->nSize = STEP ;
    pSpan->nUsed = 0    ;
    return pSpan ;
}

static  SPANPTR moreSpan(SPANPTR pOld)
{
    SPANPTR pNew ;
    int     len  ;
    
    TRACE("moreSpan\n") ;
    
    len = sizeof(SPANREC) + sizeof(RECTL) * (pOld->nSize + STEP) ;
    
    if ((pNew = (SPANPTR) malloc(len)) == NULL) {
        return NULL ;
    }
    memset(pNew, 0, len) ;
    pNew->nSize = pOld->nSize + STEP ;
    pNew->nUsed = pOld->nUsed ;
    memcpy(pNew->aSpan, pOld->aSpan, sizeof(RECTL) * pOld->nUsed) ;
    freeSpan(pOld) ;
    
    return pNew ;
}

static  SPANPTR putSpan(SPANPTR pSpan, int y, int x1, int x2)
{
    int     i ;
    PRECTL  p ;
    SPANPTR pNew ;
    
    TRACE("putSpan %d (%d %d)\n", y, x1, x2) ;
    
    /*
     * check if continuous one
     */
     
    for (i = 0 ; i < pSpan->nUsed ; i++) {
        p = &pSpan->aSpan[i] ;
	if (p->yTop == y && p->xLeft == x1 && p->xRight == x2) {
	    p->yTop += 1 ;
	    return pSpan ;      /* merged */
	}
    }
    
    /*
     * if not enough space, expand
     */

    if ((pSpan->nUsed + 1) >= pSpan->nSize) {
        if ((pNew = moreSpan(pSpan)) == NULL) {
	    return NULL ;
	}
	pSpan = pNew ;
    }

    /*
     * append a rectangle
     */
     
    pSpan->aSpan[pSpan->nUsed].yTop = y + 1 ;
    pSpan->aSpan[pSpan->nUsed].yBottom = y  ;
    pSpan->aSpan[pSpan->nUsed].xLeft   = x1 ;
    pSpan->aSpan[pSpan->nUsed].xRight  = x2 ;
    pSpan->nUsed += 1 ;
    
    return pSpan ;
}

#ifdef  SHPDEBUG

static  BOOL    ptisin(SPANPTR pSpan, int x, int y)
{
    int     i ;
    
    for (i = 0 ; i < pSpan->nUsed ; i++) {
        if (y >= pSpan->aSpan[i].yTop) {
	    continue ;
	}
	if (y < pSpan->aSpan[i].yBottom) {
	    continue ;
	}
	if (x < pSpan->aSpan[i].xLeft) {
	    continue ;
	}
	if (x >= pSpan->aSpan[i].xRight) {
	    continue ;
	}
	return TRUE ;
    }
    return FALSE ;
}

static  void    dumpSpan(SPANPTR pSpan)
{
    int     i, maxx, maxy, x, y ;
    
    TRACE("dumpSpan %d\n", pSpan->nUsed) ;
    
    maxx = maxy = 0 ;
    
    for (i = 0 ; i < pSpan->nUsed ; i++) {
        if (pSpan->aSpan[i].yTop > maxy) {
	    maxy = pSpan->aSpan[i].yTop ;
	}
	if (pSpan->aSpan[i].xRight > maxx) {
	    maxx = pSpan->aSpan[i].xRight ;
	}
    }
    
    for (y = maxy - 1 ; y >= 0 ; y--) {
        printf("%03d : ", y) ;
        for (x = 0 ; x < maxx ; x++) {
	    if (ptisin(pSpan, x, y)) {
	        printf("#") ;
	    } else {
	        printf("_") ;
	    }
        }
	printf("\n") ;
    }
}

#endif  /* SHPDEBUG */

static  SPANPTR parseMask(HPS hps, PBITMAPINFOHEADER2 bmih2)
{
    int     blen, hlen  ;
    PUCHAR          buf ;
    PBITMAPINFO2    pbm ;
    SPANPTR pSpan, pNew ;
    int     x, y, k     ;
    LONG    first, color ;
    BOOL    inspan ;
    int     left   ;
    
    TRACE("parseMask\n") ;

    hlen = sizeof(BITMAPINFO2) + sizeof(RGB) * 256 ;
    blen = ((bmih2->cBitCount * bmih2->cx + 31) / 32) * bmih2->cPlanes * 4 ;
    
    pbm = (PBITMAPINFO2) malloc(hlen) ;
    buf = (PUCHAR) malloc(blen) ;
    
    if (pbm == NULL || buf == NULL) {
        TRACE("parseMask - failed to alloc %d %d\n", hlen, blen) ;
	if (pbm) free(pbm) ;
        if (buf) free(buf) ;
        return NULL ;
    }
    memcpy(pbm, bmih2, sizeof(BITMAPINFOHEADER2)) ;

    if ((pSpan = makeSpan()) == NULL) {
        TRACE("parseMask - failed to make\n") ;
	free(pbm) ;
	free(buf) ;
	return NULL ;
    }
    
    first = - 1 ;
    
    for (y = 0 ; y < bmih2->cy ; y++) {

        TRACE("parseMask - scan line %d\n", y) ; fflush(stdout) ;

        GpiQueryBitmapBits(hps, y, 1, buf, pbm) ;

	for (x = 0, inspan = FALSE ; x < bmih2->cx ; x++) {
	    k = x * 3 ;
	    color = ((buf[k] << 16) | (buf[k+1] << 8) | buf[k+2]) ;
	    if (first < 0) {
	        first = color ;
	    }
	    if (inspan == FALSE && color != first) {
		inspan = TRUE ;
		left = x ;
	    } else if (inspan == TRUE && color == first) {
		inspan = FALSE ;
		if ((pNew = putSpan(pSpan, y, left, x)) != NULL) {
		    pSpan = pNew ;
		} else {
		    TRACE("parseMask - failed to extend\n") ;
                    break ;
		}
	    }
	}
	if (inspan == TRUE) {
            if ((pNew = putSpan(pSpan, y, left, x)) != NULL) {
	        pSpan = pNew ;
	    } else {
	        TRACE("parseMask - failed to extend\n") ;
                break ;
	    }
	}
    }

#ifdef  SHPDEBUG
    dumpSpan(pSpan) ;
#endif

    return pSpan ;
}

/*
 * ShpMakeRegion - Make Drawing Region from Mask Image (PS)
 */

static  int     ShpMakeRegion(SHPCTLPTR pCtrl, HPS hpsMask)
{
    HAB     hab ;
    HDC     hdc ;
    HPS     hps ;
    SIZEL   siz ;
    HBITMAP hbm ;
    BITMAPINFOHEADER2   bmi ;
    POINTL  apt[3] ;
    SPANPTR     pSpan ;
    SHPRGNPTR   pRegn ;
    int         i     ;
    
    TRACE("ShpMakeRegion\n") ;
    
    hab = WinQueryAnchorBlock(pCtrl->hwndSelf) ;
    
    /*
     * Create Memory DC & HPS
     */
    
    hdc = DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    hps = GpiCreatePS(hab, hdc, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;

    /*
     * Create Bitmap and relate to memory PS
     */
     
    memset(&bmi, 0, sizeof(bmi)) ;

    bmi.cbFix = sizeof(BITMAPINFOHEADER2) ;
    bmi.cx = pCtrl->cx ;
    bmi.cy = pCtrl->cy ;
    bmi.cPlanes       = 1  ;
    bmi.cBitCount     = 24 ;
    bmi.ulCompression = 0  ;
    bmi.cclrUsed      = 0  ;
    bmi.cclrImportant = 0  ;
    
    hbm = GpiCreateBitmap(hps, &bmi, 0, NULL, NULL) ;
    
    GpiSetBitmap(hps, hbm) ;
    
    /*
     * Copy in Mask Pattern
     */

    TRACE("ShpMakeRegion - copyin %d x %d mask pattern\n", bmi.cx, bmi.cy) ;

    apt[0].x = 0 ;          /* Target       */
    apt[0].y = 0 ;
    apt[1].x = bmi.cx ;
    apt[1].y = bmi.cy ;
    apt[2].x = 0 ;          /* Source       */
    apt[2].y = 0 ;

    if (GpiBitBlt(hps, pCtrl->hpsDraw, 3, apt, ROP_SRCCOPY, 0) == GPI_ERROR) {
        TRACE("MakeRect - BitBlt Failed %08x, hdc %08x, hps %08x, hbm %08x\n",
                WinGetLastError(hab), hdc, hps, hbm) ;
    }
    
    /*
     * Parse Mask Pattern
     */
    
    if ((pSpan = parseMask(hps, &bmi)) != NULL) {
        if ((pRegn = (SHPRGNPTR) malloc(sizeof(SHPRGNREC) * pSpan->nUsed)) == NULL) {
	    TRACE("ShpMakeRegion - failed to alloc\n") ;
	} else {
	    pCtrl->nRegion = pSpan->nUsed ;
	    pCtrl->aRegion = pRegn        ;
	    for (i = 0 ; i < pSpan->nUsed ; i++) {
		pRegn[i].pCtrl    = pCtrl      ;
		pRegn[i].hwnd = NULLHANDLE ;
		pRegn[i].rect.xLeft   = pSpan->aSpan[i].xLeft   ;
		pRegn[i].rect.xRight  = pSpan->aSpan[i].xRight  ;
		pRegn[i].rect.yTop    = pSpan->aSpan[i].yTop    ;
		pRegn[i].rect.yBottom = pSpan->aSpan[i].yBottom ;
	    }
        }            
        freeSpan(pSpan) ;
    }

    /*
     * dispose local resources
     */

    GpiDeleteBitmap(hbm) ;
    GpiDestroyPS(hps) ;
    DevCloseDC(hdc) ;
    
    return 0 ;
}

/*
 * regnDraw - draw a region
 */

static  void    regnDraw(HPS hps, SHPRGNPTR pRgn)
{
    POINTL      apt[3] ;

    apt[0].x = 0 ;
    apt[0].y = 0 ;
    apt[1].x = (pRgn->rect.xRight - pRgn->rect.xLeft) ;
    apt[1].y = (pRgn->rect.yTop - pRgn->rect.yBottom) ;
    apt[2].x = pRgn->rect.xLeft   ;
    apt[2].y = pRgn->rect.yBottom ;
    GpiBitBlt(hps, pRgn->pCtrl->hpsDraw, 3, apt, ROP_SRCCOPY, 0) ;
}

/*
 * Window Procedure for Drawing Regions
 */

static MRESULT EXPENTRY regnProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    SHPRGNPTR   pRgn ;
    HPS         hps  ;
    
    pRgn = (SHPRGNPTR) WinQueryWindowPtr(hwnd, 0) ;

    switch (msg) {
    
    case WM_CREATE :
        TRACE("WM_CREATE\n") ;
        pRgn = (SHPRGNPTR) PVOIDFROMMP(mp1) ;
	WinSetWindowPtr(hwnd, 0, (PVOID) pRgn) ;
	return (MRESULT) FALSE ;
	
    case WM_PAINT :
        TRACE("WM_PAINT (%d %d) (%d %d)\n", pRgn->rect.yBottom, pRgn->rect.xLeft, pRgn->rect.yTop, pRgn->rect.xRight) ;
        hps = WinBeginPaint(hwnd, NULLHANDLE, NULL) ;
	regnDraw(hps, pRgn) ;
        WinEndPaint(hps)   ;
	return (MRESULT) 0 ;

    case WM_MOUSEMOVE     :
    case WM_BUTTON1DOWN   :
    case WM_BUTTON1UP     :
    case WM_BUTTON1CLICK  :
    case WM_BUTTON1DBLCLK :
    case WM_BUTTON2DOWN   :
    case WM_BUTTON2UP     :
    case WM_BUTTON2CLICK  :
    case WM_BUTTON2DBLCLK :
    case WM_BUTTON3DOWN   :
    case WM_BUTTON3UP     :
    case WM_BUTTON3CLICK  :
    case WM_BUTTON3DBLCLK :
    case WM_CHAR          :
    case WM_VIOCHAR       :
    case WM_BEGINDRAG     :
    case WM_ENDDRAG       :
    case WM_SINGLESELECT  :
    case WM_OPEN          :
    case WM_CONTEXTMENU   :
    case WM_CONTEXTHELP   :
    case WM_TEXTEDIT      :
    case WM_BEGINSELECT   :
    case WM_ENDSELECT     :
        return WinSendMsg(pRgn->pCtrl->hwndSelf, msg, mp1, mp2) ;
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2) ;
}

/*
 * ShpOpenRegion - create window for each region
 */

static  BOOL    ShapeRegisteredRegion = FALSE ;

static  int     ShpOpenRegion(SHPCTLPTR pCtrl)
{
    int         i    ;
    SHPRGNPTR   pRgn ;
    ULONG       flStyle ;

    TRACE("ShpOpenRegion %d regions\n", pCtrl->nRegion) ;
    
    if (pCtrl->nRegion == 0 || pCtrl->aRegion == NULL) {
        TRACE("ShpOpenRegion - no region to open\n") ;
	return -1 ;
    }
   
    if (ShapeRegisteredRegion == FALSE) {
        WinRegisterClass(WinQueryAnchorBlock(pCtrl->hwndSelf),
                        "ShapeWinRegion", regnProc, CS_PARENTCLIP | CS_CLIPSIBLINGS | CS_SYNCPAINT, sizeof(PVOID)) ;
        ShapeRegisteredRegion = TRUE ;
    }

    flStyle = 0 ;
                        
    
    for (i = 0, pRgn = pCtrl->aRegion ; i < pCtrl->nRegion ; i++, pRgn++) {
        pRgn->hwnd = WinCreateWindow(
                pCtrl->hwndParent,      /* was HWND_DESKTOP, Parent Window    */
                "ShapeWinRegion",       /* Window Class     */
	        NULL,                   /* Window Text      */
	        flStyle,                /* Window Style     */
		(pCtrl->x + pRgn->rect.xLeft),
		(pCtrl->y + pRgn->rect.yBottom),
                (pRgn->rect.xRight - pRgn->rect.xLeft),
		(pRgn->rect.yTop - pRgn->rect.yBottom),
	        pCtrl->hwndOwner,       /* Owner Window     */
	        HWND_TOP,               /* Z-Order          */
	        i,                      /* Window ID        */
	        pRgn,                   /* Control Data     */
	        NULL) ;                 /* Pres. Param.     */
	        
    }
    return 0 ;
}

/*
 * ShpFreeRegion - Dispose Region Window and Related Resources
 */

static  int     ShpFreeRegion(SHPCTLPTR pCtrl)
{
    int         i    ;
    SHPRGNPTR   pRgn ;

    for (i = 0, pRgn = pCtrl->aRegion ; i < pCtrl->nRegion ; i++, pRgn++) {
        WinDestroyWindow(pRgn->hwnd) ;
    }
    free(pCtrl->aRegion) ;

    return 0 ;
}

/*
 * ShpAdjustRegion - Controls Region's Position, Visibility, etc.
 */

static  int     ShpAdjustRegion(SHPCTLPTR pCtrl, PSWP pSwp)
{
    int         i    ;
    SHPRGNPTR   pRgn ;
    ULONG       fl   ;
    HPS         hps  ;

    PSWP pswpArray ;

    TRACE("ShpAdjustRegion - %d, %08x\n", pCtrl->nRegion, pSwp->fl) ;

    if (pCtrl->nRegion == 0 || pCtrl->aRegion == NULL) {
        TRACE("ShpDrawRegion - no region to open\n") ;
	return -1 ;
    }

    if ((fl = pSwp->fl) & SWP_MOVE) {
        pCtrl->x = pSwp->x ;
	pCtrl->y = pSwp->y ;
    }

    /* Mr.Suwa said use of WinSetMultWindowPos speed up redrawing   */
    /* operation.                                                   */
    pswpArray = (PSWP) malloc(sizeof(SWP) * pCtrl->nRegion) ;

    for (i = 0, pRgn = pCtrl->aRegion ; i < pCtrl->nRegion ; i++, pRgn++) {
        pswpArray[i].fl = fl ;
        pswpArray[i].cy = (pRgn->rect.yTop - pRgn->rect.yBottom) ;
        pswpArray[i].cx = (pRgn->rect.xRight - pRgn->rect.xLeft) ;
        pswpArray[i].y = (pCtrl->y + pRgn->rect.yBottom) ;
        pswpArray[i].x = (pCtrl->x + pRgn->rect.xLeft) ;
        pswpArray[i].hwndInsertBehind = pSwp->hwndInsertBehind ;
        pswpArray[i].hwnd = pRgn->hwnd ;
    }
    WinSetMultWindowPos(WinQueryAnchorBlock(pCtrl->hwndSelf),
                                    &pswpArray[0], pCtrl->nRegion) ;
    free((void*) pswpArray) ;

    /* Mr.Moeller said it not required, becuase WinSetMultWindowPos     */
    /* manages update region, and really, it speed up lot!!             */
#if 0
    for (i = 0, pRgn = pCtrl->aRegion ; i < pCtrl->nRegion ; i++, pRgn++) {
        if (fl & (SWP_MOVE | SWP_ZORDER)) {
            WinInvalidateRect(pRgn->hwnd, NULL, FALSE) ;
        }
    }
#endif
    return 0 ;
}

/*
 * ShpUpdatetRegion - update drawing image
 */

static  int     ShpUpdateRegion(SHPCTLPTR pCtrl, PRECTL pRect)
{
    RECTL       rect, intern ;
    int         i    ;
    SHPRGNPTR   pRgn ;
    HAB         hab  ;
    HPS         hps  ;
    
    TRACE("ShpUpdateRegion\n") ;

    hab = WinQueryAnchorBlock(pCtrl->hwndSelf) ;
    
    if (pRect == NULL) {
        rect.xLeft   = 0        ;
	rect.yBottom = 0        ;
	rect.xRight = pCtrl->cx ;
	rect.yTop   = pCtrl->cy ;
	pRect = &rect ;
    }
    for (i = 0, pRgn = pCtrl->aRegion ; i < pCtrl->nRegion ; i++, pRgn++) {
        if (WinIntersectRect(hab, &intern, pRect, &pRgn->rect) == FALSE) {
	    continue ;
        } else {
            WinInvalidateRect(pRgn->hwnd, NULL, FALSE) ;
        }
    }
    return 0 ;
}

/*
 * ShpInit - Initialize Widget's Control Block
 */

static  SHPCTLPTR   ShpInit(HWND hwnd, PCREATESTRUCT pWin, PSHAPEWIN pData)
{
    SHPCTLPTR   pCtrl ;
    
    if (pData->hpsDraw == NULLHANDLE || pData->hpsMask == NULLHANDLE) {
        return NULL ;
    }
    if ((pCtrl = (SHPCTLPTR) malloc(sizeof(SHPCTLREC))) == NULL) {
        return NULL ;
    }

    /*
     * Setup Common Window Parameters
     */

    pCtrl->hwndSelf   = hwnd ;
    pCtrl->hwndParent = pWin->hwndParent ;
    pCtrl->hwndOwner  = pWin->hwndOwner  ;
    pCtrl->id         = pWin->id ;
    pCtrl->x          = pWin->x  ;
    pCtrl->y          = pWin->y  ;
    pCtrl->cx         = pWin->cx ;
    pCtrl->cy         = pWin->cy ;
    
    /*
     * Setup Image Window 's Control Data
     */

    pCtrl->cx = pData->cx ;
    pCtrl->cy = pData->cy ;
    pCtrl->hpsDraw = pData->hpsDraw ;

    ShpMakeRegion(pCtrl, pData->hpsMask) ;
    ShpOpenRegion(pCtrl) ;

    return pCtrl ;
}

/*
 * ShpTerm - Dispose Widget's Control Block
 */

static  void    ShpTerm(SHPCTLPTR pCtrl)
{
    if (pCtrl == NULL) {
        return ;
    }
    ShpFreeRegion(pCtrl) ;
    free(pCtrl) ;
}

/*
 * Window Procedure
 */

MRESULT EXPENTRY    ShapeWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    SHPCTLPTR   pCtrl ;
    HPS         hps   ;
    PSWP        pswp  ;
    SHORT       sx, sy ;
    
    pCtrl = (SHPCTLPTR) WinQueryWindowPtr(hwnd, 0) ;
    
    switch (msg) {
    
    case WM_CREATE  :
        TRACE("WM_CREATE\n") ;
        pCtrl = ShpInit(hwnd, 
            (PCREATESTRUCT) PVOIDFROMMP(mp2), (PSHAPEWIN) PVOIDFROMMP(mp1)) ;
        if (pCtrl == NULL) {
	    return (MRESULT) TRUE ;
	}
	WinSetWindowPtr(hwnd, 0, (PVOID) pCtrl) ;
	return (MRESULT) FALSE ;
	
    case WM_DESTROY :
        TRACE("WM_DESTORY\n") ;
        ShpTerm(pCtrl) ;
	return (MRESULT) 0 ;

    case WM_ADJUSTWINDOWPOS :
        TRACE("WM_ADJUSTWINDOWPOS\n") ;
	pswp = (PSWP) PVOIDFROMMP(mp1) ;
	DUMPSWP(pswp) ;
	pswp->cx = pCtrl->cx ;          /* size is fixed        */
	pswp->cy = pCtrl->cy ;
	ShpAdjustRegion(pCtrl, pswp) ;  /* adjust sub-windows   */
	pswp->fl &= ~SWP_SHOW ;         /* not show itself      */
	return (MRESULT) 0 ;
    
    case SHAPEWIN_MSG_UPDATE :
        ShpUpdateRegion(pCtrl, (PRECTL) PVOIDFROMMP(mp1)) ;
	return (MRESULT) 0 ;

    case WM_QUERYDLGCODE :
        TRACE("WM_QUERYDLGCODE\n") ;
	return (MRESULT) DLGC_STATIC ;

    case WM_PAINT :
        TRACE("WM_PAINT\n") ;
        hps = WinBeginPaint(hwnd, NULLHANDLE, NULL) ;
	WinEndPaint(hps) ;
	return (MRESULT) 0 ;

    case WM_MOUSEMOVE     :
    case WM_BUTTON1DOWN   :
    case WM_BUTTON1UP     :
    case WM_BUTTON1CLICK  :
    case WM_BUTTON1DBLCLK :
    case WM_BUTTON2DOWN   :
    case WM_BUTTON2UP     :
    case WM_BUTTON2CLICK  :
    case WM_BUTTON2DBLCLK :
    case WM_BUTTON3DOWN   :
    case WM_BUTTON3UP     :
    case WM_BUTTON3CLICK  :
    case WM_BUTTON3DBLCLK :
    case WM_CHAR          :
    case WM_VIOCHAR       :
    case WM_BEGINDRAG     :
    case WM_ENDDRAG       :
    case WM_SINGLESELECT  :
    case WM_OPEN          :
    case WM_CONTEXTMENU   :
    case WM_CONTEXTHELP   :
    case WM_TEXTEDIT      :
    case WM_BEGINSELECT   :
    case WM_ENDSELECT     :
        return WinSendMsg(pCtrl->hwndOwner, msg, mp1, mp2) ;

    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2) ;
}
