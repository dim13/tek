#include <stdlib.h>
#include <stdio.h>
#include "iplot.h"
#include <poll.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>

struct disps {
	int     scrnx[2];
	int     scrny[2];
}
        dispxy;

int     foo11, foo12, ohy, oly, ohx, oex, foo13, foo14;
int     lastx, lasty;

int     side = 0;
int     teklf = 90;
int     plot12 = 0;		/* 12 bit plotting Tek 4014 etc */
int     Mechanical = 0;

static int pen, maxx, oldx, oldy;
static char Zxystring[] = "4507430165672321";
static char xystring[] = "BJAIBFAEHJHIDFDE";


static char pendown[10] = "P";
static char penup[10] = " ";


xycvt(x, y)
	int     x, y;
{
	register c;
	register char *p;
	char    pbuf[8];
	p = pbuf;
	*p++ = 0;

	/* queue low order x */
	*p++ = ((x >> 2) & 037) | 0100;

	/* if hi x changed, queue it and force xmsn of low order y */
	if (ohx != (c = ((x >> 7) & 037) | 040))
		*p++ = oly = ohx = c;

	/* calculate extra byte */
	c = (x & 03) | ((y & 03) << 2) | 0140;
	if (y & 010000)
		c |= 020;

	/* queue lo y and extra if extra changed */
	if (plot12 && (oex != c)) {
		*p++ = oly = ((y >> 2) & 037) | 0140;
		*p++ = oex = c;
	} else if (oly != (c = ((y >> 2) & 037) | 0140))
		*p++ = oly = c;

	if (ohy != (c = ((y >> 7) & 037) | 040))
		*p++ = ohy = c;

	while (c = *--p)
		putchar(c);
	if (waitflash) {
		if (((abs(x - lastx) > 1500) || (abs(y - lasty) > 1500)) && (plotf == VECTOR))
			ohy = oly = ohx = oex = 0;
		lastx = x;
		lasty = y;
	}
}


/* i = 0  dark vector
 * i = -1  point plot
 * i = -32 to -126   special point plot z axis setting
 */

void
iplot(i, ix, iy)
	register i;
{

	if (Mechanical != 0) {
		xyplot(i, ix, iy);
		return;
	}
	if (iy < 0 || iy > 4095 || ix < 0 || ix > 4095) {
		return;		/* crude clip */
	}
	if (i <= 0) {
		switch (i) {
		case -1:
			if (plotf != POINT) {
				putchar(FS);
				plotf = POINT;
			}
			break;
		case 0:
			if (plotf != VECTOR && plotf != ALPHA)
				putchar(US);
			putchar(GS);
			plotf = VECTOR;
			break;
		default:
			if (plotf != SPOINT) {
				if (plotf != ALPHA)
					putchar(US);
				putchar(ESC);
				putchar(FS);
				plotf = SPOINT;
			}
			i = -i;
			if (i > 125)
				i = 125;
			if (i < 23)
				i = 32;
			putchar(i);
			break;
		}
	}
	xycvt(ix, iy);
	return;
}

void
alpha()
{
	if (Mechanical)
		printf(penup);
	if (waitflash && plotf == VECTOR) {
		putchar(0);
		putchar(0);
		putchar(0);
		putchar(0);
	}
	/* ohy=oly=ohx=oex=0; */
	plotf = ALPHA;
	pen = 1;
	lastx = lasty = -1;
	putchar(US);
	fflush(stdout);
}

void
inittek()
{
	register char *p;
	register t;
	static  done = 0;
	char   *getenv();
	if (!done) {
		if ((p = getenv("TERM")) != 0) {
			if ((strcmp(p, "xterm") == 0) || (strcmp(p, "dumb") == 0)) {
				teklf = 88;
				XTerm = plot12 = 1;
				waitflash = 0;
				printf("\033[?38h");
			} else if (strcmp(p, "tek4014") == 0) {
				teklf = 88;
				plot12 = 1;
			} else {
				waitflash = isdigit(*p);
				t = atoi(p);
				if ((t > 4012) && (t < 4020)) {
					teklf = 56;
					plot12 = 1;
				} else {
					teklf = 88;
				}
			}
		}
		++done;
	}
}

