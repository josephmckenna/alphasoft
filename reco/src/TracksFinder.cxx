// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "TracksFinder.hh"
#include "TFitLine.hh"
// #include "TFitHelix.hh"
#include <iostream>

TracksFinder::TracksFinder(std::vector<TSpacePoint*>* points):
						 fNtracks(0),
						 fSeedRadCut(150.),
						 fPointsDistCut(8.1),
						 fSmallRad(_cathradius),
						 fNpointsCut(7)
{
  uint size=points->size();
  fPointsArray.reserve(size);
  for (uint i=0; i<size; i++)
     fPointsArray.push_back((TSpacePoint*)points->at(i));
  #if BUILD_EXCLUSION_LIST
  fExclusionList.clear();
  #endif
  fTrackVector.clear();
  // Reasons for failing:
  track_not_advancing = 0;
  points_cut = 0;
  rad_cut = 0;
  //  std::cout<<"TracksFinder::TracksFinder"<<std::endl;
}

TracksFinder::~TracksFinder()
{
   Clear();
}

void TracksFinder::Clear(Option_t *option)
{
  fPointsArray.clear();
  #if BUILD_EXCLUSION_LIST
  fExclusionList.clear();
  #endif
  fTrackVector.clear();
}

void TracksFinder::AddTrack( track_t& atrack ) // currently not used
{
  //  std::cout<<"TracksFinder::AddTrack( track_t& atrack )"<<std::endl;
  TFitLine l;
  for(auto it: atrack) l.AddPoint( fPointsArray.at(it) );
  l.SetPointsCut( fNpointsCut );
  l.SetChi2Cut( 29. );
  l.Fit();
  if( l.IsGood() )
    {
      fTrackVector.push_back( atrack );
      for(auto it: atrack)
      {
         #if BUILD_EXCLUSION_LIST
         fExclusionList.push_back(fPointsArray[it]);
         #endif
         fPointsArray[it]=NULL;//Remove pointer from local vector
      }
      ++fNtracks;
    }

  //  std::cout<<"TracksFinder::AddTrack( track_t& atrack ) DONE"<<std::endl;
}

//==============================================================================================
int TracksFinder::RecTracks()
{
  int Npoints = fPointsArray.size();
  if( Npoints<=0 )
    return -1;

  // Pattern Recognition algorithm
  TSpacePoint* SeedPoint=0;
  TSpacePoint* NextPoint=0;
  for(int i=0; i<Npoints; ++i)
    {
      TSpacePoint* point=fPointsArray[i];
      if (!point) continue;
      // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
      if( !point->IsGood(_cathradius, _fwradius-1.) )
      {
        #if BUILD_EXCLUSION_LIST
        fExclusionList.push_back(fPointsArray[i]);
        #endif
        fPointsArray[i]=NULL;
        continue;
      }

      track_t atrack;

      // do not start a track far from the anode
      if( point->GetR() < fSeedRadCut ) break;
      else SeedPoint = point;

      for(int j=i+1; j<Npoints; ++j)
      {
        NextPoint = fPointsArray[j];
        if (!NextPoint) continue;
        if( SeedPoint->Distance(NextPoint) <= fPointsDistCut )
        {
          //      pdg_code = SeedPoint->GetPDG();
          SeedPoint = NextPoint;
          atrack.push_back(j);
        }
      }// j loop

      TSpacePoint* LastPoint = fPointsArray.at( atrack.back() );
      if( int(atrack.size()) > fNpointsCut && LastPoint->GetR() < fSmallRad )
      {
        atrack.push_front(i);
        fTrackVector.push_back( atrack );
        for(auto& it: atrack)
        {
          #if BUILD_EXCLUSION_LIST
          fExclusionList.push_back(fPointsArray[it]);
          #endif
          fPointsArray[it]=NULL;
        }
        ++fNtracks;
        //AddTrack( atrack );
        // TTrack* aTrack;
        // if( fMagneticField>0. )
        //   aTrack = new TFitHelix;
        // else
        //   aTrack = new TFitLine;
        // ++fNtracks;
        // atrack.push_front(i);

        // for(auto it: atrack)
        //   {
        //     aTrack->AddPoint( (TSpacePoint*) fPointsArray.At(it) );
        //     fExclusionList.push_back(it);
        //   }// found points
        // tracks_array.AddLast(aTrack);
      }
    }//i loop

  if( fNtracks != int(fTrackVector.size()) )
    std::cerr<<"TracksFinder::RecTracks(): Number of found tracks "<<fNtracks
             <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;

  return fNtracks;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
