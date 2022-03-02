#include <fstream>
#include <numeric>
#include <cmath>

#include <TDirectory.h>

#include "DeconvPAD.h"

#ifdef BUILD_AG_SIM
#include "TWaveform.hh"
#endif

TH2D* DeconvPAD::hPWBped=0;
TProfile* DeconvPAD::hPWBped_prox=0;

// pads
TH1D* DeconvPAD::hAvgRMSPad=0;
// anodes
TH1D* DeconvPAD::hAvgRMSTop=0;

DeconvPAD::DeconvPAD(double adc, double pwb,
	       double aw, double pad): fTrace(false), fDiagnostic(false), fAged(false),
                                     fPADbinsize(16),
				       fPWBdelay(0.), // to be guessed
				       pedestal_length(100),fScale(-1.), // values fixed by DAQ
				       thePadBin(6),
				       fPWBThres(pwb),
				       fPWBpeak(pad),
				       isalpha16(false),pmap(),
                                      fPWBmax(pow(2.,12.)),
                                      fPWBrange(fPWBmax*0.5-1.)
{
   Setup();
}

DeconvPAD::DeconvPAD(std::string json):fTrace(false), fDiagnostic(false), fAged(false),
                                fPADbinsize(16),
				 fPWBdelay(0.), // to be guessed
				 pedestal_length(100),fScale(-1.), // values fixed by DAQ
				 thePadBin(6),
				 isalpha16(false),
                                pmap(),
                                fPWBmax(pow(2.,12.)),
                                fPWBrange(fPWBmax*0.5-1.),
                                ana_settings(new AnaSettings(json.c_str())),
                                fPWBThres(ana_settings->GetDouble("DeconvModule","PWBthr")),
                                fPWBpeak(ana_settings->GetDouble("DeconvModule","PADthr"))
{
   Setup();
}

DeconvPAD::DeconvPAD(AnaSettings* s):fTrace(false), fDiagnostic(false), fAged(false),
                               ana_settings(s), 
                               fPADbinsize(16),
                               fPWBdelay(0.), // to be guessed
                               pedestal_length(100),fScale(-1.), // values fixed by DAQ
                               thePadBin(6),
                               isalpha16(false),pmap(),
                               fPWBmax(pow(2.,12.)),
                               fPWBrange(fPWBmax*0.5-1.),
                               fPWBThres(ana_settings->GetDouble("DeconvModule","PWBthr")),
                               fPWBpeak(ana_settings->GetDouble("DeconvModule","PADthr"))
{
   //   Setup();
}

DeconvPAD::~DeconvPAD()
{
   delete pmap;
}

void DeconvPAD::Setup()
{
   SetupPWBs(0,0);
}

void DeconvPAD::SetupPWBs(TFile* fout, int run, bool norm, bool diag)
{
   fPadSecMask.reserve(32);
   fPadRowMask.reserve(576);
 

   int s = ReadPADResponseFile(fPADbinsize);
   std::cout<<"Deconv::SetupADCs() Response status: "<<s<<std::endl;
   assert(s>0);
   if( norm )
      {
         s = ReadPWBRescaleFile();
         std::cout<<"DeconvPAD::SetupPWBs() Response status: "<<s<<std::endl;
         assert(s>0);
      }
   else
      fPwbRescale.assign(32*576,1.0);

   if( diag )
      {
         fDiagnostic = true;
         if(fout) {
            fout->cd();
            if( !gDirectory->cd("paddiag") )
               gDirectory->mkdir("paddiag")->cd();
            else
               gDirectory->cd("paddiag");

            if( !hAvgRMSPad )
               hAvgRMSPad = new TH1D("hAvgRMSPad","Average Deconv Remainder Pad",500,0.,5000.);
            
            if( !hPWBped )
               hPWBped = new TH2D("hPWBped","PWB pedestal per Pad",
                               32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,
                                  1000,-4096.,4096.);
            
            if( !hPWBped_prox )
               hPWBped_prox = new TProfile("hPWBped_prox","Average PWB pedestal per Pad;Pad;PWB",
                                        32*576,0.,ALPHAg::_padcol*ALPHAg::_padrow,-4096.,4096.);
            
            // hPadOverflow = new TH2D("hPadOverflow","Distribution of Overflow Pads;row;sec;N",
            //                         576,0.,_padrow,32,0.,_padcol);
         }
      }

   // by run settings
   if( run == 0 ) 
      fPWBdelay=52.;
   else if( run == 2246 || run == 2247 || run == 2248 || run == 2249 || run == 2251 )
      fPWBdelay = -50.;
   else if( run == 2272 || run ==  2273 || run == 2274 )
      fPWBdelay = 136.;
   else if( run >= 3870 && run < 900000 )
      fPWBdelay = 0.;

      

   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
       run == 3249 || run == 3250 || run == 3251 ||
       run == 3253 || run == 3254 || run == 3255 ||
       run == 3260 || run == 3263 || run == 3265 ||
       run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
      fPWBdelay = -100.;

   if( run > 904100 && run <= 904400 )// fPWBdelay = -96.;
      fPWBdelay = -112.;
   else if( run > 904400 ) fPWBdelay = -64.;
   
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
   else if( run > 903862 )
      {
         fPadSecMask.push_back(11);
         fPadRowMask.push_back(324);
         fPadRowMask.push_back(325);
         fPadRowMask.push_back(326);
         fPadRowMask.push_back(337);
      }
   if( run == 903941 )
      {
         fPadSecMask.push_back(17);
         fPadSecMask.push_back(18);
         for(int x=432; x<468; ++x) fPadRowMask.push_back(x);
      }
}

