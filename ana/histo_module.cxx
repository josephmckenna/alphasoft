//
// histo_module.cxx
//
// histogram of processed TPC events
//

#include <stdio.h>
#include <iostream>

#include "TH2D.h"
#include "TPolyMarker3D.h"

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "Signals.hh"
#include "TPCBase.hh"
#include "TStoreEvent.hh"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;

class HistoRun: public TARunObject
{
public:
   TH2D* h_aw_padcol;
   TH1D* h_timediff;
   TH1D* h_atimes;
   TH1D* h_ptimes;
   TH1D* h_firsttimediff;
   TH2D* h_firsttime;

   TH2D* h_times_aw_p;
   TH2D* h_times_aw_p_match;
   TH1D* h_timediff_aw_p_match;

   TH2D* htH_anode;
   TH2D* htH_pad;
   TH2D *h_unmatched;
   TH1D *h_unmatched_a;
   TH1D *h_unmatched_p;
   TH2D *h_unmatched_p_v_NP;
   TH2D *h_unmatched_p_v_NA;
   TH2D *h_unmatched_a_v_NP;
   TH2D *h_unmatched_a_v_NA;

   TH2D *h_resRMS_a;
   TH2D *h_resRMS_p;

   TH2D *h_amp_a;
   TH2D *h_amp_p;

   TH1D* h3DDist;
   TH1D* hRphiDist;
   TH2D* hpoints2D;
   TH2D* hpoints3D;
   TH2D* hpointsXY;
   TH2D* hpointsZR;
   TH2D* htw;
   TH2D* h3DDistVsR;
   TH2D* hRphiDistVsR;

   TH1D* hNhits;
   TH1D* hNtracks;
   TH1D* hPattRecEff;
   TH1D* hcosang;

   TCanvas *cTimes;

   HistoRun(TARunInfo* runinfo)
      : TARunObject(runinfo)
   {
      printf("HistoRun::ctor!\n");
   }

