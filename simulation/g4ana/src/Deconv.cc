#include <fstream>

#include "Deconv.hh"
#include "TWaveform.hh"

Deconv::Deconv(double adc, double pwb, 
	       double aw, double pad): fbinsize(1),
				       fAWbinsize(16.), fPADbinsize(16.), 
				       fADCdelay(0.),fPWBdelay(0.), // to be guessed
				       pedestal_length(100),fScale(-1.), // values fixed by DAQ
				       theAnodeBin(1), thePadBin(6),
				       fADCThres(adc), fPWBThres(pwb),
				       fAvalancheSize(0.), // to be set later
				       fADCpeak(aw),fPWBpeak(pad),
				       isalpha16(false)
{
  fTrace = false;

  fAnodeFactors = {
    0.1275,        // neighbour factor
    0.0365,        // 2nd neighbour factor
    0.012,         // 3rd neighbour factor
    0.0042         // 4th neighbour factor
  };

  fAwMask.reserve(256);
  fPadSecMask.reserve(32);
  fPadRowMask.reserve(576);

  int s = ReadResponseFile(fAWbinsize,fPADbinsize);
  std::cout<<"DeconvModule BeginRun Response status: "<<s<<std::endl;
  assert(s>0);

  pmap = new padmap;

  fPWBdelay=52.;
}

Deconv::~Deconv()
{
  delete pmap;
}

int Deconv::FindAnodeTimes(TClonesArray* AWsignals)
{
  fbinsize = fAWbinsize;
  fAvalancheSize = fADCpeak;

  int Nentries = AWsignals->GetEntries(); 

  // prepare vector with wf to manipulate
  std::vector<std::vector<double>*>* subtracted=new std::vector<std::vector<double>*>;
  subtracted->reserve( Nentries );

  // clear/initialize "output" vectors
  //      std::cout<<"DeconvModule::FindAnodeTimes clear/initialize \"output\" vectors"<<std::endl;
  fAnodeIndex.clear();
  fAnodeIndex.reserve( Nentries );
  sanode.clear();
  sanode.reserve( Nentries );
  aTimes.clear();

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
	  std::cerr<<"DeconvModule::FindAnodeTimes ERROR wf samples: "
		   <<data.size()<<std::endl;
	  continue;
	}

      // CALCULATE PEDESTAL
      double ped(0.);
      for(int b = 0; b < pedestal_length; b++) ped += data.at( b );
      ped /= double(pedestal_length);
      if( fTrace )
	std::cout<<"DeconvModule::FindAnodeTimes pedestal for anode wire: "<<el.idx
		 <<" is "<<ped<<std::endl;
      // CALCULATE PEAK HEIGHT
      auto minit = std::min_element(data.begin(), data.end());
      double max = el.gain * fScale * ( double(*minit) - ped );

      if(max > fADCThres)     // Signal amplitude < thres is considered uninteresting
	{
	  if(fTrace)
	    std::cout<<"\tsignal above threshold ch: "<<j<<" aw: "<<aw_number<<std::endl;

	  // SUBTRACT PEDESTAL
	  std::vector<double>* waveform=new std::vector<double>(data.begin()+pedestal_length,data.end());
	  std::for_each(waveform->begin(), waveform->end(), [ped](double& d) { d-=ped;});

	  // fill vector with wf to manipulate
	  subtracted->emplace_back( waveform );
                  
	  // STORE electrode
	  // electrode el(aw_number);
	  fAnodeIndex.push_back( el );
	}// max > thres
    } // channels

  // DECONVOLUTION
  int nsig = Deconvolution(subtracted,sanode,aTimes,fAnodeIndex,fAnodeResponse,theAnodeBin,true);
  std::cout<<"DeconvModule::FindAnodeTimes "<<nsig<<" found"<<std::endl;
  //

  // prepare control variable (deconv remainder) vector
  resRMS_a.clear();
  resRMS_a.reserve( subtracted->size() );
  // calculate remainder of deconvolution
  for(auto s: *subtracted)
    resRMS_a.push_back( sqrt(
			     std::inner_product(s->begin(), s->end(), s->begin(), 0.)
			     / static_cast<double>(s->size()) )
			);

  for (uint i=0; i<subtracted->size(); i++)
    delete subtracted->at(i);
  delete subtracted;
  return nsig;
}

