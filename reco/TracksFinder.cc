// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#include "TPCconstants.hh"
#include "TracksFinder.hh"
#include "TSpacePoint.hh"
// #include "TFitLine.hh"
// #include "TFitHelix.hh"

#include <iostream>

TracksFinder::TracksFinder(const TClonesArray* points): fPointsArray(points),
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
  //  std::cout<<"TracksFinder::TracksFinder"<<std::endl;
}

TracksFinder::~TracksFinder()
{
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

// void TracksFinder::AddTrack( track_t& atrack )
// {
//   // //TTrack* aTrack;
//   // if( fMagneticField>0. )
//   //   //aTrack = new TFitHelix;
//   //   new(tracks_array[fNtracks]) TFitHelix;
//   // else
//   //   //aTrack = new TFitLine;
//   //   new(tracks_array[fNtracks]) TFitLine;

//   new(tracks_array[fNtracks]) TTrack;
//   ++fNtracks;
  
//   for(auto it: atrack)
//     {
//       ( (TTrack*)tracks_array.ConstructedAt(fNtracks) )->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
//       fExclusionList.push_back(it);
//     }// found points	  
//   ++fNtracks;
// }

//==============================================================================================
int TracksFinder::RecTracks()
{
  // Pattern Recognition algorithm
  TSpacePoint* SeedPoint=0;
  TSpacePoint* NextPoint=0;

  int Npoints = fPointsArray->GetEntries();
  for(int i=0; i<Npoints; ++i)
    {
      if( Skip(i) ) continue;

      if( !( (TSpacePoint*) fPointsArray->At(i) )->IsGood(_cathradius, _fwradius) )
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

      //      if( !atrack.empty() )
      if( int(atrack.size()) > fNpointsCut )
	{
	  atrack.push_front(i);
	  fTrackVector.push_back( atrack );
	  for(auto& it: atrack) fExclusionList.push_back(it);
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
	  //     aTrack->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
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
  int Npoints = fPointsArray->GetEntries(); 
  if( Npoints<=0 )
    return -1;
  //  std::cout<<"TracksFinder::AdaptiveFinder() # of points: "<<Npoints<<std::endl;

  // Pattern Recognition algorithm
  //  TSpacePoint* SeedPoint=0;  
  for(int i=0; i<Npoints; ++i)
    {
      if( Skip(i) ) continue;

      if( !( (TSpacePoint*) fPointsArray->At(i) )->IsGood(_cathradius, _fwradius) )
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
  
      if( int(vector_points.size()) > fNpointsCut )
	{
	  vector_points.push_front(i);
	  fTrackVector.push_back( vector_points );
	  // std::cout<<"TracksFinder::AdaptiveFinder Check Track # "
	  // 	   <<fNtracks<<" "<<std::endl;
	  for(auto& it: vector_points) 
	    {
	      fExclusionList.push_back(it);
	      //std::cout<<it<<", ";
	      //fPointsArray->At( it )->Print("rphi");
	    }
	  //std::cout<<"\n";
	  ++fNtracks;
	}
    }//i loop

  if( fNtracks != int(fTrackVector.size()) )
    std::cerr<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks
	     <<" does not match the number of entries "<<fTrackVector.size()<<std::endl;
  // else
  //   std::cout<<"TracksFinder::AdaptiveFinder(): Number of found tracks "<<fNtracks<<std::endl;

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
