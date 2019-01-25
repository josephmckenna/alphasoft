// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "TracksFinder.hh"
#include "TFitLine.hh"
// #include "TFitHelix.hh"
#include <iostream>

TracksFinder::TracksFinder(TClonesArray* points):
						 fNtracks(0),
						 fSeedRadCut(150.),
						 fPointsDistCut(8.1),
						 fSmallRad(_cathradius),
						 fLastPointRadCut(135.),
						 fPointsRadCut(4.),
						 fPointsPhiCut( _anodepitch*2. ),
						 fPointsZedCut( _padpitch*1.1 ),
						 fNpointsCut(7),
						 fMaxIncreseAdapt(41.)
{
  uint size=points->GetEntriesFast();
  fPointsArray.reserve(size);
  for (uint i=0; i<size; i++)
     fPointsArray.push_back((TSpacePoint*)points->At(i));
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
  fPointsArray.clear();
  #if BUILD_EXCLUSION_LIST
  fExclusionList.clear();
  #endif
  fTrackVector.clear();
}

void TracksFinder::Clear(Option_t *option)
{
  fPointsArray.clear();
  #if BUILD_EXCLUSION_LIST
  fExclusionList.clear();
  #endif
  fTrackVector.clear();
}

void TracksFinder::AddTrack( track_t& atrack )
{
  //  std::cout<<"TracksFinder::AddTrack( track_t& atrack )"<<std::endl;
  TFitLine l;
  for(auto it: atrack) l.AddPoint( fPointsArray[it] );
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

//==============================================================================================
int TracksFinder::AdaptiveFinder()
{
  int Npoints = fPointsArray.size(); 
  if( Npoints<=0 )
    return -1;
  //  std::cout<<"TracksFinder::AdaptiveFinder() # of points: "<<Npoints<<std::endl;

  // Pattern Recognition algorithm
  for(int i=0; i<Npoints; ++i)
    {
      TSpacePoint* point=fPointsArray[i];
      if (!point) continue;
      // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
      // thus I include spacepoints up to r=173mm
      // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
      if( !point->IsGood(_cathradius, _fwradius-1.) )
      {
        #if BUILD_EXCLUSION_LIST
        fExclusionList.push_back(fPointsArray[i]);
        #endif
        fPointsArray[i]=NULL;
        continue;
      }

      // do not start a track far from the anode
      if( point->GetR() < fSeedRadCut ) break;

      track_t vector_points;
      vector_points.clear();

      int gapidx = NextPoint( point, i , Npoints, fPointsDistCut, vector_points );
      TSpacePoint* LastPoint =  fPointsArray.at( gapidx );

      double AdaptDistCut = fPointsDistCut*1.1;
      while( LastPoint->GetR() > fSmallRad )
	{
	  // LastPoint->Print("rphi");
	  // std::cout<<"AdaptDistCut: "<<AdaptDistCut<<" mm"<<std::endl;
	  if( AdaptDistCut > fMaxIncreseAdapt ) break;
	  gapidx = NextPoint( LastPoint, gapidx ,Npoints, AdaptDistCut, vector_points );
	  LastPoint = fPointsArray.at( gapidx );
	  AdaptDistCut*=1.1;
	}
   
      if( int(vector_points.size()) < fNpointsCut )
	{
	  ++points_cut;
	  continue;
	}
      else if( LastPoint->GetR() > fLastPointRadCut )
	{
	  ++rad_cut;
	  continue;
	}
      else
	{
	  vector_points.push_front(i);

	  fTrackVector.push_back( vector_points );
	  for(auto& it: vector_points)
	  {
        #if BUILD_EXCLUSION_LIST
        fExclusionList.push_back(fPointsArray[it]);
        #endif
        fPointsArray[it]=NULL;
      }
	  ++fNtracks;

	  //AddTrack( vector_points );
	}
    }//i loop

  if( fNtracks != int(fTrackVector.size()) )
    std::cerr<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks
	     <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;
  else
    {
      std::cout<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks<<std::endl;
      std::cout<<"TracksFinder::AdaptiveFinder() -- Reasons: Track Not Advancing "<<track_not_advancing
	       <<" Points Cut: "<<points_cut
	       <<" Radius Cut: "<<rad_cut<<std::endl;
    }

  return fNtracks;
}

int TracksFinder::NextPoint(TSpacePoint* SeedPoint, int index, int Npoints, double distcut, track_t& atrack)
{
  TSpacePoint* NextPoint = 0;

  int LastIndex = index;
  for(int j = index+1; j < Npoints; ++j)
    {
      NextPoint = fPointsArray[j];
      if (!NextPoint) continue;
      if( SeedPoint->Distance( NextPoint ) <= distcut )
      {
        SeedPoint = NextPoint;
        atrack.push_back(j);
        LastIndex = j;
        distcut = fPointsDistCut;
        //Just an idea: but right now I change the results
        /*if( int(atrack.size()) > fNpointsCut )
        {
          //Track already has more points than the points cut... abort
          return LastIndex;
        }*/
      }
    }// j loop
  return LastIndex;
}

int TracksFinder::NextPoint(int index, 
			    double radcut, double phicut, double zedcut,
			    track_t& atrack)
{
  TSpacePoint* SeedPoint = fPointsArray.at( index );
  TSpacePoint* NextPoint = 0;

  int LastIndex = index;
  int Npoints = fPointsArray.size(); 
  for(int j = index+1; j < Npoints; ++j)
    {
      NextPoint = fPointsArray[j];
      if (!NextPoint) continue;
      if( SeedPoint->MeasureRad( NextPoint ) <= radcut && 
      SeedPoint->MeasurePhi( NextPoint ) <= phicut &&
      SeedPoint->MeasureZed( NextPoint ) <= zedcut )
      {
        SeedPoint = NextPoint;
        atrack.push_back(j);
        LastIndex = j;
        radcut = fPointsRadCut;
        phicut = fPointsPhiCut;
        zedcut = fPointsZedCut;
      }
    }// j loop

  return LastIndex;
}


//==============================================================================================
// int TracksFinder::FitLines()
// {
//   fTracksArray = new TObjArray();
//   int Nt = AdaptiveFinder(fTracksArray);
//   if( Nt == -1 )
//     return -1;

//   int NGoodTracks=0;
//   for(int it=0; it<fNtracks; ++it)
//     {
//       TFitLine* aLine = (TFitLine*) GetTrack(it);

//       aLine->SetPointsCut(gpointscut);
//       aLine->SetChi2Cut(gchi2cut);
//       aLine->SetChi2Min(gchi2min);

//       // debug
//       if( gVerb > 1 )
// 	{
// 	  std::cout<<"------------------------------------------------------------------------"<<std::endl;
// 	  std::cout<<"Line Number "<<it<<"\t";
// 	  std::cout<<" Number of Points "<<aLine->GetNumberOfPoints()<<std::endl;
// 	  if( aLine->GetParticleType() != 0 )
// 	    std::cout<<" PDG: "<<aLine->GetParticleType()<<std::endl;
// 	  std::cout<<"Magnetic Field: "<<aLine->GetMagneticField()<<std::endl;
// 	}

//       aLine->Fit();

//       // calculate residuals, i.e., distance between points and straight line
//       aLine->CalculateResiduals();
//       // debug
//       if( gVerb > 1 )
//       	std::cout<<" Residuals Squared = "<<aLine->GetResidualsSquared()<<" mm^2"<<std::endl;

//       if( aLine->IsGood() )
// 	++NGoodTracks;

//       // debug
//       if( gVerb > 1 )
// 	{
// 	  std::cout<<" Status: "<<aLine->GetStatus()<<std::endl;
// 	  if( gVerb > 2 ) aLine->Print();
// 	  std::cout<<"------------------------------------------------------------------------"<<std::endl;
// 	}
//     }
//   return NGoodTracks;
// }
