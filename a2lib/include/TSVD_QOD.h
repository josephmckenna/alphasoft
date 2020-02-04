#ifndef _SVDQOD_
#define _SVDQOD_

#include "TH1I.h"
#include "TAlphaEvent.h"
#include "TSiliconEvent.h"
#include "manalyzer.h"

class TSVD_QOD: public TObject
{
   public:
   int RunNumber;
   double VF48Timestamp;
   int VF48NEvent=-1;
   int CosmicTracks=0;
   int ProjectedVertices=0;
   int NRawHits=0; //And TH1D?
   int PRawHits=0; //And TH1D?
   int NHits=0;
   int NTracks=0;
   int NVertices=0;
   int NPassedCuts=0;
   int MVA=0;
   double x=0;
   double y=0;
   double z=0;
   double t=0;
   int HitOccupancy[72];
   int OccupancyUS=0;
   int OccupancyDS=0;
//TTC
//Inner layer TTC Counts Median
// Inner layer TTC Counts Mean
// Inner layer TTC Counts Mean Error
//Middle layer TTC Counts Median
// Inner layer TTC Counts Mean
// Inner layer TTC Counts Mean Error
//Outer layer TTC Counts Median
// Inner layer TTC Counts Mean
// Inner layer TTC Counts Mean Error

   public:
   TSVD_QOD();
   TSVD_QOD(TAlphaEvent*a, TSiliconEvent*s );
   virtual ~TSVD_QOD();
   double X() { return x; }
   double Y() { return y; }
   double Z() { return z; }
   double GetTime() { return t; }
   double R() { return TMath::Sqrt(x*x+y*y); }

   ClassDef(TSVD_QOD,1); 
};

#endif

