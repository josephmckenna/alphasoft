//
// Analyze waveforms and extract
// Leading Edge information
// 
// Author: A. Capra
// Date: Nov. 2019
//

#include "Ledge.hh"
#include "TPCconstants.hh"

#include "TWaveform.hh"

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
  double bmean=0.,brms=0.;
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

int Ledge::FindAnodeTimes(TClonesArray* AWsignals)
{
  int Nentries = AWsignals->GetEntries();

  std::vector<signal>* sanodes = new std::vector<signal>;
  sanodes->reserve(Nentries);
   
  // find intresting channels
  unsigned int index=0; //wfholder index
  for( int j=0; j<Nentries; ++j )
    {
      TWaveform* w = (TWaveform*) AWsignals->ConstructedAt(j);
      std::vector<int> data(w->GetWaveform());
      double time, amp, err;
      int status = Analyze(&data, time, amp, err );

      std::string wname = w->GetElectrode();
      //std::cout<<"Deconv::FindAnodeTimes "<<j<<" wire: "<<wname<<" size: "<<data.size()<<std::endl;
      int aw_number = std::stoi( wname.substr(1) );
      electrode el(aw_number);
      if(fDebug) std::cout<<"Ledge::Analyze AWsignals status: "<<status<<std::endl;
      if( status > 0 )
	{
	  if(fDebug)
	    {
	      el.print();
	      std::cout<<"t: "<<time<<" A: "<<amp<<" E: "<<err<<std::endl;
	    }
	  sanodes->emplace_back( el, time, amp, err, true );
	}
    }

  fSignals=sanodes;
  return int(sanodes->size());
}

int Ledge::FindPadTimes(TClonesArray* PADsignals)
{
  int Nentries = PADsignals->GetEntries();
  std::vector<signal>* spads = new std::vector<signal>;
  spads->reserve(Nentries);

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

      // int coli = int(col);
      // int pad_index = pmap->index(coli,row);
      // assert(!std::isnan(pad_index));
      // CREATE electrode
      electrode el(col,row);
  
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
	  spads->emplace_back( el, time, amp, err, false );
	}
    }

  fSignals=spads;
  return int(spads->size());
}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
