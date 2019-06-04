// Cosmic Track class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: May 2019

#ifndef __COSFIND__
#define __COSFIND__ 1

#include <vector>
#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include "SignalsType.h"
#include "TStoreEvent.hh"

class TCosmic;
class CosmicFinder
{
private:
   double fMagneticField;

   std::vector<TCosmic*> fLines;
   // std::vector<double> fDCA;
   // std::vector<double> fphi;

   int nTracks;

   unsigned fNspacepointsCut;
   double fLineChi2Cut;
   double fLineChi2Min;

   TH1D* hDCAeq2;
   TH1D* hDCAgr2;
  
   TH1D* hAngeq2;
   TH1D* hAnggr2;

   TH2D* hAngDCAeq2;
   TH2D* hAngDCAgr2;

   TH1D* hcosaw;
   TH2D* hcospad;
   TH1D* hRes2min;

   // double temp;
   // TH1D* hpois;

   TH1D* hcosphi;
   TH1D* hcostheta;
   TH2D* hcosthetaphi;

   TH1D* hlr;
   TH1D* hlz;
   TH1D* hlp;
   TH2D* hlzp;
   TH2D* hlzr;
   TH2D* hlrp;
   TH2D* hlxy;

   padmap* pmap;

   int fIdx;
   double fRes2;
   int fStatus;

public:
   CosmicFinder(double);
   CosmicFinder(double,int,double,double);
   
   ~CosmicFinder();

   int Create(TStoreEvent*);
   //int Create(TClonesArray*);
   int Create(std::vector<TTrack*>*);

   int Process();
  
   int Residuals();
   void MakeOccupancyHisto();
   void FillOccupancyHisto();

   void Reset();

   void Status();

   inline const TCosmic* GetCosmic() const { return fLines.at(fIdx); }
   inline double GetResidual() const { return fRes2; }
   inline int GetStatus() const { return fStatus; }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
