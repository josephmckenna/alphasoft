#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm> //sort
#include <fstream>
#include <set>
#include <future>
#include <numeric>

#include "TH1.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TMath.h"


#include "SignalsType.h"
#include "tinyspline.hh"

#include "AnalysisTimer.h"




class DeconvFlags
{
public:
   double fADCthr=1000.;
   double fPWBthr=200.;
   double fAWthr=10.;
   double fPADthr=10.;
   bool fRecOff = false; //Turn reconstruction off
   bool fDiag=false;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   bool fBatch = true;
   bool fPWBmap = false;
public:
   DeconvFlags() // ctor
   { }

   ~DeconvFlags() // dtor
   { }
};

inline bool comp_hist(wfholder* lhs, wfholder* rhs)
{
   //return lhs->val >= rhs->val; //segfault
   return lhs->val > rhs->val;
}



class DeconvModule: public TARunObject
{
public:
   DeconvFlags* fFlags = 0;
   //bool fTrace = true;
   bool fTrace = false;
   int fCounter = 0;

private:
   // input
   std::vector<double> fAnodeFactors;

   std::vector<double> fAnodeResponse;
   std::vector<double> fPadResponse;

   std::vector<double> fAdcRescale;
   std::vector<double> fPwbRescale;

   // control
   bool diagnostics; // dis/en-able histogramming
   bool display;     // dis/en-able wf storage for aged

   int fbinsize;
   int fAWbinsize;
   int fPADbinsize;

   double fADCdelay;
   double fPWBdelay;

   int nAWsamples;
   int pedestal_length;
   double fScale;

   int theAnodeBin;
   int thePadBin;

   double fADCThres;
   double fPWBThres;

   double fAvalancheSize;
   double fADCpeak;
   double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<electrode> fAnodeIndex;
   std::vector<electrode> fPadIndex;

   std::vector<signal> sanode;
   std::vector<signal> spad;

   std::set<double> aTimes;
   std::set<double> pTimes;

   // check
   std::vector<double> resRMS_a;
   std::vector<double> resRMS_p;

   // to use in aged display
   std::vector<wf_ref> wirewaveforms;
   std::vector<wf_ref> feamwaveforms;

   // waveform max
   std::vector<signal> fAdcPeaks;
   std::vector<signal> fAdcRange;
   std::vector<signal> fPwbPeaks;
   std::vector<signal> fPwbRange;

   // anodes
   TH1D* hAvgRMSBot;
   TH1D* hAvgRMSTop;

   TH2D* hADCped;
   TProfile* hADCped_prox;
   TH2D* hPWBped;
   TProfile* hPWBped_prox;

   // pads
   TH1D* hAvgRMSPad;

   // pwb map
   std::ofstream pwbmap;

   // anode mask
   std::vector<int> fAwMask;
   // pad mask
   std::vector<int> fPadSecMask;
   std::vector<int> fPadRowMask;

public:

   DeconvModule(TARunInfo* runinfo, DeconvFlags* f)
      : TARunObject(runinfo),
        fbinsize(1),
        fADCdelay(0.),fPWBdelay(0.), // to be guessed
        nAWsamples(335),// maximum value that works for mixed ADC, after pedestal
        pedestal_length(100),fScale(-1.), // values fixed by DAQ
        fAvalancheSize(0.), // to be set later
        isalpha16(false)
   {
      if (fTrace)
         printf("DeconvModule::ctor!\n");

      fFlags = f;

      fAnodeFactors = {
         0.1275,        // neighbour factor
         0.0365,        // 2nd neighbour factor
         0.012,         // 3rd neighbour factor
         0.0042         // 4th neighbour factor
      };

      fAWbinsize=int(_timebin);
      fPADbinsize=int(_timebin);

      theAnodeBin=1;
      thePadBin=6;

      fADCThres=fFlags->fADCthr;
      fPWBThres=fFlags->fPWBthr;
      fADCpeak=fFlags->fAWthr;
      fPWBpeak=fFlags->fPADthr;

      fAwMask.reserve(256);
      fPadSecMask.reserve(32);
      fPadRowMask.reserve(576);

      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      display=!fFlags->fBatch;   // dis/en-able wf storage for aged
   }