   ~HistoRun()
   {
      printf("HistoRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("HistoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      TDirectory* dir = gDirectory->mkdir("signalAnalysis");
      dir->cd();

      int naw = TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires();
      int nps = TPCBase::TPCBaseInstance()->GetNumberPadsRow();
      h_aw_padcol = new TH2D("h_aw_padcol", "anode pad coincidences;anode;pad sector", naw, 0, naw, nps, 0, nps);
      h_timediff = new TH1D("h_timediff", "pad / anode time difference;t_a-t_p",4000,-2000,2000);
      h_atimes = new TH1D("h_atimes", "anode times;t in ns",1000,0,10000);
      h_ptimes = new TH1D("h_ptimes", "pad times;t in ns",500,0,8000);
      h_firsttimediff = new TH1D("h_firsttimediff", "pad / anode first time difference;t_a0-t_p0",4000,-2000,2000);
      h_firsttime = new TH2D("h_firsttime", "pad / anode first time;t_a0;t_p0",1000,0,10000,500,0,8000);

      h_times_aw_p = new TH2D("h_times_aw_p", "pad / anode times;t_a;t_p",1000,0,10000,500,0,8000);
      h_times_aw_p_match = new TH2D("h_times_aw_p_match", "pad / anode times matched;t_a;t_p",1000,0,10000,500,0,8000);
      h_timediff_aw_p_match = new TH1D("h_timediff_aw_p_match", "pad / anode time difference;t_a-t_p",4000,-2000,2000);

      htH_anode = new TH2D("htH_anode","TimeVsAmplitude DeconvAnode;t [ns];H [a.u]",1000,0.,10000.,2000,0.,2000.);
      htH_pad = new TH2D("htH_pad","TimeVsAmplitude DeconvPad;t [ns];H [a.u]",1000,0.,10000.,5000,0.,50000.);
      h_unmatched = new TH2D("h_unmatched", "Unmatched signals;number of unmatched anode signals;number of unmatched pad signals",1000,0,1000,1000,0,1000);
      h_unmatched_a = new TH1D("h_unmatched_a", "Unmatched anode signals ratio;number of unmatched anode signals / total number of anode signals",1000,0,1);
      h_unmatched_p = new TH1D("h_unmatched_p", "Unmatched pad signals ratio;number of unmatched pad signals / total number of pad signals",1000,0,1);
      h_unmatched_a_v_NA = new TH2D("h_unmatched_a_v_NA", "Unmatched anode signals vs NA;total number of anode signals;number of unmatched anode signals",1000,0,1000,1000,0,1000);
      h_unmatched_a_v_NP = new TH2D("h_unmatched_a_v_NP", "Unmatched anode signals vs NP;total number of pad signals;number of unmatched anode signals",1000,0,1000,1000,0,1000);
      h_unmatched_p_v_NA = new TH2D("h_unmatched_p_v_NA", "Unmatched pad signals vs NA;total number of anode signals;number of unmatched pad signals",1000,0,1000,1000,0,1000);
      h_unmatched_p_v_NP = new TH2D("h_unmatched_p_v_NP", "Unmatched pad signals vs NP;total number of pad signals;number of unmatched pad signals",1000,0,1000,1000,0,1000);

      h_resRMS_a = new TH2D("h_resRMS_a","RMS of anode deconvolution residual;anode;RMS", naw, 0, naw,2000,0,1000);
      h_resRMS_p = new TH2D("h_resRMS_p","RMS of pad deconvolution residual;pad;RMS", nps*TPCBase::TPCBaseInstance()->GetNumberPadsColumn(), 0, nps*TPCBase::TPCBaseInstance()->GetNumberPadsColumn(),2000,0,10000);

      h_amp_a = new TH2D("h_amp_a","Amplitude of waveform of anode;anode;ADC[?]", naw, 0, naw,2000,0,10000);
      h_amp_p = new TH2D("h_amp_p","Amplitude of waveform of pad;pad;ADC[?]", nps*TPCBase::TPCBaseInstance()->GetNumberPadsColumn(), 0, nps*TPCBase::TPCBaseInstance()->GetNumberPadsColumn(),2000,0,10000);

      runinfo->fRoot->fOutputFile->mkdir("analysis")->cd();
      h3DDist = new TH1D("h3DDist","3D Distance;d [mm];Points",1000,0.,100.);
      hRphiDist = new TH1D("hRphiDist","#sqrt{R#phi^{2}+z^{2}};#delta [mm];Points",
                           1000,0.,100.);

      hpoints2D = new TH2D("hpoints2D","Points;#phi [rad];r [mm];Points",
                           1200,0.,TMath::TwoPi(),
                           1000,
                           TPCBase::TPCBaseInstance()->GetCathodeRadius(true),
                           TPCBase::TPCBaseInstance()->GetROradius(true));
      hpoints3D = new TH2D("hpoints3D","Points;r#phi [mm];z [mm];Points",
                           2000,
                           0.,
                           TPCBase::TPCBaseInstance()->GetROradius(true)*TMath::TwoPi(),
                           2500,
                           -TPCBase::TPCBaseInstance()->GetHalfLengthZ(true),
                           TPCBase::TPCBaseInstance()->GetHalfLengthZ(true));

      hpointsXY = new TH2D("hpointsXY","Points;x [mm];y [mm];Points",
                           1000,
                           -TPCBase::TPCBaseInstance()->GetROradius(true),
                           TPCBase::TPCBaseInstance()->GetROradius(true),
                           1000,
                           -TPCBase::TPCBaseInstance()->GetROradius(true),
                           TPCBase::TPCBaseInstance()->GetROradius(true));
      hpointsZR = new TH2D("hpointsZR","Points;z [mm];r [mm];Points",
                           1000,
                           -TPCBase::TPCBaseInstance()->GetHalfLengthZ(true),
                           TPCBase::TPCBaseInstance()->GetHalfLengthZ(true),
                           1000,
                           TPCBase::TPCBaseInstance()->GetCathodeRadius(true),
                           TPCBase::TPCBaseInstance()->GetROradius(true));

      htw = new TH2D("htw","Drift Time Vs Wire;t [ns];anode",10000,0.,10000.,256,0.,256.);

      h3DDistVsR = new TH2D("h3DDistVsR","3D Distance;d [mm];r [mm];Points",100,0.,100.,
                            81,
                            TPCBase::TPCBaseInstance()->GetCathodeRadius(true),
                            TPCBase::TPCBaseInstance()->GetROradius(true));
      hRphiDistVsR = new TH2D("hRphiDistVsR","#sqrt{R#phi^{2}+z^{2}};r [mm];#delta [mm];Points",
                              100,0.,100.,
                              81,
                              TPCBase::TPCBaseInstance()->GetCathodeRadius(true),
                              TPCBase::TPCBaseInstance()->GetROradius(true));

      hNhits = new TH1D("hNhits","Number of Spacepoints per Event;Points [a.u.];Events [a.u.]",
                        2000,0.,2000.);
      hNtracks = new TH1D("hNtracks","Number of Tracks per Event;Tracks [a.u.];Events [a.u.]",
                          10,0.,10.);
      hPattRecEff = new TH1D("hPattRecEff","Number of Spacepoints per Track per Event;SP/Tr [a.u.];Events [a.u.]",
                             1000,0.,1000.);
      hcosang = new TH1D("hcosang", "Cosine of Angle formed by Cosmics;cos#alpha;Events",
                         8000,-1.,1.);

      runinfo->fRoot->fOutputFile->cd();
      gDirectory->cd("signalAnalysis");

      cTimes = new TCanvas("cTimes","aw and pad times");
      h_atimes->SetFillStyle(0);
      h_ptimes->SetFillStyle(0);
      h_atimes->SetLineColor(kBlue);
      h_ptimes->SetLineColor(kRed);
      h_atimes->SetMinimum(0);
      h_atimes->SetMaximum(2);
      h_atimes->Draw();
      h_ptimes->Draw("same");
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("HistoRun::EndRun, run %d\n", runinfo->fRunNo);
      time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("HistoRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("HistoRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("HistoRun::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if( !ef || !ef->fEvent )
         return flow;
      AgEvent* age = ef->fEvent;

      if( !age->feam || !age->a16 )
         return flow;
      if( !age->feam->complete || !age->a16->complete || age->feam->error || age->a16->error )
         return flow;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
         return flow;

      AgAnalysisFlow* analysis_flow = flow->Find<AgAnalysisFlow>();
      if( !analysis_flow || !analysis_flow->fEvent )
        return flow;
      TStoreEvent* anEvent = analysis_flow->fEvent;


      double t_pad_first = 1e6;
      double t_aw_first = 1e6;
      // if(age->feam && age->a16){

      //    if(age->feam->complete && age->a16->complete && !age->feam->error && !age->a16->error){

      // pf->Reset();
      // pf->GetSignals()->Reset(age,10,16);
      h_atimes->Reset();
      h_ptimes->Reset();
      // int ntimes = signals->Analyze(age,1,1);
      // cout << "KKKK " << ntimes << " times: " << signals->sanode.size() << '\t' << signals->spad.size() << endl;
      // int nmax = std::max(SigFlow->fSig->sanode.size(), SigFlow->fSig->spad.size());
      // for(int i = 0; i < nmax; i++){
      //    cout << "KKKK " << ((i<signals->sanode.size())?(signals->sanode[i].t):-1) << '\t' << ((i<signals->spad.size())?(signals->spad[i].t):-1) << endl;
      // }

      int unmatchedAnodeSignals = anEvent->GetNumberOfUnmatchedAnodes(),
         unmatchedPadSignals = anEvent->GetNumberOfUnmatchedPads();
      //      anEvent.GetNumberOfUnmatched(unmatchedAnodeSignals,unmatchedPadSignals);
      h_unmatched->Fill(unmatchedAnodeSignals,unmatchedPadSignals);
      int NA = SigFlow->awSig.size();
      int NP = SigFlow->pdSig.size();
      h_unmatched_a->Fill(double(unmatchedAnodeSignals)/double(NA));
      h_unmatched_p->Fill(double(unmatchedPadSignals)/double(NP));
      h_unmatched_a_v_NA->Fill(NA,unmatchedAnodeSignals);
      h_unmatched_a_v_NP->Fill(NP,unmatchedAnodeSignals);
      h_unmatched_p_v_NA->Fill(NA,unmatchedPadSignals);
      h_unmatched_p_v_NP->Fill(NP,unmatchedPadSignals);

      bool first = true;
      cout << "NA: " << NA << endl;
      for(auto sa: SigFlow->awSig){
         if(sa.t < t_aw_first) t_aw_first = sa.t;
         h_atimes->Fill(sa.t);
         double r, phi;
         TPCBase::TPCBaseInstance()->GetAnodePosition(sa.i, r, phi, true);
         std::pair<int,int> pad = TPCBase::TPCBaseInstance()->FindPad(0, phi);
         for(auto sp: SigFlow->pdSig){
            if(first){
               if(sp.t < t_pad_first) t_pad_first = sp.t;
               h_ptimes->Fill(sp.t);
            }
            // cout << "KKKK " << sa.i << '\t' << sp.sec << endl;
            h_timediff->Fill(sa.t-sp.t);
            h_times_aw_p->Fill(sa.t, sp.t);

            if(sp.sec == pad.first){
               h_times_aw_p_match->Fill(sa.t, sp.t);
               h_timediff_aw_p_match->Fill(sa.t-sp.t);
            }

            if(abs(sa.t-sp.t) < 16)
               h_aw_padcol->Fill(sa.i, sp.sec);
         }
         first = false;
      }
      cTimes->Modified();
      cTimes->Update();

      h_firsttimediff->Fill(t_aw_first-t_pad_first);
      h_firsttime->Fill(t_aw_first,t_pad_first);

      const vector<TPCBase::electrode> &anodes = SigFlow->awIndex;
      const vector<double> &resRMS_a = SigFlow->awResRMS;
      for(unsigned int i= 0; i < anodes.size(); i++){
         h_resRMS_a->Fill(anodes[i].i, resRMS_a[i]);
         int wf_aw_index = SigFlow->AWwf[i].i;
         const vector<int> *wf = SigFlow->AWwf[i].wf;
         h_amp_a->Fill( wf_aw_index, *std::max_element( wf->begin(),
                                                        wf->end() ) );
      }
      const vector<TPCBase::electrode> &pads = SigFlow->pdIndex;
      const vector<double> &resRMS_p = SigFlow->pdResRMS;
      for(unsigned int i= 0; i < pads.size(); i++){
         h_resRMS_p->Fill(pads[i].sec*TPCBase::TPCBaseInstance()->GetNumberPadsColumn()+pads[i].i, resRMS_p[i]);
         const vector<int> *wf = SigFlow->PADwf[i].wf;

         h_amp_p->Fill( SigFlow->PADwf[i].sec*TPCBase::TPCBaseInstance()->GetNumberPadsColumn()+SigFlow->PADwf[i].i,
                        *std::max_element( wf->begin(),
                                           wf->end() ) );
      }
      for(auto sa: SigFlow->awSig)
         htH_anode->Fill(sa.t,sa.height);
      for(auto sp: SigFlow->pdSig)
         htH_pad->Fill(sp.t,sp.height);

      const TObjArray* points = anEvent->GetSpacePoints();
      for(int i=0; i<points->GetEntries(); ++i)
         {
            TSpacePoint* spi = (TSpacePoint*) points->At(i);
            double ri = spi->GetR(),
               phii = spi->GetPhi(),
               zi = spi->GetZ(),
               yi = spi->GetY(),
               xi = spi->GetX();
            hpoints2D->Fill(phii,ri);
            double rphii = ri*phii;
            hpoints3D->Fill(rphii,zi);

            hpointsXY->Fill(xi,yi);
            hpointsZR->Fill(zi,ri);

            double td = spi->GetTime(),
               aw = double(spi->GetWire());
            htw->Fill(td,aw);

            for(int j=0; j<points->GetEntries(); ++j)
               {
                  if( i == j ) continue;

                  TSpacePoint* spj = (TSpacePoint*) points->At(j);
                  double rj = spj->GetR();

                  if( ri<rj ) continue;

                  double dist = spi->Distance( spj );
                  double rpd = spi->DistanceRphi( spj );

                  h3DDist->Fill(dist);
                  hRphiDist->Fill(rpd);

                  h3DDistVsR->Fill(dist,ri);
                  hRphiDistVsR->Fill(rpd,ri);
               }
         }

      int Nhits = anEvent->GetNumberOfHits();
      // printf("FinalRun::Analyze   Number of Hits: %d\n",Nhits);
      hNhits->Fill(Nhits);
      int Ntracks = anEvent->GetNumberOfTracks();
      hNtracks->Fill(Ntracks);
      if( Ntracks > 0 )
         printf("@@@Tracks in run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      hPattRecEff->Fill( anEvent->GetNumberOfHitsPerTrack() );
      if( Ntracks == 2 )
         {
            hcosang->Fill( anEvent->GetAngleBetweenTracks() );
         }

      //    }
      // }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("HistoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class HistoModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("HistoModuleFactory::Init!\n");
   }
   void Finish()
   {
      printf("HistoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("HistoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HistoRun(runinfo);
   }
};


static TARegister tar(new HistoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
