// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "TracksFinder.hh"
#include "TSpacePoint.hh"
#include "TFitLine.hh"
// #include "TFitHelix.hh"

#include <iostream>

TracksFinder::TracksFinder(TClonesArray* points):fPointsArray(points),
							fNtracks(0),
							fSeedRadCut(150.),
							fPointsDistCut(8.1),
							fSmallRad(110.),
							fPointsRadCut(4.),
							fPointsPhiCut( _anodepitch*2. ),
							fPointsZedCut( _padpitch*1.1 ),
							fNpointsCut(7),
							fMaxIncreseAdapt(41.)
{
  fExclusionList.clear();
  fTrackVector.clear();
  std::cout<<"TracksFinder::TracksFinder"<<std::endl;
}

TracksFinder::~TracksFinder()
{
  if(fPointsArray->GetEntries()) fPointsArray->Delete();
  fExclusionList.clear();
  fTrackVector.clear();
}

bool TracksFinder::Skip(int idx)
{
  bool skip=false;
  for(auto iex: fExclusionList)
    {
      if( iex == idx ) 
	{
	  skip=true;
	  break;
	}
    }
  return skip;
}

void TracksFinder::AddTrack( track_t& atrack )
{
  //  std::cout<<"TracksFinder::AddTrack( track_t& atrack )"<<std::endl;
  TFitLine *l = new TFitLine();
  for(auto it: atrack) l->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
  l->SetPointsCut( fNpointsCut );
  l->SetChi2Cut( 29. );
  l->Fit();
  if( l->IsGood() )
    {
      fTrackVector.push_back( atrack );
      for(auto it: atrack) fExclusionList.push_back(it);
      ++fNtracks;
    }
  delete l;
  //  std::cout<<"TracksFinder::AddTrack( track_t& atrack ) DONE"<<std::endl;
}

//==============================================================================================
int TracksFinder::RecTracks()
{
  int Npoints = fPointsArray->GetEntries(); 
  if( Npoints<=0 )
    return -1;

  // Pattern Recognition algorithm
  TSpacePoint* SeedPoint=0;
  TSpacePoint* NextPoint=0;

  for(int i=0; i<Npoints; ++i)
    {
      if( Skip(i) ) continue;

      // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
      if( !( (TSpacePoint*) fPointsArray->At(i) )->IsGood(_cathradius, _fwradius-1.) )
	{
	  fExclusionList.push_back(i);
	  continue;
	}
      
      track_t atrack;
      atrack.clear();

      // do not start a track far from the anode
      if( ( (TSpacePoint*) fPointsArray->At(i) )->GetR() < fSeedRadCut ) break;
      else SeedPoint = (TSpacePoint*) fPointsArray->At(i);

      for(int j=i+1; j<Npoints; ++j)
	{
	  if( Skip(j) ) continue;
	  
	  NextPoint = (TSpacePoint*) fPointsArray->At(j);

	  if( SeedPoint->Distance(NextPoint) <= fPointsDistCut )
	    {
	      //	      pdg_code = SeedPoint->GetPDG();
	      SeedPoint = (TSpacePoint*) fPointsArray->At(j);
	      atrack.push_back(j);
	    }

	}// j loop

      TSpacePoint* LastPoint = (TSpacePoint*) fPointsArray->At( atrack.back() );
      if( int(atrack.size()) > fNpointsCut && LastPoint->GetR() < fSmallRad )
	{
	  atrack.push_front(i);
	  fTrackVector.push_back( atrack );
	  for(auto& it: atrack) fExclusionList.push_back(it);
	  ++fNtracks;
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
  int Npoints = fPointsArray->GetEntries(); 
  if( Npoints<=0 )
    return -1;
  //  std::cout<<"TracksFinder::AdaptiveFinder() # of points: "<<Npoints<<std::endl;

  // Pattern Recognition algorithm
  //  TSpacePoint* SeedPoint=0;  
  for(int i=0; i<Npoints; ++i)
    {
      if( Skip(i) ) continue;

      //      if( !( (TSpacePoint*) fPointsArray->At(i) )->IsGood(_cathradius, _fwradius) )
      // spacepoints in the proportional region and "near" the fw (r=174mm) are messy
      // thus I include spacepoints up to r=173mm
      if( !( (TSpacePoint*) fPointsArray->At(i) )->IsGood(_cathradius, _fwradius-1.) )
	{
	  fExclusionList.push_back(i);
	  continue;
	}

      // do not start a track far from the anode
      if( ( (TSpacePoint*) fPointsArray->At(i) )->GetR() < fSeedRadCut ) break;
      //      else SeedPoint = (TSpacePoint*) fPointsArray->At(i);

      track_t vector_points;
      vector_points.clear();

      int gapidx = NextPoint( i , fPointsDistCut, vector_points );
      TSpacePoint* LastPoint = (TSpacePoint*) fPointsArray->At( gapidx );

      if( gapidx > i )
	{
	  double AdaptDistCut = fPointsDistCut*1.1;
	  while( LastPoint->GetR() > fSmallRad )
	    {
	      // LastPoint->Print("rphi");
	      // std::cout<<"AdaptDistCut: "<<AdaptDistCut<<" mm"<<std::endl;
	      if( AdaptDistCut > fMaxIncreseAdapt ) break;
	      gapidx = NextPoint( gapidx , AdaptDistCut, vector_points );
	      LastPoint = (TSpacePoint*) fPointsArray->At( gapidx );
	      AdaptDistCut*=1.1;
	    }
	}
      else
	continue;

      if( int(vector_points.size()) > fNpointsCut && LastPoint->GetR() < fSmallRad )
	{
	  vector_points.push_front(i);

	  fTrackVector.push_back( vector_points );
	  for(auto& it: vector_points) fExclusionList.push_back(it);
	  ++fNtracks;

	  //AddTrack( vector_points );
	}
    }//i loop

  if( fNtracks != int(fTrackVector.size()) )
    std::cerr<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks
	     <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;
  else
    std::cout<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks<<std::endl;

  return fNtracks;
}

int TracksFinder::NextPoint(int index, double distcut, track_t& atrack)
{
  TSpacePoint* SeedPoint = (TSpacePoint*) fPointsArray->At( index );
  TSpacePoint* NextPoint = 0;

  int LastIndex = index;
  for(int j = index+1; j < fPointsArray->GetEntries(); ++j)
    {
      if( Skip(j) ) continue;
	  
      NextPoint = (TSpacePoint*) fPointsArray->At(j);

      if( SeedPoint->Distance( NextPoint ) <= distcut )
	{
	  SeedPoint = (TSpacePoint*) fPointsArray->At(j);
	  atrack.push_back(j);
	  LastIndex = j;
	  distcut = fPointsDistCut;
	}
    }// j loop

  return LastIndex;
}

int TracksFinder::NextPoint(int index, 
			    double radcut, double phicut, double zedcut,
			    track_t& atrack)
{
  TSpacePoint* SeedPoint = (TSpacePoint*) fPointsArray->At( index );
  TSpacePoint* NextPoint = 0;

  int LastIndex = index;
  for(int j = index+1; j < fPointsArray->GetEntries(); ++j)
    {
      if( Skip(j) ) continue;
	  
      NextPoint = (TSpacePoint*) fPointsArray->At(j);

      if( SeedPoint->MeasureRad( NextPoint ) <= radcut && 
	  SeedPoint->MeasurePhi( NextPoint ) <= phicut &&
	  SeedPoint->MeasureZed( NextPoint ) <= zedcut )
	{
	  SeedPoint = (TSpacePoint*) fPointsArray->At(j);
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
