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


class AdaptiveFinder: public TracksFinder
{
private:
   double fLastPointRadCut;
   double fPointsRadCut;
   double fPointsPhiCut;
   double fPointsZedCut;
   double fMaxIncreseAdapt;

public:
   AdaptiveFinder(std::vector<TSpacePoint*>*);
   ~AdaptiveFinder(){};

   inline void SetMaxIncreseAdapt(double m) {fMaxIncreseAdapt = m;}
   inline double GetMaxIncreseAdapt() const {return fMaxIncreseAdapt;}
   inline void SetLastPointRadCut(double c) { fLastPointRadCut=c; }
   inline double GetLastPointRadCut() const { return fLastPointRadCut; }

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
