#define ETX 3
#define ENQ 5
#define BELL 7
#define BS 010
#define HT 011
#define LF 012
#define VT 013
#define FF 014
#define CR 015
#define SO 0xE
#define SI 0xF
#define SUB 032
#define ESC 033
#define FS 034
#define GS 035
#define RS 036
#define US 037
#define DEL 0177

int	plotf;
#define ALPHA 0
#define VECTOR 1
#define POINT 2
#define SPOINT 3
#define SPOINTFETCH 4
#define IPLOT 5
#define CROSSHAIR 9

// arguments to setzw()
#define TEKNZNV	96		// Normal Z axis and Normal vectors
#define TEKNZDOT 97		// Normal Z dotted vectors
#define TEKNZDOTDASH 98		// Normal Z dot-dash vectors
#define TEKNZSHORTDASH 99	// short dash
#define TEKNZLONGDASH 100	// long dash

#define TEKWZNV 112	// Write-thru,  Normal vectors
#define TEKWZDOT 113	// Write-thru,  Dotted vectors
#define TEKDZNV 108	// Defocused,  Normal vectors

int	plot12;		/* 12 bit plotting Tek 4014 etc */
int	Notatty;	/* Output is not a tty - likely pipe to phys */
int	waitflash;
int	XTerm;		// running under XTerm

void iplot(int, int, int);	// i x y
void alpha();
void page();
void setzw(int);
int kurse(int*, int*, int*);		// char x y
