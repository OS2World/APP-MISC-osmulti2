/*
 * anime.c - animate multi
 */

#include <stdio.h>
#include <stdlib.h>

#define INCL_PM
#include <os2.h>

#include "shapewin.h"

#include "osmulti2.h"
#include "osmulres.h"

/*
 * Animation Modes
 */
 
#define ANIME_NORM  0
#define ANIME_URUR  1
#define ANIME_OHNO  2
#define ANIME_PSYU  3

static  int     modeAnime = ANIME_NORM ;

/*
 * timers for each modes - UNIT of mili-seconds
 */

static  int     ururInit =   8000 ;
static  int     ururTime =      0 ;
static  int     ohnoInit =   8000 ;
static  int     ohnoTime =      0 ;
static  int     psyuInit =  16000 ;
static  int     psyuTime =      0 ;
static  int     psyuClear = 10000 ;

/*
 * counters controls PSYU/BLINK
 */
 
static  int     funiLimit = 20 ;    /* Win : 25, X : 40 */
static  int     funiCount =  0 ;

static  ULONG   blinkCount = 0 ;

#define BLINKCYCLE1     20
#define BLINKCYCLE2     24

/*
 * animeNade - action 'nadenade'
 */

static  int     nadeReduce = 0 ;

void    animeNade(void)
{
    if (modeAnime == ANIME_PSYU) {
        return ;
    }
    if (nadeReduce > 0) {
        nadeReduce -= 1 ;
	return ;
    }

    balloonNade() ;

    if (modeAnime != ANIME_URUR) {
        bitmapDrawBody(hbmMulUru1) ;
        modeAnime = ANIME_URUR ;
    }
    ururTime = ururInit ;
    nadeReduce = 3 ;
}

/*
 * animeFuni - action 'funifuni'
 */

void    animeFuni(void)
{
    if (modeAnime == ANIME_PSYU) {
        return ;
    }
    if (modeAnime == ANIME_OHNO) {
        if ((funiCount += 1) > funiLimit) {
	    balloonPsyu() ;
	    funiCount = 0 ;
            bitmapDrawBody(hbmMulPsyu) ;
            modeAnime = ANIME_PSYU ;
	    psyuTime = psyuInit    ;
	    return ;
	}
    }

    balloonFuni() ;

    if (modeAnime != ANIME_OHNO) {
        bitmapDrawBody(hbmMulOhno) ;
	modeAnime = ANIME_OHNO ;
	funiCount += 1 ;
    }
    ohnoTime = ohnoInit ;
}

/*
 * animeTimer - timer processing
 */

void    animeTimer(void)
{
    HBITMAP hbm = NULLHANDLE ;
    ULONG   blink1, blink2   ;
    BOOL    body = FALSE ;
    BOOL    face = FALSE ;
    
    blink1 = blinkCount % BLINKCYCLE1 ;
    blink2 = blinkCount % BLINKCYCLE2 ;
    blinkCount += 1 ;

    TRACE("animeTimer mode %d, count %d (%d, %d)\n",
                        modeAnime, blinkCount, blink1, blink2) ;

    switch (modeAnime) {
    
    case ANIME_NORM :
	if (blink1 == 0 || blink2 == 0) {
	    hbm = hbmMulClos ;
	    face = TRUE ;
	} else if (blink1 == 1 || blink2 == 1) {
	    hbm = hbmMulNorm ;
	    face = TRUE ;
	}
	break ;
        
    case ANIME_URUR :
        if ((ururTime -= TIMER_MS) < 0) {
	    balloonClear() ;
	    modeAnime = ANIME_NORM ;
	    hbm = hbmMulNorm ;
	    body = TRUE ;
	} else if ((blinkCount & 0x01) == 0) {
	    hbm = hbmMulUru1 ;
	    face = TRUE ;
        } else {
	    hbm = hbmMulUru2 ;
	    face = TRUE ;
        }
	break ;
	
    case ANIME_OHNO :
        if ((ohnoTime -= TIMER_MS) < 0) {
	    balloonClear() ;
	    modeAnime = ANIME_NORM ;
	    hbm = hbmMulNorm ;
	    body = TRUE ;
	} else if (blink1 == 0 || blink2 == 0) {
	    hbm = hbmMulPsyu ;
	    face = TRUE ;
	} else if (blink1 == 1 || blink2 == 1) {
	    hbm = hbmMulOhno ;
	    face = TRUE ;
	}
	break ;
    
    case ANIME_PSYU :
        if ((psyuTime -= TIMER_MS) < 0) {
	    modeAnime = ANIME_NORM ;
	    hbm = hbmMulNorm ;
	    body = TRUE ;
        }
	if (psyuTime < psyuClear) {
	    balloonClear() ;
	}
	break ;
    }
    
    if (body && hbm != NULLHANDLE) {
        bitmapDrawBody(hbm) ;
    } else if (face && hbm != NULLHANDLE) {
        bitmapDrawFace(hbm) ;
    }
}
