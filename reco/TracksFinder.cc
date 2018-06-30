// Tracks finder class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: Sep. 2016

#include "TracksFinder.hh"
#include "TSpacePoint.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"

#include <iostream>

extern int gVerb;
extern double ghitdistcut;
extern int gpointscut;
extern double gMagneticField;
extern double gMinRad;

extern double gPadZed;
extern double gchi2min;
extern double gchi2cut;

TracksFinder::TracksFinder(TObjArray* array):fPointsArray(array),
					     fTracksArray(0),
					     fNtracks(0),
					     fSeedRadCut(gMinRad),
					     fPointsRadCut_1(5.),
					     fPointsRadCut_2(8.),
					     fPointsRadCut_3(8.),
					     fPointsDistCut(ghitdistcut),
					     fPointsPhiCut(TPCBase::TPCBaseInstance()->GetAnodePitch()*2.),
					     fPointsZedCut( gPadZed*1.1 ),
					     fSmallRad(110.),
					     fNpointsCut(7)
{
  fNpoints=fPointsArray->GetEntries();
  fPointsArray->Sort();

  if( gVerb > 1 )
    {
      std::cout<<"TracksFinder::TracksFinder"<<std::endl;
      std::cout<<"TracksFinder::Number of points: "<<fNpoints<<std::endl;
      if( fNpoints > 0 )
	std::cout<<"TracksFinder: First Point radius: "<< ((TSpacePoint*)fPointsArray->First())->GetR()<<" mm\t@ t: "<<((TSpacePoint*)fPointsArray->First())->GetTime()<<" ns"<<std::endl;
    }
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

//==============================================================================================
int TracksFinder::RecTracks(TObjArray* tracks_array)
{
  // Pattern Recognition algorithm
  TSpacePoint* SeedPoint=0;
  TSpacePoint* NextPoint=0;

  for(int i=0; i<fNpoints; ++i)
    {
      if( Skip(i) ) continue;
      
      track_t atrack;
      atrack.clear();

      // do not start a track far from the anode
      if( ( (TSpacePoint*) fPointsArray->At(i) )->GetR() < fSeedRadCut ) break;
      else SeedPoint = (TSpacePoint*) fPointsArray->At(i);

      int pdg_code = SeedPoint->GetPDG();

      for(int j=i+1; j<fNpoints; ++j)
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
	  TTrack* aTrack;
	  if( gMagneticField>0. )
	    aTrack = new TFitHelix;
	  else
	    aTrack = new TFitLine;
	  ++fNtracks;
	  atrack.push_front(i);
	  aTrack->SetParticleType(pdg_code);
	  for(auto it: atrack)
	    {
	      aTrack->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
	      fExclusionList.push_back(it);
	    }// found points
	  tracks_array->AddLast(aTrack);
	}
    }//i loop

  if( fNtracks != tracks_array->GetEntries() )
    std::cerr<<"TracksFinder::RecTracks(TObjArray*): Number of found tracks "<<fNtracks
	     <<" does not match the number of entries "<<tracks_array->GetEntries()<<std::endl;

  return fNtracks;
}

//==============================================================================================
int TracksFinder::AdaptiveFinder(TObjArray* tracks_array)
{
  if( fNpoints<=0 )
    return -1;
  //  fPointsRadCut_3 = 20.; //mm
  // Pattern Recognition algorithm
  TSpacePoint* SeedPoint=0;
 
  for(int i=0; i<fNpoints; ++i)
    {
      if( Skip(i) ) continue;

      // do not start a track far from the anode
      if( ( (TSpacePoint*) fPointsArray->At(i) )->GetR() < fSeedRadCut ) break;
      else SeedPoint = (TSpacePoint*) fPointsArray->At(i);

      int pdg_code = SeedPoint->GetPDG();

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
	      if( AdaptDistCut > 41. ) break;
	      gapidx = NextPoint( gapidx , AdaptDistCut, vector_points );
	      LastPoint = (TSpacePoint*) fPointsArray->At( gapidx );
	      AdaptDistCut*=1.1;
	    }
	}
      else
	continue;
  
      if( int(vector_points.size()) > fNpointsCut )
	{
	  TTrack* aTrack;
	  if( gMagneticField>0. )
	    aTrack = new TFitHelix;
	  else
	    aTrack = new TFitLine;
	  ++fNtracks;

	  vector_points.push_front(i);
	  aTrack->SetParticleType(pdg_code);

	  for(auto it: vector_points)
	    {
	      aTrack->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
	      fExclusionList.push_back(it);
	    }// found points
	  tracks_array->AddLast(aTrack);
	}
    }//i loop

  if( fNtracks != tracks_array->GetEntries() )
    std::cerr<<"TracksFinder::RecTracks(TObjArray*): Number of found tracks "<<fNtracks
	     <<" does not match the number of entries "<<tracks_array->GetEntries()<<std::endl;

  return fNtracks;
}

