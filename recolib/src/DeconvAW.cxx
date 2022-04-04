#include <fstream>
#include <numeric>
#include <cmath>

#include <TDirectory.h>

#include "DeconvAW.h"

#ifdef BUILD_AG_SIM
#include "TWaveform.hh"
#endif

TH2D* DeconvAW::hADCped=0;
TProfile* DeconvAW::hADCped_prox=0;

// pads
TH1D* DeconvAW::hAvgRMSPad=0;
// anodes
TH1D* DeconvAW::hAvgRMSTop=0;

DeconvAW::DeconvAW(double adc, 
	       double aw): fTrace(false), fDiagnostic(false), fAged(false),
                                       fAWbinsize(16),
                                       fADCmax(pow(2.,14.)),
                                       fADCrange(fADCmax*0.5-1.),
                                       fADCdelay(0.), // to be guessed
                                       pedestal_length(100),fScale(-1.), // values fixed by DAQ
                                       theAnodeBin(1),
                                       fADCThres(adc),
                                       fADCpeak(aw),
                                       isalpha16(false)
                                       
{
   Setup();
}

DeconvAW::DeconvAW(std::string json):fTrace(false), fDiagnostic(false), fAged(false),
                                ana_settings(new AnaSettings(json.c_str())),
                                fAWbinsize(16),
                                fADCmax(pow(2.,14.)),
                                fADCrange(fADCmax*0.5-1.),
                                fADCdelay(0.), // to be guessed
                                pedestal_length(100),fScale(-1.), // values fixed by DAQ
                                theAnodeBin(1), 
                                fADCThres(ana_settings->GetDouble("DeconvModule","ADCthr")),
                                fADCpeak(ana_settings->GetDouble("DeconvModule","AWthr")),
                                isalpha16(false)
                                
{
   Setup();
}

DeconvAW::DeconvAW(AnaSettings* s):fTrace(false), fDiagnostic(false), fAged(false),
                               ana_settings(s),
                               fAWbinsize(16),
                               fADCmax(pow(2.,14.)),
                               fADCrange(fADCmax*0.5-1.),
                               fADCdelay(0.), // to be guessed
                               pedestal_length(100),fScale(-1.), // values fixed by DAQ
                               theAnodeBin(1), 
                               fADCThres(ana_settings->GetDouble("DeconvModule","ADCthr")),
                               fADCpeak(ana_settings->GetDouble("DeconvModule","AWthr")),
                               isalpha16(false)
{
   //   Setup();
}

DeconvAW::~DeconvAW()
{

}

void DeconvAW::Setup()
{
   SetupADCs(0,0);
}

