//
// reco_module.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "TPolyMarker3D.h"
#include "TTree.h"

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "Signals.hh"
#include "PointsFinder.hh"
#include "TSpacePoint.hh"
#include "TLookUpTable.hh"
#include "TPCBase.hh"

#include "TrackViewer.hh"

#include "settings.hh"
#include "TEvent.hh"
#include "TStoreEvent.hh"
extern int gVerb;
extern double gMagneticField;
extern double ghitdistcut;
extern int gpointscut;

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;

class RecoRun: public TARunObject
{
public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo)
      : TARunObject(runinfo)
   {
      printf("RecoRun::ctor!\n");
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
 
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      analyzed_event = new TStoreEvent();
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);

      TPCBase::TPCBaseInstance()->SetPrototype(true);
 
      gMagneticField=0.;
      gVerb = 2;
      TLookUpTable::LookUpTableInstance()->SetGas("arco2",0.28);
      TLookUpTable::LookUpTableInstance()->SetB(gMagneticField);

      TrackViewer::TrackViewerInstance()->StartViewer();
      //      TrackViewer::TrackViewerInstance()->StartViewer(true);

      TrackViewer::TrackViewerInstance()->StartDeconv();
      TrackViewer::TrackViewerInstance()->StartCoincView();
      TrackViewer::TrackViewerInstance()->StartHitsView();
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoRun::EndRun, run %d\n", runinfo->fRunNo);
      time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("RecoRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("RecoRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("RecoRun::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      //int force_plot = false;

      AgEvent* age = ef->fEvent;

      // gpointscut = 44;
      // ghitdistcut = 1.1; // mm
      TEvent anEvent( event->serial_number, runinfo->fRunNo );
      //cout<<"\n@@@ Event # "<<anEvent.GetEventNumber()<<endl;
      
      // use:
      //
      // age->feam --- pads data
      // age->a16  --- aw data
      //

      if(age->feam && age->a16){

         if(age->feam->complete && age->a16->complete && !age->feam->error && !age->a16->error){
            
            // START the reconstuction
            anEvent.RecEvent( age );
            anEvent.Print();

            // STORE the reconstucted event
            analyzed_event->Reset();
            analyzed_event->SetEvent(&anEvent);
            flow = new AgAnalysisFlow(flow, analyzed_event);

            EventTree->Fill();
            //            analyzed_event->Print();   

            // if( anEvent.GetSignals()->sanode.size() > 0 )
            //    flow = new AgAwSignalsFlow(flow, anEvent.GetSignals()->sanode);
            flow = new AgSignalsFlow(flow, anEvent.GetSignals());

            cout<<"CCCCC"<<endl;
         }
      }
      // TrackViewer::TrackViewerInstance()->DrawPoints( pf->GetPoints() );
      // TrackViewer::TrackViewerInstance()->DrawPoints2D(anEvent.GetPointsArray() );
      printf("RecoRun Analyze  Points: %d\n",anEvent.GetPointsArray()->GetEntries());
      //      TrackViewer::TrackViewerInstance()->DrawPoints(anEvent.GetPointsArray() );
      printf("RecoRun Analyze  Lines: %d\n",anEvent.GetLineArray()->GetEntries());
      //      TrackViewer::TrackViewerInstance()->DrawTracks( anEvent.GetLineArray() );
      //      printf("RecoRun Analyze  Done With Drawing, for now...\n");

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
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