int TracksFinder::NextPoint(int index, double distcut, track_t& atrack)
{
  TSpacePoint* SeedPoint = (TSpacePoint*) fPointsArray->At( index );
  TSpacePoint* NextPoint = 0;

  int LastIndex = index;
  for(int j = index+1; j < fNpoints; ++j)
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
  for(int j = index+1; j < fNpoints; ++j)
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
	  radcut = fPointsRadCut_3;
	  phicut = fPointsPhiCut;
	  zedcut = fPointsZedCut;
	}
    }// j loop

  return LastIndex;
}


//==============================================================================================
// JasonAnodeMethod
track_t TracksFinder::FirstPass(int i)
{
  std::cout<<"TracksFinder::FirstPass"<<std::endl;
  track_t atrack;
  TSpacePoint* pi = (TSpacePoint*) fPointsArray->At(i);
  for(int j=i+1; j<fNpoints; ++j)
    {
      //      if( Skip(j) ) break;
      if( Skip(j) ) continue;
      TSpacePoint* pj = (TSpacePoint*) fPointsArray->At(j);
      if( pi->GetWire() == pj->GetWire() && pi->MeasureRad(pj) < fPointsRadCut_1 )
	{
	  atrack.push_back(j);
	}
    }// j loop

  return atrack;
}

// JasonAnodeMethod
int TracksFinder::SecondPass( int seed, track_t& atrack )
{
  std::cout<<"TracksFinder::SecondPass"<<std::endl;
  int tt;
  if( atrack.empty() )
    tt=seed;
  else
    tt = atrack.back();
  if( tt > fNpoints )
    std::cerr<<"TracksFinder::SecondPass pp Error: tt = "<<tt<<std::endl;
  
  TSpacePoint* pp = (TSpacePoint*) fPointsArray->At(tt);
  int anode_distance = 999999;
  
  for(int i=0; i<fNpoints; ++i)
    {
      if( seed == i ) continue;
      bool skip=false;
      for(auto it: atrack)
	{
	  if( it == i ) 
	    {
	      skip=true;
	      break;
	    }
	}
      if( skip || Skip(i) ) continue;

      //      if( Skip(i) ) break;
    
      TSpacePoint* pi = (TSpacePoint*) fPointsArray->At(i);
      anode_distance = pi->GetWire() - pp->GetWire();

      if( anode_distance ==  0 ||
	  anode_distance ==  1 || // jason
	  anode_distance == -1 || // jason
	  anode_distance ==  2 || // gap?
	  anode_distance == -2 )  // gap?
	{
	  if( pp->MeasureRad(pi) < fPointsRadCut_2 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	      break;
	    }
	}
      else if( pp->GetWire() == 254 && pi->GetWire() == 0 )
	{
	  if( pp->MeasureRad(pi) < fPointsRadCut_2 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	      anode_distance = 2;
	      break;
	    }
	}
      else if( pp->GetWire() == 255 && ( pi->GetWire() == 0 || pi->GetWire() == 1 ) )
	{	  
	  if( pp->MeasureRad(pi) < fPointsRadCut_2 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	      anode_distance = 1;
	      break;
	    }
	}
      else if( pp->GetWire() == 1 && pi->GetWire() == 255 )
	{	  
	  if( pp->MeasureRad(pi) < fPointsRadCut_2 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	      anode_distance = -2;
	      break;
	    }
	}
      else if( pp->GetWire() == 0 && ( pi->GetWire() == 255 || pi->GetWire() == 254 ))
	{
	  if( pp->MeasureRad(pi) < fPointsRadCut_2 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	      anode_distance = -1;
	      break;
	    }
	}
    }
  
  if( anode_distance < 0 ) return -1;
  else if( anode_distance > 0 ) return 1;
  else return 0;
}

