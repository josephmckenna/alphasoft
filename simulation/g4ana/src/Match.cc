#include "Match.hh"

Match::Match(std::string json):fTrace(false),fCoincTime(16.)
{
  ana_settings=new AnaSettings(json.c_str());

  maxPadGroups = ana_settings->GetDouble("MatchModule","maxPadGroups");
  padSigma = ana_settings->GetDouble("MatchModule","padSigma");
  padSigmaD = ana_settings->GetDouble("MatchModule","padSigmaD");
  padFitErrThres = ana_settings->GetDouble("MatchModule","padFitErrThres");
  use_mean_on_spectrum=ana_settings->GetBool("MatchModule","use_mean_on_spectrum");
  spectrum_mean_multiplyer = ana_settings->GetDouble("MatchModule","spectrum_mean_multiplyer");
  spectrum_cut = ana_settings->GetDouble("MatchModule","spectrum_cut");
  spectrum_width_min = ana_settings->GetDouble("MatchModule","spectrum_width_min");
}

Match::~Match()
{
  delete ana_settings;
}

void Match::Init()
{
  fCombinedPads.clear();
  spacepoints.clear();
}

std::set<short> Match::PartionBySector(std::vector<signal>* padsignals, std::vector< std::vector<signal> >& pad_bysec)
{
  std::set<short> secs;
  pad_bysec.clear();
  pad_bysec.resize(32);

  for( auto ipd=padsignals->begin(); ipd!=padsignals->end(); ++ipd )
    {
      //ipd->print();
      secs.insert( ipd->sec );
      pad_bysec.at(ipd->sec).push_back(*ipd);
    }
  return secs;
}

std::vector< std::vector<signal> > Match::PartitionByTime( std::vector<signal>& sig )
{
  std::multiset<signal, signal::timeorder> sig_bytime(sig.begin(),
						      sig.end());
  double temp=-999999.;
  std::vector< std::vector<signal> > pad_bytime;
  for( auto isig = sig_bytime.begin(); isig!=sig_bytime.end(); ++isig )
    {
      if( isig->t > temp )
	{
	  temp=isig->t;
	  pad_bytime.emplace_back();
	  pad_bytime.back().push_back( *isig );
	}
      else
	pad_bytime.back().push_back( *isig );
    }
  sig_bytime.clear();
  return pad_bytime;
}

std::vector<std::vector<signal>> Match::CombPads(std::vector<signal>* padsignals)
{
  // combine pads in the same column only
  std::vector< std::vector<signal> > pad_bysec;
  std::set<short> secs = PartionBySector( padsignals, pad_bysec ) ;
  std::vector< std::vector<signal> > comb;
  for( auto isec=secs.begin(); isec!=secs.end(); ++isec )
    {
      short sector = *isec;
      if( sector < 0 || sector > 31 ) continue;
      if( fTrace )
	std::cout<<"MatchModule::CombinePads sec: "<<sector
		 <<" sector: "<<pad_bysec[sector].at(0).sec
		 <<" size: "<<pad_bysec[sector].size()<<std::endl;
      // combine pads in the same time slice only
      std::vector< std::vector<signal> > pad_bytime = PartitionByTime( pad_bysec[sector] );
      for( auto it=pad_bytime.begin(); it!=pad_bytime.end(); ++it )
	{
	  if( it->size() <= 2 ) continue;
	  if( it->begin()->t < 0. ) continue;
	  comb.push_back( *it );
	}
      pad_bytime.clear();
    }
  secs.clear();
  //for (uint i=0; i<pad_bysec.size(); i++)
  //   delete pad_bysec[i];
  pad_bysec.clear();
  return comb;
}

void Match::CombinePads(std::vector<signal>* padsignals)
{
  //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
  std::vector< std::vector<signal> > comb = CombPads( padsignals );
  fCombinedPads.clear();
  for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
    {
      //CentreOfGravity(*sigv);
      //New function without fitting (3.5x faster...
      //... but does it fit well enough?):
      //CentreOfGravity_nofit(*sigv);
      CentreOfGravity_nohisto(*sigv);
    }

  for (uint i=0; i<comb.size(); i++)
    comb.at(i).clear();
  comb.clear();
}

