#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <algorithm>
#include <future>

#include "TCanvas.h"
#include "TH1.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraphPolar.h"
#include "TMath.h"

#include "SignalsType.h"
#include "tinyspline.h"


class DeconvFlags
{
public:
   double fADCthr=1000.;
   double fPWBthr=100.;
   double fAWthr=120.;
   double fPADthr=100.;

public:
   DeconvFlags() // ctor
   { }

   ~DeconvFlags() // dtor
   { }
};

class DeconvModule: public TARunObject
{
public:
   DeconvFlags* fFlags = NULL;
   //   bool fTrace = true;
   bool fTrace = false;
   //   bool do_plot = false;
   int fCounter = 0;

private:
   // input
   std::vector<double> fAnodeFactors;
   std::vector<double> fResponse;
   std::vector<double> fAnodeResponse;
   std::vector<double> fPadResponse;

   // control
   bool isanode;
   int fbinsize;
   int fAWbinsize;
   int fPADbinsize;

   double fADCdelay;
   double fPWBdelay;

   int nAWsamples;
   int pedestal_length;
   double fScale;

   int theBin;
   int theAnodeBin;
   int thePadBin;

   double fThres;
   double fADCThres;
   double fPWBThres;

   double fAvalancheSize;
   double fADCpeak;
   double fPWBpeak;

   // output
   //   std::vector<electrode> aresIndex;
   std::vector<electrode> fElectrodeIndex;
   std::vector<electrode> fAnodeIndex;
   std::vector<electrode> fPadIndex;
   std::vector<signal> fSignals;
   std::vector<signal> sanode;
   std::vector<signal> spad;
   // //   std::vector<std::vector<double> > aresult;
   // std::vector<std::vector<double> > fResult;
   std::set<double> fTimes;
   std::set<double> aTimes;
   std::set<double> pTimes;

   // check
   std::vector<double> resRMS_a;
   std::vector<double> resRMS_p;

   // to use in aged display
   std::vector<wf_ref> wirewaveforms;
   std::vector<wf_ref> feamwaveforms;

   // // plots
   // TCanvas* ct;
 
   // anodes  
   TH1D* hAvgRMSBot;
   TH1D* hAvgRMSTop;

   // pads
   TH1D* hAvgRMSPad;

   // // pwb map
   // std::ofstream pwbmap;

public:

   DeconvModule(TARunInfo* runinfo, DeconvFlags* f)
      : TARunObject(runinfo),
        fbinsize(1),
        fADCdelay(0.),fPWBdelay(0.), // to be guessed
        nAWsamples(335),// maximum value that works for mixed ADC, after pedestal
        pedestal_length(100),fScale(-1.), // values fixed by DAQ
        theBin(-1),// initialization value
        fThres(0.),fAvalancheSize(0.) // to be set later
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

      isanode=true;

      //fAWbinsize=10;
      fAWbinsize=int(_timebin);

      fPADbinsize=16;
      //fPADbinsize=_timebin;

      theAnodeBin=1;
      thePadBin=6;

      fADCThres=f->fADCthr;
      fPWBThres=f->fPWBthr;
      fADCpeak=f->fAWthr;
      fPWBpeak=f->fPADthr;
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

      //do_plot = (runinfo->fRoot->fgApp != NULL);
      //if(do_plot) ct = new TCanvas("ct","deconv",1600,1600);

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // anodes histograms
      gDirectory->mkdir("awdeconv")->cd();
      
      hAvgRMSBot = new TH1D("hAvgRMSBot","Average Deconv Remainder Bottom",500,0.,10000.);
      hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",500,0.,10000.);

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // pads histograms
      gDirectory->mkdir("paddeconv")->cd();

      hAvgRMSPad = new TH1D("hAvgRMSPad","Average Deconv Remainder Pad",500,0.,10000.);

      int s = ReadResponseFile(fAWbinsize,fPADbinsize);
      if( fTrace )
         std::cout<<"Response status: "<<s<<std::endl;

      int run_number = runinfo->fRunNo;
      if( run_number == 2246 || run_number == 2247 || run_number == 2248 || run_number == 2249 || run_number == 2251 )
         fPWBdelay = -50.;
      else if( run_number == 2272 || run_number ==  2273 || run_number == 2274 )
         fPWBdelay = 136.;
     else if ( run_number == 2282 || run_number == 2284 || run_number == 2285 ||
               run_number > 2300 )
        {
           fADCdelay = -120.;
           fPWBdelay = 0.;
        }

