#include "Deconv.hh"
#include <fstream>
#include <numeric>
#include <cmath>

#include <TDirectory.h>

TH2D* DeconvWire::hADCped=0;
TProfile* DeconvWire::hADCped_prox=0;
TH1D* DeconvWire::hAvgRMSTop=0;
TH2D* DeconvPad::hPWBped=0;
TProfile* DeconvPad::hPWBped_prox=0;
TH1D* DeconvPad::hAvgRMSPad=0;

std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped) {
	if(binsize == 1) {
	   return in;
	}
	std::vector<double> result;
	result.reserve(in.size()/binsize);
	for(unsigned int i = 0; i < in.size(); i++) {
	   if(i / binsize == result.size()-1) {
		   result.back() += double(in[i])-ped;
	   }
	   else if(i / binsize == result.size()){
		   result.push_back(double(in[i])-ped);
	   }
	}
	if(result.size()*binsize > in.size()) {
	   result.pop_back();
	}
	return result;
}

uint8_t IsAnodeClose(int w1, int w2)
{
  uint8_t c=w1-w2;
  return Min(c,256-c);
}

uint8_t Min(uint8_t x, uint8_t y)
{
  return (x < y)? x : y;
}

bool IsAnodeNeighbour(int w1, int w2, int dist)
{
  uint8_t c=w1-w2;
  return (Min(c,256-c)==dist);
}

Deconv::Deconv(AnaSettings* s, bool diagn) : fTrace(false), fAged(false), ana_settings(s), 
	fBinSize(16), fDelay(0), pedestal_length(100), fScale(-1), fDiagnostic(diagn),
	isalpha16(false) {
}

void Deconv::ReadResponseFile(const int bin, const int scale, const std::string f_name) {
	std::string filename(std::string(getenv("AGRELEASE")) + "/ana/" + f_name);

	std::ifstream respFile(filename);
	if(respFile.good()) {
		double binedge, val;
		std::vector<double> resp;
		while(true) {
			respFile >> binedge >> val;
			//Why isn't the last read pushed?
			if(!respFile.good()) {
				break;
			}
			resp.push_back(scale * val);
		}
		fResponse = Rebin(resp, bin);

		double frac = 0.1;
		double max = *std::max_element(fResponse.begin(), fResponse.end());
		double thres = frac * max;
		for(unsigned b = 0; b < fResponse.size(); ++b) {
			if(fResponse[b] > thres) {
				theBin = b;
			}
		}

		if(fTrace) {
			std::cout<<"Deconv::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theBin<<std::endl;
		}
	}
	return;
}

void Deconv::ReadRescaleFile(const std::string f_name) {
	std::string filename(std::string(getenv("AGRELEASE")) + "/ana/" + f_name);

	std::ifstream f_resc(filename);
	if(f_resc.good()) {
		double rescale_factor;
		while(true) {
			f_resc >> rescale_factor;
			if(!f_resc.good()) {
				break;
			}
			fRescale.push_back(rescale_factor);
		}
	}
	return;
}

double Deconv::CalculatePedestal(std::vector<int>& adc_samples)//
{
  double ped(0.);
  for(int b = 0; b < pedestal_length; b++) ped += double(adc_samples.at( b ));
  if( pedestal_length > 0 )
	 ped /= double(pedestal_length);
  return ped;
}

double Deconv::GetPeakHeight(std::vector<int>& adc_samples, int& i, double& ped)//
{
	auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
	double y=double(*minit);
	double amp = fScale * y;
	if(amp < fRange) {
	  return fRescale[i] * fScale * (y - ped);
	}else {
	  return fMax;
	}
}

double Deconv::GetPeakTime(std::vector<int>& adc_samples)//
{
  auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
  double peak_time = ( (double) std::distance(adc_samples.begin(),minit) + 0.5 );
  peak_time *= double(fBinSize);
  peak_time += fDelay;
  return peak_time;
}