   ~DeconvModule()
   {
      if (fTrace)
         printf("DeconvModule::dtor!\n");
       //if(ct) delete ct;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      if( diagnostics )
         {
            std::cout<<"DeconvModule::BeginRun with Diagnostics"<<std::endl;
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            // anodes histograms
            gDirectory->mkdir("awdeconv")->cd();

            hAvgRMSBot = new TH1D("hAvgRMSBot","Average Deconv Remainder Bottom",500,0.,10000.);
            hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",500,0.,10000.);

            hADCped = new TH2D("hADCped","ADC pedestal per AW",256,0.,256.,2000,-33000.,33000);
            hADCped_prox = new TProfile("hADCped_prox","Average ADC pedestal per AW;AW;ADC",
                                        256,0.,256.,-33000.,33000);

            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            // pads histograms
            gDirectory->mkdir("paddeconv")->cd();

            hAvgRMSPad = new TH1D("hAvgRMSPad","Average Deconv Remainder Pad",500,0.,10000.);

            hPWBped = new TH2D("hPWBped","PWB pedestal per Pad",32*576,0.,_padcol*_padrow,2000,-33000.,33000);
            hPWBped_prox = new TProfile("hPWBped_prox","Average PWB pedestal per Pad;Pad;PWB",
                                        32*576,0.,_padcol*_padrow,-33000.,33000);
         }

      // by run settings
      int run_number = runinfo->fRunNo;
      if( run_number < 2724 ) // new FMC-32
         {
            fAWbinsize=10;
            isalpha16=true;
         }

      if( run_number == 2246 || run_number == 2247 || run_number == 2248 || run_number == 2249 || run_number == 2251 )
         fPWBdelay = -50.;
      else if( run_number == 2272 || run_number ==  2273 || run_number == 2274 )
            fPWBdelay = 136.;
      else if( run_number >= 2282 && run_number < 2724 )
         {
            fADCdelay = -120.;
            fPWBdelay = 0.;
         }
      else if( run_number >= 2724 && run_number < 3032 ) // new FMC-32
         {
            fADCdelay = 0.;
            fPWBdelay = 0.;
         }
      else if( run_number >= 3032 )
         {
            fADCdelay = -250.;
            fPWBdelay = 0.;
         }
      if( run_number == 3169 || run_number == 3209 || run_number == 3226 || run_number == 3241 ||
          run_number == 3249 || run_number == 3250 || run_number == 3251 ||
          run_number == 3253 || run_number == 3254 || run_number == 3255 ||
          run_number == 3260 || run_number == 3263 || run_number == 3265 ||
          run_number == 3875 || run_number == 3866 || run_number == 3859 || run_number == 3855) // TrigBscMult
         {
            fADCdelay = -400.;
            fPWBdelay = -100.;
         }
      if( run_number == 3170 || run_number == 3195 || run_number == 3190 ||
          run_number == 3187 || run_number == 3186 || run_number == 3184 ||
          run_number == 3181 || run_number == 3178 || run_number == 3208 ||
          run_number == 3210 || run_number == 3245 ||
          run_number == 3874 || run_number == 3858 ) // TrigCoinc
         {
            fADCdelay = 0.;
            fPWBdelay = 0.;
         }

      // electrodes masking
      if( run_number == 2635 )
         {
            // only top aw are connected
            // numbering [256,511]
            fAwMask.push_back(3+256);
            fPadSecMask.push_back(0);
            for(int i=0; i<=10; ++i)
               {
                  fPadRowMask.push_back(361+i);
               }
         }
      else if( run_number == 2605 || run_number == 2617 || run_number == 2638 )
         {
            fPadSecMask.push_back(18);
            fPadRowMask.push_back(215);
         }
      else if( run_number == 2731 || run_number == 2732 || run_number == 2734 || run_number == 2735 )
         {
            fPadSecMask.push_back(31);
            fPadRowMask.push_back(144);
            fPadRowMask.push_back(216);

            fPadSecMask.push_back(2);
            fPadRowMask.push_back(143);

            fPadSecMask.push_back(18);
            fPadRowMask.push_back(215);
         }
      else if( run_number == 3865 )
         {
            fPadSecMask.push_back(21);
            fPadRowMask.push_back(503);
            fPadRowMask.push_back(504);
            fPadRowMask.push_back(505);
         }
      // if( run_number >= 3003 )
      //    fAwMask.push_back(142+256);


      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Deconv Settings"<<std::endl;
      std::cout<<" ADC time bin: "<<fAWbinsize<<" ns\tPWB time bin: "<<fPADbinsize<<" ns"<<std::endl;
      std::cout<<" ADC delay: "<<fADCdelay<<"\tPWB delay: "<<fPWBdelay<<std::endl;
      std::cout<<" ADC thresh: "<<fADCThres<<"\tPWB thresh: "<<fPWBThres<<std::endl;
      std::cout<<" AW thresh: "<<fADCpeak<<"\tPAD thresh: "<<fPWBpeak<<std::endl;
      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Masked Electrodes"<<std::endl;
      std::cout<<"AW: ";
      for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
         std::cout<<*it-256<<", ";
      std::cout<<"\n"<<std::endl;
      std::cout<<"PAD: ";
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
         for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
            std::cout<<"["<<*it<<","<<*jt<<"],"<<std::endl;
      std::cout<<"\n"<<std::endl;

      if( fFlags->fPWBmap )
         {
            std::string mapname="pwbR";
            mapname += std::to_string(run_number);
            mapname += ".map";
            pwbmap.open(mapname.c_str());
         }

      int s = ReadResponseFile(fAWbinsize,fPADbinsize);
      std::cout<<"DeconvModule BeginRun Response status: "<<s<<std::endl;
      assert(s>0);

      std::string basepath(getenv("AGRELEASE"));
      // // std::ifstream fadcres("ana/AdcRescale.dat");
      // std::ifstream fadcres(basepath+"/ana/AdcRescale.dat");
      // double rescale_factor;
      // while(1)
      //    {
      //       fadcres>>rescale_factor;
      //       if( !fadcres.good() ) break;
      //       //fAdcRescale.push_back(rescale_factor);
      //       fAdcRescale.push_back(1.);
      //    }
      // fadcres.close();
      fAdcRescale.assign(256,1.0);
      if( fAdcRescale.size() == 256 )
         std::cout<<"DeconvModule BeginRun ADC rescaling factors OK"<<std::endl;
      else
         std::cout<<"DeconvModule BeginRun ADC rescaling factors NOT ok (size: "
                  <<fAdcRescale.size()<<")"<<std::endl;

      // //std::ifstream fpwbres("ana/PwbRescale.dat");
      // std::ifstream fpwbres(basepath+"/ana/PwbRescale.dat");
      // while(1)
      //    {
      //       fpwbres>>rescale_factor;
      //       if( !fpwbres.good() ) break;
      //       //fPwbRescale.push_back(rescale_factor);
      //       fPwbRescale.push_back(1.);
      //    }
      // fpwbres.close();
      fPwbRescale.assign(32*576,1.0);
      if( fPwbRescale.size() == 32*576 )
         std::cout<<"DeconvModule BeginRun PWB rescaling factors OK"<<std::endl;
      else
         std::cout<<"DeconvModule BeginRun PWB rescaling factors NOT ok (size: "
                  <<fPwbRescale.size()<<")"<<std::endl;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DeconvModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      if( fFlags->fPWBmap ) pwbmap.close();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
         return flow;

      if(fTrace)
         printf("DeconvModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      const AgEvent* e = ef->fEvent;
      if (fFlags->fTimeCut)
      {
        if (e->time<fFlags->start_time)
          return flow;
        if (e->time>fFlags->stop_time)
          return flow;
      }

      if (fFlags->fEventRangeCut)
      {
         if (e->counter<fFlags->start_event)
           return flow;
         if (e->counter>fFlags->stop_event)
           return flow;
      }

      std::future<int> stat_aw, stat_pwb;
      const Alpha16Event* aw = e->a16;
      if( !aw )
         {
            std::cout<<"DeconvModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            return flow;
         }
      else
         stat_aw = std::async( &DeconvModule::FindAnodeTimes, this, aw );

      const FeamEvent* pwb = e->feam;
      if( !pwb ) // allow for events without pwbs
         {
            std::cout<<"DeconvModule::AnalyzeFlowEvent(...) No FeamEvent in AgEvent # "
                     <<e->counter<<std::endl;
            //return flow;
         }
      else
         stat_pwb = std::async( &DeconvModule::FindPadTimes, this, pwb );

      int stat = stat_aw.get();
      if( stat == 0 ) return flow;
      if( diagnostics ) AWdiagnostic();

      if( pwb )
         {
            stat = stat_pwb.get();
            if( stat && diagnostics ) PADdiagnostic();
         }

      //      AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, sanode, spad);
      AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, sanode);
      if( stat && pwb )
         {
            flow_sig->AddPadSignals(spad);
         }

      if( diagnostics )
         {
            flow_sig->adc32max = fAdcPeaks;
            flow_sig->adc32range = fAdcRange;

            flow_sig->pwbMax = fPwbPeaks;
            flow_sig->pwbRange = fPwbRange;
         }

      if( display )
         {
            flow_sig->AddWaveforms(wirewaveforms, feamwaveforms);
         }

      flow = flow_sig;
      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"deconv_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   int FindAnodeTimes(const Alpha16Event* anodeSignals)
   {
      fbinsize = fAWbinsize;
      fAvalancheSize = fADCpeak;
      auto& channels = anodeSignals->hits; // vector<Alpha16Channel*>
      if( fTrace )
         std::cout<<"DeconvModule::FindAnodeTimes Channels Size: "<<channels.size()<<std::endl;

      // prepare vector with wf to manipulate
      std::vector<wfholder*> AnodeWaves;
      AnodeWaves.reserve( channels.size() );

      // clear/initialize "output" vectors
      //      std::cout<<"DeconvModule::FindAnodeTimes clear/initialize \"output\" vectors"<<std::endl;
      fAnodeIndex.clear();
      fAnodeIndex.reserve( channels.size() );
      sanode.clear();
      sanode.reserve(channels.size());
      aTimes.clear();

      wirewaveforms.clear();
      wirewaveforms.reserve(channels.size());

      fAdcPeaks.clear();
      fAdcPeaks.reserve(channels.size());
      fAdcRange.clear();
      fAdcRange.reserve(channels.size());

      // find intresting channels
      int index=0; //wfholder index
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan < 16 && !isalpha16 ) continue; // it's bv

            int aw_number = ch->tpc_wire;
            // std::cout<<"DeconvModule::FindAnodeTimes anode wire: "<<aw_number<<std::endl;
            if( aw_number < 0 || aw_number >= 512 ) continue;
            // CREATE electrode
            electrode el(aw_number);
            el.setgain( fAdcRescale.at(el.idx) ); // this checks that gain > 0
            //el.print();

            // mask hot wires
            bool mask=false;
            for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
               {
                  if( *it == aw_number )
                     {
                        mask = true;
                        break;
                     }
               }
            if( mask ) continue;

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += ch->adc_samples.at( b );
            ped /= double(pedestal_length);
            if( fTrace )
               std::cout<<"DeconvModule::FindAnodeTimes pedestal for anode wire: "<<el.idx
                        <<" is "<<ped<<std::endl;
            // CALCULATE PEAK HEIGHT
            auto minit = std::min_element(ch->adc_samples.begin(), ch->adc_samples.end());
            double max = el.gain * fScale * ( double(*minit) - ped );
            if( fTrace )
               std::cout<<"DeconvModule::FindAnodeTimes amplitude for anode wire: "<<el.idx
                        <<" is "<<max<<std::endl;
            if( diagnostics )
               {
                  hADCped->Fill(double(el.idx),ped);
                  hADCped_prox->Fill(double(el.idx),ped);

                  // CALCULATE PEAK POSITION (TIME)
                  double peak_time = ( (double) std::distance(ch->adc_samples.begin(),minit) + 0.5 )*fAWbinsize + fADCdelay;

                  // diagnostics for hot wires
                  fAdcPeaks.emplace_back(el.idx,peak_time,max);
                  auto maxit = std::max_element(ch->adc_samples.begin(), ch->adc_samples.end());
                  double min = el.gain * fScale * ( double(*maxit) - ped );
                  fAdcRange.emplace_back(el.idx,peak_time,max-min);
               }

            if(max > fADCThres)     // Signal amplitude < thres is considered uninteresting
               {
                  if(fTrace)
                     std::cout<<"\tsignal above threshold ch: "<<i<<" aw: "<<aw_number<<std::endl;

                  // SUBTRACT PEDESTAL
                    wfholder* waveform=new wfholder;
                    waveform->h=new std::vector<double>(ch->adc_samples.begin()+pedestal_length,ch->adc_samples.end());
                    waveform->index=index;
                    index++;
                  std::for_each(waveform->h->begin(), waveform->h->end(), [ped](double& d) { d-=ped;});

                  // fill vector with wf to manipulate
                  AnodeWaves.emplace_back( waveform );

                  // STORE electrode
                  // electrode el(aw_number);
                  fAnodeIndex.push_back( el );

                  wirewaveforms.emplace_back(el,waveform->h);
               }// max > thres
         }// channels


