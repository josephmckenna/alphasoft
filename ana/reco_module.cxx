//
// reco_module.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TTree.h>
#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraph.h>

#include "TPCconstants.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"

#include "TStoreEvent.hh"

class RecoRun: public TARunObject
{
public:
   bool do_plot = false;
   bool fTrace = false;
   //bool fTrace = true;
private:
   TClonesArray fPointsArray;
   TClonesArray fTracksArray;
   TClonesArray fLinesArray;
   TClonesArray fHelixArray;

   LookUpTable* fSTR;

   // useful histos
   TH2D* hsprp; // spacepoints in found tracks

   TH1D* hchi2; // chi^2 of line fit
   TH2D* hchi2sp; // chi^2 of line fit Vs # of spacepoints

   double MagneticField;
   unsigned fNhitsCut;
   int fNspacepointsCut;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo): TARunObject(runinfo), 
                                fPointsArray("TSpacePoint",1000),
                                fTracksArray("TTrack",50),
                                fLinesArray("TFitLine",50),
                                fHelixArray("TFitHelix",50),
                                MagneticField(_MagneticField),
                                fNhitsCut(5000),fNspacepointsCut(29)
   {
      printf("RecoRun::ctor!\n");
      fSTR = new LookUpTable(runinfo->fRunNo);
      //fSTR = new LookUpTable(0.3,MagneticField);
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
      delete fSTR;     
      //if(creco) delete creco;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      gDirectory->mkdir("reco")->cd();
      hsprp = new TH2D("hsprp","Spacepoints in Tracks;#phi [deg];r [mm]",
                       180,0.,TMath::TwoPi(),200,0.,175.);
      hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,100.);
      hchi2sp = new TH2D("hchi2sp","#chi^{2} of Straight Lines Vs Number of Spacepoints",
                         100,0.,100.,100,0.,100.);
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoRun::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("RecoRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("RecoRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("RecoRun::Analyze, run %d\n", runinfo->fRunNo);
      
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;
      std::cout<<"RecoRun::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      printf("RecoModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
      printf("RecoModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));

      printf("RecoModule::Analyze, SP # %d\n", int(SigFlow->matchSig.size()));

      if( SigFlow->matchSig.size() > fNhitsCut )
         {
            std::cout<<"RecoModule::Analyze Too Many Points... quitting"<<std::endl;
            return flow;
         }

      AddSpacePoint( &SigFlow->matchSig );
      printf("RecoRun Analyze  Points: %d\n",fPointsArray.GetEntries());

      TracksFinder pattrec( &fPointsArray );
      //      pattrec.SetSmallRadCut(125.);
      pattrec.SetSmallRadCut(135.);
      pattrec.SetMaxIncreseAdapt(45.1);
      pattrec.SetNpointsCut(fNspacepointsCut);
      //pattrec.SetPointsDistCut(4.1);
      pattrec.SetPointsDistCut(8.1);
      pattrec.AdaptiveFinder();
      AddTracks( pattrec.GetTrackVector() );
      printf("RecoRun Analyze  Tracks: %d\n",fTracksArray.GetEntries());

      int nlin = FitLines();
      std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
      printf("RecoRun Analyze  Lines: %d\n",fLinesArray.GetEntries());

      // int nhel = FitHelix();
      // std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;
      // printf("RecoRun Analyze  Helices: %d\n",fHelixArray.GetEntries());

      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetTimeOfEvent( age->time );
      analyzed_event->SetEvent(&fPointsArray,&fLinesArray,&fHelixArray);

      flow = new AgAnalysisFlow(flow, analyzed_event);
      EventTree->Fill();
 
      fHelixArray.Delete();
      fLinesArray.Delete();
      fTracksArray.Delete();
      fPointsArray.Delete();
      std::cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<std::endl;
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints )
   {
      int n = 0;
      //std::cout<<"RecoRun::AddSpacePoint  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
      for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
         {
            // STR: t->(r,phi)
            double time = sp->first.t;
            double r = fSTR->GetRadius( time ),
               correction = fSTR->GetAzimuth( time ),
               err = fSTR->GetdRdt( time );
            
            if( fTrace )
               {
                  double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
                  std::cout<<"RecoRun::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                           <<" t: "<<time<<" r: "<<r
                           <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                           <<" = "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
                  //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
               }

            new(fPointsArray[n]) TSpacePoint(sp->first.idx,
                                             sp->second.sec,sp->second.idx,
                                             time,
                                             r,correction,sp->second.z,
                                             err,0.,sp->second.errz,
                                             sp->first.height);
            ++n;
         }
      fPointsArray.Compress();
      fPointsArray.Sort();
      if( fTrace )
         std::cout<<"RecoRun::AddSpacePoint # entries: "<<fPointsArray.GetEntries()<<std::endl;
   }

   void AddTracks( const std::vector< std::list<int> >* track_vector )
   {
      int n=0;
      for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
         {
            new(fTracksArray[n]) TTrack(MagneticField);
            //std::cout<<"RecoRun::AddTracks Check Track # "<<n<<" "<<std::endl;
            for( auto ip=it->begin(); ip!=it->end(); ++ip)
               {
                  TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(*ip);
                  ( (TTrack*)fTracksArray.ConstructedAt(n) ) ->AddPoint( ap );
                  //std::cout<<*ip<<", ";
                  //ap->Print("rphi");
                  hsprp->Fill( ap->GetPhi(), ap->GetR() );
               }
            //            std::cout<<"\n";
            ++n;
         }
      fTracksArray.Compress();
      assert(n==int(track_vector->size()));
      assert(fTracksArray.GetEntries()==int(track_vector->size()));
      if( fTrace )
         std::cout<<"RecoRun::AddTracks # entries: "<<fTracksArray.GetEntries()<<std::endl;
   }

   int FitLines()
   {
      int n=0;
      for(int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            //at->Print();
            new(fLinesArray[n]) TFitLine(*at);
            //( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetChi2Cut( 15. );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetChi2Cut( 25. );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->SetPointsCut( fNspacepointsCut );
            ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Fit();
            if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetStat() > 0 )
               {
                  double ndf= (double) ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetDoF();
                  if( ndf > 0. )
                     {
                        double chi2 = ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetChi2();
                        hchi2->Fill(chi2/ndf);
                        double nn = (double) ( (TFitLine*)fLinesArray.ConstructedAt(n) )->GetNumberOfPoints();
                        hchi2sp->Fill(chi2,nn);
                     }
               }
            if( ( (TFitLine*)fLinesArray.ConstructedAt(n) )->IsGood() )
               {
                  //( (TFitLine*)fLinesArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) )->Print();
                  ++n;
               }
            else
               {
                  ( (TFitLine*)fLinesArray.ConstructedAt(n) ) -> Reason();
                  fLinesArray.RemoveAt(n);
               }
         }
      fLinesArray.Compress();
      return n;
   }

   int FitHelix()
   {
      int n=0;
      for(int it=0; it<fTracksArray.GetEntries(); ++it )
         {
            TTrack* at = (TTrack*) fTracksArray.At(it);
            //at->Print();
            new(fHelixArray[n]) TFitHelix(*at);
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetChi2ZCut( 30. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->SetDCut( 99999. );
            ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Fit();
            if( ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->IsGood() )
               {
                  // calculate momumentum
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Momentum();
                  //( (TFitHelix*)fHelixArray.ConstructedAt(n) )->CalculateResiduals();
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Print();
                  ++n;
               }
            else
               {
                  ( (TFitHelix*)fHelixArray.ConstructedAt(n) )->Reason();
                  fHelixArray.RemoveAt(n);
               }
         }
      fHelixArray.Compress();
      return n;
   }
};

class RecoModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("RecoModuleFactory::Init!\n");
   }
   void Finish()
   {
      printf("RecoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RecoRun(runinfo);
   }
};


static TARegister tar(new RecoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