std::vector<ALPHAg::signal>* Deconv::Deconvolution( std::vector<ALPHAg::wfholder*>* subtracted,
                                            bool isanode)
{
   if(subtracted->size()==0) return 0;
   size_t nsamples = subtracted->back()->h->size();
   std::vector<ALPHAg::signal>* fSignals=new std::vector<ALPHAg::signal>;
   assert(nsamples >= theBin);
   fSignals->reserve(nsamples-theBin);
   assert(nsamples < 1000);
   if( fTrace )
      std::cout<<"Deconv::Deconvolution Subtracted Size: "<<subtracted->size()
               <<"\t# samples: "<<nsamples<<"\ttheBin: "<<theBin<<std::endl;

   for(size_t b = theBin; b < nsamples; ++b)// b is the current bin of interest
      {
         std::vector<ALPHAg::wfholder*>* histset = wforder( subtracted, b );
         double neTotal = 0.0;
         for (auto const it : *histset)
            {
               unsigned int i = it->index;
               std::vector<double>* wf=it->h;
               ALPHAg::electrode anElectrode = fIndex.at( i );
               double ne = anElectrode.gain * fScale * wf->at(b) / fResponse[theBin];
               if( ne >= fPeak )
                  {
                     neTotal += ne;
					 Subtract(it,subtracted,b,ne);
                     if( int(b-theBin) >= 0)
                        {
                           double t = ( double(b-theBin) + 0.5 ) * double(fBinSize) + fDelay;
                           fSignals->emplace_back(anElectrode,t,ne,sqrt(it->val),isanode);
                        }
                  }// if deconvolution threshold Avalanche Size
            }// loop set of ordered waveforms
         delete histset;
      }// loop bin of interest
   return fSignals;
}

std::vector<ALPHAg::wfholder*>* Deconv::wforder(std::vector<ALPHAg::wfholder*>* subtracted, const unsigned b)
{
   std::vector<ALPHAg::wfholder*>* histset=new std::vector<ALPHAg::wfholder*>;
   size_t size = subtracted->size();
   histset->reserve(size);
   for(unsigned int i=0; i<size;++i)
      {
         ALPHAg::wfholder* mh=subtracted->at(i);
         mh->val = fScale*mh->h->at(b);
         histset->push_back(mh);
      }
   std::sort(histset->begin(), histset->end(),wf_comparator);
   return histset;
}

DeconvPad::DeconvPad(AnaSettings* s, TFile* fout, int run, bool norm, bool diag)
   	: Deconv(s,diag), pmap() {
		theBin = 6;
		fThres = ana_settings->GetDouble("DeconvModule", "PWBthr");
		fPeak = ana_settings->GetDouble("DeconvModule", "PADthr");

		fMax = pow(2.,12.);
		fRange = fMax * 0.5 - 1;

		ReadResponseFile(fBinSize, 1, "padResponse_deconv.dat"); 
		assert(fResponse.size() > 0);

		if(norm) {
			ReadRescaleFile("PwbRescale.dat");
			assert(fRescale.size() > 0);
		}else {
			fRescale.assign(32 * 576,1.0);
		}

	   if( diag ){
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
			 }
		  }

	   if( run == 0 ) 
		  fDelay=52.;
	   else if( run == 2246 || run == 2247 || run == 2248 || run == 2249 || run == 2251 )
		  fDelay = -50.;
	   else if( run == 2272 || run ==  2273 || run == 2274 )
		  fDelay = 136.;
	   else if( run >= 3870 && run < 900000 )
		  fDelay = 0.;
	   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
		   run == 3249 || run == 3250 || run == 3251 ||
		   run == 3253 || run == 3254 || run == 3255 ||
		   run == 3260 || run == 3263 || run == 3265 ||
		   run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
		  fDelay = -100.;
	   if( run > 904100 && run <= 904400 )// fPWBdelay = -96.;
		  fDelay = -112.;
	   else if( run > 904400 ) fDelay = -64.;
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