// JasonAnodeMethod
int TracksFinder::ThirdPass( track_t& atrack, int sign )
{
  std::cout<<"TracksFinder::ThirdPass"<<std::endl;
  int tt = atrack.back();
  TSpacePoint* pp = (TSpacePoint*) fPointsArray->At(tt);

  for(int i=0; i<fNpoints; ++i)
    {
      bool skip=false;
      for(auto it: atrack)
	{
	  if( it == i ) 
	    {
	      skip=true;
	      break;
	    }
	}
      if( skip || Skip(i) ) continue;
      //      if( Skip(i) ) break;

      TSpacePoint* pi = (TSpacePoint*) fPointsArray->At(i);
      int anode_distance = pi->GetWire() - pp->GetWire();

      switch( sign )
	{
	case 0:
	  if( anode_distance == 0 && 
	      pp->MeasureRad(pi) < fPointsRadCut_3 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  break;
	case 1:
	  if( anode_distance <= 2 && 
	      anode_distance >= 0 && 
	      pp->MeasureRad(pi) < fPointsRadCut_3 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  else if( pp->GetWire() == 254 && pi->GetWire() == 0 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  else if( pp->GetWire() == 255 && ( pi->GetWire() == 0 || pi->GetWire() == 1 ) )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  break;
	case -1:
	  if( anode_distance >= -2 && 
	      anode_distance <=  0 && 
	      pp->MeasureRad(pi) < fPointsRadCut_3 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    } 
	  else if( pp->GetWire() == 1 && pi->GetWire() == 255 )
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  else if( pp->GetWire() == 0 && ( pi->GetWire() == 255 || pi->GetWire() == 254 ))
	    {
	      atrack.push_back(i);
	      pp = pi;
	    }
	  break;
	default:
	  std::cerr<<"TracksFinder::ThirdPass Error"<<std::endl;
	  return -1;
	}
    }

  return 0;
}

void TracksFinder::JasonAnodeMethod()
{
  fTracksArray = new TObjArray();

  for(int i=0; i<fNpoints; ++i)
    {     
      //      if( Skip(i) ) break;
      if( Skip(i) ) continue;

      if( ( (TSpacePoint*) fPointsArray->At(i) )->GetR() < fSeedRadCut ) break;
 
      track_t atrack = FirstPass(i);

      int sg = SecondPass(i,atrack);
      if(atrack.empty()) continue;
      else atrack.push_front(i);
      int pdg_code = ( (TSpacePoint*) fPointsArray->At(i) )->GetPDG();

      int stat = ThirdPass(atrack,sg);
      if(!atrack.empty() && stat==0)
	{
	  // TObjArray* points = new TObjArray();
	  // ++fNtracks;
	  // for(auto it: atrack)
	  //   {
	  //     points->AddLast((TSpacePoint*) fPointsArray->At(it) );
	  //     fExclusionList.push_back(it);
	  //   }// found points
	  // fTracksArray.push_back(points);
	  TTrack* aTrack;
	  if( gMagneticField>0. )
	    aTrack = new TFitHelix;
	  else
	    aTrack = new TFitLine;
	  ++fNtracks;
	  atrack.push_front(i);
	  aTrack->SetParticleType(pdg_code);
	  for(auto it: atrack)
	    {
	      aTrack->AddPoint( (TSpacePoint*) fPointsArray->At(it) );
	      fExclusionList.push_back(it);
	    }// found points
	  fTracksArray->AddLast(aTrack);
	}
    }// i loop

  //  if( fExclusionList.size() > 0 )
  std::cerr<<"TracksFinder::JasonAnodeMethod() Efficiency: "<<double(fNpoints)/double(fExclusionList.size())/double(fNpoints)<<std::endl;
}

//==============================================================================================
// this method is used for the initial test of the prototype
// bc it performs the pattern recognition and the fitting at once
int TracksFinder::FitLines()
{
  fTracksArray = new TObjArray();
  int Nt = AdaptiveFinder(fTracksArray);
  if( Nt == -1 )
    return -1;

  int NGoodTracks=0;
  for(int it=0; it<fNtracks; ++it)
    {
      TFitLine* aLine = (TFitLine*) GetTrack(it);

      aLine->SetPointsCut(gpointscut);
      aLine->SetChi2Cut(gchi2cut);
      aLine->SetChi2Min(gchi2min);

      // debug
      if( gVerb > 1 )
	{
	  std::cout<<"------------------------------------------------------------------------"<<std::endl;
	  std::cout<<"Line Number "<<it<<"\t";
	  std::cout<<" Number of Points "<<aLine->GetNumberOfPoints()<<std::endl;
	  if( aLine->GetParticleType() != 0 )
	    std::cout<<" PDG: "<<aLine->GetParticleType()<<std::endl;
	  std::cout<<"Magnetic Field: "<<aLine->GetMagneticField()<<std::endl;
	}

      aLine->Fit();

      // calculate residuals, i.e., distance between points and straight line
      aLine->CalculateResiduals();
      // debug
      if( gVerb > 1 )
      	std::cout<<" Residuals Squared = "<<aLine->GetResidualsSquared()<<" mm^2"<<std::endl;

      if( aLine->IsGood() )
	++NGoodTracks;

      // debug
      if( gVerb > 1 )
	{
	  std::cout<<" Status: "<<aLine->GetStatus()<<std::endl;
	  if( gVerb > 2 ) aLine->Print();
	  std::cout<<"------------------------------------------------------------------------"<<std::endl;
	}
    }
  return NGoodTracks;
}
