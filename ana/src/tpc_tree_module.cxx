//
// tpc_tree_module.cxx
//
// store tpc data into root tree
//
// L.Martin
//

#include <stdio.h>
#include <cassert>
#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "TTree.h"

class TpcTreeModule: public TARunObject
{
public:
   bool fTrace = true;
   TTree *fPadTree = NULL;
   TTree *fAnodeTree = NULL;
   AgAwHit awbuf;
   AgPadHit padbuf;

   TpcTreeModule(TARunInfo* runinfo): TARunObject(runinfo)
   {
      if(fTrace)
         printf("TpcTreeModule::ctor!\n");
   }

   ~TpcTreeModule()
   {
      if(fTrace)
         printf("TpcTreeModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("TpcTreeModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      TDirectory* dir = gDirectory->mkdir("tpc_tree");
      dir->cd(); // select correct ROOT directory

      fAnodeTree = new TTree("fAnodeTree", "Anode Hits");
      fPadTree = new TTree("fPadTree", "Pad Hits");
      fAnodeTree->Branch("mod",&awbuf.adc_module,"mod/I");
      fAnodeTree->Branch("chan",&awbuf.adc_chan,"chan/I");
      fAnodeTree->Branch("wire",&awbuf.wire,"wire/I");
      fAnodeTree->Branch("time",&awbuf.time,"time/D");
      fAnodeTree->Branch("amp",&awbuf.amp,"amp/D");
      fPadTree->Branch("mod",&padbuf.imodule,"mod/I");
      fPadTree->Branch("seqsca",&padbuf.seqsca,"seqsca/I");
      fPadTree->Branch("col",&padbuf.tpc_col,"col/I");
      fPadTree->Branch("row",&padbuf.tpc_row,"row/I");
      fPadTree->Branch("time",&padbuf.time_ns,"time/D");
      fPadTree->Branch("amp",&padbuf.amp,"amp/D");

   }

   void EndRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("TpcTreeModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("TpcTreeModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("TpcTreeModule::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgAwHitsFlow* eawh = flow->Find<AgAwHitsFlow>();
      AgPadHitsFlow* eph = flow->Find<AgPadHitsFlow>();


      if(eawh){
         for (unsigned j=0; j<eawh->fAwHits.size(); j++) {
            fAnodeTree->GetBranch("mod")->SetAddress(&eawh->fAwHits[j].adc_module);
            fAnodeTree->GetBranch("chan")->SetAddress(&eawh->fAwHits[j].adc_chan);
            fAnodeTree->GetBranch("wire")->SetAddress(&eawh->fAwHits[j].wire);
            fAnodeTree->GetBranch("time")->SetAddress(&eawh->fAwHits[j].time);
            fAnodeTree->GetBranch("amp")->SetAddress(&eawh->fAwHits[j].amp);
            // awbuf = eawh->fAwHits[j];
            fAnodeTree->Fill();
            std::cout << "EE Filled fAnodeTree, now " << fAnodeTree->GetEntries() << " entries." << std::endl;
         }
      }

      if(eph){
         for (unsigned i=0; i<eph->fPadHits.size(); i++) {
            fPadTree->GetBranch("mod")->SetAddress(&eph->fPadHits[i].imodule);
            fPadTree->GetBranch("chan")->SetAddress(&eph->fPadHits[i].seqsca);
            fPadTree->GetBranch("col")->SetAddress(&eph->fPadHits[i].tpc_col);
            fPadTree->GetBranch("row")->SetAddress(&eph->fPadHits[i].tpc_row);
            fPadTree->GetBranch("time")->SetAddress(&eph->fPadHits[i].time_ns);
            fPadTree->GetBranch("amp")->SetAddress(&eph->fPadHits[i].amp);
            padbuf = eph->fPadHits[i];
            fPadTree->Fill();
            std::cout << "EE Filled fPadTree, now " << fPadTree->GetEntries() << " entries." << std::endl;
         }
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("TpcTreeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

};

class TpcTreeModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("TpcTreeModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++)
         {    }
   }

   void Finish()
   {
      printf("TpcTreeModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TpcTreeModule::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TpcTreeModule(runinfo);
   }
};

static TARegister tar(new TpcTreeModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
