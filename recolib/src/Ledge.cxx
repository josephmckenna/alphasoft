//
// Analyze waveforms and extract
// Leading Edge information
// 
// Author: A. Capra
// Date: Nov. 2019
//

#include "Ledge.hh"
#include "TPCconstants.hh"

#ifdef BUILD_AG_SIM
#include "TWaveform.hh"
#endif

int Ledge::FindAnodeTimes(const Alpha16Event* anodeSignals)
{
  fAnodeSignals = Analyze( anodeSignals->hits );
  return int(fAnodeSignals.size());
}

int Ledge::FindPadTimes(const FeamEvent* padSignals)
{  
  fPadSignals = Analyze( padSignals->hits );
  return int(fPadSignals.size());
}

int Ledge::Analyze(const std::vector<int>* wf, double& time, double& amp, double& err)
{
  double bmean=0.,brms=0.;
  ComputeMeanRMS(wf->begin(), 
		 wf->begin()+fBaseline,
		 bmean,brms);
  if(fDebug) std::cout<<"Ledge::Analyze WF baseline mean: "<<bmean<<" rms: "<<brms<<std::endl;
  if(brms > fCutBaselineRMS) return -2;
  
  double wmin = *std::min_element(wf->begin(), wf->end());
  double ph = fabs( bmean - wmin );
  if(fDebug) std::cout<<"Ledge::Analyze WF peak: "<<wmin<<" pulse height: "<<ph<<std::endl;
  if( ph < fPulseHeightThreshold ) return -1;
  
  double cfd_thr = fCFDfrac*ph;
  double le = FindLeadingEdge(wf->begin()+fBaseline, wf->end(), bmean, cfd_thr);
  time = le * fBinSize + fTimeOffset;
  if( time < 0. || time > fMaxTime ) return 0;
  amp = ph * fGain + fOffset;
  err = brms;
  return 1;
}

std::vector<ALPHAg::TWireSignal> Ledge::Analyze(std::vector<Alpha16Channel*> channels)
{
  std::vector<ALPHAg::TWireSignal> sanodes;
  sanodes.reserve(channels.size());
  for(unsigned int i = 0; i < channels.size(); ++i)
    {
      const Alpha16Channel* ch = channels.at(i);
      if( ch->adc_chan < 16 ) continue; // it's bv
      int iwire = ch->tpc_wire;
      if( iwire < 0 ) continue;
      ALPHAg::electrode elec(iwire);
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
	  sanodes.emplace_back( elec, time, amp, err );
	}
    }
  return sanodes;
}

std::vector<ALPHAg::TPadSignal> Ledge::Analyze(std::vector<FeamChannel*> channels)
{
  std::vector<ALPHAg::TPadSignal> spads;
  spads.reserve(channels.size());
  for(unsigned int i = 0; i < channels.size(); ++i)
    {
      const FeamChannel* ch = channels.at(i);
      if( !(ch->sca_chan>0) ) continue;

      short col = short( ch->pwb_column * MAX_FEAM_PAD_COL + ch->pad_col );
      col+=1;
      if( col == 32 ) col = 0;
      if(fDebug) std::cout<<"Ledge::Analyze(FeamChannel) col: "<<col;
      if( col>=32 || col<0 ) continue;

      int row = ch->pwb_ring * MAX_FEAM_PAD_ROWS + ch->pad_row;
      if(fDebug) std::cout<<" row: "<<row<<std::endl;
      if( row>=576 || row<0 ) continue;
  
      // CREATE electrode
      ALPHAg::electrode elec(col,row);
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
	  spads.emplace_back( elec, time, amp, err);
	}
    }
  return spads;
}

#ifdef BUILD_AG_SIM
int Ledge::FindAnodeTimes(TClonesArray* AWsignals)
{
  int Nentries = AWsignals->GetEntries();

  fAnodeSignals.clear();
  fAnodeSignals.reserve(Nentries);
   
  // find intresting channels
  //  unsigned int index=0; //wfholder index -- unused 14 Oct 2020  -- AC
  for( int j=0; j<Nentries; ++j )
    {
      TWaveform* w = (TWaveform*) AWsignals->ConstructedAt(j);
      std::vector<int> data(w->GetWaveform());
      std::string wname = w->GetElectrode();
      if(fDebug) std::cout<<"Ledge::FindAnodeTimes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
      double time, amp, err;
      int status = Analyze(&data, time, amp, err );

      int aw_number = std::stoi( wname.substr(1) );
      ALPHAg::electrode el(aw_number);
      if(fDebug) std::cout<<"Ledge::Analyze AWsignals status: "<<status<<std::endl;
      if( status > 0 )
	{
	  if(fDebug)
	    {
	      el.print();
	      std::cout<<"t: "<<time<<" A: "<<amp<<" E: "<<err<<std::endl;
	    }
	  fAnodeSignals.emplace_back( el, time, amp, err);
	}
    }

  return int(fAnodeSignals.size());
}

int Ledge::FindPadTimes(TClonesArray* PADsignals)
{
  int Nentries = PADsignals->GetEntries();
  fPadSignals.clear();
  fPadSignals.reserve(Nentries);

  std::string delimiter = "_";

  // find intresting channels
  //  unsigned int index=0; //wfholder index -- unused 14 Oct 2020  -- AC
  for( int j=0; j<Nentries; ++j )
    {
      TWaveform* w = (TWaveform*) PADsignals->ConstructedAt(j);
      std::vector<int> data(w->GetWaveform());
      std::string wname = w->GetElectrode();

      size_t pos = wname.find(delimiter);
      std::string p = wname.substr(0, pos);
      if( p != "p" )
	std::cerr<<"Ledge Error: Wrong Electrode? "<<p<<std::endl;
      wname = wname.erase(0, pos + delimiter.length());

      pos = wname.find(delimiter);
      short col = std::stoi( wname.substr(0, pos) );
      assert(col<32&&col>=0);
      if(fDebug) std::cout<<"Ledge::FindPadTimes() col: "<<col;
      wname = wname.erase(0, pos + delimiter.length());

      pos = wname.find(delimiter);
      int row = std::stoi( wname.substr(0, pos) );
      //std::cout<<"Ledge::FindPadTimes() row: "<<row<<std::endl;
      if(fDebug) std::cout<<" row: "<<row<<std::endl;
      assert(row<576&&row>=0);

      // int coli = int(col);
      // int pad_index = pmap->index(coli,row);
      // assert(!std::isnan(pad_index));
      // CREATE electrode
      ALPHAg::electrode el(col,row);
  
      if( data.size() == 0 ) continue;

      double time, amp, err;
      int status = Analyze(&data,time, amp, err );
      if(fDebug) std::cout<<"Ledge::Analyze FeamChannel status: "<<status<<std::endl;
      if( status > 0 )
	{
	  if(fDebug)
	    {
	      el.print();
	      std::cout<<"t: "<<time<<" A: "<<amp<<" E: "<<err<<std::endl;
	    }
	  fPadSignals.emplace_back( el, time, amp, err);
	}
    }

  return int(fPadSignals.size());
}
#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
