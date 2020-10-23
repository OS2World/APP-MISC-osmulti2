/*
 * balloon.c - Message Balloon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_PM
#include <os2.h>

#include "shapewin.h"

#include "osmulti2.h"
#include "osmulres.h"

/*
 * control message outputs
 */

BOOL    useMsg = TRUE  ;
BOOL    useSnd = FALSE ;

/*
 * Dimensions for Message Balloon
 */

#define TALK_CX         160     /* width            */
#define TALK_CY         104     /* height           */
#define TALK_OFF        16      /* Y above Bitmap   */

#define TALK_RND        15      /* Rounding Value   */
#define TALK_RIM        3       /* Width of Rim     */

#define TALK_XADJUST    6
#define TALK_YADJUST    10

#define TALK_CLR_FIL    CLR_BLUE        /* Filler       */
#define TALK_CLR_RIM    CLR_BLACK       /* RIM          */
#define TALK_CLR_BAK    CLR_WHITE       /* Message Back */
#define TALK_CLR_CHR    CLR_BLACK       /* Message Char */

static  SIZEL       talkSiz ;           /* Size of Message Area     */
static  RECTL       talkRct ;           /* Rect of Message Area     */
static  SIZEL       talkBox ;           /* Size of Char Box         */
static  POINTL      talkPos ;           /* Message Drawing Position */

/*
 * Memory DC/PS & Bitmap for Balloon Message
 */

HDC     hdcBalloon = NULLHANDLE ;
HPS     hpsBalloon = NULLHANDLE ;
HBITMAP hbmBalloon = NULLHANDLE ;

/*
 * Mssages from Resource
 */

static  PUCHAR  strNade  = NULL ;
static  PUCHAR  strFuni  = NULL ;
static  PUCHAR  strPsyu  = NULL ;
static  PUCHAR  strNade1 = NULL ;
static  PUCHAR  strNade2 = NULL ;
static  PUCHAR  strNade3 = NULL ;
static  PUCHAR  strNade4 = NULL ;
static  PUCHAR  strFuni1 = NULL ;
static  PUCHAR  strFuni2 = NULL ;
static  PUCHAR  strFuni3 = NULL ;
static  PUCHAR  strFuni4 = NULL ;

static  UCHAR   buff[256] ;

static  void    freeMessages(void)
{
    if (strNade) free(strNade) ;
    if (strFuni) free(strFuni) ;
    if (strPsyu) free(strPsyu) ;
    
    if (strNade1) free(strNade1) ;
    if (strNade2) free(strNade2) ;
    if (strNade3) free(strNade3) ;
    if (strNade4) free(strNade4) ;

    if (strFuni1) free(strFuni1) ;
    if (strFuni2) free(strFuni2) ;
    if (strFuni3) free(strFuni3) ;
    if (strFuni4) free(strFuni4) ;
    
    strNade = strFuni = strPsyu = NULL ;
    strNade1 = strNade2 = strNade3 = strNade4 = NULL ;
    strFuni1 = strFuni2 = strFuni3 = strFuni4 = NULL ;
}