void DeconvPAD::Reset()
{ 

}

#ifdef BUILD_AG_SIM
int DeconvPAD::FindAnodeTimes(TClonesArray* AWsignals)
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
         //std::cout<<"DeconvPAD::FindAnodeTimes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
         int aw_number = std::stoi( wname.substr(1) );
         ALPHAg::electrode el(aw_number);
         //std::cout<<"DeconvPAD::FindAnodeTimes Electrode: "<<el.idx<<std::endl;

         // nothing dumb happens
         if( (int)data.size() < 410 + pedestal_length )
            {
               std::cerr<<"DeconvPAD::FindAnodeTimes ERROR wf samples: "
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
   std::cout<<"DeconvPAD::FindAnodeTimes "<<nsig<<" found"<<std::endl;
   // ===========================================

   
   for (uint i=0; i<AnodeWaves.size(); i++)
      {
         //delete AnodeWaves.at(i)->h;
         delete AnodeWaves.at(i);
      }
   AnodeWaves.clear();
   
   return nsig;
}

int DeconvPAD::FindPadTimes(TClonesArray* PADsignals)
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
            std::cerr<<"Deconv Error: Wrong Electrode? "<<p<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         short col = std::stoi( wname.substr(0, pos) );
         assert(col<32&&col>=0);
         //std::cout<<"DeconvPAD::FindPadTimes() col: "<<col<<std::endl;
         wname = wname.erase(0, pos + delimiter.length());

         pos = wname.find(delimiter);
         int row = std::stoi( wname.substr(0, pos) );
         //std::cout<<"DeconvPAD::FindPadTimes() row: "<<row<<std::endl;
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
               std::cerr<<"DeconvPAD::FindPadTimes ERROR wf samples: "
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
   std::cout<<"DeconvPAD::FindPadTimes "<<nsig<<" found"<<std::endl;
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

