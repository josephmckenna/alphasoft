#include <fstream>
#include <numeric>
#include <cmath>

#include <TDirectory.h>

#include "Deconv.hh"
#include "TWaveform.hh"

Deconv::Deconv(double adc, double pwb,
	       double aw, double pad): fTrace(false), fDiagnostic(false), fAged(false),
                                       fbinsize(1), fAWbinsize(16), fPADbinsize(16),
				       fADCdelay(0.),fPWBdelay(0.), // to be guessed
				       pedestal_length(100),fScale(-1.), // values fixed by DAQ
				       theAnodeBin(1), thePadBin(6),
				       fADCThres(adc), fPWBThres(pwb),
				       fADCpeak(aw),fPWBpeak(pad),
				       isalpha16(false),pmap()
{
   Setup();
}

Deconv::Deconv(std::string json):fTrace(false), fDiagnostic(false), fAged(false),
                                 fbinsize(1), fAWbinsize(16), fPADbinsize(16),
				 fADCdelay(0.),fPWBdelay(0.), // to be guessed
				 pedestal_length(100),fScale(-1.), // values fixed by DAQ
				 theAnodeBin(1), thePadBin(6),
				 isalpha16(false),pmap()
{
   ana_settings=new AnaSettings(json.c_str());
   fADCThres=ana_settings->GetDouble("DeconvModule","ADCthr");
   fPWBThres=ana_settings->GetDouble("DeconvModule","PWBthr");
   fADCpeak=ana_settings->GetDouble("DeconvModule","AWthr");
   fPWBpeak=ana_settings->GetDouble("DeconvModule","PADthr");
   Setup();
}

Deconv::Deconv(AnaSettings* s):fTrace(false), fDiagnostic(false), fAged(false),
                               ana_settings(s), fbinsize(1),
                               fAWbinsize(16), fPADbinsize(16),
                               fADCdelay(0.),fPWBdelay(0.), // to be guessed
                               pedestal_length(100),fScale(-1.), // values fixed by DAQ
                               theAnodeBin(1), thePadBin(6),
                               isalpha16(false),pmap()
{
   fADCThres=ana_settings->GetDouble("DeconvModule","ADCthr");
   fPWBThres=ana_settings->GetDouble("DeconvModule","PWBthr");
   fADCpeak=ana_settings->GetDouble("DeconvModule","AWthr");
   fPWBpeak=ana_settings->GetDouble("DeconvModule","PADthr");
   //   Setup();
}

Deconv::~Deconv()
{
   delete pmap;
}

void Deconv::Setup()
{
   SetupADCs(0);
   SetupPWBs(0);
}

void Deconv::SetupADCs(int run, bool norm, bool diag)
{
   fAnodeFactors = {
      0.1275,        // neighbour factor
      0.0365,        // 2nd neighbour factor
      0.012,         // 3rd neighbour factor
      0.0042         // 4th neighbour factor
   };

   fAwMask.reserve(256);

   fADCmax = pow(2.,14.);
   fADCrange = fADCmax*0.5-1.;

   int s = ReadAWResponseFile(fAWbinsize);
   std::cout<<"Deconv::SetupADCs() Response status: "<<s<<std::endl;
   assert(s>0);

   if( norm )
      {
         s = ReadADCRescaleFile();
         std::cout<<"Deconv::SetupADCs() Rescale status: "<<s<<std::endl;
         assert(s>0);
      }
   else
      fAdcRescale.assign(256,1.0);

   if( diag )
      {
         fDiagnostic = true;

         gDirectory->cd("/");
         gDirectory->mkdir("awdeconv")->cd();

         hAvgRMSBot = new TH1D("hAvgRMSBot","Average Deconv Remainder Bottom",1000,0.,50000.);
         hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",1000,0.,50000.);

         hADCped = new TH2D("hADCped","ADC pedestal per AW",256,0.,256.,1600,-16384.,16384.);
         hADCped_prox = new TProfile("hADCped_prox","Average ADC pedestal per AW;AW;ADC",
                                     256,0.,256.,16384.,16384.);
      }
   
   // by run settings
   if( run == 0 )
      {}
   else if( run < 2724 && run > 0 ) // new FMC-32
      {
         fAWbinsize=10;
         isalpha16=true;
      }
   else if( run >= 2282 && run < 2724 )
         fADCdelay = -120.;
   else if( run >= 2724 && run < 3032 ) // new FMC-32
         fADCdelay = 0.;
   else if( run >= 3032 )
         fADCdelay = -250.;

   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
       run == 3249 || run == 3250 || run == 3251 ||
       run == 3253 || run == 3254 || run == 3255 ||
       run == 3260 || run == 3263 || run == 3265 ||
       run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
         fADCdelay = -400.;
   if( run == 3170 || run == 3195 || run == 3190 ||
       run == 3187 || run == 3186 || run == 3184 ||
       run == 3181 || run == 3178 || run == 3208 ||
       run == 3210 || run == 3245 ||
       run == 3874 || run == 3858 ) // TrigCoinc
         fADCdelay = 0.;
}