      // DECONVOLUTION
      int nsig = Deconv(&AnodeWaves,sanode,aTimes,fAnodeIndex,fAnodeResponse,theAnodeBin,true);
      std::cout<<"DeconvModule::FindAnodeTimes "<<nsig<<" found"<<std::endl;
      //

      if( diagnostics )
         {
            // prepare control variable (deconv remainder) vector
            resRMS_a.clear();
            resRMS_a.reserve( AnodeWaves.size() );
            // calculate remainder of deconvolution
            for(auto s: AnodeWaves)
               resRMS_a.push_back( sqrt(
                                        std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
                                        / static_cast<double>(s->h->size()) )
                                   );
         }
      for (uint i=0; i<AnodeWaves.size(); i++)
      {
         delete AnodeWaves.at(i)->h;
         delete AnodeWaves.at(i);
      }
      AnodeWaves.clear();
      return nsig;
   }

   int FindPadTimes(const FeamEvent* padSignals)
   {
      fbinsize = fPADbinsize;
      fAvalancheSize = fPWBpeak;

      auto& channels = padSignals->hits; // vector<FeamChannel*>
      if( fTrace )
         std::cout<<"DeconvModule::FindPadTimes Channels Size: "<<channels.size()<<std::endl;

      // prepare vector with wf to manipulate
      std::vector<wfholder*> PadWaves;
      PadWaves.reserve( channels.size() );

      // clear/initialize "output" vectors
      fPadIndex.clear();
      fPadIndex.reserve( channels.size() );

      spad.clear();
      spad.reserve(channels.size());
      pTimes.clear();

      feamwaveforms.clear();
      feamwaveforms.reserve(channels.size());

      // find intresting channels
      int index=0; //wfholder index
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // FeamChannel*
            if( !PwbPadMap::chan_is_pad(ch->sca_chan) ) continue;

            short col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
            col+=1;
            if( col == 32 ) col = 0;
            assert(col<32);
            // std::cout<<"DeconvModule::FindPadTimes() col: "<<col<<std::endl;
            int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
            // std::cout<<"DeconvModule::FindPadTimes() row: "<<row<<std::endl;
            assert(row<576);
            // CREATE electrode
            electrode el(col,row);
            int pad_index = col + 32 * row;
            el.setgain( fPwbRescale.at(pad_index) );

            // mask hot pads
            bool mask = false;
            for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
               {
                  for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
                     {
                        if( *it == col && *jt == row )
                           {
                              mask = true;
                              break;
                           }
                     }
                  if( mask ) break;
               }
            if( mask ) continue;

            // nothing dumb happens
            if( ch->adc_samples.size() < 510 )
               {
                  std::cerr<<"DeconvModule::FindPadTimes ERROR wf samples: "
                           <<ch->adc_samples.size()<<std::endl;
                  continue;
               }

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += ch->adc_samples.at( b );
            ped /= pedestal_length;
            // CALCULATE PEAK HEIGHT
            auto minit = std::min_element(ch->adc_samples.begin(), ch->adc_samples.end());
            double max = el.gain * fScale * ( double(*minit) - ped );

            if( diagnostics )
               {
                  hPWBped->Fill(double(pad_index),ped);
                  hPWBped_prox->Fill(double(pad_index),ped);

                  // CALCULATE PEAK POSITION (TIME)
                  double peak_time = ( (double) std::distance(ch->adc_samples.begin(),minit) + 0.5 )*fPADbinsize + fPWBdelay;
                  // diagnostics for pads wires
                  fPwbPeaks.emplace_back(el,peak_time,max);
                  auto maxit = std::max_element(ch->adc_samples.begin(), ch->adc_samples.end());
                  double min = el.gain * fScale * ( double(*maxit) - ped );
                  fPwbRange.emplace_back(el,peak_time,max-min);
               }

            if(max > fPWBThres)     // Signal amplitude < thres is considered uninteresting
               {
                  if(fTrace && 0)
                     std::cout<<"\tsignal above threshold ch: "<<i<<std::endl;

                    // SUBTRACT PEDESTAL
                    wfholder* waveform=new wfholder;
                    waveform->h=new std::vector<double>(ch->adc_samples.begin()+pedestal_length,ch->adc_samples.end());
                    waveform->index=index;
                    index++;
                  std::for_each(waveform->h->begin(), waveform->h->end(), [ped](double& d) { d-=ped;});

                  // fill vector with wf to manipulate
                  PadWaves.emplace_back( waveform );
                  //aresult.emplace_back( waveform.size() );

                  // STORE electrode
                  //electrode el(col,row);
                  fPadIndex.push_back( el );
                  if( fTrace && 0 )
                     std::cout<<"DeconvModule::FindPadTimes() pwb"<<ch->imodule
                              <<" col: "<<col
                              <<" row: "<<row
                              <<" ph: "<<max<<std::endl;
                  // make me a map of pads -> pwbs
                  if( fFlags->fPWBmap )
                     pwbmap<<col<<"\t"<<row<<"\t" // pad
                           <<ch->pad_col<<"\t"<<ch->pad_row<<"\t" // local pad
                           <<ch->imodule<<std::endl; // pwb S/N

                  feamwaveforms.emplace_back(el,waveform->h);
               }// max > thres
         }// channels


      // DECONVOLUTION
      int nsig = Deconv(&PadWaves,spad,pTimes,fPadIndex,fPadResponse,thePadBin,false);
      std::cout<<"DeconvModule::FindPadTimes "<<nsig<<" found"<<std::endl;
      //

      if( diagnostics )
         {
            // prepare control variable (deconv remainder) vector
            resRMS_p.clear();
            resRMS_p.reserve( PadWaves.size() );
            // calculate remainder of deconvolution
            for(auto s: PadWaves)
               resRMS_p.push_back( sqrt(
                                        std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
                                        / static_cast<double>(s->h->size()) )
                                   );
         }
      for (uint i=0; i<PadWaves.size(); i++)
      {
         delete PadWaves.at(i)->h;
         delete PadWaves.at(i);
      }
      PadWaves.clear();
      return nsig;
   }

   int ReadResponseFile(const double awbin, const double padbin)
   {
      std::string filename(getenv("AGRELEASE"));
      filename+="/ana/anodeResponse.dat";
      std::cout << "DeconvModule:: Reading in response file (anode) " << filename << std::endl;
      std::ifstream respFile(filename.c_str());
      if( !respFile.good() ) return 0;
      double binedge, val;
      std::vector<double> resp;
      while(1)
         {
            respFile >> binedge >> val;
            if( !respFile.good() ) break;
            resp.push_back(fScale*val);
         }
      respFile.close();
      fAnodeResponse=Rebin(resp, awbin);

      double frac = 0.1;
      double max = *std::max_element(fAnodeResponse.begin(), fAnodeResponse.end());
      double thres = frac*max;
      for(unsigned b=0; b<fAnodeResponse.size(); ++b)
         {
            if( fAnodeResponse.at(b) > thres )
               theAnodeBin=b;
         }
      if( fTrace )
         std::cout<<"DeconvModule::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theAnodeBin<<std::endl;

      //      filename = "padResponse_deconv.dat";
      filename=getenv("AGRELEASE");
      filename+="/ana/padResponse_deconv.dat";
      std::cout << "DeconvModule:: Reading in response file (pad) " << filename << std::endl;
      respFile.open(filename.c_str());
      if( !respFile.good() ) return 0;
      resp.clear();
      while(1)
         {
            respFile >> binedge >> val;
            if( !respFile.good() ) break;
            resp.push_back(val);
         }
      respFile.close();
      fPadResponse=Rebin(resp, padbin);

      max = *std::max_element(fPadResponse.begin(), fPadResponse.end());
      thres = frac*max;
      for(unsigned b=0; b<fPadResponse.size(); ++b)
         {
            if( fPadResponse.at(b) > thres )
               thePadBin=b;
         }
      if( fTrace )
         std::cout<<"DeconvModule::ReadResponseFile pad max: "<<max<<"\tpad bin: "<<thePadBin<<std::endl;

      return fAnodeResponse.size() && fPadResponse.size();
   }

   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.)
   {
      if( fTrace )
         std::cout<<"DeconvModule::Rebin "<<binsize<<std::endl;
      if(binsize == 1) return in;
      std::vector<double> result;
      result.reserve(in.size()/binsize);
      for(unsigned int i = 0; i < in.size(); i++)
         {
            if(i / binsize == result.size()-1)
               result.back() += double(in[i])-ped;
            else if(i / binsize == result.size())
               result.push_back(double(in[i])-ped);
         }
      if(result.size()*binsize > in.size())
         {
            result.pop_back();
            // if( verbose )
            // std::cout << "Signals::Rebin: Cannot rebin without rest, dropping final "
            //           << in.size() % result.size() << std::endl;
         }
      return result;
   }

   template <typename T>
   std::vector<double> Interpolate(const std::vector<T> &in,
                                   double inbinsize, double outbinsize,
                                   double tshift = 0.)
   {
      std::vector<tinyspline::real> workVec;   // tinyspline vector of shape {x0, y0, x1, y1, ...}
      workVec.reserve(2*in.size());
      workVec.clear();
      for(unsigned int i = 0; i < in.size(); i++){
         workVec.push_back(double(i)*inbinsize);
         workVec.push_back(in[i]);
      }
      // high_resolution_clock::time_point t1 = high_resolution_clock::now();
      // Create a spline...
      tinyspline::BSpline spline(
                                 workVec.size()/2, // ... consisting of control points...
                                 2, // ... in 2D...
                                 3, // ... of degree 3...
                                 TS_CLAMPED // ... using a clamped knot vector.
                                 );
      spline.setCtrlp(workVec);
      // high_resolution_clock::time_point t2 = high_resolution_clock::now();
      std::vector<tinyspline::real> tmpresult;

      unsigned int nOut = in.size()*inbinsize/outbinsize;
      std::vector<double> result;
      result.reserve(nOut);

      double delta = outbinsize/(double(in.size())*inbinsize); // spline interpolation uses parameter of range range [0,1]
      double lastX(0), lastY(0);
      for(double t = 0.; t <= 1.; t += delta){
         tmpresult = spline.evaluate(t).result();
         double nextX = result.size()*outbinsize + tshift; // drop first tshift ns
         while(tmpresult[0] > nextX){
            double dx = (nextX-lastX)/(tmpresult[0]-lastX);
            double dy = tmpresult[1]-lastY;
            double y = lastY + dx*dy;
            result.push_back(y);
            nextX = result.size()*outbinsize + tshift;
         }

         lastX = tmpresult[0];
         lastY = tmpresult[1];
      }

      for(unsigned int i = result.size(); i < nOut; i++) // Fill up waveform to correct length with copies of last value
         result.push_back(lastY);
      return result;
   }

   int Deconv( std::vector<wfholder*>* subtracted,
               std::vector<signal> &fSignals, std::set<double> &fTimes,
               std::vector<electrode> &fElectrodeIndex,
               std::vector<double> &fResponse, int theBin, bool isanode )
   {
      if(subtracted->size()==0) return 0;
      int nsamples = subtracted->back()->h->size();
      assert(nsamples < 1000);
      if( fTrace )
         std::cout<<"DeconvModule::Deconv Subtracted Size: "<<subtracted->size()
                  <<"\t# samples: "<<nsamples<<std::endl;

      double t_delay = 0.;
      if( isanode )
         t_delay = fADCdelay;
      else
         t_delay = fPWBdelay;
      if( fTrace )
         std::cout<<"DeconvModule::Deconv delay: "<<t_delay
                  <<" ns for "<<isanode<<std::endl;

      for(int b = theBin; b < int(nsamples); ++b)// b is the current bin of interest
         {
            // For each bin, order waveforms by size,
            // i.e., start working on largest first
            std::vector<wfholder*>* histset = wforder( subtracted, b );
            // std::cout<<"DeconvModule::Deconv bin of interest: "<<b
            //          <<" workable wf: "<<histset.size()<<std::endl;
            // this is useful to split deconv into the "Subtract" method
            // map ordered wf to corresponding electrode
            //std::map<int,wfholder*>* histmap = wfordermap(histset,fElectrodeIndex);
            double neTotal = 0.0;
            for (auto const it : *histset)
               {
                  unsigned int i = it->index;
                  std::vector<double>* wf=it->h;
                  electrode anElectrode = fElectrodeIndex.at( i );
                  double ne = anElectrode.gain * fScale * wf->at(b) / fResponse[theBin]; // number of "electrons"

                  if( ne >= fAvalancheSize )
                     {
                        neTotal += ne;
                        // loop over all bins for subtraction
                        Subtract(it,subtracted,b,ne,fElectrodeIndex,fResponse,theBin,isanode);

                        if(b-theBin >= 0)
                           {
                              //aresult[i][b-theBin] = 1./fAvalancheSize*ne;
                              // time in ns of the bin b centre
                              double t = ( double(b-theBin) + 0.5 ) * double(fbinsize) + t_delay;
                              fSignals.emplace_back(anElectrode,t,ne);
                              fTimes.insert(t);
                           }
                     }// if deconvolution threshold Avalanche Size
               }// loop set of ordered waveforms
               /*for (auto const it : *histset)
               {
                  delete it;
               }*/
            delete histset;
            //delete histmap;
         }// loop bin of interest


      return int(fSignals.size());
   }
   void Subtract(wfholder* hist1,
                 std::vector<wfholder*>* wfmap,
                 const int b,
                 const double ne,std::vector<electrode> &fElectrodeIndex,
                 std::vector<double> &fResponse, int theBin, bool isanode)
   {
      std::vector<double> *wf1 = hist1->h;
      int wf1size=wf1->size();
      unsigned int i1 = hist1->index;
      electrode wire1 = fElectrodeIndex[ i1 ]; // mis-name for pads

      uint AnodeSize=fAnodeFactors.size();
      uint ElectrodeSize=fElectrodeIndex.size();
      int AnodeResponseSize=(int)fAnodeResponse.size();
      int respsize=fResponse.size();
      if( isanode )
         {
            for(unsigned int k = 0; k < ElectrodeSize; ++k)
               {
                  electrode wire2 = fElectrodeIndex[ k ];
                  //check for top/bottom
                  if( wire2.sec != wire1.sec ) continue;
                  //Skip early if wires not close...
                  if (IsAnodeClose(wire1.idx,wire2.idx)>4) continue;
                  std::vector<double>* wf2=wfmap->at(k)->h;
                  for(unsigned int l = 0; l < AnodeSize; ++l)
                     {
                        //Take advantage that there are 256 anode wires... use uint8_t
                      //if( !IsNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
                        if( !IsAnodeNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;

                        for(int bb = b-theBin; bb < wf1size; ++bb)
                           {
                              // the bin corresponding to bb in the response
                              int respBin = bb-b+theBin;
                              if (respBin<0) continue;
                              if (respBin >= AnodeResponseSize) continue;

                              if(respBin < AnodeResponseSize && respBin >= 0)
                                 {
                                    // remove neighbour induction
                                  (*wf2)[bb] += ne/fScale/wire1.gain*fAnodeFactors[l]*fAnodeResponse[respBin];
                                 }
                           }// loop over all bins for subtraction
                     }// loop over factors
               }// loop all electrodes' signals looking for neighbours
         }
      for(int bb = b-theBin; bb < wf1size; ++bb)
         {
            // the bin corresponding to bb in the response
            int respBin = bb-b+theBin;
            if( respBin < respsize && respBin >= 0 )
               {
                  // Remove signal tail for waveform we're currently working on
                  (*wf1)[bb] -= ne/fScale/wire1.gain*fResponse[respBin];
               }
         }// bin loop: subtraction
   }

   void RescaleNeighbour(std::vector<double>& wf, const double& amp,
                         int& resp_bin, int& curr_bin, unsigned& aw)
   {
      //if( curr_bin < wf.size() )
      wf.at( curr_bin ) += amp/fScale*fAnodeFactors.at(aw)*fAnodeResponse.at(resp_bin);
   }

   inline uint8_t Min(uint8_t x, uint8_t y)
   {
      return (x < y)? x : y;
   }

//Take advantage that there are 256 anode wires
   inline bool IsAnodeNeighbour(int w1, int w2, int dist)
   {
      uint8_t c=w1-w2;
      return (Min(c,256-c)==dist);
   }
   inline uint8_t IsAnodeClose(int w1, int w2)
   {
      uint8_t c=w1-w2;
      return Min(c,256-c);
   }
   bool IsNeighbour(int w1, int w2, int dist)
   {
      int diff=abs(w2 - w1);
      if ( diff == dist ) return true;
      if ( diff - 256 == dist ) return true;
      if ( diff + 256 == dist ) return true;
      return false;
   }

   bool IsNeighbour(int w1, int w2)
   {
      int diff=abs(w2 - w1);
      int ansize=int(fAnodeFactors.size());
      return ( ( diff <= ansize ) ||
               ( diff - 256 <= ansize ) ||
               ( diff + 256 <= ansize ) );
   }


 
   //std::set<wfholder*,comp_hist>* wforder(std::vector<std::vector<double>*>* subtracted, const int b)
   std::vector<wfholder*>*  wforder(std::vector<wfholder*>* subtracted, const int b)
   {
      //std::set<wfholder*,comp_hist>* histset=new std::set<wfholder*,comp_hist>;
      // For each bin, order waveforms by size,
      // i.e., start working on largest first
      
      std::vector<wfholder*>* histset=new std::vector<wfholder*>;
      unsigned int size=subtracted->size();
      histset->reserve(size);
      for(unsigned int i=0; i<size;++i)
         {
            wfholder* mh=subtracted->at(i);
            mh->val = fScale*subtracted->at(i)->h->at(b);
            histset->push_back(mh);
            //histset->insert(mh);
         }
      std::sort(histset->begin(), histset->end(),comp_hist);
      return histset;
   }


   std::map<int,wfholder*>* wfordermap(std::vector<wfholder*>* histset,std::vector<electrode> &fElectrodeIndex)
   {
      std::map<int,wfholder*>* wfmap=new std::map<int,wfholder*>;
      for(unsigned int k = 0; k < fElectrodeIndex.size(); ++k)
         {
            for (auto const it : *histset)
               {
                  if( k == it->index )
                  {
                     wfmap->insert({k,it});
                     break;
                  }
               }
         }
      return wfmap;
   }

   void AWdiagnostic()
   {
      double mbot=0.,mtop=0.,rbot=0.,rtop=0.;
      for(unsigned iEl = 0; iEl<fAnodeIndex.size(); ++iEl)
         {
            if( fAnodeIndex.at(iEl).sec )
               {
                  rbot += resRMS_a.at(iEl);
                  ++mbot;
               }
            else
               {
                  rtop += resRMS_a.at(iEl);
                  ++mtop;
               }
         }
      //std::cout<<"DeconvModule::AWdiagnostic() RMS bot: "<<rbot<<" el: "<<mbot;
      if( mbot!=0.) rbot /= mbot;
      //std::cout<<" avg RMS: "<<rbot<<std::endl;
      hAvgRMSBot->Fill(rbot);
      //std::cout<<"DeconvModule::AWdiagnostic() RMS top: "<<rtop<<" el: "<<mtop<<std::endl;
      if( mtop!=0.) rtop /= mtop;
      //std::cout<<" avg RMS: "<<rtop<<std::endl;
      hAvgRMSTop->Fill(rtop);
   }

   void PADdiagnostic()
   {
      double mr=0.,r=0.;
      for(unsigned iEl = 0; iEl<fPadIndex.size(); ++iEl)
         {
            r += resRMS_p.at(iEl);
            ++mr;
         }
      if( mr != 0. ) r /= mr;
      hAvgRMSPad->Fill(r);
   }
};

