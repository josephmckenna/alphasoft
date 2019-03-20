// Tracks finder class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#ifndef __TFINDER__
#define __TFINDER__ 1
#include "TSpacePoint.hh"
#include <vector>
#include <deque>

typedef std::deque<int> track_t;

#include "TClonesArray.h"

#define BUILD_EXCLUSION_LIST 0

class TracksFinder
{
protected:
   std::vector<TSpacePoint*> fPointsArray;
   std::vector<track_t> fTrackVector;
   int fNtracks;

   // Reasons for failing:
   int track_not_advancing;
   int points_cut;
   int rad_cut;

private:
   double fSeedRadCut;
   double fPointsDistCut;
   double fSmallRad;
   int fNpointsCut;
#if BUILD_EXCLUSION_LIST
   //Do we care about keeping a list of excluded TSpacePoints if we don't use a map anymore?
   //I am guessing not so putting it behind a pre-compiler if statement and turning it off:
   std::vector<TSpacePoint*> fExclusionList;
#endif

public:
   TracksFinder(TClonesArray*);
   ~TracksFinder();

   virtual void Clear(Option_t* option="");
   inline int GetNumberOfTracks() const {return fNtracks;}
   inline const std::vector<track_t>* GetTrackVector() const { return &fTrackVector; }

   virtual int RecTracks();
   void AddTrack(track_t&);

   inline void GetReasons(int& t, int& n, int& r) { t=track_not_advancing; n=points_cut; r=rad_cut;}
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