void Deconv::SetupPWBs(int run, bool norm, bool diag)
{
   fPadSecMask.reserve(32);
   fPadRowMask.reserve(576);
 
   fPWBmax = pow(2.,12.);
   fPWBrange = fPWBmax*0.5-1.;

   int s = ReadPADResponseFile(fAWbinsize);
   std::cout<<"Deconv::SetupADCs() Response status: "<<s<<std::endl;
   assert(s>0);

   if( norm )
      {
         s = ReadPWBRescaleFile();
         std::cout<<"Deconv::SetupPWBs() Response status: "<<s<<std::endl;
         assert(s>0);
      }
   else
      fPwbRescale.assign(32*576,1.0);

   if( diag )
      {
         fDiagnostic = true;

         gDirectory->cd("/");
         gDirectory->mkdir("paddeconv")->cd();

         hAvgRMSPad = new TH1D("hAvgRMSPad","Average Deconv Remainder Pad",500,0.,5000.);

         hPWBped = new TH2D("hPWBped","PWB pedestal per Pad",32*576,0.,_padcol*_padrow,
                            1000,-4096.,4096.);
         hPWBped_prox = new TProfile("hPWBped_prox","Average PWB pedestal per Pad;Pad;PWB",
                                     32*576,0.,_padcol*_padrow,-4096.,4096.);

         // hPadOverflow = new TH2D("hPadOverflow","Distribution of Overflow Pads;row;sec;N",
         //                         576,0.,_padrow,32,0.,_padcol);
      }

   // by run settings
   if( run == 0 ) 
      fPWBdelay=52.;
   else if( run == 2246 || run == 2247 || run == 2248 || run == 2249 || run == 2251 )
      fPWBdelay = -50.;
   else if( run == 2272 || run ==  2273 || run == 2274 )
      fPWBdelay = 136.;
   else if( run >= 2282 && run < 2724 )
      fPWBdelay = 0.;
   else if( run >= 2724 && run < 3032 ) // new FMC-32
      fPWBdelay = 0.;
   else if( run >= 3032 )
      fPWBdelay = 0.;

   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
       run == 3249 || run == 3250 || run == 3251 ||
       run == 3253 || run == 3254 || run == 3255 ||
       run == 3260 || run == 3263 || run == 3265 ||
       run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
      
      fPWBdelay = -100.;

   if( run == 3170 || run == 3195 || run == 3190 ||
       run == 3187 || run == 3186 || run == 3184 ||
       run == 3181 || run == 3178 || run == 3208 ||
       run == 3210 || run == 3245 ||
       run == 3874 || run == 3858 ) // TrigCoinc
      fPWBdelay = 0.;
   
   // electrodes masking
   if( run == 0 )
      {  }
   else if( run == 2635 )
      {
         // only top aw are connected
         // numbering [256,511]
         fPadSecMask.push_back(0);
         for(int i=0; i<=10; ++i)
            {
               fPadRowMask.push_back(361+i);
            }
      }
   else if( run == 2605 || run == 2617 || run == 2638 )
      {
         fPadSecMask.push_back(18);
         fPadRowMask.push_back(215);
      }
   else if( run == 2731 || run == 2732 || run == 2734 || run == 2735 )
      {
         fPadSecMask.push_back(31);
         fPadRowMask.push_back(144);
         fPadRowMask.push_back(216);

         fPadSecMask.push_back(2);
         fPadRowMask.push_back(143);

         fPadSecMask.push_back(18);
         fPadRowMask.push_back(215);
      }
   else if( run == 3865 )
      {
         fPadSecMask.push_back(21);
         fPadRowMask.push_back(503);
         fPadRowMask.push_back(504);
         fPadRowMask.push_back(505);
      }
   // if( run >= 3003 )
   //    fAwMask.push_back(142+256);
   else if( run == 3873 || run == 3864 )
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
}