static  BOOL    loadMessages(HAB hab)
{
    ULONG   idNade, idFuni, idPsyu ;
    ULONG   idNade1, idNade2, idNade3, idNade4 ;
    ULONG   idFuni1, idFuni2, idFuni3, idFuni4 ;
    
    switch (ProgramLang) {
    case NLS_JA :
        idNade  = IDS_NADE  ;
	idFuni  = IDS_FUNI  ;
	idPsyu  = IDS_PSYU  ;
	idNade1 = IDS_NADE1 ;
	idNade2 = IDS_NADE2 ;
	idNade3 = IDS_NADE3 ;
	idNade4 = IDS_NADE4 ;
	idFuni1 = IDS_FUNI1 ;
	idFuni2 = IDS_FUNI2 ;
	idFuni3 = IDS_FUNI3 ;
	idFuni4 = IDS_FUNI4 ;
        break ;
    case NLS_EN :
        idNade  = IDSE_NADE  ;
	idFuni  = IDSE_FUNI  ;
	idPsyu  = IDSE_PSYU  ;
	idNade1 = IDSE_NADE1 ;
	idNade2 = IDSE_NADE2 ;
	idNade3 = IDSE_NADE3 ;
	idNade4 = IDSE_NADE4 ;
	idFuni1 = IDSE_FUNI1 ;
	idFuni2 = IDSE_FUNI2 ;
	idFuni3 = IDSE_FUNI3 ;
	idFuni4 = IDSE_FUNI4 ;
        break ;
    default     :
        idNade  = IDSE_NADE  ;
	idFuni  = IDSE_FUNI  ;
	idPsyu  = IDSE_PSYU  ;
	idNade1 = IDSE_NADE1 ;
	idNade2 = IDSE_NADE2 ;
	idNade3 = IDSE_NADE3 ;
	idNade4 = IDSE_NADE4 ;
	idFuni1 = IDSE_FUNI1 ;
	idFuni2 = IDSE_FUNI2 ;
	idFuni3 = IDSE_FUNI3 ;
	idFuni4 = IDSE_FUNI4 ;
        break ;
    }
    
    if (strNade == NULL) {
        WinLoadString(hab, NULLHANDLE, idNade, 256, buff) ;
	strNade = strdup(buff) ;
    }
    if (strFuni == NULL) {
        WinLoadString(hab, NULLHANDLE, idFuni, 256, buff) ;
	strFuni = strdup(buff) ;
    }
    if (strPsyu == NULL) {
        WinLoadString(hab, NULLHANDLE, idPsyu, 256, buff) ;
	strPsyu = strdup(buff) ;
    }
    if (strNade1 == NULL) {
        WinLoadString(hab, NULLHANDLE, idNade1, 256, buff) ;
	strNade1 = strdup(buff) ;
    }
    if (strNade2 == NULL) {
        WinLoadString(hab, NULLHANDLE, idNade2, 256, buff) ;
	strNade2 = strdup(buff) ;
    }
    if (strNade3 == NULL) {
        WinLoadString(hab, NULLHANDLE, idNade3, 256, buff) ;
	strNade3 = strdup(buff) ;
    }
    if (strNade4 == NULL) {
        WinLoadString(hab, NULLHANDLE, idNade4, 256, buff) ;
	strNade4 = strdup(buff) ;
    }

    if (strFuni1 == NULL) {
        WinLoadString(hab, NULLHANDLE, idFuni1, 256, buff) ;
	strFuni1 = strdup(buff) ;
    }
    if (strFuni2 == NULL) {
        WinLoadString(hab, NULLHANDLE, idFuni2, 256, buff) ;
	strFuni2 = strdup(buff) ;
    }
    if (strFuni3 == NULL) {
        WinLoadString(hab, NULLHANDLE, idFuni3, 256, buff) ;
	strFuni3 = strdup(buff) ;
    }
    if (strFuni4 == NULL) {
        WinLoadString(hab, NULLHANDLE, idFuni4, 256, buff) ;
	strFuni4 = strdup(buff) ;
    }
    
    if (strNade == NULL || strFuni == NULL || strPsyu == NULL) {
        return FALSE ;
    }
    if (strNade1 == NULL || strNade2 == NULL || strNade3 == NULL || strNade4 == NULL) {
        return FALSE ;
    }
    if (strFuni1 == NULL || strFuni2 == NULL || strFuni3 == NULL || strFuni4 == NULL) {
        return FALSE ;
    }
    return TRUE ;
}

/*
 * balloonDispose - dispose resources for Balloon Message
 */

void    balloonDispose(void)
{
    freeMessages() ;
    
    if (hbmBalloon != NULLHANDLE) {
        GpiDeleteBitmap(hbmBalloon) ;
	hbmBalloon = NULLHANDLE ;
    }
    if (hpsBalloon != NULLHANDLE) {
        GpiSetBitmap(hpsBalloon, NULLHANDLE) ;
        GpiDestroyPS(hpsBalloon) ;
	hpsBalloon == NULLHANDLE ;
    }
    if (hdcBalloon != NULLHANDLE) {
        DevCloseDC(hdcBalloon) ;
	hdcBalloon = NULLHANDLE ;
    }
}