void DeconvAW::SetupADCs(TFile* fout, int run, bool norm, bool diag)
{
   fAnodeFactors = {
      0.1275,        // neighbour factor
      0.0365,        // 2nd neighbour factor
      0.012,         // 3rd neighbour factor
      0.0042         // 4th neighbour factor
   };

   fAwMask.reserve(256);



   int s = ReadAWResponseFile(fAWbinsize);
   std::cout<<"DeconvAW::SetupADCs() Response status: "<<s<<std::endl;
   assert(s>0);
   if( norm )
      {
         s = ReadADCRescaleFile();
         std::cout<<"DeconvAW::SetupADCs() Rescale status: "<<s<<std::endl;
         assert(s>0);
      }
   else
      fAdcRescale.assign(256,1.0);

   if( diag )
      {
         fDiagnostic = true;
         if( fout ) {
            fout->cd();
            if( !gDirectory->cd("awdiag") )
               gDirectory->mkdir("awdiag")->cd();
            else
               gDirectory->cd("awdiag");
            
            if( !hAvgRMSTop )
               hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",1000,0.,50000.);
            //hAvgRMSBot = new TH1D("hAvgRMSBot","Average Deconv Remainder Bottom",1000,0.,50000.);
            
            if( !hADCped )
               hADCped = new TH2D("hADCped","ADC pedestal per AW",256,0.,256.,
                                  1600,-16384.,16384.);
            if( !hADCped_prox )
               hADCped_prox = new TProfile("hADCped_prox","Average ADC pedestal per AW;AW;ADC",
                                           256,0.,256.,16384.,16384.);
         }
      }
   
   // by run settings
   if( run == 0 ) // simulation
      {
         std::cout<<"DeconvAW::SetupADCs() Monte Carlo settings"<<std::endl;
         //fADCdelay = -54.;
         //fPWBdelay = 54.;
      }
   else if( run < 2724 && run > 0 ) // new FMC-32
      {
         fAWbinsize=10;
         isalpha16=true;
      }
   else if( run >= 2282 && run < 2724 )
      fADCdelay = -120.;
   else if( run >= 2724 && run < 3032 ) // new FMC-32
      fADCdelay = 0.;
   else if( run >= 3032 && run < 4488 )
      fADCdelay = -256.;
   else if( run >= 4488 && run < 4590 )
      fADCdelay = 0.;//fADCdelay = -304.;
   else if( run >= 4590 && run < 900000 )
      fADCdelay = -232.;

   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
       run == 3249 || run == 3250 || run == 3251 ||
       run == 3253 || run == 3254 || run == 3255 ||
       run == 3260 || run == 3263 || run == 3265 ||
       run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
         fADCdelay = -400.;

   if( run > 903837 && run < 904100 ) fADCdelay = -120.;
   else if( run > 904100 ) fADCdelay = 0.;
   //   else if( run > 904100 && run <= 904400 ) fADCdelay = -80.;
   // else if( run > 904400 && run <= 904500 ) fADCdelay = -32.;
   // else if( run > 904500 ) fADCdelay = 0.;
}

void DeconvAW::Reset()
{ 

}

#ifdef BUILD_AG_SIM
int DeconvAW::FindAnodeTimes(TClonesArray* AWsignals)
{
   int Nentries = AWsignals->GetEntries();
   // prepare vector with wf to manipulate
   std::vector<ALPHAg::wfholder*> AnodeWaves;
   AnodeWaves.reserve( Nentries );
   // clear/initialize "output" vectors
   fAnodeIndex.clear();
   fAnodeIndex.reserve( Nentries );
 
   // find intresting channels
   unsigned int index=0; //wfholder index
   for( int j=0; j<Nentries; ++j )
      {
         TWaveform* w = (TWaveform*) AWsignals->ConstructedAt(j);
         std::vector<int> data(w->GetWaveform());
         std::string wname = w->GetElectrode();
         //std::cout<<"DeconvAW::FindAnodeTimes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
         int aw_number = std::stoi( wname.substr(1) );
         ALPHAg::electrode el(aw_number);
         //std::cout<<"DeconvAW::FindAnodeTimes Electrode: "<<el.idx<<std::endl;

         // nothing dumb happens
         if( (int)data.size() < 410 + pedestal_length )
            {
               std::cerr<<"DeconvAW::FindAnodeTimes ERROR wf samples: "
                        <<data.size()<<std::endl;
               continue;
            }

         // mask hot wires
         if( MaskWires(aw_number) ) continue;

         double ped = CalculatePedestal(data);
         double peak_h = GetPeakHeight(data,el.idx,ped,true);
         //std::cout<<"aw: "<<aw_number<<" PH: "<<peak_h<<" (ped:"<<ped<<")"<<std::endl;

         // CREATE WAVEFORM
         ALPHAg::wfholder* waveform=new ALPHAg::wfholder( index, 
                                          std::next(data.begin(),pedestal_length),
                                          data.end());
         if(peak_h > fADCThres)     // Signal amplitude < thres is considered uninteresting
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<j<<" aw: "<<aw_number<<std::endl;

               // SUBTRACT PEDESTAL
               waveform->massage(ped,fAdcRescale.at(el.idx));

               // fill vector with wf to manipulate
               AnodeWaves.emplace_back( waveform );

               // STORE electrode
               fAnodeIndex.push_back( el );

               ++index;
            }// max > thres 
         else
            delete waveform;
      } // channels

   // ============== DECONVOLUTION ==============
   sanode = Deconvolution(&AnodeWaves,fAnodeIndex,fAnodeResponse,theAnodeBin, true);
   int nsig=-1;
   if(!sanode) nsig=0;
   else nsig = sanode->size();
   std::cout<<"DeconvAW::FindAnodeTimes "<<nsig<<" found"<<std::endl;
   // ===========================================

   
   for (uint i=0; i<AnodeWaves.size(); i++)
      {
         //delete AnodeWaves.at(i)->h;
         delete AnodeWaves.at(i);
      }
   AnodeWaves.clear();
   
   return nsig;
}