      std::cout<<"-------------------------"<<std::endl;
      std::cout<<"Deconv Settings"<<std::endl;
      std::cout<<" ADC delay: "<<fADCdelay<<"\tPWB delay: "<<fPWBdelay<<std::endl;
      std::cout<<" ADC thresh: "<<fADCThres<<"\tPWB thresh: "<<fPWBThres<<std::endl;
      std::cout<<" AW thresh: "<<fADCpeak<<"\tPAD thresh: "<<fPWBpeak<<std::endl;
      std::cout<<"-------------------------"<<std::endl;     
      // pwbmap.open("pwb.map");      
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DeconvModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      // pwbmap.close();
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
      printf("DeconvModule::Analyze, run %d, counter %d\n", 
             runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      const AgEvent* e = ef->fEvent;

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
      if( !pwb ) 
         {
            std::cout<<"DeconvModule::AnalyzeFlowEvent(...) No FeamEvent in AgEvent # "
                     <<e->counter<<std::endl;
            return flow;
         }
      else
         stat_pwb = std::async( &DeconvModule::FindPadTimes, this, pwb );

      int stat = stat_aw.get();
      if( stat ) AWdiagnostic();
      stat = stat_pwb.get();
      if( stat ) PADdiagnostic();


      AgSignalsFlow* flow_sig = new AgSignalsFlow(flow, sanode, spad, 
                                                  wirewaveforms, feamwaveforms);
      flow = flow_sig;
      ++fCounter;
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
      theBin = theAnodeBin;
      fThres = fADCThres;
      fAvalancheSize = fADCpeak;
      fResponse.clear();
      fResponse.reserve( fAnodeResponse.size() );
      fResponse = fAnodeResponse;
  
      isanode=true;

      auto& channels = anodeSignals->hits; // vector<Alpha16Channel*>
      if( fTrace )
         std::cout<<"DeconvModule::FindAnodeTimes Channels Size: "<<channels.size()<<std::endl;
      
      // // prepare vector with wf to manipulate
      std::vector<std::vector<double>> subtracted;
      subtracted.reserve( channels.size() );
      subtracted.clear();

      // clear/initialize "output" vectors
      fElectrodeIndex.clear();
      fElectrodeIndex.reserve( channels.size() );
      // aresult.clear();
      // aresult.reserve( channels.size() );
      fSignals.clear();
      fSignals.reserve(channels.size());
      fTimes.clear(); 

      wirewaveforms.clear();

      // find intresting channels
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += ch->adc_samples.at( b );
            ped /= double(pedestal_length);

            // CALCULATE PEAK HEIGHT
            double max = fScale * (double(*std::min_element(ch->adc_samples.begin(), 
                                                            ch->adc_samples.end()) ) - ped);

            if(max > fThres)     // Signal amplitude < thres is considered uninteresting
               {
                  if(fTrace && 0)
                     std::cout<<"\tsignal above threshold ch: "<<i<<std::endl;

                  // SUBTRACT PEDESTAL
                  std::vector<double> waveform(ch->adc_samples.begin()+pedestal_length,
                                               ch->adc_samples.end());
                  std::for_each(waveform.begin(), waveform.end(), [ped](double& d) { d-=ped;});

                  // if( ch->adc_chan < 16 )
                  //    {
                  //       // bin size = 10 ns --> 100 MHz ADC
                  //       // waveform = Interpolate(&waveform,10.,16.,0.);
                  //       waveform = Interpolate(waveform,10.,double(fbinsize),0.);
                  //    } 
                  // else 
                  //    {
                  //       // bin size = 16 ns --> 62.5 MHz ADC
                  //    }

                  // // chop waveform to nAWsamples x 16 ns = 335 x 16 ns = 5.36 us
                  // waveform.resize( nAWsamples  );

                  // fill vector with wf to manipulate
                  subtracted.emplace_back( waveform.begin(), waveform.end() );
                  //aresult.emplace_back( waveform.size() );

                  // CREATE electrode
                  electrode el(ch->tpc_wire);
                  fElectrodeIndex.push_back( el );

                  wirewaveforms.emplace_back(el,&waveform);
               }// max > thres
         }// channels
      

      // DECONVOLUTION
      int nsig = Deconv(subtracted);
      std::cout<<"DeconvModule::FindAnodeTimes "<<nsig<<" found"<<std::endl;
      //

      fAnodeIndex.clear();
      fAnodeIndex.reserve( fElectrodeIndex.size() );
      fAnodeIndex = fElectrodeIndex;

      sanode.clear();
      sanode.reserve( fSignals.size() );
      sanode=fSignals;

      aTimes.clear();
      aTimes=fTimes;

