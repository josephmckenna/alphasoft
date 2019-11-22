//
// Analyze waveforms and extract
// Leading Edge information
// 
// Author: A. Capra
// Date: Nov. 2019
//

#include "Ledge.hh"

Ledge::Ledge():fBaseline(100),
	       fGain(1.),fOffset(0.0),
	       fCutBaselineRMS(500.),
	       fPulseHeightThreshold(750.),
	       fCFDfrac(0.6),fMaxTime(4500.)
{
  fBinSize = 1000.0/62.5;// 62.5 MHz ADC
  fTimeOffset=-double(fBaseline)*fBinSize;
}

int Ledge::FindAnodeTimes(const Alpha16Event* anodeSignals)
{
   std::vector<Alpha16Channel*> channels = anodeSignals->hits;
   fSignals->clear();
   fGain = 4.0/3.0; // fmc-adc32-rev1 with gain 3; 
   for(unsigned int i = 0; i < channels.size(); ++i) Analyze( channels.at(i) );
   return int(fSignals->size());
}

int Ledge::FindPadTimes(const FeamEvent* padSignals)
{
  std::vector<FeamChannel*> channels = padSignals->hits;
  fSignals->clear();
  fPulseHeightThreshold = 100.;
  for(unsigned int i = 0; i < channels.size(); ++i) Analyze( channels.at(i) );
  return int(fSignals->size());
}

int Ledge::Analyze(const std::vector<int>* wf, double& time, double& amp, double& err)
{
  double bmean,brms;
  ComputeMeanRMS(wf->begin(), 
		 wf->begin()+fBaseline,
		 bmean,brms);
  if(brms > fCutBaselineRMS) return -2;
  
  double wmin = *std::min_element(wf->begin(), wf->end());
  double ph = bmean - wmin;
  if( ph < fPulseHeightThreshold ) return -1;
  
  double cfd_thr = fCFDfrac*ph;
  double le = find_pulse_time(wf->data(), wf->size(), 
			      bmean, -1.0, cfd_thr);
  
  time = le * fBinSize + fTimeOffset;
  if( time < 0. || time > fMaxTime ) return 0;
  amp = ph * fGain + fOffset;
  err = brms;
  return 1;
}

void Ledge::Analyze(const Alpha16Channel* ch)
{
  if( ch->adc_chan < 16 ) return; // it's bv
  int iwire = ch->tpc_wire;
  if( iwire < 0 ) return;
  electrode elec(iwire);
  double time, amp, err;
  if( Analyze(&ch->adc_samples,time, amp, err ) > 0 )
    fSignals->emplace_back( elec, time, amp, err, true );
}

void Ledge::Analyze(const FeamChannel* ch)
{
  if( !(ch->sca_chan>0) ) return;
  short col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
  col+=1;
  if( col == 32 ) col = 0;
  assert(col<32&&col>=0);
  //std::cout<<"Ledge::Analyze(FeamChannel) col: "<<col;
  int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
  //std::cout<<" row: "<<row;
  assert(row<576&&row>=0);
  // int pad_index = pmap->index(col,row);
  // assert(!std::isnan(pad_index));
  //std::cout<<" index: "<<pad_index<<std::endl;
  
  // CREATE electrode
  electrode elec(col,row);
  double time, amp, err;
  if( Analyze(&ch->adc_samples,time, amp, err ) > 0 )
    fSignals->emplace_back( elec, time, amp, err, false );
}