int DeconvAW::FindPadTimes(TClonesArray* PADsignals)
{
   int Nentries = PADsignals->GetEntries();

   // prepare vector with wf to manipulate
   std::vector<ALPHAg::wfholder*> PadWaves;
   PadWaves.reserve( Nentries );
   // clear/initialize "output" vectors
   fPadIndex.clear();
   fPadIndex.reserve( Nentries );

   std::string delimiter = "_";

   // find intresting channels
   unsigned int index=0; //ALPHAg::wfholder index
   for( int j=0; j<Nentries; ++j )
      {
         TWaveform* w = (TWaveform*) PADsignals->ConstructedAt(j);
         std::vector<int> data(w->GetWaveform());
         std::string wname = w->GetElectrode();

         size_t pos = wname.find(delimiter);
         std::string p = wname.substr(0, pos);
         if( p != "p" )
            std::cerr<<"DeconvAW Error: Wrong Electrode? "<<p<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         short col = std::stoi( wname.substr(0, pos) );
         assert(col<32&&col>=0);
         //std::cout<<"DeconvAW::FindPadTimes() col: "<<col<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         int row = std::stoi( wname.substr(0, pos) );
         //std::cout<<"DeconvAW::FindPadTimes() row: "<<row<<std::endl;
         assert(row<576&&row>=0);

         int coli = int(col);
         int pad_index = pmap->index(coli,row);
         assert(!std::isnan(pad_index));
         // CREATE electrode
         ALPHAg::electrode el(col,row);
  
         if( data.size() == 0 ) continue;

         // nothing dumb happens
         if( (int)data.size() < 410 + pedestal_length )
            {
               std::cerr<<"DeconvAW::FindPadTimes ERROR wf samples: "
                        <<data.size()<<std::endl;
               continue;
            }
         // mask hot pads
         if( MaskPads(col,row) ) continue;

         double ped = CalculatePedestal(data);
         double peak_h = GetPeakHeight(data,pad_index,ped,false);

         // CREATE WAVEFORM
         ALPHAg::wfholder* waveform=new ALPHAg::wfholder( index, 
                                          std::next(data.begin(),pedestal_length),
                                          data.end());
         
         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fPWBThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<j<<" index: "<<index<<std::endl;

               // SUBTRACT PEDESTAL
               waveform->massage(ped,fPwbRescale.at(pad_index));

               // fill vector with wf to manipulate
               PadWaves.emplace_back( waveform );
                  
               // STORE electrode
               fPadIndex.push_back( el );     
                  
               ++index;
            }// max > thres
         else
            delete waveform;
      }// channels

   // ============== DECONVOLUTION ==============
   spad = Deconvolution(&PadWaves,fPadIndex,fPadResponse,thePadBin,false);
   int nsig=-1;
   if(!spad) nsig=0;
   else nsig = spad->size();
   std::cout<<"DeconvAW::FindPadTimes "<<nsig<<" found"<<std::endl;
   // ===========================================   

   for (uint i=0; i<PadWaves.size(); i++)
      {
         // delete PadWaves.at(i)->h;
         delete PadWaves.at(i);
      }
   PadWaves.clear();
  
   return nsig;
}
#endif