      // prepare control variable (deconv remainder) vector
      resRMS_a.clear();
      resRMS_a.reserve( subtracted.size() );
      // calculate remainder of deconvolution
      for(auto s: subtracted)
         resRMS_a.push_back( sqrt(
                                  std::inner_product(s.begin(), s.end(), s.begin(), 0.)
                                  / static_cast<double>(s.size()) )
                             );
      return nsig;
   }

   int FindPadTimes(const FeamEvent* padSignals)
   {
      fbinsize = fPADbinsize;
      theBin = thePadBin;
      fThres = fPWBThres;
      fAvalancheSize = fPWBpeak;
      fResponse.clear();
      fResponse.reserve( fPadResponse.size() );
      fResponse = fPadResponse;

      isanode=false;

      auto& channels = padSignals->hits; // vector<FeamChannel*>
      if( fTrace )
         std::cout<<"DeconvModule::FindPadTimes Channels Size: "<<channels.size()<<std::endl;

      // prepare vector with wf to manipulate
      std::vector<std::vector<double>> subtracted;
      subtracted.reserve( channels.size() );
      subtracted.clear();

      // clear/initialize "output" vectors
      fElectrodeIndex.clear();
      fElectrodeIndex.reserve( channels.size() );
      fSignals.clear();
      fSignals.reserve(channels.size());
      fTimes.clear();

      feamwaveforms.clear();
  
      // prepare control variable (deconv remainder) vector
      resRMS_p.clear();
      resRMS_p.reserve( channels.size() );

      // find intresting channels
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // FeamChannel*
            if( !PwbPadMap::chan_is_pad(ch->sca_chan) ) continue;
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
            double max = fScale * (double(*std::min_element(ch->adc_samples.begin(), 
                                                            ch->adc_samples.end()) ) - ped);
            
            if(max > fThres)     // Signal amplitude < thres is considered uninteresting
               {
                  if(fTrace && 0)
                     std::cout<<"\tsignal above threshold ch: "<<i<<std::endl;

                  // SUBTRACT PEDESTAL
                  std::vector<double> waveform(ch->adc_samples.begin()+pedestal_length,
                                               ch->adc_samples.end());
                  std::for_each(waveform.begin(), waveform.end(), [ped](double& d) { d-=ped;});

                  // fill vector with wf to manipulate
                  subtracted.emplace_back( waveform.begin(), waveform.end() );
                  //aresult.emplace_back( waveform.size() );

                  // CREATE electrode
                  int col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
                  col+=1;
                  if( col == 32 ) col = 0;
                  assert(col<32);
                  // std::cout<<"DeconvModule::FindPadTimes() col: "<<col<<std::endl;
                  int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
                  // std::cout<<"DeconvModule::FindPadTimes() row: "<<row<<std::endl;
                  assert(row<576);
                  if( fTrace && 0 )
                     std::cout<<"DeconvModule::FindPadTimes() pwb"<<ch->imodule
                              <<" col: "<<col
                              <<" row: "<<row
                              <<" ph: "<<max<<std::endl;
                  //pwbmap<<col<<"\t"<<row<<"\t"<<ch->imodule<<std::endl;
                  electrode el(col,row);
                  fElectrodeIndex.push_back( el );

                  feamwaveforms.emplace_back(el,&waveform);
               }// max > thres
         }// channels


      // DECONVOLUTION
      //int nsig = DeconvAndSubtract(subtracted);
      int nsig = Deconv(subtracted);
      std::cout<<"DeconvModule::FindPadTimes "<<nsig<<" found"<<std::endl;
      //

      fPadIndex.clear();
      fPadIndex.reserve( fElectrodeIndex.size() );
      fPadIndex = fElectrodeIndex;

      spad.clear();
      spad.reserve( fSignals.size() );
      spad=fSignals;
      
      pTimes.clear();
      pTimes=fTimes;

      // prepare control variable (deconv remainder) vector
      resRMS_p.clear();
      resRMS_p.reserve( subtracted.size() );
      // calculate remainder of deconvolution
      for(auto s: subtracted)
         resRMS_p.push_back( sqrt(
                                  std::inner_product(s.begin(), s.end(), s.begin(), 0.)
                                  / static_cast<double>(s.size()) )
                             );
      
      return nsig;
   }

