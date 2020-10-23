/*
 * shapewin.h - Shape Window Control, external definitions
 */

#ifndef _SHAPEWIN_H
#define _SHAPEWIN_H

/*
 * Name of Control
 */

#define ShapeWinName    "ShapeWin"

/*
 * Control Data for Shape Window Control
 */
 
typedef struct _SHAPEWIN {
    SHORT   cx, cy  ;       /* Size of Shape                            */
    HPS     hpsMask ;       /* Mask Pattern, defines Shape of Window    */
    HPS     hpsDraw ;       /* Draw Data, displayed on Shape Window     */
} SHAPEWIN, *PSHAPEWIN ;

/*
 * Window Procedure for Image Window Control
 */
 
MRESULT EXPENTRY ShapeWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) ;

/*
 * Window Messages Specific for Shape Window
 */

#define SHAPEWIN_MSG_UPDATE (WM_USER + 1)
    /* SHAPEWIN_MSG_UPDATE          Update Drawing Image                    */
    /*      MP1 PRECTL              Update Region, NULL for entire region   */
    /*      MP2 NULL                not used                                */

#endif  /* _SHAPEWIN_H */
