/*
 * profile.c - load/save option settings
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_PM
#include <os2.h>

#include "osmulti2.h"

/*
 * Data to Load/Save Here
 */

extern  int     posRatio ;      /* track.c      */
extern  BOOL    useMsg   ;      /* balloon.c    */
extern  BOOL    useSnd   ;      /* balloon.c    */

/*
 * profileLoad - load option setting
 */

BOOL    profileLoad(HAB hab)
{
    HINI    hini ;
    BOOL    stat ;
    LONG    len, val ;

    if ((hini = PrfOpenProfile(hab, ProfilePath)) == NULLHANDLE) {
        TRACE("profileLoad - failed to open %s\n", ProfilePath) ;
        return FALSE ;
    }

    len = sizeof(LONG) ;
    stat = PrfQueryProfileData(hini, ProgramName, "POS_RATIO", &val, &len) ;
    if (stat == TRUE && len == sizeof(LONG)) {
        posRatio = (int) val ;
    }

    len = sizeof(LONG) ;
    stat = PrfQueryProfileData(hini, ProgramName, "USE_MSG", &val, &len) ;
    if (stat == TRUE && len == sizeof(LONG)) {
        useMsg = (BOOL) val ;
    }

    len = sizeof(LONG) ;
    stat = PrfQueryProfileData(hini, ProgramName, "USE_SND", &val, &len) ;
    if (stat == TRUE && len == sizeof(LONG)) {
        useSnd = (BOOL) val ;
    }

    PrfCloseProfile(hini) ;

    TRACE("profileLoad Pos %d, Msg %d, Snd %d\n", posRatio, useMsg, useSnd) ;
    
    return TRUE ;
}

/*
 * profileSave - save options setting
 */

void    profileSave(HAB hab)
{
    HINI    hini ;
    LONG    val  ;

    if ((hini = PrfOpenProfile(hab, ProfilePath)) == NULLHANDLE) {
        TRACE("profileSave - failed to open %s\n", ProfilePath) ;
        return ;
    }

    TRACE("profileSave Pos %d, Msg %d, Snd %d\n", posRatio, useMsg, useSnd) ;

    val = (LONG) posRatio ;
    PrfWriteProfileData(hini, ProgramName, "POS_RATIO", &val, sizeof(LONG)) ;
    val = (LONG) useMsg ;
    PrfWriteProfileData(hini, ProgramName, "USE_MSG", &val, sizeof(LONG)) ;
    val = (LONG) useSnd ;
    PrfWriteProfileData(hini, ProgramName, "USE_SND", &val, sizeof(LONG)) ;

    PrfCloseProfile(hini) ;
}