class DeconvModuleFactory: public TAFactory
{
public:
   DeconvFlags fFlags;

public:
   void Help()
   {
     printf("DeconvModuleFactory::Help!\n");
     printf("\t--adcthr XXX\t\tADC Threshold\n");
     printf("\t--pwbthr XXX\t\tPWB Threshold\n");
     printf("\t--awthr XXX\t\tAW Threshold\n");
     printf("\t--padthr XXX\t\tPAD Threshold\n");
     printf("\t--recoff     Turn off reconstruction\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("DeconvModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
           Help();
         if( args[i] == "--adcthr" )
            fFlags.fADCthr = atof(args[i+1].c_str());
         if( args[i] == "--pwbthr" )
            fFlags.fPWBthr = atof(args[i+1].c_str());
         if( args[i] == "--awthr" )
            fFlags.fAWthr = atof(args[i+1].c_str());
         if( args[i] == "--padthr" )
            fFlags.fPADthr = atof(args[i+1].c_str());
         if( args[i] == "--diag" )
            fFlags.fDiag = true;
         if( args[i] == "--usetimerange" )
            {
               fFlags.fTimeCut=true;
               i++;
               fFlags.start_time=atof(args[i].c_str());
               i++;
               fFlags.stop_time=atof(args[i].c_str());
               printf("Using time range for reconstruction: ");
               printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
            }
         if( args[i] == "--useeventrange" )
            {
               fFlags.fEventRangeCut=true;
               i++;
               fFlags.start_event=atoi(args[i].c_str());
               i++;
               fFlags.stop_event=atoi(args[i].c_str());
               printf("Using event range for reconstruction: ");
               printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
            }
         if (args[i] == "--recoff")
            fFlags.fRecOff = true;
         if( args[i] == "--aged" )
            fFlags.fBatch = false;

         if( args[i] == "--pwbmap" )
            fFlags.fPWBmap = true;
      }
   }

   void Finish()
   {
      printf("DeconvModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DeconvModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DeconvModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DeconvModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
