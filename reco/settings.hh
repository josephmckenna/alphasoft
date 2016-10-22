// settings.hh
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Nov 2014

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdio>
#include <cassert>

#include <TMath.h>
#include <TApplication.h>
#include <TClonesArray.h>
#include <TObjArray.h>
#include <TTree.h>
#include <TFile.h>

// -------- TPC settings --------
// geometry
double gTrapRadius=22.275; // mm 
//double gInnerRadius = 1.03e2; // mm
double gInnerRadius = 1.0925e2; // mm
double gROradius = 1.9e2; // mm
double gTPCLength = 2.304e3; // mm
double gTPCHalfLength=0.5*gTPCLength;
// Magnetic Field - Solenoid
double gMagneticField = 0.65; // T
//double gMagneticField = 1.;
// pads
double gPadZed = 4.0;  // mm --> can be changed at runtime
double gPadRphi = 4.0; // mm --> can be changed at runtime
//double gPadTime = 100.0; // ns
double gPadTime = 20.;
int gNpadsRphi = 32;
//int gNpadsRphi = 16;
//int gNpadsRphi = 64;
double gPadWidth = TMath::TwoPi()*gROradius / double(gNpadsRphi);
int gNpadsZ = int(gTPCLength/gPadZed);
int gNpads = gNpadsZ * gNpadsRphi;
// anodes
double gNwires_d = 256.;
double gAnglePitch = TMath::TwoPi()/gNwires_d;
double gAnodeTime = 50.; // ns
double gAnodeZres = 100.; // mm
// -------------------------------
// - Scintillating Bars settings -
int gNbars = 64;
double gBarRadius = 230.;//
// -------------------------------

// ------ ANALYSIS settings ------ 
// management
int kMCstud=0;
int gNofEvents=-1;
int gVerb=0;

char* gfilename; // name of rootfile containing data
TString gfiletag; // additional info to the output file

// -------------------------------
double gChargedPionMass = 139.566; //MeV/c^2
double gRadiationLength = 32.0871; // mm : averaged over the material stack in ALPHA2

// -------------------------------
// ignore hits in the amplification region
//double gMinTime = 54.37; // ns
double gMinTime = 100.;//ns for ArCO2 - 90:10
// do not start a track far from the anode
double gMinRad=160.;
//track reconstruction cut on distance between spacepoints
//double ghitdistcut=11.5; // mm
double ghitdistcut=50.;

// helix cuts
int gdigi=0;

double ghelrcut=18.; // normalized chi^2 cuts
double ghelzcut=10.;
double ghelimpcut=40.; // mm : impact parameter

double ghelrmin=1.;
double ghelzmin=0.5;

double ghelcurvcut=0.016; // mm^-1
double ghelmomcut=15.; // MeV/c

bool kDcut = true;
bool kccut = false;
bool kpcut = false;

// analysis selection (either alpha-g or alpha algorithm)
int avtx=0;

// r constaint flag
bool rcons=false;

// vertex improving cut
// normalized chi^2 
double gvtxchi2cut=3.;

// alphavertex improving cuts
// 1. radial cut on vertices (used when selecting helices to remove)
double gavtxradcut=999.; //mm
// 2. relative chi2 reduction cut
//  -> optimization of resolutions gives better default value of 0.60
double gavtxchi2redcut=0.6;
//double gavtxchi2redcut=0.4;

// -------------------------------
std::vector<double> gPadLimits;
void SetPadLimits()
{
  int wiresXpad = int(gNwires_d) / gNpadsRphi;
  int p=0,a=0;
  for(int w=0; w<int(gNwires_d); w+=wiresXpad)
    {
      for(int i=0; i<wiresXpad; ++i)
	{
	  gPadLimits.push_back( gPadWidth * p );
	  if(0) std::cout<<a
			 <<"\t["<<gPadWidth * (p-1)<<", "<<gPadWidth * p<<")\t"
			 <<"mm \t["<<gPadWidth * (p-1) / gROradius * TMath::RadToDeg()
			 <<", "<<gPadWidth * p / gROradius * TMath::RadToDeg()<<") deg\n";
	  ++a;
	}
      ++p;
    }
}
