// Store event class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017

#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
//#include "TStoreTrack.hh"
#include "TPCconstants.hh"

#include <iostream>

TStoreEvent::TStoreEvent():fID(-1),
			   fNpoints(-1.),fNtracks(-1.),
			   fStoreHelixArray(20), fStoreLineArray(20),
			   fSpacePoints(5000),
			   fUsedHelices(20),
			   fVertex(kUnknown,kUnknown,kUnknown),
			   fVertexStatus(-3),
			   fPattRecEff(-1.)
{}

void TStoreEvent::SetEvent(const TClonesArray* points, const TClonesArray* lines, 
			   const TClonesArray* helices)
{
  //  fNpoints = double(points->GetEntries());
  //  std::cout<<"TStoreEvent::SetEvent N points: "<<points->GetEntries()<<std::endl;
  for(int i=0; i<points->GetEntries(); ++i)
    {
      fSpacePoints.AddLast( points->At(i) );
    }
  //  fSpacePoints.SetOwner(kTRUE);
  fSpacePoints.Compress();

  //  std::cout<<"TStoreEvent::SetEvent N lines: "<<tracks->GetEntries()<<std::endl;
  for(int i=0; i<lines->GetEntries(); ++i)
    {
      TFitLine* aLine = (TFitLine*) lines->At(i);
      if( aLine->GetStatus() > 0 )
	{
	  fStoreLineArray.AddLast( new TStoreLine( aLine, aLine->GetPointsArray() ) );
	  fNpoints += double(aLine->GetNumberOfPoints());
	}
    }
  //  fStoreLineArray.SetOwner(kTRUE);
  fStoreLineArray.Compress();


  //  std::cout<<"TStoreEvent::SetEvent N helices: "<<helices->GetEntries()<<std::endl;
  for(int i=0; i<helices->GetEntries(); ++i)
    {
      TFitHelix* anHelix = (TFitHelix*) helices->At(i);
      if( anHelix->GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( anHelix, anHelix->GetPointsArray() ) );
	  fNpoints += double(anHelix->GetNumberOfPoints());
	}
      //      std::cout<<"TStoreEvent::SetEvent "<<i<<" N points: "<<fNpoints<<std::endl;
    }
  fStoreHelixArray.Compress();

  //  fNtracks = helices->GetEntries()>lines->GetEntries()?double(helices->GetEntries()):double(lines->GetEntries());
  fNtracks = helices->GetEntries();
  //  std::cout<<"TStoreEvent::SetEvent N tracks: "<<fNtracks<<std::endl;

  if( fNtracks > 0. )
    fPattRecEff = fNpoints/fNtracks;
  else
    fPattRecEff = 0.;
  //  std::cout<<"TStoreEvent::SetEvent return"<<std::endl;
}

TStoreEvent::~TStoreEvent()
{
  Reset();
}
void TStoreEvent::Print(Option_t*) const
{
  std::cout<<"=============== TStoreEvent "<<std::setw(5)
	   <<fID<<" ==============="<<std::endl;
  std::cout<<"Number of Points: "<<fNpoints
	   <<"\tNumber Of Tracks: "<<fNtracks<<std::endl;
  std::cout<<"*** Vertex Position ***"<<std::endl;
  fVertex.Print();
  std::cout<<"***********************"<<std::endl;
  // for(int i=0; i<fStoreLineArray.GetEntries(); ++i)
  //   {
  //     ((TFitLine*)fStoreLineArray.At(i))->Print();
  //   }
  std::cout<<"======================================="<<std::endl;
}

void TStoreEvent::Reset()
{
  fID = -1;
  fNpoints = -1.;
  fNtracks = -1.;

  fStoreLineArray.Delete();
  fStoreHelixArray.Delete();
  //  fStoreLineArray.Clear();
  //  fStoreHelixArray.Clear();
  fUsedHelices.Delete();
  fSpacePoints.Clear();

  fVertex.SetXYZ(kUnknown,kUnknown,kUnknown);

  fPattRecEff = -1.;
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)