void DeconvAW::BuildWFContainer(
    const Alpha16Event* anodeSignals,
    std::vector<ALPHAg::wfholder>& AnodeWaves,
    std::vector<ALPHAg::electrode>& AnodeIndex,
    std::vector<ALPHAg::wf_ref>& wirewaveforms,  // to use in aged display
    std::vector<ALPHAg::TWireSignal>& AdcPeaks // waveform max
) const
{
   const std::vector<Alpha16Channel*> channels = anodeSignals->hits;
   // prepare vector with wf to manipulate
   AnodeWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   AnodeIndex.clear();
   AnodeIndex.reserve( channels.size() );

   if( fDiagnostic ) 
   {
      AdcPeaks.reserve( channels.size() );
   }

   if( fAged ) 
   {
      wirewaveforms.reserve( channels.size() );
   }

   // find intresting channels
   unsigned int index=0; //wfholder index
   for(unsigned int i = 0; i < channels.size(); ++i)
      {
         Alpha16Channel* ch = channels[i];
         if( ch->adc_chan < 16 && !isalpha16 ) continue; // it's bv

         int aw_number = ch->tpc_wire;
         // std::cout<<"DeconvAWModule::FindAnodeTimes anode wire: "<<aw_number<<std::endl;
         if( aw_number < 0 || aw_number >= 512 ) continue;
         // CREATE electrode
         ALPHAg::electrode el(aw_number);
           
         // mask hot wires
         if( MaskWires(aw_number) ) continue;

         double ped = CalculatePedestal(ch->adc_samples);
         double peak_h = GetPeakHeight(ch->adc_samples,el.idx,ped);

         if( fDiagnostic )
            {
               hADCped->Fill(el.idx,ped);
               hADCped_prox->Fill(el.idx,ped);
               double peak_t = GetPeakTime(ch->adc_samples);
               AdcPeaks.emplace_back(el.idx,peak_t,peak_h,0.);
            }
            

         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fADCThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" aw: "<<aw_number<<std::endl;

               // fill vector with wf to manipulate
               AnodeWaves.emplace_back(index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end() );

               // CREATE WAVEFORM
               ALPHAg::wfholder& waveform = AnodeWaves.back();

               // SUBTRACT PEDESTAL
               waveform.massage(ped,fAdcRescale[el.idx]);

               // STORE electrode
               AnodeIndex.push_back( el );

               if( fAged )
                  wirewaveforms.emplace_back(el,new std::vector<double>(waveform.h));

               ++index;
            }// max > thres
      }// channels
   }

void DeconvAW::Deconvolution(
    std::vector<ALPHAg::wfholder>& subtracted,
    const std::vector<ALPHAg::electrode> &fElectrodeIndex,
    std::vector<ALPHAg::TWireSignal>& signals) const
{
   if(subtracted.empty())
      return;

   size_t nsamples = subtracted.back().h.size();
   assert(nsamples >= theAnodeBin);
   
   signals.clear();
   signals.reserve(nsamples - theAnodeBin);
   assert(nsamples < 1000);
   if( fTrace )
      std::cout<<"DeconvAW::Deconvolution Subtracted Size: "<<subtracted.size()
               <<"\t# samples: "<<nsamples<<"\ttheBin: "<<theAnodeBin<<std::endl;

   return Deconvolution( subtracted, fElectrodeIndex, signals, theAnodeBin,nsamples);
}