/*
 * balloonCreate - create Memory DC/PS & Bitmap for Balloon Message
 */

BOOL    balloonCreate(HAB hab)
{
    SIZEL   siz ;
    POINTL  pt1, pt2 ;
    BITMAPINFOHEADER2   bmi ;
    POINTL  apt[TXTBOX_COUNT] ;

    if (loadMessages(hab) != TRUE) {
        freeMessages() ;
	return FALSE ;
    }
    
    /*
     * Create Memory DC/PS for Message Ballooon
     */

    hdcBalloon =DevOpenDC(hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE) ;
    siz.cx = siz.cy = 0 ;
    hpsBalloon = GpiCreatePS(hab, hdcBalloon, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC) ;
    if (hdcBalloon == NULLHANDLE || hpsBalloon == NULLHANDLE) {
        return FALSE ;
    }
    
    /*
     * create and associate bitmap for Message Balloon
     */

    memset(&bmi, 0, sizeof(bmi)) ;
    bmi.cbFix = sizeof(bmi) ;
    bmi.cx = TALK_CX ;
    bmi.cy = TALK_CY ;
    bmi.cPlanes       = 1  ;
    bmi.cBitCount     = 24 ;
    bmi.ulCompression = 0  ;
    bmi.cclrUsed      = 0  ;
    bmi.cclrImportant = 0  ;

    hbmBalloon = GpiCreateBitmap(hpsBalloon, &bmi, 0, NULL, NULL) ;
    
    if (hbmBalloon == NULLHANDLE) {
        return FALSE ;
    }
    GpiSetBitmap(hpsBalloon, hbmBalloon) ;
    
    /*
     * draw message balloon
     */

    pt1.x = 0 ;
    pt1.y = 0 ;
    pt2.x = TALK_CX ;
    pt2.y = TALK_CY ;
    
    GpiMove(hpsBalloon, &pt1) ;
    GpiSetColor(hpsBalloon, TALK_CLR_FIL) ;
    GpiBox(hpsBalloon, DRO_FILL, &pt2, 0, 0) ;

    GpiMove(hpsBalloon, &pt1) ;
    GpiSetColor(hpsBalloon, TALK_CLR_RIM) ;
    GpiBox(hpsBalloon, DRO_FILL, &pt2, TALK_RND, TALK_RND) ;

    pt1.x += TALK_RIM ;
    pt1.y += TALK_RIM ;
    pt2.x -= TALK_RIM ;
    pt2.y -= TALK_RIM ;
    GpiMove(hpsBalloon, &pt1) ;
    GpiSetColor(hpsBalloon, TALK_CLR_BAK) ;
    GpiBox(hpsBalloon, DRO_FILL, &pt2, TALK_RND, TALK_RND) ;

    /*
     * Calc. Message Positions
     */

    talkRct.xLeft   = TALK_XADJUST ;
    talkRct.yBottom = TALK_YADJUST ;
    talkRct.xRight  = TALK_CX - TALK_XADJUST ;
    talkRct.yTop    = TALK_CY - TALK_YADJUST ;

    TRACE("Message Area %d, %d - %d, %d\n",
            talkRct.xLeft, talkRct.yBottom, talkRct.xRight, talkRct.yTop) ;

    talkSiz.cx = talkRct.xRight - talkRct.xLeft ;
    talkSiz.cy = talkRct.yTop - talkRct.yBottom ;

    /*
     * Query Text Size
     */
     
    GpiQueryTextBox(hpsBalloon, 1, "M", TXTBOX_COUNT, apt) ;
    talkBox.cx = apt[TXTBOX_TOPRIGHT].x - apt[TXTBOX_TOPLEFT].x   ;
    talkBox.cy = apt[TXTBOX_TOPLEFT].y - apt[TXTBOX_BOTTOMLEFT].y ;
    
    TRACE("TextBox %d x %d\n", talkBox.cx, talkBox.cy) ;
    
    talkPos.x = talkRct.xLeft ;
    talkPos.y = talkRct.yTop  ;
    
    return TRUE ;
}