#if 0
   template<typename CH>
   std::vector<std::vector<double>> InitDeconv( std::vector<CH*> const& channels)
   {
      // prepare vector with wf to manipulate
      std::vector<std::vector<double>> subtracted;
      subtracted.reserve( channels.size() );
      subtracted.clear();

      // clear/initialize "output" vectors
      fElectrodeIndex.clear();
      fElectrodeIndex.reserve( channels.size() );

      // find intresting channels
      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            //auto& ch = channels.at(i);
            CH* ch = channels.at(i);
            std::vector<double> waveform(ch->adc_samples.begin(),ch->adc_samples.end());

            // CALCULATE PEDESTAL
            double ped(0.);
            for(int b = 0; b < pedestal_length; b++) ped += waveform.at( b );
            ped /= pedestal_length;

            // CALCULATE PEAK HEIGHT
            double max = fScale * (double(*std::min_element(waveform.begin(), 
                                                            waveform.end()) ) - ped);

            if(max > fThres)     // Signal amplitude < thres is considered uninteresting
               {
                  if(fTrace && 0)
                     std::cout<<"\tsignal above threshold ch: "<<i<<std::endl;

                  // CREATE electrode
                  electrode el;
                  // std::cout<<typeid(ch).name()<<"\t";
                  // std::cout<<typeid(Alpha16Channel).name()<<std::endl;
                  if( typeid(ch) == typeid(Alpha16Channel*) )
                     el.setwire(ch->tpc_wire);
                  else if( typeid(ch) == typeid(FeamChannel*) )
                     el.setpad( ch->pad_col, ch->pad_row );

                  fElectrodeIndex.push_back( el );

                  // fill vector with wf to manipulate
                  subtracted.emplace_back( waveform.begin()+pedestal_length, waveform.end() );
                  //aresult.emplace_back( waveform.size() );
               }// max > thres
         }// channels
      return subtracted;
   }
