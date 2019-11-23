//
// Analyze waveforms and extract
// Leading Edge information
// 
// Author: A. Capra
// Date: Nov. 2019
//

#include "Ledge.hh"
#include "TPCconstants.hh"

int Ledge::FindAnodeTimes(const Alpha16Event* anodeSignals)
{
  fSignals = Analyze( anodeSignals->hits );
  return int(fSignals->size());
}

int Ledge::FindPadTimes(const FeamEvent* padSignals)
{  
  fSignals = Analyze( padSignals->hits );
  return int(fSignals->size());
}

int Ledge::Analyze(const std::vector<int>* wf, double& time, double& amp, double& err)
{
  double bmean,brms;
  ComputeMeanRMS(wf->begin(), 
		 wf->begin()+fBaseline,
		 bmean,brms);
  if(fDebug) std::cout<<"Ledge::Analyze WF baseline mean: "<<bmean<<" rms: "<<brms<<std::endl;
  if(brms > fCutBaselineRMS) return -2;
  
  double wmin = *std::min_element(wf->begin(), wf->end());
  double ph = fabs( bmean - wmin );
  if( ph < fPulseHeightThreshold ) return -1;
  
  double cfd_thr = fCFDfrac*ph;
  double le = FindLeadingEdge(wf->begin()+fBaseline, wf->end(), bmean, cfd_thr);
  time = le * fBinSize + fTimeOffset;
  if( time < 0. || time > fMaxTime ) return 0;
  amp = ph * fGain + fOffset;
  err = brms;
  return 1;
}

std::vector<signal>* Ledge::Analyze(std::vector<Alpha16Channel*> channels)
{
  std::vector<signal>* sanodes = new std::vector<signal>;
  sanodes->reserve(channels.size());
  for(unsigned int i = 0; i < channels.size(); ++i)
    {
      const Alpha16Channel* ch = channels.at(i);
      if( ch->adc_chan < 16 ) continue; // it's bv
      int iwire = ch->tpc_wire;
      if( iwire < 0 ) continue;
      electrode elec(iwire);
      double time, amp, err;
      int status = Analyze(&ch->adc_samples,time, amp, err );
      if(fDebug) std::cout<<"Ledge::Analyze Alpha16Channel status: "<<status<<std::endl;
      if( status > 0 )
	{
	  if(fDebug)
	    {
	      elec.print();
	      std::cout<<"t: "<<time<<" A: "<<amp<<" E: "<<err<<std::endl;
	    }
	  sanodes->emplace_back( elec, time, amp, err, true );
	}
    }
  return sanodes;
}

std::vector<signal>* Ledge::Analyze(std::vector<FeamChannel*> channels)
{
  std::vector<signal>* spads = new std::vector<signal>;
  spads->reserve(channels.size());
  for(unsigned int i = 0; i < channels.size(); ++i)
    {
      const FeamChannel* ch = channels.at(i);
      if( !(ch->sca_chan>0) ) continue;

      short col = ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col;
      col+=1;
      if( col == 32 ) col = 0;
      if(fDebug) std::cout<<"Ledge::Analyze(FeamChannel) col: "<<col;
      if( col>=32 || col<0 ) continue;

      int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
      if(fDebug) std::cout<<" row: "<<row<<std::endl;
      if( row>=576 || row<0 ) continue;
  
      // CREATE electrode
      electrode elec(col,row);
      double time, amp, err;
      int status = Analyze(&ch->adc_samples,time, amp, err );
      if(fDebug) std::cout<<"Ledge::Analyze FeamChannel status: "<<status<<std::endl;
      if( status > 0 )
	{
	   if(fDebug)
	     {
	       elec.print();
	       std::cout<<"t: "<<time<<" A: "<<amp<<" E: "<<err<<std::endl;
	     }
	  spads->emplace_back( elec, time, amp, err, false );
	}
    }
  return spads;
}