/*
 * balloonShow/Hide - show / hide Balloon Message
 */

static  BOOL    visible = FALSE ;

void    balloonShow(void)
{
    SWP     scr ;
    SWP     swp ;
    
    TRACE("balloonShow\n") ;
    
    if (useMsg == FALSE) {
        WinSetWindowPos(hwndTalk, NULLHANDLE, 0, 0, 0, 0, SWP_HIDE) ;
        visible = FALSE ;
        return ;
    }
    
    WinQueryWindowPos(HWND_DESKTOP, &scr) ;
    WinQueryWindowPos(hwndShape, &swp)    ;
    
    if (swp.x < (scr.cx / 2)) {
        swp.x = swp.x + swp.cx ;        /* balloon at left  */
    } else {
        swp.x = swp.x - TALK_CX ;       /* balloon at right */
    }
    swp.y = swp.y + TALK_OFF ;

    WinSetWindowPos(hwndTalk, HWND_TOP, 
            swp.x, swp.y, 0, 0, (SWP_MOVE | SWP_ZORDER | SWP_SHOW)) ;
    visible = TRUE ;
}

void    balloonHide(void)
{
    TRACE("balloonHide\n") ;

    WinSetWindowPos(hwndTalk, NULLHANDLE, 0, 0, 0, 0, SWP_HIDE) ;
    visible = FALSE ;
}

/*
 * balloonMove - Move Balloon with Bitmap
 */

void    balloonMove(void)
{
    TRACE("balloonMove\n") ;

    if (visible) {
        balloonShow() ;
    } else {
        balloonHide() ;
    }
}

/*
 * message drawing functions
 */

static  void    cleanup(void)
{
    POINTL  pt1, pt2 ;
    
    /*
     * clear message area
     */
     
    pt1.x = talkRct.xLeft   ;
    pt1.y = talkRct.yBottom ;
    pt2.x = talkRct.xRight  ;
    pt2.y = talkRct.yTop    ;
    GpiMove(hpsBalloon, &pt1) ;
    GpiSetColor(hpsBalloon, TALK_CLR_BAK) ;
    GpiBox(hpsBalloon, DRO_FILL, &pt2, 0, 0) ;

    /*
     * reset drawing position
     */
     
    talkPos.x = talkRct.xLeft ;
    talkPos.y = talkRct.yTop  ;
}

static  void    scrollup(void)
{
    POINTL  apt[3] ;
    POINTL  pt1, pt2 ;
    
    /*
     * scrollup messages
     */
     
    apt[0].x = talkRct.xLeft          ;
    apt[0].y = talkRct.yBottom + talkBox.cy - 2 ;
    apt[1].x = talkRct.xRight         ;
    apt[1].y = talkRct.yTop           ;
    apt[2].x = talkRct.xLeft          ;
    apt[2].y = talkRct.yBottom - 2    ;
    
    GpiBitBlt(hpsBalloon, hpsBalloon, 3, apt, ROP_SRCCOPY, 0) ;

    /*
     * clear bottom area
     */

    pt1.x = talkRct.xLeft              ;
    pt1.y = talkRct.yBottom - 2        ;
    pt2.x = talkRct.xRight             ;
    pt2.y = talkPos.y + talkBox.cy - 2 ;
    GpiMove(hpsBalloon, &pt1) ;
    GpiSetColor(hpsBalloon, TALK_CLR_BAK) ;
    GpiBox(hpsBalloon, DRO_FILL, &pt2, 0, 0) ;
}

static  void    drawLine(PUCHAR str, int len)
{
    talkPos.x = talkRct.xLeft ;
    if ((talkPos.y - talkBox.cy) >= talkRct.yBottom) {
        talkPos.y -= talkBox.cy ;
    } else {
        scrollup() ;
    }
    GpiSetColor(hpsBalloon, TALK_CLR_CHR) ;
    GpiCharStringAt(hpsBalloon, &talkPos, len, str) ;
}

#define iskanji(c)  (((c)>0x80 && (c)<0xa0) || ((c)>=0xe0 && (c)<0xfd))

