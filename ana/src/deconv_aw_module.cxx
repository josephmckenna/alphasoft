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



class DeconvAWModule: public TARunObject
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

   std::vector<double> fAdcRescale;

   // control
   bool diagnostics; // dis/en-able histogramming
   bool display;     // dis/en-able wf storage for aged

   int fAWbinsize;

   double fADCdelay;

   double fADCmax;
   double fADCrange;

   int nAWsamples;
   int pedestal_length;
   double fScale;

   int theAnodeBin;

   double fADCThres;

   double fAvalancheSize;
   double fADCpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<electrode>* fAnodeIndex;

   std::vector<signal>* sanode;

   std::set<double>* aTimes;

   // check
   std::vector<double> resRMS_a;

   // to use in aged display
   std::vector<wf_ref> wirewaveforms;

   // waveform max
   std::vector<signal> fAdcPeaks;
   std::vector<signal> fAdcRange;

   // anodes
   TH1D* hAvgRMSBot;
   TH1D* hAvgRMSTop;

   TH2D* hADCped;
   TProfile* hADCped_prox;



   // anode mask
   std::vector<int> fAwMask;


public:

   DeconvAWModule(TARunInfo* runinfo, DeconvFlags* f)
      : TARunObject(runinfo),
        fADCdelay(0.), // to be guessed
        nAWsamples(335),// maximum value that works for mixed ADC, after pedestal
        pedestal_length(100),fScale(-1.), // values fixed by DAQ
        isalpha16(false)
   {
      if (fTrace)
         printf("DeconvAWModule::ctor!\n");

      fFlags = f;

      fAnodeFactors = {
         0.1275,        // neighbour factor
         0.0365,        // 2nd neighbour factor
         0.012,         // 3rd neighbour factor
         0.0042         // 4th neighbour factor
      };

      fAWbinsize=int(_timebin);

      theAnodeBin=1;

      fADCmax = pow(2.,14.);
      fADCrange = fADCmax*0.5-1.;

      assert( fFlags->ana_settings );
      fADCThres=fFlags->ana_settings->GetDouble("DeconvModule","ADCthr");
      fADCpeak=fFlags->ana_settings->GetDouble("DeconvModule","AWthr");

      fAwMask.reserve(256);
      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      display=!fFlags->fBatch;   // dis/en-able wf storage for aged
   }

   ~DeconvAWModule()
   {
      if (fTrace)
         printf("DeconvAWModule::dtor!\n");
      //if(ct) delete ct;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      if( diagnostics )
         {
            std::cout<<"DeconvAWModule::BeginRun with Diagnostics"<<std::endl;
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
            // anodes histograms
            gDirectory->mkdir("awdeconv")->cd();

            hAvgRMSBot = new TH1D("hAvgRMSBot","Average Deconv Remainder Bottom",1000,0.,50000.);
            hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",1000,0.,50000.);

            hADCped = new TH2D("hADCped","ADC pedestal per AW",256,0.,256.,1600,-16384.,16384.);
            hADCped_prox = new TProfile("hADCped_prox","Average ADC pedestal per AW;AW;ADC",
                                        256,0.,256.,16384.,16384.);

         }

      // by run settings
      int run_number = runinfo->fRunNo;
      if( run_number < 2724 ) // new FMC-32
         {
            fAWbinsize=10;
            isalpha16=true;
         }

      else if( run_number >= 2282 && run_number < 2724 )
         {
            fADCdelay = -120.;
         }
      else if( run_number >= 2724 && run_number < 3032 ) // new FMC-32
         {
            fADCdelay = 0.;
         }
      else if( run_number >= 3032 )
         {
            fADCdelay = -250.;
         }
      if( run_number == 3169 || run_number == 3209 || run_number == 3226 || run_number == 3241 ||
          run_number == 3249 || run_number == 3250 || run_number == 3251 ||
          run_number == 3253 || run_number == 3254 || run_number == 3255 ||
          run_number == 3260 || run_number == 3263 || run_number == 3265 ||
          run_number == 3875 || run_number == 3866 || run_number == 3859 || run_number == 3855) // TrigBscMult
         {
            fADCdelay = -400.;
         }
      if( run_number == 3170 || run_number == 3195 || run_number == 3190 ||
          run_number == 3187 || run_number == 3186 || run_number == 3184 ||
          run_number == 3181 || run_number == 3178 || run_number == 3208 ||
          run_number == 3210 || run_number == 3245 ||
          run_number == 3874 || run_number == 3858 ) // TrigCoinc
         {
            fADCdelay = 0.;
         }

      // electrodes masking
      if( run_number == 2635 )
         {
            // only top aw are connected
            // numbering [256,511]
            fAwMask.push_back(3+256);
         }
      // if( run_number >= 3003 )
      //    fAwMask.push_back(142+256);


      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Deconv AW Settings"<<std::endl;
      std::cout<<" ADC max: "<<fADCmax<<std::endl;
      std::cout<<" ADC range: "<<fADCrange<<std::endl;
      std::cout<<" ADC time bin: "<<fAWbinsize<<" ns"<<std::endl;
      std::cout<<" ADC delay: "<<fADCdelay<<std::endl;
      std::cout<<" ADC thresh: "<<fADCThres<<std::endl;
      std::cout<<" AW thresh: "<<fADCpeak<<std::endl;
      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Masked Electrodes"<<std::endl;
      std::cout<<"AW: ";
      for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
         std::cout<<*it-256<<", ";
      std::cout<<"\n"<<std::endl;
      int s = ReadAWResponseFile(fAWbinsize);
      std::cout<<"DeconvAWModule BeginRun Response status: "<<s<<std::endl;
      assert(s>0);

      // std::string basepath(getenv("AGRELEASE"));
      // std::ifstream fadcres(basepath+"/ana/AdcRescale.dat");
      // double rescale_factor;
      // while(1)
      //    {
      //       fadcres>>rescale_factor;
      //       if( !fadcres.good() ) break;
      //       fAdcRescale.push_back(rescale_factor);
      //       //fAdcRescale.push_back(1.);
      //    }
      // fadcres.close();
      // //      fAdcRescale.assign(256,1.0);
      // if( fAdcRescale.size() == 256 )
      //    std::cout<<"DeconvAWModule BeginRun ADC rescaling factors OK"<<std::endl;
      // else
      //    std::cout<<"DeconvAWModule BeginRun ADC rescaling factors NOT ok (size: "
      //             <<fAdcRescale.size()<<")"<<std::endl;

      // std::ifstream fpwbres(basepath+"/ana/PwbRescale.dat");
      // while(1)
      //    {
      //       fpwbres>>rescale_factor;
      //       if( !fpwbres.good() ) break;
      //       fPwbRescale.push_back(rescale_factor);
      //       //fPwbRescale.push_back(1.);
      //    }
      // fpwbres.close();
      // // fPwbRescale.assign(32*576,1.0);
      // if( fPwbRescale.size() == 32*576 )
      //    std::cout<<"DeconvAWModule BeginRun PWB rescaling factors OK"<<std::endl;
      // else
      //    std::cout<<"DeconvAWModule BeginRun PWB rescaling factors NOT ok (size: "
      //             <<fPwbRescale.size()<<")"<<std::endl;

   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DeconvAWModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
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
         printf("DeconvAWModule::Analyze, run %d, counter %d\n",
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

      const Alpha16Event* aw = e->a16;
      if( !aw )
         {
            std::cout<<"DeconvAWModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            #ifdef _TIME_ANALYSIS_
               if (TimeModules) flow=new AgAnalysisReportFlow(flow,"deconv_aw_module (No Alpha16Event)",timer_start);
            #endif
            return flow;
         }
      else
         FindAnodeTimes( aw );

      if( diagnostics ) AWdiagnostic();


      //      AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, sanode, spad);
      AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, sanode);

      if( diagnostics )
         {
            flow_sig->adc32max = fAdcPeaks;
            flow_sig->adc32range = fAdcRange;
         }

      if( display )
         {
            flow_sig->AddAWWaveforms(wirewaveforms);
         }

      flow = flow_sig;
      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"deconv_aw_module",timer_start);
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
      auto& channels = anodeSignals->hits; // vector<Alpha16Channel*>
      if( fTrace )
         std::cout<<"DeconvAWModule::FindAnodeTimes Channels Size: "<<channels.size()<<std::endl;

      // prepare vector with wf to manipulate
      std::vector<wfholder*> AnodeWaves;
      AnodeWaves.reserve( channels.size() );

      // clear/initialize "output" vectors
      //      std::cout<<"DeconvAWModule::FindAnodeTimes clear/initialize \"output\" vectors"<<std::endl;

      fAnodeIndex=new std::vector<electrode>;
      fAnodeIndex->reserve( channels.size() );

      aTimes=new std::set<double>;

      if( display )
         {
            wirewaveforms.clear();
            wirewaveforms.reserve(channels.size());
         }

      if( diagnostics )
         {
            fAdcPeaks.clear();
            fAdcPeaks.reserve(channels.size());
            fAdcRange.clear();
            fAdcRange.reserve(channels.size());
         }
      
      // find intresting channels
      int index=0; //wfholder index
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan < 16 && !isalpha16 ) continue; // it's bv

            int aw_number = ch->tpc_wire;
            // std::cout<<"DeconvAWModule::FindAnodeTimes anode wire: "<<aw_number<<std::endl;
            if( aw_number < 0 || aw_number >= 512 ) continue;
            // CREATE electrode
            electrode el(aw_number);
            //el.setgain( fAdcRescale.at(el.idx) ); // this checks that gain > 0
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

            // std::cout<<"DeconvAWModule::FindAnodeTimes aw: "<<aw_number
            //          <<" i: "<<i
            //          <<" # adc samples: "<<ch->adc_samples.size()<<std::endl;

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += double(ch->adc_samples.at( b ));
            ped /= double(pedestal_length);
            if( fTrace )
               std::cout<<"DeconvAWModule::FindAnodeTimes pedestal for anode wire: "<<el.idx
                        <<" is "<<ped<<std::endl;
            // CALCULATE PEAK HEIGHT
            auto minit = std::min_element(ch->adc_samples.begin(), ch->adc_samples.end());
            //double max = el.gain * fScale * ( double(*minit) - ped );
            //double max =  fAdcRescale.at(el.idx) * fScale * ( double(*minit) - ped );
            double amp = fScale * double(*minit), max;
            if( amp < fADCrange )
               max = el.gain * fScale * ( double(*minit) - ped );
            else
               max = fADCmax;
            
            if( fTrace )
               std::cout<<"DeconvAWModule::FindAnodeTimes aw: "<<aw_number<<" ped: "<<ped
                        <<" minit: "<<double(*minit)<<" amp: "<<amp<<" max: "<<max<<std::endl;
            
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
                  //double min = fAdcRescale.at(el.idx) * fScale * ( double(*maxit) - ped );
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

                  // NORMALIZE WF
                  //double norm = fAdcRescale.at(el.idx);
                  //std::for_each(waveform->h->begin(), waveform->h->end(), [norm](double& v) { v*=norm;});

                  // fill vector with wf to manipulate
                  AnodeWaves.emplace_back( waveform );

                  // STORE electrode
                  fAnodeIndex->push_back( el );

                  if( display )
                     wirewaveforms.emplace_back(el,waveform->h);
               }// max > thres
         }// channels


      // DECONVOLUTION
      //
      sanode= DeconvAW(&AnodeWaves,aTimes,fAnodeIndex,fAnodeResponse,theAnodeBin);
      int nsig=-1;
      if (!sanode)
         nsig=0;
      else
         nsig= sanode->size();
      std::cout<<"DeconvAWModule::FindAnodeTimes "<<nsig<<" found"<<std::endl;
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


   int ReadAWResponseFile(const double awbin)
   {
      std::string filename(getenv("AGRELEASE"));
      filename+="/ana/anodeResponse.dat";
      std::cout << "DeconvAWModule:: Reading in response file (anode) " << filename << std::endl;
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
         std::cout<<"DeconvAWModule::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theAnodeBin<<std::endl;

    
      return fAnodeResponse.size();
   }

   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.)
   {
      if( fTrace )
         std::cout<<"DeconvAWModule::Rebin "<<binsize<<std::endl;
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

   std::vector<signal>* DeconvAW( std::vector<wfholder*>* subtracted,
               std::set<double>* fTimes,
               std::vector<electrode>* fElectrodeIndex,
               std::vector<double> &fResponse, int theBin)
   {

      if(subtracted->size()==0) return 0;
      int nsamples = subtracted->back()->h->size();
      std::vector<signal>* fSignals=new std::vector<signal>;
      fSignals->reserve(nsamples*subtracted->size());
      assert(nsamples < 1000);
      if( fTrace )
         std::cout<<"DeconvAWModule::Deconv Subtracted Size: "<<subtracted->size()
                  <<"\t# samples: "<<nsamples<<std::endl;

      double t_delay = 0.;
      double fAvalancheSize =0.;
      int fbinsize = 0.;
      
      {
         t_delay = fADCdelay;
         fbinsize = fAWbinsize;
         fAvalancheSize = fADCpeak;
      }

      if( fTrace )
         std::cout<<"DeconvAWModule::DeconvAW delay: "<<t_delay<<" ns"<<std::endl;

      for(int b = theBin; b < int(nsamples); ++b)// b is the current bin of interest
         {
            // For each bin, order waveforms by size,
            // i.e., start working on largest first
            std::vector<wfholder*>* histset = wforder( subtracted, b );
            // std::cout<<"DeconvAWModule::Deconv bin of interest: "<<b
            //          <<" workable wf: "<<histset.size()<<std::endl;
            // this is useful to split deconv into the "Subtract" method
            // map ordered wf to corresponding electrode
            //std::map<int,wfholder*>* histmap = wfordermap(histset,fElectrodeIndex);
            double neTotal = 0.0;
            for (auto const it : *histset)
               {
                  unsigned int i = it->index;
                  std::vector<double>* wf=it->h;
                  electrode anElectrode = fElectrodeIndex->at( i );
                  double ne = anElectrode.gain * fScale * wf->at(b) / fResponse[theBin]; // number of "electrons"


                  if( ne >= fAvalancheSize )
                     {
                        neTotal += ne;
                        // loop over all bins for subtraction
                        SubtractAW(it,subtracted,b,ne,fElectrodeIndex,fResponse,theBin);

                        if(b-theBin >= 0)
                           {
                              //aresult[i][b-theBin] = 1./fAvalancheSize*ne;
                              // time in ns of the bin b centre
                              double t = ( double(b-theBin) + 0.5 ) * double(fbinsize) + t_delay;
                              fSignals->emplace_back(anElectrode,t,ne);
                              fTimes->insert(t);
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


      return fSignals;
   }
   void SubtractAW(wfholder* hist1,
                 std::vector<wfholder*>* wfmap,
                 const int b,
                 const double ne,std::vector<electrode>* fElectrodeIndex,
                 std::vector<double> &fResponse, int theBin)
   {
      std::vector<double> *wf1 = hist1->h;
      int wf1size=wf1->size();
      unsigned int i1 = hist1->index;
      electrode wire1 = fElectrodeIndex->at( i1 ); // mis-name for pads

      uint AnodeSize=fAnodeFactors.size();
      uint ElectrodeSize=fElectrodeIndex->size();
      int AnodeResponseSize=(int)fAnodeResponse.size();
      int respsize=fResponse.size();
      for(unsigned int k = 0; k < ElectrodeSize; ++k)
           {
               electrode wire2 = fElectrodeIndex->at( k );
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
      for(unsigned iEl = 0; iEl<fAnodeIndex->size(); ++iEl)
         {
            if( fAnodeIndex->at(iEl).sec )
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
      //std::cout<<"DeconvAWModule::AWdiagnostic() RMS bot: "<<rbot<<" el: "<<mbot;
      if( mbot!=0.) rbot /= mbot;
      //std::cout<<" avg RMS: "<<rbot<<std::endl;
      hAvgRMSBot->Fill(rbot);
      //std::cout<<"DeconvAWModule::AWdiagnostic() RMS top: "<<rtop<<" el: "<<mtop<<std::endl;
      if( mtop!=0.) rtop /= mtop;
      //std::cout<<" avg RMS: "<<rtop<<std::endl;
      hAvgRMSTop->Fill(rtop);
   }

};

class DeconvAWModuleFactory: public TAFactory
{
public:
   DeconvFlags fFlags;

public:
   void Help()
   {
      printf("DeconvAWModuleFactory::Help!\n");
      printf("\t--recoff     Turn off reconstruction\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {    
      TString json="default";
      printf("DeconvAWModuleFactory::Init!\n");

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
      printf("DeconvAWModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DeconvAWModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DeconvAWModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DeconvAWModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