int DeconvPad::FindTimes(const FeamEvent* padSignals)
{
   std::vector<FeamChannel*> channels = padSignals->hits;
   // prepare vector with wf to manipulate
   std::vector<ALPHAg::wfholder*> PadWaves;
   PadWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   fIndex.clear();
   fIndex.reserve( channels.size() );

   if( fDiagnostic ) 
      {
         fPeaks = new std::vector<ALPHAg::signal>;
         fPeaks->reserve( channels.size() );
      }
   if( fAged ) 
      {
         waveforms = new std::vector<ALPHAg::wf_ref>;
         waveforms->reserve( channels.size() );
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
               std::cout<<"Deconv::FindPadTimes() col: "<<col
                        <<" pwb column: "<<ch->pwb_column
                        <<" pad col: "<<ch->pad_col
                        <<" PAD SEC ERROR"<<std::endl;
            }
         int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
         if( row<0 || row>576 )
            {
               std::cout<<"Deconv::FindPadTimes() row: "<<row
                        <<" pwb ring: "<<ch->pwb_ring
                        <<" pad row: "<<ch->pad_row
                        <<" PAD ROW EROOR"<<std::endl;
            }         
         int pad_index = pmap->index(col,row);
         assert(!std::isnan(pad_index));
         if( pad_index < 0 || pad_index >= (int)ALPHAg::_padchan )
            {
               std::cout<<"Deconv::FindPadTimes() index: "<<pad_index
                        <<" col: "<<col
                        <<" row: "<<row
                        <<" PAD INDEX ERROR"<<std::endl;
               continue;
            }
         
         // CREATE electrode
         ALPHAg::electrode el(col,row);

         // mask hot pads
         if( MaskPads(col,row) ) 
            {
               std::cout<<"Deconv::FindPadTimes(const FeamEvent*) MaskPad sec: "<<col<<", row:"<<row<<std::endl;
               continue;
            }

         if( ch->adc_samples.size() < 510 )
            {
               std::cerr<<"Deconv::FindPadTimes ch: "<<i<<"\tpad: "<<pad_index
                        <<"\tERROR # of adc samples = "<<ch->adc_samples.size()
                        <<std::endl;
               continue;
            }

         double ped = CalculatePedestal(ch->adc_samples);
         double peak_h = GetPeakHeight(ch->adc_samples,pad_index,ped);
          
         if( fDiagnostic )
            {
               hPWBped->Fill(pad_index,ped);
               hPWBped_prox->Fill(pad_index,ped);
               double peak_t = GetPeakTime(ch->adc_samples);
               fPeaks->emplace_back(el,peak_t,peak_h,0.,false);
            }
            
         // CREATE WAVEFORM
         ALPHAg::wfholder* waveform=new ALPHAg::wfholder( index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end());
         
         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" index: "<<index<<" p.h.: "<<peak_h<<std::endl;

               // SUBTRACT PEDESTAL
               waveform->massage(ped,fRescale[pad_index]);

               // fill vector with wf to manipulate
               PadWaves.emplace_back( waveform );
                  
               // STORE electrode
               fIndex.push_back( el );     

               if( fAged )
                  waveforms->emplace_back(el,new std::vector<double>(*waveform->h));

               ++index;
            }// max > thres
         else
            delete waveform;
      }// channels

   // ============== DECONVOLUTION ==============
   fS = Deconvolution(&PadWaves,false);
   int nsig=-1;
   if(!fS) nsig=0;
   else nsig = fS->size();
   std::cout<<"Deconv::FindPadTimes "<<nsig<<" found"<<std::endl;
   // ===========================================   

   if( fDiagnostic )
      {
         double mr=0.,r=0.;
         for(auto s: PadWaves)
            {
               r+=sqrt( std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
                        / static_cast<double>(s->h->size()) );
               ++mr;
            }
         if( mr != 0. ) r /= mr;
         hAvgRMSPad->Fill(r);
      }

   for (uint i=0; i<PadWaves.size(); i++)
      {
         delete PadWaves[i];
      }
   PadWaves.clear();
   return nsig;
}

bool DeconvPad::MaskPads(short& sec, int& row)//In PAD
{
  for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
	 for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
		if( *it == sec && *jt == row ) return true;
  return false;
}

