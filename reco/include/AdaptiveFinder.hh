// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#ifndef __AFINDER__
#define __AFINDER__ 1
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include <vector>
#include <list>
#include <set>

#include "TClonesArray.h"

class AdaptiveFinder: public TracksFinder
{
private:
   double fLastPointRadCut;
   double fPointsRadCut;
   double fPointsPhiCut;
   double fPointsZedCut;
   double fMaxIncreseAdapt;
   double fSeedRadCut;
   double fPointsDistCut;
   double fSmallRad;
   int fNpointsCut;

public:
   AdaptiveFinder(TClonesArray*);
   ~AdaptiveFinder(){};

   inline void SetSeedRadCut(double cut)    { fSeedRadCut=cut; }
   inline double GetSeedRadCut() const      { return fSeedRadCut; }
   inline void SetPointsDistCut(double cut) { fPointsDistCut=cut; }
   inline double GetPointsDistCut() const   { return fPointsDistCut; }
   inline void SetSmallRadCut(double cut)   { fSmallRad=cut; }
   inline double GetSmallRadCut() const     { return fSmallRad; }
   inline void SetNpointsCut(int cut)       { fNpointsCut=cut; }
   inline int GetNpointsCut() const         { return fNpointsCut; }
   inline void SetMaxIncreseAdapt(double m) {fMaxIncreseAdapt = m;}
   inline double GetMaxIncreseAdapt() const {return fMaxIncreseAdapt;}
   inline void SetLastPointRadCut(double c) { fLastPointRadCut=c; }
   inline double GetLastPointRadCut() const { return fLastPointRadCut; }

   inline int GetNumberOfTracks() const {return fNtracks;}

   bool Skip(int);

   virtual int RecTracks();

   int NextPoint( TSpacePoint*, int, int, double, track_t&);
   int NextPoint( int, double, double, double, track_t&);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
