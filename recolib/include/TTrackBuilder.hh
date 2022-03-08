#ifndef __TTRACK_BUILDER__
#define __TTRACK_BUILDER__

#include <vector>
#include "SignalsType.hh"
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "TTrack.hh"
#include <assert.h>

#include "LookUpTable.hh"
//Class to convert std::vector<track_t> into std::vector<TTrack>
class TTrackBuilder
{
   private:
      double fMagneticField;
      LookUpTable* fSTR;
      std::string fLocation;
      const bool fTrace;
   public:
      TTrackBuilder(double B, std::string location, bool trace);
      void BuildTracks( const std::vector<track_t> track_vector, const std::vector<TSpacePoint*> PointsArray, std::vector<TTrack>& TTrackArray );
      void UseSTRfromData(int runNumber)
      {
         delete fSTR;
         std::cout<<"Reco::UseSTRfromData( "<<runNumber<<" )"<<std::endl;
         fSTR = new LookUpTable(runNumber);
         fMagneticField=0.; // data driven STR valid only for B=0T   
      }
};

#endif