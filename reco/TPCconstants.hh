#ifndef __TPC_CONSTANTS__
#define __TPC_CONSTANTS__ 1

static const double _halflength=1152.; // mm

static const double _anodes=256.;
static const double _anodepitch = 0.02454369260617026;// rad
static const double _overall_rotation = 0.; // rad

static const double _padcol=32.;
static const double _padrow=576.;
static const double _padpitch=4.; // mm

//static const double _timebin=16.; // ns
static const double _timebin=10.; // ns

static const double _sq12=0.2886751345948129;

static const double kUnknown = -9999999.;

static const double _trapradius = 22.275; //mm
static const double _cathradius = 109.2; // mm
static const double _fwradius = 174.; // mm
static const double _anoderadius = 182.; // mm
static const double _padradius = 190.; // mm

static const double _ChargedPionMass = 139.566; //MeV/c^2
static const double _RadiationLength = 32.0871; // mm : averaged over ALPHA-2 material

//static const double _MagneticField = 0.; // T
static const double _MagneticField = 1.e-4; // T

#endif