void Match::CentreOfGravity_nohisto( std::vector<signal> &vsig )
{
  if(!vsig.size()) return;
  double time = vsig.begin()->t;
  short col = vsig.begin()->sec;

  int vsigsize=vsig.size();
  //Declare enough memory to lay out the whole pad array in order
  const int pads=576;
  int spectrum[pads+2];

  //Track the mean so that we can apply a threshold cut based on it
  //(we may want to just hard code a threshold
  double specmean=0;
  int speccount=0;
  memset(spectrum, 0, sizeof(spectrum));
  for ( int i=0; i<vsigsize; i++)
    {
      int h=vsig[i].height;
      spectrum[vsig[i].idx+1]=h;
      specmean+=h;
      speccount++;
    }
  specmean/=speccount;

  //Group peaks, track width and height
  int tmpmax=0;
  int width=0;
  double tmpz=-999.;
  std::vector<double> peakpos;
  std::vector<double> peakwidth;

  int starts=0;
  int ends=0;

  //Tune the threshold for peak combination here!
  double thresh=-1;
  if (use_mean_on_spectrum)
    thresh=specmean*spectrum_mean_multiplyer;
  else
    thresh=spectrum_cut;

  for (int i=1; i<pads+1; i++)
    {
      //Start of peak
      if (spectrum[i-1]<thresh && spectrum[i]>thresh)
	{
	  starts++;
	  width++;
	  if (tmpmax<spectrum[i])
            {
	      tmpmax=spectrum[i];
	      tmpz=( double(i-1) + 0.5 ) * _padpitch - _halflength;
            }
	}

      //End of peak
      if (spectrum[i+1]<thresh && spectrum[i]>thresh)
	{
	  ends++;
	  peakpos.push_back(tmpz);
	  peakwidth.push_back(width);
	  width=0;
	  tmpz=-9999;
	  tmpmax=0.;
	}
      //Middle of peak
      else if (spectrum[i-1]>thresh && spectrum[i+1]>thresh)
	{
	  width++;
	}
    }
  int nfound=peakpos.size();
  if (!nfound) return;
  if( fTrace )
    std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;

  //Compare loop to histogram version:
#define TEST_NFOUND 0

  double peakx[nfound];
  for(int i = 0; i < nfound; i++)
    {
      //peakx[i]=spec.GetPositionX()[i];
      peakx[i]=peakpos.at(i);
      //std::cout<<"PEAK AT:"<<peakx[i]<<std::endl;
      //peaky[i]=spec.GetPositionY()[i];
      double min=peakx[i]-5.*padSigma;
      double max=peakx[i]+5.*padSigma;

#if TEST_NFOUND
      TString hname = TString::Format("hhhhhh_%d_%1.0f",col,time);
      int bins=(max-min)/_padpitch;
      TH1D* hhh = new TH1D(hname.Data(),"",bins,min,max);
#endif

      double sum = 0;
      double sq_sum = 0;
      int n=0;
      int n2=0;
      double peak=-1;

      for( auto& s: vsig )
	{
	  // s.print();
	  double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
	  if (z<min) continue;
	  if (z>max) continue;
	  int h=s.height;
	  double wz=h*z;
	  n+=h;
	  n2+=h*h;
	  if (h>peak)
	    peak=h;
	  sum+=wz;
	  for (int sq_it=0; sq_it<h; sq_it++) sq_sum+=z*z;
#if TEST_NFOUND
	  hhh->Fill(z,s.height);
#endif
	}

      double mean=sum/(double)n;
      bool stat=true;
      //RMS (TH1 style)
      double sigma = sqrt(sq_sum / n - mean * mean);
      if (nfound>1 && sigma<spectrum_width_min) continue;
      //N Effective entries (TH1 style)
      double neff=((double)n)*((double)n)/((double)n2);
      double err = sqrt(sq_sum / n - mean*mean)/sqrt(neff);
#if TEST_NFOUND
      std::cout<<"Sigma:"<< hhh->GetRMS();<< " vs "<< sigma <<std::endl;
      std::cout <<"Error:"<< hhh->GetMeanError() << " vs " << err  <<std::endl;
#endif
      if( sigma == 0. || err == 0. ) stat=false;
      if( err < padFitErrThres &&
	  fabs(sigma-padSigma)/padSigma < padSigmaD && stat )
	{
	  double amp = peak;
	  double pos = mean;
#if TEST_NFOUND
	  std::cout<<"AMP: "<<hhh->GetBinContent(hhh->GetMaximumBin())<<" vs "<< amp<<std::endl;
	  std::cout<<"POS: "<<hhh->GetMean()<<" vs "<< pos<<std::endl;
#endif
	  //double amp = (double)peak;
	  //double pos =mean;
	  double zix = ( pos + _halflength ) / _padpitch - 0.5;
	  int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	  // create new signal with combined pads
	  fCombinedPads.emplace_back( col, index, time, amp, pos, err );

	  if( fTrace )
	    std::cout<<"Combination Found! s: "<<col
		     <<" i: "<<index
		     <<" t: "<<time
		     <<" a: "<<amp
		     <<" z: "<<pos
		     <<" err: "<<err<<std::endl;
	}
#if TEST_NFOUND
      delete hhh;
#endif
    } // wizard peak finding failed
  if( fTrace )
    std::cout<<"-------------------------------"<<std::endl;
}

void Match::MatchElectrodes(std::vector<signal>* awsignals)
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
						     awsignals->end());
  std::multiset<signal, signal::timeorder> pad_bytime(fCombinedPads.begin(),
						      fCombinedPads.end());
  spacepoints.clear();
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      short sector = short(iaw->idx/8);
      if( fTrace )
	std::cout<<"MatchModule::Match aw: "<<iaw->idx
		 <<" t: "<<iaw->t<<" pad sector: "<<sector<<std::endl;
      for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
	{
	  bool tmatch=false;
	  bool pmatch=false;

	  double delta = fabs( iaw->t - ipd->t );
	  if( delta < fCoincTime ) tmatch=true;

	  if( sector == ipd->sec ) pmatch=true;

	  if( tmatch && pmatch )
	    {
	      spacepoints.push_back( std::make_pair(*iaw,*ipd) );
	      //pad_bytime.erase( ipd );
	      ++Nmatch;
	      if( fTrace )
		std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx<<std::endl;
	    }
	}
    }
  //  if( fTrace )
  std::cout<<"MatchModule::Match Number of Matches: "<<Nmatch<<std::endl;
  if( int(spacepoints.size()) != Nmatch )
    std::cerr<<"Match::MatchElectrodes ERROR: number of matches differs from number of spacepoints: "<<spacepoints.size()<<std::endl;
}

void Match::FakePads(std::vector<signal>* awsignals)
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
						     awsignals->end());
  spacepoints.clear();
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      short sector = short(iaw->idx/8);
      //signal fake_pad( sector, 288, iaw->t, 1., 0.0 );
      signal fake_pad( sector, 288, iaw->t, 1., 0.0, kUnknown);
      spacepoints.push_back( std::make_pair(*iaw,fake_pad) );
      ++Nmatch;
    }
  std::cout<<"MatchModule::FakePads Number of Matches: "<<Nmatch<<std::endl;
}