void DeconvAW::Deconvolution(
    std::vector<ALPHAg::wfholder>& subtracted,
    const std::vector<ALPHAg::electrode> &fElectrodeIndex,
    std::vector<ALPHAg::TWireSignal>& signals, const int start, const int stop) const
{

   for(int b = start; b < stop; ++b)// b is the current bin of interest
      {
         // For each bin, order waveforms by size,
         // i.e., start working on largest first
         std::vector<ALPHAg::wfholder*> histset = wforder( subtracted, b );
         // std::cout<<"DeconvAW::Deconvolution bin of interest: "<<b
         //          <<" workable wf: "<<histset.size()<<std::endl;
         // this is useful to split deconv into the "Subtract" method
         // map ordered wf to corresponding electrode
         double neTotal = 0.0;
         for (auto const it : histset)
            {
               unsigned int i = it->index;
               const std::vector<double>& wf=it->h;
               const ALPHAg::electrode &anElectrode = fElectrodeIndex[i];
               // number of "electrons"
               double ne = anElectrode.gain * fScale * wf[b] / fAnodeResponse[theAnodeBin];
               if( ne >= fADCpeak )
                  {
                     neTotal += ne;
                     // loop over all bins for subtraction
                     SubtractAW(it,subtracted,b,ne,fElectrodeIndex);
                     if( int( b - theAnodeBin) >= 0)
                        {
                           // time in ns of the bin b centre
                           double t = ( double(b - theAnodeBin) + 0.5 ) * double(fAWbinsize) + fADCdelay;
                           signals.emplace_back(anElectrode,t,ne,GetNeErr(ne,it->val));
                        }
                  }// if deconvolution threshold Avalanche Size
            }// loop set of ordered waveforms
      }// loop bin of interest
   return;
}

void DeconvAW::LogDeconvRemaineder( std::vector<ALPHAg::wfholder>& AnodeWaves )
{
    
   if( fDiagnostic )
   {
      // resRMS_a.clear();
      // resRMS_a.reserve( AnodeWaves.size() );
      // calculate remainder of deconvolution
      double mtop=0.,rtop=0.;
      for(const auto& s: AnodeWaves)
      {
         rtop += sqrt( std::inner_product(s.h.begin(), s.h.end(), s.h.begin(), 0.)
                     / static_cast<double>(s.h.size()) );
         // resRMS_a.push_back( sqrt(
         //                          std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
         //                          / static_cast<double>(s->h->size()) )
         //                     );
         ++mtop;
      }
      if( mtop!=0.)
         rtop /= mtop;
      //std::cout<<"DeconvAWModule:: RMS top: "<<rtop<<" el: "<<mtop<<" avg RMS: "<<rtop<<std::endl;
      hAvgRMSTop->Fill(rtop);
   }
}