#endif

   int ReadResponseFile(const double awbin, const double padbin)
   {
      std::string filename = "anodeResponse.dat";
      if( fTrace )
         std::cout << "DeconvModule:: Reading in response file (anode)" << filename << std::endl;
      std::ifstream respFile(filename.c_str());
      if( !respFile.good() ) return false;
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

      filename = "padResponse_deconv.dat";
      if( fTrace )
         std::cout << "DeconvModule:: Reading in response file (pad) " << filename << std::endl;
      respFile.open(filename.c_str());
      if( !respFile.good() ) return false;
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
      result.clear();
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

   int Deconv( std::vector<std::vector<double>>& subtracted )
   {
      if(subtracted.size()==0) return 0;
      int nsamples = subtracted.back().size();
      assert(nsamples < 1000);
      if( fTrace )
         std::cout<<"DeconvModule::Deconv Subtracted Size: "<<subtracted.size()
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
            std::set<wfholder,comp_hist> histset = wforder( subtracted, b );
            // std::cout<<"DeconvModule::Deconv bin of interest: "<<b
            //          <<" workable wf: "<<histset.size()<<std::endl;
            
            // this is useful to split deconv into the "Subtract" method
            // map ordered wf to corresponding electrode
            std::map<int,wfholder> histmap = wfordermap(histset);

            double neTotal = 0.0;
            for(auto it = histset.begin(); it != histset.end(); ++it)
               {
                  std::vector<double> &wf = *it->h;
                  unsigned int i = it->index;
                  auto anElectrode = fElectrodeIndex.at( i );
                  double ne = fScale*wf[b]/fResponse[theBin]; // number of "electrons"

                  if( ne >= fAvalancheSize )
                     {
                        neTotal += ne;
                        // loop over all bins for subtraction
                        Subtract(histmap,i,b,ne);

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
         }// loop bin of interest

      return int(fSignals.size());
   }
   
   void Subtract(std::map<int,wfholder>& wfmap,
                 const unsigned i, const int b,
                 const double ne)
   {
      wfholder hist1 = wfmap[i];
      std::vector<double> &wf1 = *(hist1.h);
      unsigned int i1 = hist1.index;
      auto wire1 = fElectrodeIndex.at( i1 ); // mis-name for pads

      // loop over all bins for subtraction
      for(int bb = b-theBin; bb < int(wf1.size()); ++bb)
         {
            // the bin corresponding to bb in the response
            int respBin = bb-b+theBin;

            if( isanode ) // neighbour subtraction for anodes only
               {
                  // loop over all signals looking for neighbours
                  for(unsigned int k = 0; k < fElectrodeIndex.size(); ++k)
                     {                                               
                        auto wire2 = fElectrodeIndex.at( k );
 
                        //check for top/bottom
                        if( wire2.sec != wire1.sec ) continue;

                        for(unsigned int l = 0; l < fAnodeFactors.size(); ++l)
                           {
                              if( !IsNeighbour( wire1.idx, wire2.idx, int(l+1) ) ) continue;

                              //std::vector<double> &wf2 = subtracted[k];
                              wfholder hist2 = wfmap[k];
                              std::vector<double> &wf2 = *(hist2.h);

                              if(respBin < int(fAnodeResponse.size()) && respBin >= 0)
                                 {
                                    // remove neighbour induction
                                    wf2.at(bb) += ne/fScale*fAnodeFactors.at(l)*fAnodeResponse.at(respBin);
                                 }
                           }// loop over factors
                     }// loop all signals looking for neighbours
               }
            if( respBin < int(fResponse.size()) && respBin >= 0 )
               {
                  // Remove signal tail for waveform we're currently working on
                  wf1.at(bb) -= ne/fScale*fResponse.at(respBin);
               }
         }// bin loop: subtraction
   }

   void RescaleNeighbour(std::vector<double>& wf, const double& amp, 
                         int& resp_bin, int& curr_bin, unsigned& aw)
   {
      //if( curr_bin < wf.size() )
      wf.at( curr_bin ) += amp/fScale*fAnodeFactors.at(aw)*fAnodeResponse.at(resp_bin);
   }

#if 0
   int DeconvAndSubtract( std::vector<std::vector<double>>& subtracted )
   {
      int nsamples = subtracted.back().size();
      if( fTrace )
         std::cout<<"DeconvModule::Deconv Subtracted Size: "<<subtracted.size()
                  <<"\t# samples: "<<nsamples<<std::endl;
      
      double neTotal = 0.0;
      for(int b = theBin; b < int(nsamples); ++b)// b is the current bin of interest
         {
            // std::cout<<b;
            for( unsigned int k = 0; k < fElectrodeIndex.size(); ++k)
               {
                  auto anElectrode = fElectrodeIndex.at( k );
                  std::vector<double> &wf = subtracted[k];
                  double ne = fScale*wf[b]/fResponse[theBin]; // number of "electrons"

                  // std::cout<<"\t"<<k<<"\t"<<ne<<std::endl;
                  if( ne >= fAvalancheSize )
                     {
                        neTotal += ne;
                        // loop over all bins for subtraction
                        for(int bb = b-theBin; bb < int(wf.size()); ++bb)
                           {
                              // the bin corresponding to bb in the response
                              int respBin = bb-b+theBin;

                              if( respBin < int(fResponse.size()) && respBin >= 0 )
                                 {
                                    // Remove signal tail for waveform we're currently working on
                                    wf.at(bb) -= ne/fScale*fResponse.at(respBin);
                                 }// valid bin
                           }// bin loop: subtraction

                        if(b-theBin >= 0)
                           {
                              // time in ns of the bin b centre
                              double t = ( double(b-theBin) + 0.5 ) * double(fbinsize);
                              t+=fPWBdelay;
                              fSignals.emplace_back(anElectrode,t,ne);
                              fTimes.insert(t);
                           }
                     }// if deconvolution threshold Avalanche Size
               }// loop pads
         }// loop bin of interest

      return int(fSignals.size());
   }
#endif

   bool IsNeighbour(int w1, int w2, int dist)
   {
      return ( ( abs(w2 - w1) == dist ) || 
               ( abs(w2 - w1 - 256 ) == dist ) || 
               ( abs(w2 - w1 + 256 ) == dist ) );
   }

   bool IsNeighbour(int w1, int w2)
   {
      return ( ( abs(w2 - w1) <= int(fAnodeFactors.size()) ) || 
               ( abs(w2 - w1 - 256 ) <= int(fAnodeFactors.size()) ) || 
               ( abs(w2 - w1 + 256 ) <= int(fAnodeFactors.size()) ) );
   }

   
   std::set<wfholder,comp_hist> wforder(std::vector<std::vector<double>>& subtracted, const int b)
   {
      std::set<wfholder,comp_hist> histset;
      histset.clear();    
      // For each bin, order waveforms by size,
      // i.e., start working on largest first
      for(unsigned int i=0; i<subtracted.size(); ++i)
         {
            wfholder mh;
            mh.h = &subtracted[i];
            mh.index = i;
            mh.val = fScale*subtracted[i][b];
            histset.insert(mh);
         }
      return histset;
   }


   std::map<int,wfholder> wfordermap(std::set<wfholder,comp_hist> histset)
   {
      std::map<int,wfholder> wfmap;
      for(unsigned int k = 0; k < fElectrodeIndex.size(); ++k)
         {
            for(auto it = histset.begin(); it != histset.end(); ++it)
               {
                  if( k == it->index )
                     wfmap[k]=*it;
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
