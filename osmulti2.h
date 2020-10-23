/*
 * osmulti2.h - Osuwari Multi for OS/2 PM, Global Definitions
 */

#ifndef _OSMULTI2
#define _OSMULTI2

/*
 * Debugging Macro
 */
 
#ifdef  DEBUG
#define TRACE       printf
#else
#define TRACE
#endif

extern  UCHAR   ProgramPath[] ;
extern  UCHAR   ProgramName[] ;
extern  UCHAR   ProfilePath[] ;

/*
 * National Language Supports
 */

#define NLS_JA      0
#define NLS_EN      1

extern  int     ProgramLang ;

/*
 * Hack for display trouble
 */

extern  BOOL    ProgramFull ;

/*
 * Bitmaps to Load (in bitmap.c)
 */
 
extern  HBITMAP     hbmMulNorm ;
extern  HBITMAP     hbmMulOhno ;
extern  HBITMAP     hbmMulPsyu ;
extern  HBITMAP     hbmMulUru1 ;
extern  HBITMAP     hbmMulUru2 ;
extern  HBITMAP     hbmMulClos ;

BOOL    bitmapLoad(HAB hab) ;
void    bitmapFree(void) ;

extern  HDC     hdcBitmap ;
extern  HPS     hpsBitmap ;
extern  HBITMAP hbmBitmap ;

BOOL    bitmapCreate(HAB hab) ;
void    bitmapDispose(void)   ;

BOOL    bitmapAtNade(PPOINTL pt) ;
BOOL    bitmapAtFuni(PPOINTL pt) ;

void    bitmapDrawBody(HBITMAP hbm) ;
void    bitmapDrawFace(HBITMAP hbm) ;

/*
 * Message Balloon
 */
 
extern  HDC     hdcBalloon ;
extern  HPS     hpsBalloon ;
extern  HBITMAP hbmBalloon ;

BOOL    balloonCreate(HAB hab) ;
void    balloonDispose(void)   ;

void    balloonShow(void) ;
void    balloonHide(void) ;

void    balloonNade(void) ;
void    balloonFuni(void) ;
void    balloonPsyu(void) ;
void    balloonClear(void) ;
void    balloonMail(BOOL exist) ;

/*
 * Windows to Use
 */

extern  HWND    hwndFrame ;
extern  HWND    hwndShape ;
extern  HWND    hwndTalk  ;

BOOL    windowCreate(HAB hab)  ;
void    windowDispose(HAB hab) ;

/*
 * Use Timer for Trace Focus / Animation
 */

#define TIMER_ID    1
#define TIMER_MS    200

/*
 * track focus window
 */

void    trackFocus(HAB hab) ;

/*
 * animation
 */

void    animeTimer(void) ;
void    animeNade(void)  ;
void    animeFuni(void)  ;

/*
 * message balloon
 */
 
BOOL    balloonCreate(HAB hab) ;
void    balloonDispose(void) ;

void    balloonShow(void) ;
void    balloonHide(void) ;
void    balloonMove(void) ;

void    balloonNade(void) ;
void    balloonFuni(void) ;
void    balloonPsyu(void) ;
void    balloonClear(void) ;
void    balloonMail(BOOL exist) ;

/*
 * Setting Options
 */

void    setupDialog(void) ;

/*
 * About Dialog
 */

void    aboutDialog(void) ;

/*
 * Profileing
 */

BOOL    profileLoad(HAB hab) ;
void    profileSave(HAB hab) ;
 
#endif  /* _OSMULTI2 */
