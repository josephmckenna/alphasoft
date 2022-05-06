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
   const double fLastPointRadCut;
   const double fPointsRadCut;
   const double fPointsPhiCut;
   const double fPointsZedCut;
   const double fMaxIncreseAdapt;

public:
   AdaptiveFinder(const std::vector<TSpacePoint>*, const double MaxIncrease, const double LastPointRadCut );
   ~AdaptiveFinder(){};

   inline double GetMaxIncreseAdapt() const {return fMaxIncreseAdapt;}
   inline double GetLastPointRadCut() const { return fLastPointRadCut; }

   virtual int RecTracks(std::vector<track_t>&);

   int NextPoint( const TSpacePoint*, const int, const int, double, track_t&) const;
   int NextPoint( const int, double, double, double, track_t&) const;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
