//
// tpc_tree_module.cxx
//
// store tpc data into root tree
//
// L.Martin
//

#include <stdio.h>
#include <cassert>
#include <numeric>
#include <algorithm> 
#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

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
   ALPHAg::signal sigbuf;

   struct _sum_aw_time
   {
      double sum=0.0;
      int n=0;
      void operator()(AgAwHit& h1)
      {
         double t=h1.time;
         if( t<2000. && h1.amp>20000 ){ 
            ++n;
            sum+=t; }
      }
      double GetTime() { return n>0?sum/double(n):0.0; }
   } aw_dtime;

   struct _sum_pads_time
   {
      double sum=0.0;
      int n=0;
      void operator()(AgPadHit& h1)
      {
         double t=h1.time_ns;
         if( t< 2000.){
            ++n;
            sum+=t;
         }
      }
      double GetTime() { return n>0?sum/double(n):0.0; }
   } pads_dtime;


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

      fAnodeTree->Branch("dtime",&awbuf.time,"dtime/D");
      fPadTree->Branch("dtime",&padbuf.time_ns,"dtime/D");
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
         aw_dtime.sum=0.0;
         std::for_each(eawh->fAwHits.begin(),eawh->fAwHits.end(),
                       aw_dtime);
         double t0=aw_dtime.GetTime();

         for (unsigned j=0; j<eawh->fAwHits.size(); j++) {
            fAnodeTree->GetBranch("mod")->SetAddress(&eawh->fAwHits[j].adc_module);
            fAnodeTree->GetBranch("chan")->SetAddress(&eawh->fAwHits[j].adc_chan);
            fAnodeTree->GetBranch("wire")->SetAddress(&eawh->fAwHits[j].wire);
            double tt = eawh->fAwHits[j].time;
            double dt = tt-t0;
            fAnodeTree->GetBranch("time")->SetAddress(&tt);
            fAnodeTree->GetBranch("dtime")->SetAddress(&dt);
            fAnodeTree->GetBranch("amp")->SetAddress(&eawh->fAwHits[j].amp);
            fAnodeTree->Fill();
         }
      }

      if(eph){
         pads_dtime.sum=0.0;
         std::for_each(eph->fPadHits.begin(),eph->fPadHits.end(),
                       pads_dtime);
         double t0=pads_dtime.GetTime();

         for (unsigned i=0; i<eph->fPadHits.size(); i++) {
            fPadTree->GetBranch("mod")->SetAddress(&eph->fPadHits[i].imodule);
            fPadTree->GetBranch("seqsca")->SetAddress(&eph->fPadHits[i].seqsca);
            // fPadTree->GetBranch("col")->SetAddress(&eph->fPadHits[i].tpc_col);
            int col = (eph->fPadHits[i].tpc_col+1)%32; // KO's tpc_col is NOT the same as the agreed-upon "pad col 0 covers anode wire 0
            // instead follows the numbering of the power distribution
            fPadTree->GetBranch("col")->SetAddress(&col);
            fPadTree->GetBranch("row")->SetAddress(&eph->fPadHits[i].tpc_row);
            double tt = eph->fPadHits[i].time_ns;
            double dt = tt-t0;
            fPadTree->GetBranch("time")->SetAddress(&tt);
            fPadTree->GetBranch("dtime")->SetAddress(&dt);
            fPadTree->GetBranch("amp")->SetAddress(&eph->fPadHits[i].amp);
            fPadTree->Fill();
         }
      }

      if(esig){
         if( !esig->awSig ) return flow;
         for (unsigned i=0; i<esig->awSig->size(); i++) {
            fASignalTree->GetBranch("wire")->SetAddress(&esig->awSig->at(i).idx);
            fASignalTree->GetBranch("time")->SetAddress(&esig->awSig->at(i).t);
            fASignalTree->GetBranch("amp")->SetAddress(&esig->awSig->at(i).height);
            fASignalTree->Fill();
         }
         if( !esig->pdSig ) return flow;
         for (unsigned i=0; i<esig->pdSig->size(); i++) {
            fPSignalTree->GetBranch("col")->SetAddress(&esig->pdSig->at(i).sec);
            fPSignalTree->GetBranch("row")->SetAddress(&esig->pdSig->at(i).idx);
            fPSignalTree->GetBranch("time")->SetAddress(&esig->pdSig->at(i).t);
            fPSignalTree->GetBranch("amp")->SetAddress(&esig->pdSig->at(i).height);
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