int Deconv::FindPadTimes(TClonesArray* PADsignals)
{
  fbinsize = fPADbinsize;
  fAvalancheSize = fPWBpeak;

  int Nentries = PADsignals->GetEntries(); 

  // prepare vector with wf to manipulate
  std::vector<std::vector<double>*>* subtracted=new std::vector<std::vector<double>*>;
  subtracted->reserve( Nentries );

  // clear/initialize "output" vectors
  fPadIndex.clear();
  fPadIndex.reserve( Nentries );

  spad.clear();
  spad.reserve( Nentries );
  pTimes.clear();

  std::string delimiter = "_";

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
      //std::cout<<"DeconvModule::FindPadTimes() col: "<<col<<std::endl;
      wname = wname.erase(0, pos + delimiter.length());

      pos = wname.find(delimiter);
      int row = std::stoi( wname.substr(0, pos) );
      //std::cout<<"DeconvModule::FindPadTimes() row: "<<row<<std::endl;
      assert(row<576&&row>=0);

      int coli = int(col);
      int pad_index = pmap->index(coli,row);
      assert(!isnan(pad_index));
      // CREATE electrode
      electrode el(col,row);
      //el.setgain( fPwbRescale.at(pad_index) );

      // mask hot pads
      bool mask = false;
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
	{
	  for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
	    {
	      if( *it == col && *jt == row )
		{
		  mask = true;
		  break;
		}
	    }
	  if( mask ) break;
	}
      if( mask ) continue;

      // nothing dumb happens
      if( data.size() < 510 )
	{
	  std::cerr<<"DeconvModule::FindPadTimes ERROR wf samples: "
		   <<data.size()<<std::endl;
	  continue;
	}

      // CALCULATE PEDESTAL
      double ped(0.);
      for(int b = 0; b < pedestal_length; b++) ped += data.at( b );
      ped /= pedestal_length;
      // CALCULATE PEAK HEIGHT
      auto minit = std::min_element(data.begin(), data.end());
      //double max = el.gain * fScale * ( double(*minit) - ped );
      double max = fScale * ( double(*minit) - ped );

      if(max > fPWBThres)     // Signal amplitude < thres is considered uninteresting
	{
	  if(fTrace && 0)
	    std::cout<<"\tsignal above threshold ch: "<<j<<std::endl;

	  // SUBTRACT PEDESTAL
	  std::vector<double>* waveform=new std::vector<double>(data.begin()+pedestal_length,data.end());
	  std::for_each(waveform->begin(), waveform->end(), [ped](double& d) { d-=ped;});

	  // fill vector with wf to manipulate
	  subtracted->emplace_back( waveform );
	  //aresult.emplace_back( waveform.size() );

	  // STORE electrode
	  //electrode el(col,row);
	  fPadIndex.push_back( el );
	  if( fTrace )
	    std::cout<<"DeconvModule::FindPadTimes() col: "<<col
		     <<" row: "<<row
		     <<" ph: "<<max<<std::endl;
	}// max > thres
    }// channels

  // DECONVOLUTION
  int nsig = Deconvolution(subtracted,spad,pTimes,fPadIndex,fPadResponse,thePadBin,false);
  std::cout<<"DeconvModule::FindPadTimes "<<nsig<<" found"<<std::endl;
  //

  // prepare control variable (deconv remainder) vector
  resRMS_p.clear();
  resRMS_p.reserve( subtracted->size() );
  // calculate remainder of deconvolution
  for(auto s: *subtracted)
    resRMS_p.push_back( sqrt(
			     std::inner_product(s->begin(), s->end(), s->begin(), 0.)
			     / static_cast<double>(s->size()) )
			);

  for (uint i=0; i<subtracted->size(); i++)
    delete subtracted->at(i);
  delete subtracted;
  return nsig;
}