void DeconvPad::Subtract(ALPHAg::wfholder* hist1,std::vector<ALPHAg::wfholder*>* wfmap,
                         const unsigned b,const double ne)
{
   std::vector<double> *wf1 = hist1->h;
   int wf1size=wf1->size();
   unsigned int i1 = hist1->index;
   ALPHAg::electrode wire1 = fIndex[i1]; // mis-name for pads

   int respsize=fResponse.size();
     
     
   for(size_t bb = b - theBin; bb < wf1size; ++bb)
      {
         // the bin corresponding to bb in the response
         int respBin = int(bb - b + theBin);
         if( respBin < respsize && respBin >= 0 )
            {
               // Remove signal tail for waveform we're currently working on
               (*wf1)[bb] -= ne/fScale/wire1.gain*fResponse[respBin];
            }
      }// bin loop: subtraction
}

DeconvWire::DeconvWire(AnaSettings* s, TFile* fout, int run, bool norm, bool diag)
   	: Deconv(s, diag) {
		theBin = 1;
		fThres = ana_settings->GetDouble("DeconvModule", "ADCthr");
		fPeak = ana_settings->GetDouble("DeconvModule", "AWthr");

		fFactors = {0.1275, 0.0365, 0.012, 0.0042};

		fMax = pow(2.,14.);
		fRange = fMax * 0.5 - 1;

		ReadResponseFile(fBinSize, fScale, "anodeResponse.dat"); 
		assert(fResponse.size() > 0);

		if(norm) {
			ReadRescaleFile("AdcRescale.dat");
			assert(fRescale.size() > 0);
		}else {
			fRescale.assign(256,1.0);
		}

		if(diag) {
			if( fout ) {
				fout->cd();
				if( !gDirectory->cd("awdiag") )
					gDirectory->mkdir("awdiag")->cd();
				else
					gDirectory->cd("awdiag");
				if( !hAvgRMSTop )
					hAvgRMSTop = new TH1D("hAvgRMSTop","Average Deconv Remainder Top",1000,0.,50000.);
				if( !hADCped )
					hADCped = new TH2D("hADCped","ADC pedestal per AW",256,0.,256.,
									  1600,-16384.,16384.);
				if( !hADCped_prox )
					hADCped_prox = new TProfile("hADCped_prox","Average ADC pedestal per AW;AW;ADC",
											   256,0.,256.,16384.,16384.);
			}
		}

	   if( run == 0 ) {
			 std::cout<<"Deconv::SetupADCs() Monte Carlo settings"<<std::endl;
		  }
	   else if( run < 2724 && run > 0 ) {
			 fBinSize=10;
			 isalpha16=true;
		  }
	   else if( run >= 2282 && run < 2724 )
		  fDelay = -120.;
	   else if( run >= 2724 && run < 3032 ) // new FMC-32
		  fDelay = 0.;
	   else if( run >= 3032 && run < 4488 )
		  fDelay = -256.;
	   else if( run >= 4488 && run < 4590 )
		  fDelay = 0.;//fADCdelay = -304.;
	   else if( run >= 4590 && run < 900000 )
		  fDelay = -232.;
	   if( run == 3169 || run == 3209 || run == 3226 || run == 3241 ||
		   run == 3249 || run == 3250 || run == 3251 ||
		   run == 3253 || run == 3254 || run == 3255 ||
		   run == 3260 || run == 3263 || run == 3265 ||
		   run == 3875 || run == 3866 || run == 3859 || run == 3855) // TrigBscMult
			 fDelay = -400.;
	   if( run > 903837 && run < 904100 ) fDelay = -120.;
	   else if( run > 904100 ) fDelay = 0.;
}