void DeconvPAD::BuildWFContainer(
    const FeamEvent* padSignals,
    std::vector<ALPHAg::wfholder>& PadWaves,
    std::vector<ALPHAg::electrode>& PadIndex,
    std::vector<ALPHAg::wf_ref>& feamwaveforms,  // to use in aged display
    std::vector<ALPHAg::TPadSignal>& PwbPeaks // waveform max
) const
{
   const std::vector<FeamChannel*> channels = padSignals->hits;
   // prepare vector with wf to manipulate
   PadWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   PadIndex.clear();
   PadIndex.reserve( channels.size() );

   if( fDiagnostic ) 
   {
      PwbPeaks.reserve( channels.size() );
   }

   if( fAged ) 
   {
      feamwaveforms.reserve( channels.size() );
   }

   // find intresting channels
   unsigned int index=0; //wfholder index
   for(unsigned int i = 0; i < channels.size(); ++i)
      {
         FeamChannel* ch = channels[i];
         if( !(ch->sca_chan>0) ) continue;
         short col = (short) (ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col);
         col+=1;
         if( col == 32 ) col = 0;
         if( col<0 || col >=32 ) 
            {
               std::cout<<"DeconvPAD::FindPadTimes() col: "<<col
                        <<" pwb column: "<<ch->pwb_column
                        <<" pad col: "<<ch->pad_col
                        <<" PAD SEC ERROR"<<std::endl;
            }
         int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
         if( row<0 || row>576 )
            {
               std::cout<<"DeconvPAD::FindPadTimes() row: "<<row
                        <<" pwb ring: "<<ch->pwb_ring
                        <<" pad row: "<<ch->pad_row
                        <<" PAD ROW EROOR"<<std::endl;
            }         
         int pad_index = pmap->index(col,row);
         assert(!std::isnan(pad_index));
         if( pad_index < 0 || pad_index >= (int)ALPHAg::_padchan )
            {
               std::cout<<"DeconvPAD::FindPadTimes() index: "<<pad_index
                        <<" col: "<<col
                        <<" row: "<<row
                        <<" PAD INDEX ERROR"<<std::endl;
               continue;
            }

         // mask hot pads
         if( MaskPads(col,row) ) 
            {
               std::cout<<"DeconvPAD::FindPadTimes(const FeamEvent*) MaskPad sec: "<<col<<", row:"<<row<<std::endl;
               continue;
            }

         if( ch->adc_samples.size() < 510 )
            {
               std::cerr<<"DeconvPAD::FindPadTimes ch: "<<i<<"\tpad: "<<pad_index
                        <<"\tERROR # of adc samples = "<<ch->adc_samples.size()
                        <<std::endl;
               continue;
            }

         // CREATE electrode
         ALPHAg::electrode el(col,row);

         double ped = CalculatePedestal(ch->adc_samples);
         double peak_h = GetPeakHeight(ch->adc_samples,pad_index,ped);
          
         if( fDiagnostic )
            {
               hPWBped->Fill(pad_index,ped);
               hPWBped_prox->Fill(pad_index,ped);
               double peak_t = GetPeakTime(ch->adc_samples);
               PwbPeaks.emplace_back(el,peak_t,peak_h,0.);
            }
            

         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fPWBThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" index: "<<index<<" p.h.: "<<peak_h<<std::endl;

               // fill vector with wf to manipulate
               PadWaves.emplace_back( index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end());

               // CREATE WAVEFORM
               ALPHAg::wfholder& waveform = PadWaves.back();

               // SUBTRACT PEDESTAL
               waveform.massage(ped,fPwbRescale[pad_index]);

               // STORE electrode
               PadIndex.push_back( el );

               if( fAged )
                  feamwaveforms.emplace_back(el,new std::vector<double>(waveform.h));

               ++index;
            }// max > thres
      }// channels
   }

void DeconvPAD::Deconvolution(
    std::vector<ALPHAg::wfholder>& subtracted,
    const std::vector<ALPHAg::electrode> &fElectrodeIndex,
    std::vector<ALPHAg::TPadSignal>& signals) const
{
   if(subtracted.empty())
      return;

   size_t nsamples = subtracted.back().h.size();
   assert(nsamples >= thePadBin);
   
   signals.clear();
   signals.reserve(nsamples - thePadBin);
   assert(nsamples < 1000);
   if( fTrace )
      std::cout<<"DeconvPAD::Deconvolution Subtracted Size: "<<subtracted.size()
               <<"\t# samples: "<<nsamples<<"\ttheBin: "<<thePadBin<<std::endl;

   return Deconvolution( subtracted, fElectrodeIndex, signals, thePadBin,nsamples);
}

void DeconvPAD::Deconvolution(
    std::vector<ALPHAg::wfholder>& subtracted,
    const std::vector<ALPHAg::electrode> &fElectrodeIndex,
    std::vector<ALPHAg::TPadSignal>& signals, const int start, const int stop) const
{

   for(size_t b = start; b < stop; ++b)// b is the current bin of interest
      {
         // For each bin, order waveforms by size,
         // i.e., start working on largest first
         const std::vector<ALPHAg::wfholder*> histset = wforder( subtracted, b );
         // std::cout<<"DeconvPAD::Deconvolution bin of interest: "<<b
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
               const double ne = anElectrode.gain * fScale * wf[b] / fPadResponse[thePadBin];
               if( ne >= fPWBpeak )
                  {
                     neTotal += ne;
                     // loop over all bins for subtraction
                     SubtractPAD(it,b,ne,fElectrodeIndex);
                     if( int( b - thePadBin) >= 0)
                        {
                           // time in ns of the bin b centre
                           const double t = ( double(b - thePadBin) + 0.5 ) * double(fPADbinsize) + fPWBdelay;
                           signals.emplace_back(anElectrode,t,ne,GetNeErr(ne,it->val));
                        }
                  }// if deconvolution threshold Avalanche Size
            }// loop set of ordered waveforms
      }// loop bin of interest
   return;
}