void Deconv::Reset()
{ 
   if(sanode)
      {
         sanode->clear(); 
         delete sanode;
      }
   if(spad)
      {
         spad->clear();
         delete spad;
      }
   fbinsize = 1;
   
   if( fDiagnostic )
      {

         if( fAdcPeaks )
            {
               fAdcPeaks->clear(); 
               delete fAdcPeaks;
            }
   
         if( fPwbPeaks )
            {
               fPwbPeaks->clear();
               delete fPwbPeaks;
            }
      }

   if( fAged )
      {
         if( wirewaveforms )
            {
               wirewaveforms->clear(); 
               delete wirewaveforms;
            }
         if( feamwaveforms )
            {
               feamwaveforms->clear();
               delete wirewaveforms;
            }
      }
}

int Deconv::FindAnodeTimes(TClonesArray* AWsignals)
{
   int Nentries = AWsignals->GetEntries();
   // prepare vector with wf to manipulate
   std::vector<wfholder*> AnodeWaves;
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
         //std::cout<<"Deconv::FindAnodeTimes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
         int aw_number = std::stoi( wname.substr(1) );
         electrode el(aw_number);
         //      std::cout<<"Deconv::FindAnodeTimes Electrode: "<<el.idx<<std::endl;

         // nothing dumb happens
         if( data.size() < 510 )
            {
               std::cerr<<"Deconv::FindAnodeTimes ERROR wf samples: "
                        <<data.size()<<std::endl;
               continue;
            }

         // mask hot wires
         if( MaskWires(aw_number) ) continue;

         double ped = CalculatePedestal(data);
         double peak_h = GetPeakHeight(data,el.idx,ped,true);

         // CREATE WAVEFORM
         wfholder* waveform=new wfholder( index, 
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
   std::cout<<"Deconv::FindAnodeTimes "<<nsig<<" found"<<std::endl;
   // ===========================================

   
   for (uint i=0; i<AnodeWaves.size(); i++)
      {
         //delete AnodeWaves.at(i)->h;
         delete AnodeWaves.at(i);
      }
   AnodeWaves.clear();
   
   return nsig;
}

int Deconv::FindPadTimes(TClonesArray* PADsignals)
{
   int Nentries = PADsignals->GetEntries();

   // prepare vector with wf to manipulate
   std::vector<wfholder*> PadWaves;
   PadWaves.reserve( Nentries );
   // clear/initialize "output" vectors
   fPadIndex.clear();
   fPadIndex.reserve( Nentries );

   std::string delimiter = "_";

   // find intresting channels
   unsigned int index=0; //wfholder index
   for( int j=0; j<Nentries; ++j )
      {
         TWaveform* w = (TWaveform*) PADsignals->ConstructedAt(j);
         std::vector<int> data(w->GetWaveform());
         std::string wname = w->GetElectrode();

         size_t pos = wname.find(delimiter);
         std::string p = wname.substr(0, pos);
         if( p != "p" )
            std::cerr<<"Deconv Error: Wrong Electrode? "<<p<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         short col = std::stoi( wname.substr(0, pos) );
         assert(col<32&&col>=0);
         //std::cout<<"Deconv::FindPadTimes() col: "<<col<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         int row = std::stoi( wname.substr(0, pos) );
         //std::cout<<"Deconv::FindPadTimes() row: "<<row<<std::endl;
         assert(row<576&&row>=0);

         int coli = int(col);
         int pad_index = pmap->index(coli,row);
         assert(!std::isnan(pad_index));
         // CREATE electrode
         electrode el(col,row);
  
         if( data.size() == 0 ) continue;

         // nothing dumb happens
         if( data.size() < 510 )
            {
               std::cerr<<"Deconv::FindPadTimes ERROR wf samples: "
                        <<data.size()<<std::endl;
               continue;
            }
         // mask hot pads
         if( MaskPads(col,row) ) continue;

         double ped = CalculatePedestal(data);
         double peak_h = GetPeakHeight(data,pad_index,ped,false);

         // CREATE WAVEFORM
         wfholder* waveform=new wfholder( index, 
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
   std::cout<<"Deconv::FindPadTimes "<<nsig<<" found"<<std::endl;
   // ===========================================   

   for (uint i=0; i<PadWaves.size(); i++)
      {
         // delete PadWaves.at(i)->h;
         delete PadWaves.at(i);
      }
   PadWaves.clear();
  
   return nsig;
}

int Deconv::FindAnodeTimes(const Alpha16Event* anodeSignals)
{
   std::vector<Alpha16Channel*> channels = anodeSignals->hits;
   // prepare vector with wf to manipulate
   std::vector<wfholder*> AnodeWaves;
   AnodeWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   fAnodeIndex.clear();
   fAnodeIndex.reserve( channels.size() );
   
   if( fDiagnostic ) 
      {
         fAdcPeaks = new std::vector<signal>;
         fAdcPeaks->reserve( channels.size() );
      }
   if( fAged ) 
      {
         wirewaveforms = new std::vector<wf_ref>;
         wirewaveforms->reserve( channels.size() );
      }

   // find intresting channels
   unsigned int index=0; //wfholder index
   for(unsigned int i = 0; i < channels.size(); ++i)
      {
         Alpha16Channel* ch = channels.at(i);
         if( ch->adc_chan < 16 && !isalpha16 ) continue; // it's bv

         int aw_number = ch->tpc_wire;
         // std::cout<<"DeconvAWModule::FindAnodeTimes anode wire: "<<aw_number<<std::endl;
         if( aw_number < 0 || aw_number >= 512 ) continue;
         // CREATE electrode
         electrode el(aw_number);
           
         // mask hot wires
         if( MaskWires(aw_number) ) continue;

         double ped = CalculatePedestal(ch->adc_samples);
         double peak_h = GetPeakHeight(ch->adc_samples,el.idx,ped,true);

         if( fDiagnostic )
            {
               double peak_t = GetPeakTime(ch->adc_samples,true);
               fAdcPeaks->emplace_back(el.idx,peak_t,peak_h,0.);
            }
            
         // CREATE WAVEFORM
         wfholder* waveform=new wfholder( index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end());

         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fADCThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" aw: "<<aw_number<<std::endl;
               
               // SUBTRACT PEDESTAL
               waveform->massage(ped,fAdcRescale.at(el.idx));

               // fill vector with wf to manipulate
               AnodeWaves.emplace_back( waveform );

               // STORE electrode
               fAnodeIndex.push_back( el );

               if( fAged )
                  wirewaveforms->emplace_back(el,new std::vector<double>(*waveform->h));

               ++index;
            }// max > thres
         else
            delete waveform;
      }// channels

   // ============== DECONVOLUTION ==============
   sanode = Deconvolution(&AnodeWaves,fAnodeIndex,fAnodeResponse,theAnodeBin, true);
   int nsig=-1;
   if(!sanode) nsig=0;
   else nsig = sanode->size();
   std::cout<<"Deconv::FindAnodeTimes "<<nsig<<" found"<<std::endl;
   // ===========================================

   if( fDiagnostic )
   {
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
         //delete AnodeWaves.at(i)->h;
         delete AnodeWaves.at(i);
      }
   AnodeWaves.clear();
   return nsig;
}

int Deconv::FindPadTimes(const FeamEvent* padSignals)
{
   std::vector<FeamChannel*> channels = padSignals->hits;
   // prepare vector with wf to manipulate
   std::vector<wfholder*> PadWaves;
   PadWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   fPadIndex.clear();
   fPadIndex.reserve( channels.size() );

   if( fDiagnostic ) 
      {
         fPwbPeaks = new std::vector<signal>;
         fPwbPeaks->reserve( channels.size() );
      }
   if( fAged ) 
      {
         feamwaveforms = new std::vector<wf_ref>;
         feamwaveforms->reserve( channels.size() );
      }

   // find intresting channels
   unsigned int index=0; //wfholder index
   for(unsigned int i = 0; i < channels.size(); ++i)
      {
         FeamChannel* ch = channels.at(i);
         if( !(ch->sca_chan>0) ) continue;
         short col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
         col+=1;
         if( col == 32 ) col = 0;
         assert(col<32&&col>=0);
         //std::cout<<"Deconv::FindPadTimes() col: "<<col;
         int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
         //std::cout<<" row: "<<row;
         assert(row<576&&row>=0);
         int pad_index = pmap->index(col,row);
         assert(!std::isnan(pad_index));
         //std::cout<<" index: "<<pad_index<<std::endl;
         
         // CREATE electrode
         electrode el(col,row);

         // mask hot pads
         if( MaskPads(col,row) ) continue;

         if( ch->adc_samples.size() < 510 )
            {
               std::cerr<<"Deconv::FindPadTimes ch: "<<i<<"\tpad: "<<pad_index
                        <<"\tERROR # of adc samples = "<<ch->adc_samples.size()
                        <<std::endl;
               continue;
            }

         double ped = CalculatePedestal(ch->adc_samples);
         double peak_h = GetPeakHeight(ch->adc_samples,pad_index,ped,false);
          
         if( fDiagnostic )
            {
               double peak_t = GetPeakTime(ch->adc_samples,false);
               fPwbPeaks->emplace_back(el,peak_t,peak_h,0.,false);
            }
            
         // CREATE WAVEFORM
         wfholder* waveform=new wfholder( index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end());
         
         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fPWBThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" index: "<<index<<std::endl;

               // SUBTRACT PEDESTAL
               waveform->massage(ped,fPwbRescale.at(pad_index));

               // fill vector with wf to manipulate
               PadWaves.emplace_back( waveform );
                  
               // STORE electrode
               fPadIndex.push_back( el );     

               if( fAged )
                  feamwaveforms->emplace_back(el,new std::vector<double>(*waveform->h));

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
   std::cout<<"Deconv::FindPadTimes "<<nsig<<" found"<<std::endl;
   // ===========================================   

   if( fDiagnostic )
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
         // delete PadWaves.at(i)->h;
         delete PadWaves.at(i);
      }
   PadWaves.clear();
   return nsig;
}

std::vector<signal>* Deconv::Deconvolution( std::vector<wfholder*>* subtracted,
                                            std::vector<electrode> &fElectrodeIndex,
                                            std::vector<double> &fResponse, int theBin, bool isanode)
{

   if(subtracted->size()==0) return 0;
   int nsamples = subtracted->back()->h->size();
   std::vector<signal>* fSignals=new std::vector<signal>;
   fSignals->reserve(nsamples-theBin);
   assert(nsamples < 1000);
   if( fTrace )
      std::cout<<"Deconv::Deconvolution Subtracted Size: "<<subtracted->size()
               <<"\t# samples: "<<nsamples<<"\ttheBin: "<<theBin<<std::endl;

   double t_delay = fPWBdelay;
   int fbinsize = fPADbinsize;
   double fAvalancheSize = fPWBpeak;
   if( isanode )
      {
         t_delay = fADCdelay;
         fbinsize = fAWbinsize;
         fAvalancheSize = fADCpeak;
      }

   // if( fTrace )
   //    std::cout<<"Deconv::Deconvolution delay: "<<t_delay<<" ns"<<std::endl;

   for(int b = theBin; b < int(nsamples); ++b)// b is the current bin of interest
      {
         // For each bin, order waveforms by size,
         // i.e., start working on largest first
         std::vector<wfholder*>* histset = wforder( subtracted, b );
         // std::cout<<"Deconv::Deconvolution bin of interest: "<<b
         //          <<" workable wf: "<<histset.size()<<std::endl;
         // this is useful to split deconv into the "Subtract" method
         // map ordered wf to corresponding electrode
         double neTotal = 0.0;
         for (auto const it : *histset)
            {
               unsigned int i = it->index;
               std::vector<double>* wf=it->h;
               electrode anElectrode = fElectrodeIndex.at( i );
               // number of "electrons"
               double ne = anElectrode.gain * fScale * wf->at(b) / fResponse[theBin];
               if( ne >= fAvalancheSize )
                  {
                     neTotal += ne;
                     // loop over all bins for subtraction
                     if( isanode ) 
                        SubtractAW(it,subtracted,b,ne,fElectrodeIndex,fResponse,theBin);
                     else
                        SubtractPAD(it,subtracted,b,ne,fElectrodeIndex,fResponse,theBin);
                     if(b-theBin >= 0)
                        {
                           // time in ns of the bin b centre
                           double t = ( double(b-theBin) + 0.5 ) * double(fbinsize) + t_delay;
                           fSignals->emplace_back(anElectrode,t,ne,GetNeErr(ne,it->val),isanode);
                        }
                  }// if deconvolution threshold Avalanche Size
            }// loop set of ordered waveforms
         delete histset;
         //delete histmap;
      }// loop bin of interest
   return fSignals;
}

void Deconv::SubtractAW(wfholder* hist1,
                        std::vector<wfholder*>* wfmap,
                        const int b,
                        const double ne,std::vector<electrode> &fElectrodeIndex,
                        std::vector<double> &fResponse, int theBin)
{
   std::vector<double> *wf1 = hist1->h;
   int wf1size=wf1->size();
   unsigned int i1 = hist1->index;
   electrode wire1 = fElectrodeIndex.at( i1 ); // mis-name for pads

   uint AnodeSize=fAnodeFactors.size();
   uint ElectrodeSize=fElectrodeIndex.size();
   int AnodeResponseSize=(int)fAnodeResponse.size();
   int respsize=fResponse.size();
   for(unsigned int k = 0; k < ElectrodeSize; ++k)
      {
         electrode wire2 = fElectrodeIndex.at( k );
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

void Deconv::SubtractPAD(wfholder* hist1,
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

std::vector<wfholder*>* Deconv::wforder(std::vector<wfholder*>* subtracted, const int b)
{
   // For each bin, order waveforms by size,
   // i.e., start working on largest first
   std::vector<wfholder*>* histset=new std::vector<wfholder*>;
   unsigned int size=subtracted->size();
   //   std::cout<<"Deconv::wforder subtracted size: "<<size<<" @ bin = "<<b<<std::endl;
   histset->reserve(size);
   for(unsigned int i=0; i<size;++i)
      {
         //         std::cout<<"wf# "<<i;
         wfholder* mh=subtracted->at(i);
         // std::cout<<"\twf index: "<<mh->index;
         // std::cout<<"\twf size: "<<mh->h->size();
         //std::cout<<"\twf bin: "<<b<<std::endl;
         mh->val = fScale*mh->h->at(b);
         //         std::cout<<"\twf val: "<<mh->val<<std::endl;
         histset->push_back(mh);
      }
   std::sort(histset->begin(), histset->end(),wf_comparator);
   return histset;
}

std::map<int,wfholder*>* Deconv::wfordermap(std::vector<wfholder*>* histset,std::vector<electrode> &fElectrodeIndex)
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

int Deconv::ReadResponseFile(const double awbin, const double padbin)
{
   return ReadAWResponseFile(awbin) && ReadPADResponseFile( padbin );
}

int Deconv::ReadAWResponseFile( const double awbin )
{
   std::string filename(getenv("AGRELEASE"));
   filename+="/ana/anodeResponse.dat";
   std::cout << "Deconv:: Reading in response file (anode) " << filename << std::endl;
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
      std::cout<<"Deconv::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theAnodeBin<<std::endl;
    
   return int(fAnodeResponse.size());
}

int Deconv::ReadPADResponseFile( const double padbin )
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

   return int(fPadResponse.size());
}

std::vector<double> Deconv::Rebin(const std::vector<double> &in, int binsize, double ped)
{
   if( fTrace )
      std::cout<<"Deconv::Rebin "<<binsize<<std::endl;
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
            std::cout << "Deconv::Rebin: Cannot rebin without rest, dropping final "
                      << in.size() % result.size() << std::endl;
      }
   return result;
}

int Deconv::ReadRescaleFile()
{
   return ReadADCRescaleFile()&&ReadPWBRescaleFile();
}

int Deconv::ReadADCRescaleFile()
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

int Deconv::ReadPWBRescaleFile()
{
   std::string basepath(getenv("AGRELEASE"));
   std::ifstream fpwbres(basepath+"/ana/PwbRescale.dat");
   double rescale_factor;
   while(1)
      {
         fpwbres>>rescale_factor;
         if( !fpwbres.good() ) break;
         fPwbRescale.push_back(rescale_factor);
      }
   fpwbres.close();

   if( fPwbRescale.size() == 32*576 )
      std::cout<<"DeconvAWModule BeginRun PWB rescaling factors OK"<<std::endl;
   else
      std::cout<<"DeconvAWModule BeginRun PWB rescaling factors NOT ok (size: "
               <<fPwbRescale.size()<<")"<<std::endl;
   return int(fPwbRescale.size());
}

void Deconv::AWdiagnostic()
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
   //std::cout<<"DeconvAWModule::AWdiagnostic() RMS bot: "<<rbot<<" el: "<<mbot;
   if( mbot!=0.) rbot /= mbot;
   //std::cout<<" avg RMS: "<<rbot<<std::endl;
   hAvgRMSBot->Fill(rbot);
   //std::cout<<"DeconvAWModule::AWdiagnostic() RMS top: "<<rtop<<" el: "<<mtop<<std::endl;
   if( mtop!=0.) rtop /= mtop;
   //std::cout<<" avg RMS: "<<rtop<<std::endl;
   hAvgRMSTop->Fill(rtop);
}

void Deconv::PADdiagnostic()
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

void Deconv::PrintADCsettings()
{
   std::cout<<"-------------------------"<<std::endl;
   std::cout<<"Deconv AW Settings"<<std::endl;
   std::cout<<" is ADC16: "<<isalpha16<<std::endl;
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
}

void Deconv::PrintPWBsettings()
{
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
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