int DeconvWire::FindTimes(const Alpha16Event* anodeSignals)
{
   std::vector<Alpha16Channel*> channels = anodeSignals->hits;
   // prepare vector with wf to manipulate
   std::vector<ALPHAg::wfholder*> AnodeWaves;
   AnodeWaves.reserve( channels.size() );
   // clear/initialize "output" vectors
   fIndex.clear();
   fIndex.reserve( channels.size() );
   
   if( fDiagnostic ) 
      {
         fPeaks = new std::vector<ALPHAg::signal>;
         fPeaks->reserve( channels.size() );
      }
   if( fAged ) 
      {
         waveforms = new std::vector<ALPHAg::wf_ref>;
         waveforms->reserve( channels.size() );
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
               fPeaks->emplace_back(el.idx,peak_t,peak_h,0.);
            }
         // CREATE WAVEFORM
         ALPHAg::wfholder* waveform=new ALPHAg::wfholder( index, 
                                          std::next(ch->adc_samples.begin(),pedestal_length),
                                          ch->adc_samples.end());
         // Signal amplitude < thres is considered uninteresting
         if(peak_h > fThres)
            {
               if(fTrace)
                  std::cout<<"\tsignal above threshold ch: "<<i<<" aw: "<<aw_number<<std::endl;
               // SUBTRACT PEDESTAL
               waveform->massage(ped,fRescale[el.idx]);
               // fill vector with wf to manipulate
               AnodeWaves.emplace_back( waveform );
               // STORE electrode
               fIndex.push_back( el );
               if( fAged )
                  waveforms->emplace_back(el,new std::vector<double>(*waveform->h));

               ++index;
            }// max > thres
         else
            delete waveform;
      }// channels
   // ============== DECONVOLUTION ==============
   fS = Deconvolution(&AnodeWaves, true);
   int nsig=-1;
   if(!fS) nsig=0;
   else nsig = fS->size();
   std::cout<<"Deconv::FindAnodeTimes "<<nsig<<" found"<<std::endl;
   // ===========================================
   if( fDiagnostic )
   {
      double mtop=0.,rtop=0.;
      for(auto s: AnodeWaves)
         {
            rtop += sqrt( std::inner_product(s->h->begin(), s->h->end(), s->h->begin(), 0.)
                                  / static_cast<double>(s->h->size()) );
            ++mtop;
         }
      if( mtop!=0.) rtop /= mtop;
      hAvgRMSTop->Fill(rtop);
   }
   
   for (uint i=0; i<AnodeWaves.size(); i++)
      {
         delete AnodeWaves[i];
      }
   AnodeWaves.clear();
   return nsig;
}

void DeconvWire::Subtract(ALPHAg::wfholder* hist1,std::vector<ALPHAg::wfholder*>* wfmap,
                        const unsigned b,const double ne)
{
   std::vector<double> *wf1 = hist1->h;
   size_t wf1size = wf1->size();
   unsigned int i1 = hist1->index;
   ALPHAg::electrode wire1 = fIndex.at( i1 ); // mis-name for pads

   uint AnodeSize=fFactors.size();
   uint ElectrodeSize=fIndex.size();
   int respsize=fResponse.size();
   for(unsigned int k = 0; k < ElectrodeSize; ++k)
      {
         ALPHAg::electrode wire2 = fIndex.at( k );
         //check for top/bottom
         if( wire2.sec != wire1.sec ) continue;
         //Skip early if wires not close...
         if (IsAnodeClose(wire1.idx,wire2.idx)>4) continue;
         std::vector<double>* wf2=wfmap->at(k)->h;
         for(unsigned int l = 0; l < AnodeSize; ++l)
            {
               if( !IsAnodeNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
               for(size_t bb = b - theBin; bb < wf1size; ++bb)
                  {
                     // the bin corresponding to bb in the response
                     int respBin = int (bb-b+theBin);
                     if (respBin<0) continue;
                     if (respBin >= respsize) continue;
                     if(respBin < respsize && respBin >= 0)
                        {
                           // remove neighbour induction
                           (*wf2)[bb] += ne/fScale/wire1.gain*fFactors[l]*fResponse[respBin];
                        }
                  }// loop over all bins for subtraction
            }// loop over factors
      }// loop all electrodes' signals looking for neighbours

   for(size_t bb = b - theBin; bb < wf1size; ++bb)
      {
         // the bin corresponding to bb in the response
         int respBin = int ( bb - b + theBin );
         if( respBin < respsize && respBin >= 0 )
            {
               // Remove signal tail for waveform we're currently working on
               (*wf1)[bb] -= ne/fScale/wire1.gain*fResponse[respBin];
            }
      }// bin loop: subtraction
}
