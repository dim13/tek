#define SSIZE 4096

unsigned char Screen [SSIZE][SSIZE];
int PPHist[256];	//Histogram for point plot
void Screeninit();
int WriteStyle;		// Vector writing style
int Zaxis;		// Z axis for special point plot
int Nex;		// number of 16 bit loads seen
int Npoints, Nvectors;	// number of plotted points & vectors
int Nspp;		// number of special point plots;
int Nalpha;		// number of alpha characters 
int Nprint;		// number of printed characters

int Option_n;		// LF implies CR
int Option_a;		// no alpha
