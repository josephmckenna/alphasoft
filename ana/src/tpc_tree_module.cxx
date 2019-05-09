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
   TTree *fASignalTree = NULL;
   TTree *fPSignalTree = NULL;
   AgAwHit awbuf;
   AgPadHit padbuf;
   signal sigbuf;

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

      fASignalTree = new TTree("fASignalTree", "Deconvolved anode signals");
      fPSignalTree = new TTree("fPSignalTree", "Deconvolved pad signals");
      fASignalTree->Branch("wire",&sigbuf.idx,"wire/I");
      fASignalTree->Branch("time",&sigbuf.t,"time/D");
      fASignalTree->Branch("amp",&sigbuf.height,"amp/D");
      fPSignalTree->Branch("col",&sigbuf.sec,"col/S");
      fPSignalTree->Branch("row",&sigbuf.idx,"row/I");
      fPSignalTree->Branch("time",&sigbuf.t,"time/D");
      fPSignalTree->Branch("amp",&sigbuf.height,"amp/D");

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

#ifdef LASER
      fAnodeTree->Branch("dtime",&awbuf.time,"dtime/D");
      fPadTree->Branch("dtime",&padbuf.dtime_ns,"dtime/D");
#endif
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
      AgSignalsFlow* esig = flow->Find<AgSignalsFlow>();


      if(eawh){
         for (unsigned j=0; j<eawh->fAwHits.size(); j++) {
            fAnodeTree->GetBranch("mod")->SetAddress(&eawh->fAwHits[j].adc_module);
            fAnodeTree->GetBranch("chan")->SetAddress(&eawh->fAwHits[j].adc_chan);
            fAnodeTree->GetBranch("wire")->SetAddress(&eawh->fAwHits[j].wire);
            fAnodeTree->GetBranch("time")->SetAddress(&eawh->fAwHits[j].time);
#ifdef LASER
            fAnodeTree->GetBranch("dtime")->SetAddress(&eawh->fAwHits[j].dtime);
#endif
            fAnodeTree->GetBranch("amp")->SetAddress(&eawh->fAwHits[j].amp);
            fAnodeTree->Fill();
         }
      }

      if(eph){
         for (unsigned i=0; i<eph->fPadHits.size(); i++) {
            fPadTree->GetBranch("mod")->SetAddress(&eph->fPadHits[i].imodule);
            fPadTree->GetBranch("seqsca")->SetAddress(&eph->fPadHits[i].seqsca);
            // fPadTree->GetBranch("col")->SetAddress(&eph->fPadHits[i].tpc_col);
            int col = (eph->fPadHits[i].tpc_col+1)%32; // KO's tpc_col is NOT the same as the agreed-upon "pad col 0 covers anode wire 0
            fPadTree->GetBranch("col")->SetAddress(&col);
            fPadTree->GetBranch("row")->SetAddress(&eph->fPadHits[i].tpc_row);
            fPadTree->GetBranch("time")->SetAddress(&eph->fPadHits[i].time_ns);
#ifdef LASER
            fPadTree->GetBranch("dtime")->SetAddress(&eph->fPadHits[i].dtime_ns);
#endif
            fPadTree->GetBranch("amp")->SetAddress(&eph->fPadHits[i].amp);
            fPadTree->Fill();
         }
      }

      if(esig){
         for (unsigned i=0; i<esig->awSig.size(); i++) {
            fASignalTree->GetBranch("wire")->SetAddress(&esig->awSig[i].idx);
            fASignalTree->GetBranch("time")->SetAddress(&esig->awSig[i].t);
            fASignalTree->GetBranch("amp")->SetAddress(&esig->awSig[i].height);
            fASignalTree->Fill();
         }
         for (unsigned i=0; i<esig->pdSig.size(); i++) {
            fPSignalTree->GetBranch("col")->SetAddress(&esig->pdSig[i].sec);
            fPSignalTree->GetBranch("row")->SetAddress(&esig->pdSig[i].idx);
            fPSignalTree->GetBranch("time")->SetAddress(&esig->pdSig[i].t);
            fPSignalTree->GetBranch("amp")->SetAddress(&esig->pdSig[i].height);
            fPSignalTree->Fill();
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
