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
#include "AnaSettings.h"

class DeconvFlags
{
public:
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

   AnaSettings* ana_settings=0;
   
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



class DeconvPADModule: public TARunObject
{
public:
   DeconvFlags* fFlags = 0;
   //bool fTrace = true;
   bool fTrace = false;
   int fCounter = 0;

private:
   // input
   std::vector<double> fAnodeFactors;

   std::vector<double> fPadResponse;

   std::vector<double> fPwbRescale;

   // control
   bool diagnostics; // dis/en-able histogramming
   bool display;     // dis/en-able wf storage for aged

   int fPADbinsize;

   double fPWBdelay;

   double fPWBmax;
   double fPWBrange;

   int nAWsamples;
   int pedestal_length;
   double fScale;

   int thePadBin;

   double fPWBThres;

   double fAvalancheSize;
   double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<electrode> fPadIndex;

   // check
   std::vector<double> resRMS_p;

   // to use in aged display
   std::vector<wf_ref>* feamwaveforms;
   std::vector<wf_ref>* deconvwaveforms;

   // waveform max
   std::vector<signal> fPwbPeaks;
   std::vector<signal> fPwbRange;


   TH2D* hPWBped;
   TProfile* hPWBped_prox;

   // pads
   TH1D* hAvgRMSPad;

   TH2D* hPadOverflow;

   // pwb map
   std::ofstream pwbmap;

   // pad mask
   std::vector<int> fPadSecMask;
   std::vector<int> fPadRowMask;

   padmap* pmap;

public:

   DeconvPADModule(TARunInfo* runinfo, DeconvFlags* f)
      : TARunObject(runinfo),
        fPWBdelay(0.), // to be guessed
        pedestal_length(100),fScale(-1.), // values fixed by DAQ
        isalpha16(false)
   {
      if (fTrace)
         printf("DeconvPADModule::ctor!\n");

      fFlags = f;


      fPADbinsize=int(_timebin);

      thePadBin=6;

      fPWBmax = pow(2.,12.);
      fPWBrange = fPWBmax*0.5-1.;

      assert( fFlags->ana_settings );
      fPWBThres=fFlags->ana_settings->GetDouble("DeconvModule","PWBthr");
      fPWBpeak=fFlags->ana_settings->GetDouble("DeconvModule","PADthr");

      fPadSecMask.reserve(32);
      fPadRowMask.reserve(576);

      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      display=!fFlags->fBatch;   // dis/en-able wf storage for aged
   }

