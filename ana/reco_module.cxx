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

#include "TPCconstants.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
// #include "TracksFinder.hh"
// #include "TFitLine.hh"

// #include "TEvent.hh"
// #include "TStoreEvent.hh"
// extern int gVerb;

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))


class RecoRun: public TARunObject
{
private:
   TClonesArray fPointsArray;
   TClonesArray fLineArray;

   LookUpTable* fSTR;

   TH1D* hspz;
   TH1D* hspr;
   TH1D* hspp;
   TH2D* hspxy;
   TH2D* hspzr;
   TH2D* hspzp;
   
public:
   //   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo): TARunObject(runinfo), 
                                fPointsArray("TSpacePoint",1000)/*, 
                                      fLineArray("TFitLine",1000)*/
   {
      printf("RecoRun::ctor!\n");
      fSTR = new LookUpTable(runinfo->fRunNo);
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
      delete fSTR;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
  
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("reco")->cd();

      // analyzed_event = new TStoreEvent();
      // EventTree = new TTree("StoreEventTree", "StoreEventTree");
      // EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      hspz = new TH1D("hspz","Spacepoints;z [mm]",1000,-_halflength,_halflength);
      hspr = new TH1D("hspr","Spacepoints;r [mm]",80,109.,190.); 
      hspp = new TH1D("hspp","Spacepoints;#phi [deg]",150,0.,360.);

      hspxy = new TH2D("hspxy","Spacepoints;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
      hspzr = new TH2D("hspzr","Spacepoints;z [mm];r [mm]",500,-_halflength,_halflength,80,109.,190.);
      hspzp = new TH2D("hspzp","Spacepoints;z [mm];#phi [deg]",500,-_halflength,_halflength,90,0.,360.);
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

      AddSpacePoint( &SigFlow->matchSig );

      // TracksFinder pattrec( &fPointsArray );
      // // double Npointscut = 44.;
      // // double hitdistcut = 1.1; // mm
      // // double chi2cut=20.;      
      // pattrec.AdaptiveFinder( fLineArray );


      // // STORE the reconstucted event
      // analyzed_event->Reset();
      // analyzed_event->SetEvent(&anEvent);
      // flow = new AgAnalysisFlow(flow, analyzed_event);

      // EventTree->Fill();

      // cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<endl;

      printf("RecoRun Analyze  Points: %d\n",fPointsArray.GetEntries());
      printf("RecoRun Analyze  Lines: %d\n",fLineArray.GetEntries());

      Plot();

      fPointsArray.Clear("C");
      fLineArray.Clear("C");
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   void AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints )
   {
      int n = 0;
      for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
         {
            //new(fPointsArray[n]) TSpacePoint(sp.first.idx,sp.second.idx,sp.first.t);

            double time = sp->first.t;
            double r = fSTR->GetRadius( time ),
               correction = fSTR->GetAzimuth( time ),
               err = fSTR->GetdRdt( time );

            new(fPointsArray[n]) TSpacePoint(sp->first.idx,sp->second.idx,
                                             time,
                                             r,correction,err);
            ++n;
         }

      fPointsArray.Sort();
   }

   void Plot()
   {
      for(int isp=0; isp<fPointsArray.GetEntries(); ++isp)
         {
            TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(isp);
            hspz->Fill(ap->GetZ());
            hspr->Fill(ap->GetR());
            hspp->Fill(ap->GetPhi()*TMath::RadToDeg());
            hspxy->Fill(ap->GetX(),ap->GetY());
            hspzr->Fill(ap->GetZ(),ap->GetR());
            hspzp->Fill(ap->GetZ(),ap->GetPhi()*TMath::RadToDeg());
         }
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