void DeconvAW::SubtractAW(ALPHAg::wfholder* hist1,
                        std::vector<ALPHAg::wfholder>& wfmap,
                        const int b,
                        const double ne,
                        const std::vector<ALPHAg::electrode> &fElectrodeIndex ) const
{

   std::vector<double> &wf1 = hist1->h;
   const int wf1size = wf1.size();

   unsigned int i1 = hist1->index;
   const ALPHAg::electrode& wire1 = fElectrodeIndex[ i1 ]; // mis-name for pads
   const double wiregain = ne/fScale/wire1.gain;

   const size_t AnodeSize = fAnodeFactors.size();
   const size_t ElectrodeSize = fElectrodeIndex.size();
   const int AnodeResponseSize = (int)fAnodeResponse.size();

   const int start = b - theAnodeBin;
   const int range = std::min(wf1size - start, AnodeResponseSize );

   for(unsigned int k = 0; k < ElectrodeSize; ++k)
      {
         const ALPHAg::electrode& wire2 = fElectrodeIndex[ k ];
         //check for top/bottom
         if( wire2.sec != wire1.sec ) continue;
         //Skip early if wires not close...
         if (IsAnodeClose(wire1.idx,wire2.idx)>4) continue;
         
         std::vector<double>& wf2 = wfmap[k].h;
         for(unsigned int l = 0; l < AnodeSize; ++l)
            {
               //Take advantage that there are 256 anode wires... use uint8_t
               //if( !IsNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
               if( !IsAnodeNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
               const double gainfactor = wiregain * fAnodeFactors[l];
               int respBin = 0;
               for(int bb = start; bb < start + range; ++bb)
                  {
                     // remove neighbour induction
                     wf2[bb] += gainfactor * fAnodeResponse[respBin];
                     ++respBin;
                  }// loop over all bins for subtraction
            }// loop over factors
      }// loop all electrodes' signals looking for neighbours
   

   int respBin = 0;
   for(int bb = start; bb < start + range; ++bb)
      {
         // Remove signal tail for waveform we're currently working on
         wf1[bb] -= wiregain * fAnodeResponse[respBin];
         ++respBin;
      }// bin loop: subtraction
}

std::vector<ALPHAg::wfholder*> DeconvAW::wforder(std::vector<ALPHAg::wfholder>& subtracted, const unsigned b)  const
{
   // For each bin, order waveforms by size,
   // i.e., start working on largest first

   const size_t size = subtracted.size();
   std::vector<ALPHAg::wfholder*> histset(size,nullptr);
   //   std::cout<<"DeconvAW::wforder subtracted size: "<<size<<" @ bin = "<<b<<std::endl;
   for(size_t i=0; i<size;++i)
      {
         //         std::cout<<"wf# "<<i;
         ALPHAg::wfholder& mh=subtracted[i];
         // std::cout<<"\twf index: "<<mh->index;
         // std::cout<<"\twf size: "<<mh->h->size();
         //std::cout<<"\twf bin: "<<b<<std::endl;
         mh.val = fScale*mh.h[b];
         //         std::cout<<"\twf val: "<<mh->val<<std::endl;
         histset[i] = &mh;
      }
   std::sort(histset.begin(), histset.end(),wf_comparator);
   return histset;
}

int DeconvAW::ReadResponseFile(const int awbin)
{
   return ReadAWResponseFile(awbin);
}

int DeconvAW::ReadAWResponseFile( const int awbin )
{
   std::string filename(getenv("AGRELEASE"));
   filename+="/ana/anodeResponse.dat";
   std::cout << "DeconvAW:: Reading in response file (anode) " << filename << std::endl;
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
      std::cout<<"DeconvAW::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theAnodeBin<<std::endl;

   return int(fAnodeResponse.size());
}

std::vector<double> DeconvAW::Rebin(const std::vector<double> &in, int binsize, double ped)
{
   if( fTrace )
      std::cout<<"DeconvAW::Rebin "<<binsize<<std::endl;
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
         if( fTrace )
            std::cout << "DeconvAW::Rebin: Cannot rebin without rest, dropping final "
                      << in.size() % result.size() << std::endl;
      }
   return result;
}

int DeconvAW::ReadRescaleFile()
{
   return ReadADCRescaleFile();
}

int DeconvAW::ReadADCRescaleFile()
{
   std::string basepath(getenv("AGRELEASE"));
   std::ifstream fadcres(basepath+"/ana/AdcRescale.dat");
   double rescale_factor;
   while(1)
      {
         fadcres>>rescale_factor;
         if( !fadcres.good() ) break;
         fAdcRescale.push_back(rescale_factor);
      }
   fadcres.close();

   if( fAdcRescale.size() == 256 )
      std::cout<<"DeconvAWModule BeginRun ADC rescaling factors OK"<<std::endl;
   else
      std::cout<<"DeconvAWModule BeginRun ADC rescaling factors NOT ok (size: "
               <<fAdcRescale.size()<<")"<<std::endl;
   return int(fAdcRescale.size());
}

void DeconvAW::PrintADCsettings()
{
   std::cout<<"-------------------------"<<std::endl;
   std::cout<<"Deconv AW Settings"<<std::endl;
   std::cout<<" is ADC16: "<<isalpha16<<std::endl;
   std::cout<<" ADC max: "<<fADCmax<<std::endl;
   std::cout<<" ADC range: "<<fADCrange<<std::endl;
   std::cout<<" ADC time bin: "<<fAWbinsize<<" ns"<<std::endl;
   std::cout<<" ADC delay: "<<fADCdelay<<" ns"<<std::endl;
   std::cout<<" ADC thresh: "<<fADCThres<<std::endl;
   std::cout<<" AW thresh: "<<fADCpeak<<std::endl;
   std::cout<<"-------------------------"<<std::endl;
   std::cout<<"Masked Electrodes"<<std::endl;
   std::cout<<"AW: ";
   for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
      std::cout<<*it-256<<", ";
   std::cout<<"\n"<<std::endl;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