   ~DeconvPADModule()
   {
      if (fTrace)
         printf("DeconvPADModule::dtor!\n");
      //if(ct) delete ct;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      if( diagnostics )
         {
            std::cout<<"DeconvPADModule::BeginRun with Diagnostics"<<std::endl;
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            // pads histograms
            gDirectory->mkdir("paddeconv")->cd();

            hAvgRMSPad = new TH1D("hAvgRMSPad","Average Deconv Remainder Pad",500,0.,5000.);

            hPWBped = new TH2D("hPWBped","PWB pedestal per Pad",32*576,0.,_padcol*_padrow,
                               1000,-4096.,4096.);
            hPWBped_prox = new TProfile("hPWBped_prox","Average PWB pedestal per Pad;Pad;PWB",
                                        32*576,0.,_padcol*_padrow,-4096.,4096.);

            hPadOverflow = new TH2D("hPadOverflow","Distribution of Overflow Pads;row;sec;N",
                                    576,0.,_padrow,32,0.,_padcol);
         }

      // by run settings
      int run_number = runinfo->fRunNo;
      if( run_number < 2724 ) // new FMC-32
         {
            isalpha16=true;
         }

      if( run_number == 2246 || run_number == 2247 || run_number == 2248 || run_number == 2249 || run_number == 2251 )
         fPWBdelay = -50.;
      else if( run_number == 2272 || run_number ==  2273 || run_number == 2274 )
         fPWBdelay = 136.;
      else if( run_number >= 2282 && run_number < 2724 )
         {
            fPWBdelay = 0.;
         }
      else if( run_number >= 2724 && run_number < 3032 ) // new FMC-32
         {
            fPWBdelay = 0.;
         }
      else if( run_number >= 3032 )
         {
            fPWBdelay = 0.;
         }
      if( run_number == 3169 || run_number == 3209 || run_number == 3226 || run_number == 3241 ||
          run_number == 3249 || run_number == 3250 || run_number == 3251 ||
          run_number == 3253 || run_number == 3254 || run_number == 3255 ||
          run_number == 3260 || run_number == 3263 || run_number == 3265 ||
          run_number == 3875 || run_number == 3866 || run_number == 3859 || run_number == 3855) // TrigBscMult
         {
            fPWBdelay = -100.;
         }
      if( run_number == 3170 || run_number == 3195 || run_number == 3190 ||
          run_number == 3187 || run_number == 3186 || run_number == 3184 ||
          run_number == 3181 || run_number == 3178 || run_number == 3208 ||
          run_number == 3210 || run_number == 3245 ||
          run_number == 3874 || run_number == 3858 ) // TrigCoinc
         {
            fPWBdelay = 0.;
         }

      // electrodes masking
      if( run_number == 2635 )
         {
            // only top aw are connected
            // numbering [256,511]
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
      else if( run_number == 3873 || run_number == 3864 )
         {
            fPadSecMask.push_back(21);
            fPadSecMask.push_back(23);
            fPadSecMask.push_back(24);
            fPadSecMask.push_back(27);
            fPadRowMask.push_back(190);
            fPadRowMask.push_back(194);
            fPadRowMask.push_back(385);
            fPadRowMask.push_back(386);
            fPadRowMask.push_back(503);
            fPadRowMask.push_back(504);
            fPadRowMask.push_back(554);
         }


      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Deconv Settings"<<std::endl;
      std::cout<<" PWB max: "<<fPWBmax<<std::endl;
      std::cout<<" PWB range: "<<fPWBrange<<std::endl;
      std::cout<<" PWB time bin: "<<fPADbinsize<<" ns"<<std::endl;
      std::cout<<" PWB delay: "<<fPWBdelay<<std::endl;
      std::cout<<" PWB thresh: "<<fPWBThres<<std::endl;
      std::cout<<" PAD thresh: "<<fPWBpeak<<std::endl;
      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Masked Electrodes"<<std::endl;
      std::cout<<"PAD: ";
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
         for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
            std::cout<<"["<<*it<<","<<*jt<<"],";
      std::cout<<"\n"<<std::endl;

      if( fFlags->fPWBmap )
         {
            std::string mapname="pwbR";
            mapname += std::to_string(run_number);
            mapname += ".map";
            pwbmap.open(mapname.c_str());
            pwbmap<<"sec\trow\tsca\tsca_ro\tsca_ch\tx\ty\tcol\tring\tpwb\n";
         }

      int s = ReadPADResponseFile(fPADbinsize);
      std::cout<<"Deconv PAD Module BeginRun Response status: "<<s<<std::endl;
      assert(s>0);

      pmap = new padmap;
   }

   void EndRun(TARunInfo* runinfo)
   {
      
      printf("DeconvPADModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      if( fFlags->fPWBmap ) pwbmap.close();
      delete pmap;  
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
         printf("DeconvPADModule::Analyze, run %d, counter %d\n",
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

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start(clock());
      #endif   
      
      const FeamEvent* pwb = e->feam;
      if( !pwb ) // allow for events without pwbs
         {
            std::cout<<"DeconvPADModule::AnalyzeFlowEvent(...) No FeamEvent in AgEvent # "
                     <<e->counter<<std::endl;
            //return flow;
         }
      else
         {
             AgSignalsFlow* flow_sig= flow->Find<AgSignalsFlow>();
             std::vector<signal>* spad=FindPadTimes(pwb);
             flow_sig->AddPadSignals(spad);

             if( diagnostics )
             {
                flow_sig->pwbMax = fPwbPeaks;
                flow_sig->pwbRange = fPwbRange;
             }
             if( display )
            {
               flow_sig->AddPADWaveforms(feamwaveforms);
               flow_sig->AddPADDeconvWaveforms(deconvwaveforms);
            }
         }
      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"deconv_pad_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }


   std::vector<signal>* FindPadTimes(const FeamEvent* padSignals)
   {

      auto& channels = padSignals->hits; // vector<FeamChannel*>
      if( fTrace )
         std::cout<<"DeconvPADModule::FindPadTimes Channels Size: "<<channels.size()<<std::endl;

      // prepare vector with wf to manipulate
      std::vector<wfholder*> PadWaves;
      PadWaves.reserve( channels.size() );

      // clear/initialize "output" vectors
      fPadIndex.clear();
      fPadIndex.reserve( channels.size() );
      if( display )
         {
            feamwaveforms= new std::vector<wf_ref>;
            feamwaveforms->reserve(channels.size());
            deconvwaveforms = new std::vector<wf_ref>;
            deconvwaveforms->reserve(channels.size());
         }

      // find intresting channels
      int index=0; //wfholder index
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // FeamChannel*
            if( !PwbPadMap::chan_is_pad(ch->sca_chan) ) continue;

            short col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
            col+=1;
            if( col == 32 ) col = 0;
            assert(col<32&&col>=0);
            // std::cout<<"DeconvPADModule::FindPadTimes() col: "<<col<<std::endl;
            int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
            // std::cout<<"DeconvPADModule::FindPadTimes() row: "<<row<<std::endl;
            assert(row<576&&row>=0);
            int pad_index = pmap->index(col,row);
            assert(!isnan(pad_index));
            // CREATE electrode
            electrode el(col,row);
            //el.setgain( fPwbRescale.at(pad_index) );

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
                  std::cerr<<"DeconvPADModule::FindPadTimes ERROR wf samples: "
                           <<ch->adc_samples.size()<<std::endl;
                  continue;
               }

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += ch->adc_samples.at( b );
            ped /= pedestal_length;
            // CALCULATE PEAK HEIGHT
            auto minit = std::min_element(ch->adc_samples.begin(), ch->adc_samples.end());
            //double max = el.gain * fScale * ( double(*minit) - ped );
            //double max = fPwbRescale.at(pad_index) * fScale * ( double(*minit) - ped );
            double amp = fScale * double(*minit), max;
            if( amp < fPWBrange )
               max = el.gain * fScale * ( double(*minit) - ped );
            else
               {
                  max = fPWBmax;
                  if( diagnostics ) hPadOverflow->Fill(double(row),double(col));
               }
            if( fTrace )
               std::cout<<"DeconvPADModule::FindPadTimes ("<<row<<","<<col<<") ped: "<<ped
                        <<" minit: "<<double(*minit)<<" amp: "<<amp<<" max: "<<max<<std::endl;

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
                  //double min = fPwbRescale.at(pad_index) * fScale * ( double(*maxit) - ped );
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

                  // NORMALIZE WF
                  //double norm = fPwbRescale.at(el.idx);
                  //std::for_each(waveform->h->begin(), waveform->h->end(), [norm](double& v) { v*=norm;});

                  // fill vector with wf to manipulate
                  PadWaves.emplace_back( waveform );
                  
                  // STORE electrode
                  fPadIndex.push_back( el );
                  
                  if( fTrace && 0 )
                     std::cout<<"DeconvPADModule::FindPadTimes() pwb"<<ch->imodule
                              <<" col: "<<col
                              <<" row: "<<row
                              <<" ph: "<<max<<std::endl;
                  
                  // make me a map of pads -> pwbs
                  if( fFlags->fPWBmap )
                     pwbmap<<col<<"\t"<<row<<"\t" // pad
                           <<ch->sca<<"\t"<<ch->sca_readout<<"\t"<<ch->sca_chan<<"\t" // sca 
                           <<ch->pad_col<<"\t"<<ch->pad_row<<"\t" // local pad
                           <<ch->pwb_column<<"\t"<<ch->pwb_ring<<"\t"<<ch->imodule  // pwb S/N
                           <<std::endl;

                  if( display )
                     feamwaveforms->emplace_back(el,new std::vector<double>(*waveform->h));
               }// max > thres
         }// channels


      // DECONVOLUTION
      std::vector<signal>* spad = DeconvPAD(&PadWaves,fPadIndex,fPadResponse,thePadBin,false);
      int nsig=-1;
      if (!spad)
         nsig=0;
      else
         nsig= spad->size();
      std::cout<<"DeconvPADModule::FindPadTimes "<<nsig<<" found"<<std::endl;
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
      return spad;
   }

   int ReadPADResponseFile( const double padbin)
   {
      std::string filename(getenv("AGRELEASE"));
      filename+="/ana/padResponse_deconv.dat";
      std::cout << "DeconvPADModule:: Reading in response file (pad) " << filename << std::endl;
      std::ifstream respFile(filename.c_str());
      if( !respFile.good() ) return 0;
      double binedge, val;
      std::vector<double> resp;
      while(1)
         {
            respFile >> binedge >> val;
            if( !respFile.good() ) break;
            resp.push_back(val);
         }
      respFile.close();
      fPadResponse=Rebin(resp, padbin);

      double frac = 0.1;
      double max = *std::max_element(fPadResponse.begin(), fPadResponse.end());
      double thres = frac*max;
      for(unsigned b=0; b<fPadResponse.size(); ++b)
         {
            if( fPadResponse.at(b) > thres )
               thePadBin=b;
         }
      if( fTrace )
         std::cout<<"DeconvPADModule::ReadResponseFile pad max: "<<max<<"\tpad bin: "<<thePadBin<<std::endl;

      return fPadResponse.size();
   }

   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.)
   {
      if( fTrace )
         std::cout<<"DeconvPADModule::Rebin "<<binsize<<std::endl;
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

   std::vector<signal>* DeconvPAD( std::vector<wfholder*>* subtracted,
               std::vector<electrode> &fElectrodeIndex,
               std::vector<double> &fResponse, int theBin, bool isanode )
   {
      if(subtracted->size()==0) return 0;
      int nsamples = subtracted->back()->h->size();
      std::vector<signal>* fSignals=new std::vector<signal>;
      fSignals->reserve(nsamples-theBin);
      assert(nsamples < 1000);
      if( fTrace )
         std::cout<<"DeconvPADModule::Deconv Subtracted Size: "<<subtracted->size()
                  <<"\t# samples: "<<nsamples<<std::endl;

      double t_delay = fPWBdelay;
      int fbinsize = fPADbinsize;
      double fAvalancheSize =fPWBpeak;

      if( fTrace )
         std::cout<<"DeconvPADModule::DeconvPAD delay: "<<t_delay
                  <<" ns"<<std::endl;

      for(int b = theBin; b < int(nsamples); ++b)// b is the current bin of interest
         {
            // For each bin, order waveforms by size,
            // i.e., start working on largest first
            std::vector<wfholder*>* histset = wforder( subtracted, b );
            // std::cout<<"DeconvPADModule::Deconv bin of interest: "<<b
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
                        SubtractPAD(it,subtracted,b,ne,fElectrodeIndex,fResponse,theBin);
                        if(b-theBin >= 0)
                           {
                              //aresult[i][b-theBin] = 1./fAvalancheSize*ne;
                              // time in ns of the bin b centre
                              double t = ( double(b-theBin) + 0.5 ) * double(fbinsize) + t_delay;
                              fSignals->emplace_back(anElectrode,t,ne);
                           }
                        if( display )
                           deconvwaveforms->emplace_back(anElectrode,new std::vector<double>(*wf));
                     }// if deconvolution threshold Avalanche Size
               }// loop set of ordered waveforms
            /*for (auto const it : *histset)
              {
              delete it;
              }*/
            delete histset;
            //delete histmap;
         }// loop bin of interest


      return fSignals;
   }
   void SubtractPAD(wfholder* hist1,
                 std::vector<wfholder*>* wfmap,
                 const int b,
                 const double ne,std::vector<electrode> &fElectrodeIndex,
                 std::vector<double> &fResponse, int theBin)
   {
      std::vector<double> *wf1 = hist1->h;
      int wf1size=wf1->size();
      unsigned int i1 = hist1->index;
      electrode wire1 = fElectrodeIndex.at( i1 ); // mis-name for pads

      int respsize=fResponse.size();
     
     
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

class DeconvPADModuleFactory: public TAFactory
{
public:
   DeconvFlags fFlags;

public:
   void Help()
   {
      printf("DeconvPADModuleFactory::Help!\n");
      printf("\t--recoff     Turn off reconstruction\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("DeconvPADModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
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

         if( args[i] == "--anasettings" ) json=args[i+1];
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("DeconvPADModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DeconvPADModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DeconvPADModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DeconvPADModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
