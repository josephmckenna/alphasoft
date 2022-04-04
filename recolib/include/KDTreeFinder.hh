// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#ifndef __AKDTREEFINDER__
#define __AKDTREEFINDER__ 1
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include <vector>
#include <list>
#include <set>
#include <bitset>
#include "TKDTree.h"

class KDTreeFinder: public TracksFinder
{
private:
   const double fLastPointRadCut;
   const double fPointsRadCut;
   const double fPointsPhiCut;
   const double fPointsZedCut;
   const double fMaxIncreseAdapt;

   std::vector<double> fX, fY, fZ;
   TKDTreeID* fPoints;
   // Bit masked cluster ID... limits us to 32 tracks / clusters
   std::vector<std::bitset<64>> fCluster;
   uint32_t fClusterID;

public:
   KDTreeFinder(std::vector<TSpacePoint*>*, const double MaxIncrease, const double LastPointRadCut );
   ~KDTreeFinder()
   {
       delete fPoints;
   };

   inline double GetMaxIncreseAdapt() const {return fMaxIncreseAdapt;}
   inline double GetLastPointRadCut() const { return fLastPointRadCut; }

   virtual int RecTracks(std::vector<track_t>& TrackVector);

   void PointCluster(const int index, double distcut, track_t& atrack);
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