int Deconv::Deconvolution( std::vector<std::vector<double>*>* subtracted, 
			   std::vector<signal> &fSignals, std::set<double> &fTimes,
			   std::vector<electrode> &fElectrodeIndex, 
			   std::vector<double> &fResponse, int theBin, bool isanode )
{
  if(subtracted->size()==0) return 0;
  int nsamples = subtracted->back()->size();
  assert(nsamples < 1000);
  if( fTrace )
    std::cout<<"DeconvModule::Deconv Subtracted Size: "<<subtracted->size()
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
      std::set<wfholder*,comp_hist>* histset = wforder( subtracted, b );
      // std::cout<<"DeconvModule::Deconv bin of interest: "<<b
      //          <<" workable wf: "<<histset->size()<<std::endl;
      // this is useful to split deconv into the "Subtract" method
      // map ordered wf to corresponding electrode
      std::map<int,wfholder*>* histmap = wfordermap(histset,fElectrodeIndex);

      double neTotal = 0.0;
      for (auto const it : *histset)
	{
	  unsigned int i = it->index;
	  std::vector<double>* wf=it->h;
	  electrode anElectrode = fElectrodeIndex.at( i );
	  double ne = anElectrode.gain * fScale * wf->at(b) / fResponse[theBin]; // number of "electrons"

	  if( ne >= fAvalancheSize )
	    {
	      neTotal += ne;
	      // loop over all bins for subtraction
	      Subtract(histmap,i,b,ne,fElectrodeIndex,fResponse,theBin,isanode);

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
      for (auto const it : *histset)
	{
	  delete it;
	}
      delete histset;
      delete histmap;
    }// loop bin of interest


  return int(fSignals.size());
}
   
void Deconv::Subtract(std::map<int,wfholder*>* wfmap,
		      const unsigned i, const int b,
		      const double ne,std::vector<electrode> &fElectrodeIndex, 
		      std::vector<double> &fResponse, int theBin, bool isanode)
{
      
  wfholder* hist1 = wfmap->at(i);
  std::vector<double> *wf1 = hist1->h;
  unsigned int i1 = hist1->index;
  electrode wire1 = fElectrodeIndex[ i1 ]; // mis-name for pads

  uint AnodeSize=fAnodeFactors.size();
  uint ElectrodeSize=fElectrodeIndex.size();
  int AnodeResponseSize=(int)fAnodeResponse.size();
      
  std::vector<double>* wf2[ElectrodeSize];
  if( isanode ) 
    {
      for(unsigned int k = 0; k < ElectrodeSize; ++k)
	{                                               
	  wf2[k] = wfmap->at(k)->h;
	}
      for(unsigned int k = 0; k < ElectrodeSize; ++k)
	{                                               
	  electrode wire2 = fElectrodeIndex[ k ];
	  //check for top/bottom
	  if( wire2.sec != wire1.sec ) continue;
	  //Skip early if wires not close...
	  if (IsAnodeClose(wire1.idx,wire2.idx)>4) continue;
	  for(unsigned int l = 0; l < AnodeSize; ++l)
	    {
	      //Take advantage that there are 256 anode wires... use uint8_t
	      //if( !IsNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
	      if( !IsAnodeNeighbour(  wire1.idx, wire2.idx, int(l+1) ) ) continue;
                     
	      for(int bb = b-theBin; bb < int(wf1->size()); ++bb)
		{
		  // the bin corresponding to bb in the response
		  int respBin = bb-b+theBin;
		  if (respBin<0) continue;
		  if (respBin >= AnodeResponseSize) continue;

		  if(respBin < AnodeResponseSize && respBin >= 0)
		    {
		      // remove neighbour induction
		      (*wf2[k])[bb] += ne/fScale/wire1.gain*fAnodeFactors[l]*fAnodeResponse[respBin];
		    }
		}// loop over all bins for subtraction
	    }// loop over factors
	}// loop all electrodes' signals looking for neighbours
    }
  for(int bb = b-theBin; bb < int(wf1->size()); ++bb)
    {
      // the bin corresponding to bb in the response
      int respBin = bb-b+theBin;

      if( respBin < int(fResponse.size()) && respBin >= 0 )
	{
	  // Remove signal tail for waveform we're currently working on
	  wf1->at(bb) -= ne/fScale/wire1.gain*fResponse.at(respBin);
	}
    }// bin loop: subtraction
}

std::set<wfholder*,comp_hist>* Deconv::wforder(std::vector<std::vector<double>*>* subtracted, const int b)
{
  std::set<wfholder*,comp_hist>* histset=new std::set<wfholder*,comp_hist>;
  // For each bin, order waveforms by size,
  // i.e., start working on largest first
  for(unsigned int i=0; i<subtracted->size(); ++i)
    {
      wfholder* mh=new wfholder;
      //Vector gets copied here... could be slow...
      mh->h = subtracted->at(i);
      mh->index = i;
      mh->val = fScale*subtracted->at(i)->at(b);
      histset->insert(mh);
    }
  return histset;
}


std::map<int,wfholder*>* Deconv::wfordermap(std::set<wfholder*,comp_hist>* histset,std::vector<electrode> &fElectrodeIndex)
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
  std::string filename(getenv("AGRELEASE"));
  filename+="/ana/anodeResponse.dat";
  std::cout << "DeconvModule:: Reading in response file (anode) " << filename << std::endl;
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
    std::cout<<"DeconvModule::ReadResponseFile anode max: "<<max<<"\tanode bin: "<<theAnodeBin<<std::endl;

  //      filename = "padResponse_deconv.dat";
  filename=getenv("AGRELEASE");
  filename+="/ana/padResponse_deconv.dat";
  std::cout << "DeconvModule:: Reading in response file (pad) " << filename << std::endl;
  respFile.open(filename.c_str());
  if( !respFile.good() ) return 0;
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

std::vector<double> Deconv::Rebin(const std::vector<double> &in, int binsize, double ped)
{
  if( fTrace )
    std::cout<<"DeconvModule::Rebin "<<binsize<<std::endl;
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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
