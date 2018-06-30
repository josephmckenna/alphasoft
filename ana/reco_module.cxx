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

#include "LookUpTable.hh"
#include "TSpacePoint.hh"
//#include "TFitLine.hh"

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
   
public:
   //   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo): TARunObject(runinfo), 
                                fPointsArray("TSpacePoint",1000)//, fLineArray("TFitLine",1000)
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
  
      // runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      // analyzed_event = new TStoreEvent();
      // EventTree = new TTree("StoreEventTree", "StoreEventTree");
      // EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);
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
 
      // gpointscut = 44;
      // ghitdistcut = 1.1; // mm
      // gchi2cut=20.;
      // TEvent anEvent( age->counter, runinfo->fRunNo );
      // cout<<"\t@@@ Event # "<<anEvent.GetEventNumber()<<endl;

            
      // // START the reconstuction
      // anEvent.RecEvent( age );
      // //anEvent.Print();

      // // STORE the reconstucted event
      // analyzed_event->Reset();
      // analyzed_event->SetEvent(&anEvent);
      // flow = new AgAnalysisFlow(flow, analyzed_event);

      // EventTree->Fill();

      // cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<endl;

      printf("RecoRun Analyze  Points: %d\n",fPointsArray.GetEntries());
      // printf("RecoRun Analyze  Lines: %d\n",anEvent.GetLineArray()->GetEntries());

      fPointsArray.Clear("C");
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
