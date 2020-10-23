#
# Makefile for 'Osuwari Multi'
#

CC	= gcc -c
CFLAGS	= -O2
LD	= gcc
LDFLAGS	= -Zomf -Zmap

#
# Inference Rules
#
.c.o :
	$(CC) $(CFLAGS) $*.c

#
# Target to Build
#

TARGET = osmulti2.exe

all : $(TARGET)

clean : force
	rm -f *.o *.res

#
# Files to Use
#

SRCS = osmulti2.c bitmap.c window.c track.c anime.c balloon.c \
	shapewin.c setup.c about.c profile.c
OBJS = osmulti2.o bitmap.o window.o track.o anime.o balloon.o \
	shapewin.o setup.o about.o profile.o
BMPS = mul_norm.bmp mul_ohno.bmp mul_psyu.bmp mul_urur.bmp mul_uru2.bmp mul_clos.bmp
PTRS = nade.ptr funi.ptr
TXTS = about.jpn about.eng
LIBS = 

osmulti2.exe : $(OBJS) $(LIBS) osmulti2.def osmulres.res
	$(LD) $(LDFLAGS) -o osmulti2.exe osmulti2.def osmulres.res $(OBJS) $(LIBS)

osmulres.res : osmulres.rc osmulres.h osmulti2.ico $(PTRS) $(BMPS) $(TXTS)
	rc -r osmulres.rc

osmulti2.o : osmulti2.c osmulti2.h osmulres.h shapewin.h

bitmap.o   : bitmap.c osmulti2.h osmulres.h shapewin.h

window.o   : window.c osmulti2.h osmulres.h shapewin.h

track.o    : track.c osmulti2.h

anime.o    : anime.c osmulti2.h osmulres.h

balloon.o  : balloon.c osmulti2.h osmulres.h shapewin.h

setup.o    : setup.c osmulti2.h osmulres.h

about.o    : about.c osmulti2.h osmulres.h

profile.o  : profile.c osmulti2.h 

shapewin.o : shapewin.c shapewin.h

force :
