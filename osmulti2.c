/*
 * osmulti2.c - Osuwari Multi for OS/2 PM, Program Entry
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSMISC
#define INCL_DOSPROCESS 
#define INCL_PM
#include <os2.h>
#include "shapewin.h"

#include "osmulti2.h"
#include "osmulres.h"

/*
 * myname - adjust and save program names
 */

UCHAR   ProgramPath[256] ;
UCHAR   ProgramName[256] ;
UCHAR   ProfilePath[256] ;

static  void    myname(PSZ me)
{
    PUCHAR  p, last ;

    /*
     * full pathname of program
     */

    for (p = me, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last != NULL) {
        strcpy(ProgramPath, me) ;
    } else if (DosSearchPath(7, "PATH", me, ProgramPath, 256) != 0) {
        strcpy(ProgramPath, me) ;
    }

    /*
     * basename of program
     */

    for (p = ProgramPath, last = NULL ; *p ; p++) {
        if (*p == '/' || *p == '\\') {
            last = p ;
        }
    }
    if (last == NULL) {
        strcpy(ProgramName, ProgramPath) ;
    } else {
        strcpy(ProgramName, &last[1]) ;
    }
    if ((p = strrchr(ProgramName, '.')) != NULL) {
        *p = '\0' ;
    }

    /*
     * pathname of Profile
     */

    strcpy(ProfilePath, ProgramPath) ;

    if ((p = strrchr(ProfilePath, '.')) == NULL) {
        strcat(ProfilePath, ".ini") ;
    } else {
        strcpy(p, ".ini") ;
    }
}

/*
 * parseArgs - parse command line arguments
 */

int     ProgramLang = NLS_JA ;
BOOL    ProgramIdle = FALSE  ;
BOOL    ProgramFull = FALSE  ;

static  void    parseArgs(int ac, char *av[])
{
    int     i ;

    for (i = 1 ; i < ac ; i++) {
        if (stricmp(av[i], "ja") == 0) {
	    ProgramLang = NLS_JA ;
	} else if (stricmp(av[i], "en") == 0) {
	    ProgramLang = NLS_EN ;
	} else if (stricmp(av[i], "idle") == 0) {
	    ProgramIdle = TRUE ;
	} else if (stricmp(av[i], "full") == 0) {
	    ProgramFull = TRUE ;
	}
    }
}

/*
 * Error Notify
 */

void    osmMessage(PSZ msg)
{
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, msg, ProgramName, 0, MB_OK) ;
}

/*
 * main - program start here
 */

int     main(int ac, char *av[])
{
    HAB     hab  ;
    HMQ     hmq  ;
    QMSG    qmsg ;
    SWP     swp  ;

    /*
     * Initializing
     */

    myname(av[0]) ;
    _wildcard(&ac, &av) ;
    parseArgs(ac, av)   ;

    hab = WinInitialize(0) ;
    hmq = WinCreateMsgQueue(hab, 0) ;

    profileLoad(hab) ;

    if (bitmapLoad(hab) != TRUE) {
        osmMessage("failed to load Bitmaps") ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 1 ;
    }
    if (windowCreate(hab) != TRUE) {
        osmMessage("failed to create Windows") ;
	bitmapFree() ;
        WinDestroyMsgQueue(hmq) ;
        WinTerminate(hab) ;
	return 2 ;
    }
    
    if (ProgramIdle) {
        DosSetPriority(PRTYS_PROCESS, PRTYC_IDLETIME, 31, 0) ;
    }
    
    /*
     * Start Window Processing
     */

    while (WinGetMsg(hab, &qmsg, 0, 0, 0)) {
        WinDispatchMsg(hab, &qmsg) ;
    }

    /*
     * dispose resources
     */

    windowDispose(hab) ;
    bitmapFree() ;
    
    WinDestroyMsgQueue(hmq) ;
    WinTerminate(hab) ;

    return 0 ;
}
