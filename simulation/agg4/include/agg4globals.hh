#ifndef __G4_GLOBALS__
#define __G4_GLOBALS__ 1

int gmchit     = 1; // dis/en-able storing of MC hits (ionization points)
int gdigi      = 1; // dis/en-able storing of digi
int gdigicheat = 0; // dis/en-able storing of digi cheat

int gmcbars = 0;

double gMagneticField;
double gQuencherFraction;

double gPadZed=4.0;//mm
double gPadRphi=4.0;//mm
double gPadTime;

double gAnodeTime;

int gNbars = 64;
double gBarRadius;
double gBarLength = 2600.0; // mm

bool kMat;
bool kProto;

int gVerb;

#endif