void
page()
{
	static struct pollfd dummy[1];
	inittek();
	fflush(stderr);
	fflush(stdout);
	if (!isatty(1))
		psleep(200);
	dispxy.scrnx[0] = 0;
	dispxy.scrnx[1] = 2048;
	dispxy.scrny[0] = 3070;
	dispxy.scrny[1] = 3070;
	ohy = oly = ohx = oex = 0;
	lastx = lasty = -1;
	plotf = ALPHA;
	putchar(ESC);
	putchar(FF);
	fflush(stdout);
	if (waitflash) {
		poll(dummy, 0L, 1500);
	}
}
/* Sleep for t milliseconds */
psleep(t)
{
	static struct pollfd dummy[1];
	poll(dummy, 0L, t);
}
/* VARARGS */
disp(s, t, u, v, w)
	char   *s, *t, *u, *v, *w;
{
	alpha();
	printf(s, t, u, v, w);
	fflush(stdout);
}
/* VARARGS */
spad(s, t, u, v, w)
	char   *s, *t, *u, *v, *w;
{
	inittek();
	if ((dispxy.scrny[side] -= teklf) < 0)
		dispxy.scrny[side] = 0;
	iplot(0, dispxy.scrnx[side], dispxy.scrny[side]);
	alpha();
	printf(s, t, u, v, w);
	fflush(stdout);
	psleep(40);
}
dispii(s, t)
	char   *s;
	int    *t;
{
	printf("%s ", s);
	fflush(stdout);
	scanf("%d", t);
}
spadii(s, t)
	char   *s;
	int    *t;
{
	char    line[80];
	spad(s);
	fgets(line, 77, stdin);
	if (line[0])
		sscanf(line, "%d", t);
}
spadiid(s, t)
	char   *s;
	int    *t;
{
	char    line[80];
	spad(s);
	printf(" [%d]: ", *t);
	fflush(stdout);
	fgets(line, 77, stdin);
	if (line[0])
		sscanf(line, "%d", t);
}
xyplot(i, ix, iy)
	register i;
	int     ix, iy;
{

	if (i == 0) {
		if (pen)
			printf(penup);
		pen = 0;
	} else if (i < 0) {
		if (pen)
			printf(penup);
		xymove(ix, iy);
		printf(pendown);
		printf("AHBD");
		pen = 1;
		return;
	} else if (pen == 0) {
		printf(pendown);
		pen++;
	}
	xymove(ix, iy);
}
xymove(x, y)
	int     x, y;
{
	register ix, iy, sindex, idelta;
	int     ch1, ch2;
	if (Mechanical < 0) {
		iy = -x;
		x = y;
		y = iy;
	}
	sindex = 0;
	if (x > maxx)
		maxx = x;
	if ((ix = (oldx - x)) < 0) {
		sindex++;
		ix = -ix;
	}
	if ((iy = (oldy - y)) < 0) {
		sindex += 2;
		iy = -iy;
	}
	if (iy > ix)
		sindex += 4;
	else if (ix > iy) {
		idelta = ix;
		ix = iy;
		iy = idelta;
	}
	if (iy == 0)
		return;
	sindex <<= 1;
	ch1 = xystring[sindex];
	ch2 = xystring[sindex + 1];
#define iless ix
#define igreat iy
#define icount sindex
/*
	printf("sindex=%d ix=%d iy=%d ",sindex,ix,iy);
*/
	icount = iy;

	iless <<= 1;
	idelta = iless - igreat;
	igreat <<= 1;
/*
	printf("ch1=%c ch2=%c iless=%d igreat=%d idelta=%d\n",ch1,ch2,iless,
		igreat,idelta);
*/

	while (--icount >= 0) {
		if (idelta >= 0) {
			putchar(ch2);
			idelta -= igreat;
		} else {
			putchar(ch1);
		}
		idelta += iless;
	}
	oldx = x;
	oldy = y;
}
/*
 * mech(n)
 * 0:	vectors
 * >0:	incremental plotting
 * <0:	incremental plotting rotated 90 degrees clockwise
 * 	on a drum plotter this makes y axis the long one
 */

mech(n)
{
	pen = 1;
	alpha();
	iplot(-1, 0, 0);
	alpha();
	if (n)
		putchar(RS);
	printf(penup);
	Mechanical = n;
}
/*
 * xyres()
 * tells xymove that the pen is at 0,0
 */
