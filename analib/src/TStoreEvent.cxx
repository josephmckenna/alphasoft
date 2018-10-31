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
			   fNpoints(-1),fNtracks(-1),
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
  for(int i=0; i<points->GetEntriesFast(); ++i)
    {
      fSpacePoints.AddLast( points->At(i) );
    }
  //  fSpacePoints.SetOwner(kTRUE);
  fSpacePoints.Compress();

  //  std::cout<<"TStoreEvent::SetEvent N lines: "<<tracks->GetEntries()<<std::endl;
  for(int i=0; i<lines->GetEntriesFast(); ++i)
    {
      TFitLine* aLine = (TFitLine*) lines->At(i);
      if( aLine->GetStatus() > 0 )
	{
	  fStoreLineArray.AddLast( new TStoreLine( aLine, aLine->GetPointsArray() ) );
	  //fNpoints += double(aLine->GetNumberOfPoints());
	}
    }
  //  fStoreLineArray.SetOwner(kTRUE);
  fStoreLineArray.Compress();


  //  std::cout<<"TStoreEvent::SetEvent N helices: "<<helices->GetEntries()<<std::endl;
  for(int i=0; i<helices->GetEntriesFast(); ++i)
    {
      TFitHelix* anHelix = (TFitHelix*) helices->At(i);
      if( anHelix->GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( anHelix, anHelix->GetPointsArray() ) );
	  fNpoints += anHelix->GetNumberOfPoints();
	}
      //      std::cout<<"TStoreEvent::SetEvent "<<i<<" N points: "<<fNpoints<<std::endl;
    }
  fStoreHelixArray.Compress();

  //  fNtracks = helices->GetEntries()>lines->GetEntries()?double(helices->GetEntries()):double(lines->GetEntries());
  fNtracks = helices->GetEntriesFast();
  //  std::cout<<"TStoreEvent::SetEvent N tracks: "<<fNtracks<<std::endl;

  if( fNtracks > 0 )
    fPattRecEff = (double)fNpoints/(double)fNtracks;
  else
    fPattRecEff = 0.;
  //  std::cout<<"TStoreEvent::SetEvent return"<<std::endl;
}

TStoreEvent::~TStoreEvent()
{
  Reset();
}
void TStoreEvent::Print(Option_t* o) const
{
   if (strcmp(o,"title")==0)
      std::cout<<"EventNo\tTPCtime\tNpoints\tNtracks\tX\tY\tZ\t";
   else if (strcmp(o,"line")==0) 
      printf("%d\t%f\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t",fID,fEventTime,fNpoints,fNtracks,fVertexStatus,fVertex.X(),fVertex.Y(),fVertex.Z());
   else
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
}

void TStoreEvent::Reset()
{
  fID = -1;
  fNpoints = -1;
  fNtracks = -1;

  fStoreLineArray.Delete();
  fStoreHelixArray.Delete();
  //  fStoreLineArray.Clear();
  //  fStoreHelixArray.Clear();
  //fUsedHelices.Delete();
  fUsedHelices.Clear();
  fSpacePoints.Clear();

  fVertex.SetXYZ(kUnknown,kUnknown,kUnknown);

  fPattRecEff = -1.;
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)
