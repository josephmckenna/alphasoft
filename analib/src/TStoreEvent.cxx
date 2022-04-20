// Store event class implementation
// for ALPHA-g TPC AGTPCanalysis
// Authors: A. Capra, M. Mathers
// Date: April 2017
#ifdef BUILD_AG
#include "TStoreEvent.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"
#include "TSpacePoint.hh"
//#include "TStoreTrack.hh"

#include "TFitLine.hh"
#include "TFitHelix.hh"

#include "TPCconstants.hh"

#include <iostream>

TStoreEvent::TStoreEvent():TObject(),fID(-1),
			   fNpoints(-1),fNtracks(-1),
			   fStoreHelixArray(20), fStoreLineArray(20),
			   fSpacePoints(5000),
			   fUsedHelices(20),
			   fVertex(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown),
			   fVertexStatus(-3),
			   fPattRecEff(-1.)
{}

TStoreEvent::TStoreEvent(const TStoreEvent& right):TObject(right), fID(right.fID),
						   fEventTime(right.fEventTime),
						   fNpoints(right.fNpoints),
						   fNtracks(right.fNtracks),
						   fStoreHelixArray(right.fStoreHelixArray),
						   fStoreLineArray(right.fStoreLineArray),
						   fSpacePoints(right.fSpacePoints),
						   fUsedHelices(right.fUsedHelices),
						   fVertex(right.fVertex),fVertexStatus(right.fVertexStatus),
						   fPattRecEff(right.fPattRecEff),
						   fBarHit(right.fBarHit)
{}

TStoreEvent& TStoreEvent::operator=(const TStoreEvent& right)
{
  fID = right.fID;
  fEventTime = right.fEventTime;
  fNpoints = right.fNpoints;
  fNtracks = right.fNtracks;
  fStoreHelixArray = right.fStoreHelixArray;
  fStoreLineArray = right.fStoreLineArray;
  fSpacePoints = right.fSpacePoints;
  fUsedHelices = right.fUsedHelices;
  fVertex = right.fVertex;
  fVertexStatus = right.fVertexStatus;
  fPattRecEff = right.fPattRecEff;
  fBarHit = right.fBarHit;
  return *this;
}

void TStoreEvent::SetEvent(const std::vector<TSpacePoint>* points, const std::vector<TFitLine>* lines, 
			   const std::vector<TFitHelix>* helices)
{
   if (points) {
      fNpoints = points->size();
      for (int i = 0; i < fNpoints; ++i) {
         fSpacePoints.AddLast(new TSpacePoint(points->at(i)));
      }
   } else {
      fNpoints = 0;
   }
   fSpacePoints.Compress();

if(lines){  
int nlines=lines->size();
  for(int i=0; i<nlines; ++i)
    {
      const TFitLine &aLine = lines->at(i);
      if( aLine.GetStatus() > 0 )
	{
	  fStoreLineArray.AddLast( new TStoreLine( aLine, aLine.GetPointsArray() ) );
	}
    }
}
  fStoreLineArray.Compress();

if(helices){
  fNtracks = helices->size();
  for(int i=0; i<fNtracks; ++i)
    {
      const TFitHelix &anHelix = helices->at(i);
      if( anHelix.GetStatus() > 0 )
	{
	  fStoreHelixArray.AddLast( new TStoreHelix( &anHelix, anHelix.GetPointsArray() ) );   // FIXME: why does TStoreHelix want a pointer, when TStoreLine wants a reference?
	  fNpoints += anHelix.GetNumberOfPoints();
	}
    }
} else {
	fNtracks = 0;
}
  fStoreHelixArray.Compress();

  if( fNtracks > 0 )
    fPattRecEff = (double)fNpoints/(double)fNtracks;
  else
    fPattRecEff = 0.;
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
      std::cout<<"EventTime: "<< fEventTime <<std::endl;
      std::cout<<"Number of Points: "<<fNpoints
         <<"\tNumber Of Tracks: "<<fNtracks<<std::endl;
      std::cout<<"*** Vertex Position ***"<<std::endl;
      fVertex.Print();
      std::cout<<"***********************"<<std::endl;
      /*
  std::cout <<"HelixArray:"<< fStoreHelixArray.size() <<std::endl;
  for (auto i: fStoreHelixArray)
     i.Print();
  std::cout <<"StoreLineArray: "<< fStoreLineArray.size() <<std::endl;
  for (auto i: fStoreLineArray)
     i.Print();
  std::cout <<"SpacePointArray: "<< fSpacePoints.size() <<std::endl;
  for (auto i: fSpacePoints)
     i.Print();
  std::cout <<"fUsedHelices: "<< fUsedHelices.GetEntriesFast() << std::endl;
  for (int i = 0; i < fUsedHelices.GetEntriesFast(); i++)
     fUsedHelices.At(i)->Print();
  std::cout <<"fBarHit: "<< fBarHit.size() <<std::endl;
  for (auto i: fBarHit)
     i->Print();*/
      // for(int i=0; i<fStoreLineArray.GetEntries(); ++i)
      //   {
      //     ((TFitLine*)fStoreLineArray.At(i))->Print();
      //   }
      std::cout<<"======================================="<<std::endl;
   }
}

int TStoreEvent::AddLine(const TFitLine* l) 
{ 
  fStoreLineArray.AddLast( new TStoreLine( *l, l->GetPointsArray() ) ); 
  return fStoreLineArray.GetEntriesFast(); 
}
		 
int TStoreEvent::AddHelix(const TFitHelix* h) 
{ 
  fStoreHelixArray.AddLast( new TStoreHelix( h, h->GetPointsArray() ) ); 
  return fStoreHelixArray.GetEntriesFast(); 
}

void TStoreEvent::Reset()
{
  fID = -1;
  fNpoints = -1;
  fNtracks = -1;

  fStoreLineArray.SetOwner(kTRUE);
  fStoreLineArray.Delete();
  fStoreLineArray.Clear();

  fStoreHelixArray.SetOwner(kTRUE);
  fStoreHelixArray.Delete();
  fStoreHelixArray.Clear();

  fUsedHelices.SetOwner(kTRUE);
  fUsedHelices.Delete();
  fUsedHelices.Clear();

  fSpacePoints.SetOwner(kTRUE);
  fSpacePoints.Delete();
  fSpacePoints.Clear();

  fVertex.SetXYZ(ALPHAg::kUnknown,ALPHAg::kUnknown,ALPHAg::kUnknown);

  fPattRecEff = -1.;
  
  fBarHit.clear();
  // fCosmicCosineAngle = -99.;
}

ClassImp(TStoreEvent)
#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