static  void    drawWrapJa(PUCHAR str, int len)
{
    int     n, bump, wid ;
    POINTL  apt[TXTBOX_COUNT] ;

    for (n = 0 ; len > 0 && n < len ; ) {
        if (iskanji(str[n])) {
	    bump = 2 ;
	} else {
	    bump = 1 ;
	}
        GpiQueryTextBox(hpsBalloon, (n + bump), str, TXTBOX_COUNT, apt) ;
        wid = apt[TXTBOX_TOPRIGHT].x - apt[TXTBOX_TOPLEFT].x ;
	if (wid < talkSiz.cx) {
	    n += bump ;
	    continue  ;
	}
        drawLine(str, n) ;
        str += n ;
        len -= n ;
        n = 0 ;
    }
    if (n > 0) {
        drawLine(str, n) ;
    }
}

static  void    drawWrapEn(PUCHAR str, int len)
{
    int     n, bump, wid ;
    POINTL  apt[TXTBOX_COUNT] ;

    for (n = 0, bump = 0 ; len > 0 && (n + bump) < len ; ) {
        if (! isspace(str[n + bump])) {
            bump += 1 ;
	    continue ;
	}
        GpiQueryTextBox(hpsBalloon, (n + bump), str, TXTBOX_COUNT, apt) ;
        wid = apt[TXTBOX_TOPRIGHT].x - apt[TXTBOX_TOPLEFT].x ;
	if (wid < talkSiz.cx) {
	    n += bump ;
	    bump = 1  ;
	    continue  ;
	}
        drawLine(str, n) ;
        str += n ;
	len -= n ;
        n = bump ;
	bump = 1 ;
    }
    if (n > 0) {
        drawLine(str, n) ;
    }
}

static  void    drawMessage(PUCHAR str)
{
    int     len, wid ;
    POINTL  apt[TXTBOX_COUNT] ;

    len = strlen(str) ;
    
    GpiQueryTextBox(hpsBalloon, len, str, TXTBOX_COUNT, apt) ;
    wid = apt[TXTBOX_TOPRIGHT].x - apt[TXTBOX_TOPLEFT].x ;
    
    if (wid < talkSiz.cx) {
        drawLine(str, len) ;
    } else if (ProgramLang == NLS_JA) {
        drawWrapJa(str, len) ;
    } else {
        drawWrapEn(str, len) ;
    }
    WinSendMsg(hwndTalk, SHAPEWIN_MSG_UPDATE, MPFROMP(&talkRct), NULL) ;
}

/*
 * put Messages on Balloon
 */

static  int     countNade = 0 ;
static  int     countFuni = 0 ;
static  BOOL    isMail = FALSE ;

void    balloonNade(void)
{
    drawMessage(strNade) ;
    
    countNade += 1 ;
    
    if (countNade == 4) {
        drawMessage(strNade1) ;
    } else if (countNade == 8) {
        drawMessage(strNade2) ;
    } else if (countNade == 12) {
        drawMessage(strNade3) ;
    } else if ((countNade % 4) == 0) {
        drawMessage(strNade4) ;
    }

    if (! visible) {
        balloonShow() ;
    }
}

void    balloonFuni(void)
{
    drawMessage(strFuni) ;
    
    countFuni += 1 ;
    
    if (countFuni == 4) {
        drawMessage(strFuni1) ;
    } else if (countFuni == 8) {
        drawMessage(strFuni2) ;
    } else if (countFuni == 12) {
        drawMessage(strFuni3) ;
    } else if (countFuni == 16) {   /* 20 > causes 'psyu'   */
        drawMessage(strFuni4) ;
    }

    if (! visible) {
        balloonShow() ;
    }
}

void    balloonPsyu(void)
{
    drawMessage(strPsyu) ;

    if (! visible) {
        balloonShow() ;
    }
    countNade = 0 ;
    countFuni = 0 ;
}

void    balloonClear(void)
{
    TRACE("balloonClear\n") ;

    if (visible) {
        balloonHide() ;
    }

    countNade = 0  ;
    countFuni = 0  ;

    cleanup() ;
}