xyres()
{
	oldx = oldy = maxx = 0;
}
/*
 * mpage(idelta)
 * performs a move to the coordinate maxx+idelta,0
 * 	where maxx is the largest positive x excursion of the
 * 	plotter since the last call to mpage.
 */
mpage(idelta)
{
	iplot(0, maxx + idelta, 0);
	xyres();
}
/* Activate crosshair wait for keyboard input */
kurse(i, j, k)
	register int *i, *j, *k;
{
	struct termios ttyOrg, ttyNew;
	char    str[10];
	putchar(ESC);
	putchar(SUB);
	fflush(stdout);

	tcgetattr(0, &ttyOrg);
	ttyNew = ttyOrg;
	ttyNew.c_lflag &= ~ECHO;
	ttyNew.c_iflag &= ~ICRNL;
	ttyNew.c_lflag &= ~ICANON;
	ttyNew.c_cc[VTIME] = 2;
	ttyNew.c_cc[VMIN] = 10;
	tcsetattr(0, TCSADRAIN, &ttyNew);
	read(0, str, 10);
	tcsetattr(0, TCSADRAIN, &ttyOrg);
	*j = ((str[1] & 037) * 32 + (str[2] & 037)) << 2;
	*k = ((str[3] & 037) * 32 + (str[4] & 037)) << 2;
	return (*i = str[0]);
}
/* Activate crosshair and poll coordinates */
/*
kurse2(j,k)
register int *j, *k;
{
	struct termios ttyOrg, ttyNew;
	char str[10];
	putchar(ESC);
	putchar(SUB);
	fflush(stdout);

	tcgetattr(0, &ttyOrg);
	ttyNew = ttyOrg;
	ttyNew.c_lflag &= ~ECHO;
	ttyNew.c_iflag &= ~ICRNL;
	ttyNew.c_lflag &= ~ICANON;
	ttyNew.c_cc[VTIME] = 2;
	ttyNew.c_cc[VMIN] = 10;
	tcsetattr(0, TCSADRAIN, &ttyNew);
	psleep(15);
	putchar(ESC);  putchar(ENQ);
	read(0,str,10);
	tcsetattr(0, TCSADRAIN, &ttyOrg);
	*j = ((str[0] & 037 ) * 32 + (str[1] & 037))<<2;
	*k = ((str[2] & 037 ) * 32 + (str[3] & 037))<<2;
	return(0);
}
*/

setchsize(n)
{				/* set character size */
	putchar(ESC);
	switch (n) {
	case 4:
		putchar(';');
		break;
	case 3:
		putchar(':');
		break;
	case 2:
		putchar('9');
		break;
	default:
		putchar('8');
		break;
	}
}

void
setzw(n)
{				/* set beam and vector mode */
/*
	if (plot12 == 0)
		return;
 */
	putchar(ESC);
	putchar(n);
}
chin()
{
	struct termios ttyOrg, ttyNew;
	char    ch;
	fflush(stdout);
	tcgetattr(0, &ttyOrg);
	ttyNew = ttyOrg;
	ttyNew.c_lflag &= ~ECHO;
	ttyNew.c_iflag &= ~ICRNL;
	ttyNew.c_lflag &= ~ICANON;
	ttyNew.c_cc[VTIME] = 2;
	ttyNew.c_cc[VMIN] = 10;
	tcsetattr(0, TCSADRAIN, &ttyNew);
	read(0, &ch, 1);
	tcsetattr(0, TCSADRAIN, &ttyOrg);
	return (ch & 0177);
}

int     ttcalv = 0;

void
ttcSig(a)
{
	ttcalv = 1;
}
ttcall()
{
	switch (ttcalv) {
	case 1:
		ttcalv = 0;
		return (1);
	case 0:
		signal(SIGINT, ttcSig);
		ttcalv = -1;
	case -1:
		break;
	}
	return (0);
}


/* return non 0 if console char is available */
ciready()
{
	int     c;
	fd_set  kbdfds;
	struct timeval rfdtimeout;
	FD_SET(0, &kbdfds);
	rfdtimeout.tv_sec = 0;
	rfdtimeout.tv_usec = 0;
	c = select(1, &kbdfds, (fd_set *) 0, (fd_set *) 0, &rfdtimeout);
	return c;
}