void DeconvPAD::LogDeconvRemaineder( std::vector<ALPHAg::wfholder>& PadWaves )
{
    
   if( fDiagnostic )
   {
      // prepare control variable (deconv remainder) vector
      // resRMS_p.clear();
      // resRMS_p.reserve( PadWaves.size() );
      double mr=0.,r=0.;
      // calculate remainder of deconvolution
      for(const auto& s: PadWaves)
      {
         r+=sqrt( std::inner_product(s.h.begin(), s.h.end(), s.h.begin(), 0.)
                     / static_cast<double>(s.h.size()) );
         // resRMS_p.push_back( sqrt(
         //                          std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
         //                          / static_cast<double>(s->h->size()) )
         //                     );
         ++mr;
      }
      if( mr != 0. )
         r /= mr;
      hAvgRMSPad->Fill(r);
   }
}

void DeconvPAD::SubtractPAD(ALPHAg::wfholder* hist1,
                         const int b,
                         const double ne,
                         const std::vector<ALPHAg::electrode> &fElectrodeIndex) const
{
   const unsigned int i1 = hist1->index;
   const ALPHAg::electrode& wire1 = fElectrodeIndex[i1]; // mis-name for pads

   const int respsize = fPadResponse.size();

   const double collectivegain = ne/fScale/wire1.gain;

   std::vector<double> &wf1 = hist1->h;
   const int wf1size = wf1.size();
   
   const int start = b - thePadBin;
   const int range = std::min(wf1size - start, respsize );

   int respBin = 0;
   for(int bb = start; bb < start + range; ++bb)
      {
         // Remove signal tail for waveform we're currently working on
         wf1[bb] -= collectivegain * fPadResponse[respBin];
         ++respBin;
      }// bin loop: subtraction
}

std::vector<ALPHAg::wfholder*> DeconvPAD::wforder(std::vector<ALPHAg::wfholder> & subtracted, const unsigned b) const
{
   // For each bin, order waveforms by size,
   // i.e., start working on largest first

   const size_t size = subtracted.size();
   std::vector<ALPHAg::wfholder*> histset(size,nullptr);
   //   std::cout<<"DeconvPAD::wforder subtracted size: "<<size<<" @ bin = "<<b<<std::endl;
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


int DeconvPAD::ReadResponseFile(const int padbin)
{
   return ReadPADResponseFile( padbin );
}

int DeconvPAD::ReadPADResponseFile( const int padbin )
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
         if( fPadResponse[b] > thres )
            thePadBin=b;
      }
   if( fTrace )
      std::cout<<"DeconvPADModule::ReadResponseFile pad max: "<<max<<"\tpad bin: "<<thePadBin<<std::endl;

   return int(fPadResponse.size());
}

std::vector<double> DeconvPAD::Rebin(const std::vector<double> &in, int binsize, double ped)
{
   if( fTrace )
      std::cout<<"DeconvPAD::Rebin "<<binsize<<std::endl;
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
            std::cout << "DeconvPAD::Rebin: Cannot rebin without rest, dropping final "
                      << in.size() % result.size() << std::endl;
      }
   return result;
}

int DeconvPAD::ReadRescaleFile()
{
   return ReadPWBRescaleFile();
}

int DeconvPAD::ReadPWBRescaleFile()
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

void DeconvPAD::PrintPWBsettings()
{
   std::cout<<"-------------------------"<<std::endl;
   std::cout<<"Deconv Settings"<<std::endl;
   std::cout<<" PWB max: "<<fPWBmax<<std::endl;
   std::cout<<" PWB range: "<<fPWBrange<<std::endl;
   std::cout<<" PWB time bin: "<<fPADbinsize<<" ns"<<std::endl;
   std::cout<<" PWB delay: "<<fPWBdelay<<" ns"<<std::endl;
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
