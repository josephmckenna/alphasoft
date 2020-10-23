#include "Match.hh"

#include "TH1D.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "TCanvas.h"

#include <Math/MinimizerOptions.h>

#include "fitSignals.hh"

#include <chrono>
#include <thread>       // std::thread
#include <functional>   // std::ref

//Null static pointers to histograms (histograms static so can be shared 
//between Match instances in multithreaded mode)
TH1D* Match::hsigCoarse=NULL;
TH1D* Match::hsig=NULL;
TH1D* Match::hcognpeaks=NULL;
TH2D* Match::hcognpeaksrms=NULL;
TH2D* Match::hcognpeakswidth=NULL;
TH1D* Match::hcogsigma=NULL;
TH1D* Match::hcogerr=NULL;
TH2D* Match::hcogpadssigma=NULL;
TH2D* Match::hcogpadsamp=NULL;
TH2D* Match::hcogpadsint=NULL;
TH2D* Match::hcogpadsampamp=NULL;
TH1D* Match::htimecog=NULL;
TH1D* Match::htimeblobs=NULL;
TH1D* Match::htimefit=NULL;


Match::Match(const AnaSettings* ana_set):
   fTrace(false),
   fDebug(false),
   ana_settings(ana_set),
   fCoincTime(     ana_settings->GetDouble("MatchModule","coincTime")),
   maxPadGroups(   ana_settings->GetDouble("MatchModule","maxPadGroups")),
   padsNmin(       ana_settings->GetInt("MatchModule","padsNmin")),
   padSigma(       ana_settings->GetDouble("MatchModule","padSigma")),
   padSigmaD(      ana_settings->GetDouble("MatchModule","padSigmaD")),
   padFitErrThres( ana_settings->GetDouble("MatchModule","padFitErrThres")),
   use_mean_on_spectrum(ana_settings->GetBool("MatchModule","use_mean_on_spectrum")),
   spectrum_mean_multiplyer(ana_settings->GetDouble("MatchModule","spectrum_mean_multiplyer")),
   spectrum_cut(   ana_settings->GetDouble("MatchModule","spectrum_cut")),
   spectrum_width_min(ana_settings->GetDouble("MatchModule","spectrum_width_min")),
   grassCut(       ana_settings->GetDouble("MatchModule","grassCut")),
   goodDist(       ana_settings->GetDouble("MatchModule","goodDist")),
   charge_dist_scale(ana_settings->GetDouble("MatchModule","pad_charge_dist_scale")),
   padThr(         ana_settings->GetDouble("DeconvModule","PADthr"))// This DeconvModule setting is also needed here, for wire-dependent threshold
{
  std::cout<<"Match::Loading AnaSettings from json"<<std::endl;

  TString CentreOfGravity=ana_settings->GetString("MatchModule","CentreOfGravityMethod");
  if ( CentreOfGravity.EqualTo("CentreOfGravity") ) CentreOfGravityFunction=1;
  if ( CentreOfGravity.EqualTo("CentreOfGravity_blobs") ) CentreOfGravityFunction=2;
  if ( CentreOfGravityFunction <= 0 )
    {
      std::cout<<"Match:No valid CentreOfGravityMethod function in json"<<std::endl;
      exit(1);
    }
  else
    std::cout<<"Using CentreOfGravity case "<<CentreOfGravityFunction<<": "<<CentreOfGravity<<std::endl;
}

Match::~Match()
{ }

void Match::Init()
{
  assert(CentreOfGravityFunction>=0); //CentreOfGravityFunction not set!
  if(fDebug) std::cout<<"Match::Init!"<<std::endl;
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
}

void Match::Setup(TFile* OutputFile)
{
  if( diagnostic )
    {
      if( OutputFile )
        { 
          OutputFile->cd(); // select correct ROOT directory
          if( !gDirectory->cd("padmatch") )
            gDirectory->mkdir("padmatch")->cd();
	  else 
	    gDirectory->cd("padmatch");
        }
      else
        gFile->cd();

      if (!hcognpeaks)
         hcognpeaks = new TH1D("hcognpeaks","cCombPads CoG - Number of Avals",int(maxPadGroups+1.),
                            0.,maxPadGroups+1.);
      if (!hcognpeaksrms)
        hcognpeaksrms = new TH2D("hcognpeaksrms","CombPads CoG - Number of Avals vs RMS", 500, 0., 50,int(maxPadGroups+1.),
			       0.,maxPadGroups+1.);
      if (!hcognpeakswidth)
        hcognpeakswidth = new TH2D("hcognpeakswidth","CombPads CoG - Number of Avals vs width", 20, 0., 20,int(maxPadGroups+1.),
				 0.,maxPadGroups+1.);
      if (!hcogsigma)
        hcogsigma = new TH1D("hcogsigma","CombPads CoG - Sigma Charge Induced;[mm]",700,0.,70.);
      if (!hcogerr)
        hcogerr = new TH1D("hcogerr","CombPads CoG - Error on Mean;[mm]",2000,0.,20.);

      if (!hcogpadssigma)
        hcogpadssigma = new TH2D("hcogpadssigma","CombPads CoG - Pad Index Vs. Sigma Charge Induced;pad index;#sigma [mm]",32*576,0.,32.*576.,1000,0.,140.);
      if (!hcogpadsamp)
        hcogpadsamp = new TH2D("hcogpadsamp","CombPads CoG - Pad Index Vs. Amplitude Charge Induced;pad index;Amplitude [a.u.]",32*576,0.,32.*576.,1000,0.,4000.);
      if (!hcogpadsint)
        hcogpadsint = new TH2D("hcogpadsint","CombPads CoG - Pad Index Vs. Integral Charge Induced;pad index;Tot. Charge [a.u.]",32*576,0.,32.*576.,1000,0.,10000.);
      if (!hcogpadsampamp)
        hcogpadsampamp = new TH2D("hcogpadsampamp","CombPads CoG - Gaussian fit amplitude Vs. Max. Signal height;max. height;Gauss Amplitude",1000,0.,4000.,1000,0.,4000.);
      //  hsig = new TH1D("hpadRowSig","sigma of pad combination fit",1000,0,50);      
      if (!htimecog)
        htimecog = new TH1D("htimecog","Timing of Cog;Time [us]",1000,0.,10000.);
      if (!htimeblobs)
        htimeblobs = new TH1D("htimeblobs","Timing of Blob Finding;Time [us]",1000,0.,10000.);
      if (!htimefit)
        htimefit = new TH1D("htimefit","Timing of Fit;Time [us]",1000,0.,10000.);
    }
}

std::pair<std::set<short>,std::vector< std::vector<signal> >> Match::PartitionBySector(std::vector<signal>* padsignals)
{
  std::vector< std::vector<signal> > pad_bysec;
  pad_bysec.resize(32);

  std::set<short> secs;

  for( auto ipd=padsignals->begin(); ipd!=padsignals->end(); ++ipd )
    {
      //ipd->print();
      secs.insert( ipd->sec );
      pad_bysec.at(ipd->sec).push_back(*ipd);
    }
  return {secs,pad_bysec};
}

std::vector< std::vector<signal> > Match::PartitionByTime( std::vector<signal>& sig )
{
  if( fDebug ) std::cout<<"Match::PartitionByTime  "<<sig.size()<<std::endl;
  std::multiset<signal, signal::timeorder> sig_bytime(sig.begin(),
						      sig.end());
  double temp=-999999.;
  std::vector< std::vector<signal> > pad_bytime;
  for( auto isig = sig_bytime.begin(); isig!=sig_bytime.end(); ++isig )
    {
      if( fDebug ) isig->print();
      //      if( isig->t > temp ) // 
      if( (isig->t - temp) > fCoincTime )
	{
	  temp=isig->t;
	  pad_bytime.emplace_back();
	  pad_bytime.back().push_back( *isig );
	}
      else
	pad_bytime.back().push_back( *isig );
    }
  sig_bytime.clear();
  if( fDebug ) std::cout<<"Match::PartitionByTime # of time partitions: "<<pad_bytime.size()<<std::endl;
  return pad_bytime;
}

std::vector<std::vector<signal>> Match::CombPads(std::vector<signal>* padsignals)
{
  if( fTrace )
    std::cout<<"Match::CombPads!"<<std::endl;

  // combine pads in the same column only
  std::vector< std::vector<signal> > pad_bysec;
  std::set<short> secs;
  std::tie(secs, pad_bysec) = PartitionBySector( padsignals ) ;
  
  if( fTrace )
    std::cout<<"Match::CombPads # of secs: "<<secs.size()<<std::endl;

  std::vector< std::vector<signal> > comb;
  for( auto isec=secs.begin(); isec!=secs.end(); ++isec )
    {
      short sector = *isec;
      if( sector < 0 || sector > 31 ) continue;
      if( fDebug )
        std::cout<<"Match::CombPads sec: "<<sector
         <<" = sector: "<<pad_bysec[sector].at(0).sec
         <<" size: "<<pad_bysec[sector].size()<<std::endl;
      // combine pads in the same time slice only
      std::vector< std::vector<signal> > pad_bytime = PartitionByTime( pad_bysec[sector] );
      for( auto it=pad_bytime.begin(); it!=pad_bytime.end(); ++it )
        {
          if( it->size() <= 2 ) continue; // it->size() <= padsNmin
          if( it->begin()->t < 0. ) continue;
          comb.push_back( *it );
        }
      pad_bytime.clear();
    }
  secs.clear();
  pad_bysec.clear();
  return comb;
}

std::vector<signal>* Match::CombineAPad(std::vector< std::vector<signal> > *comb,std::vector<signal>* CombinedPads, size_t PadNo)
{

  if (PadNo > comb->size())
    return CombinedPads;

  switch(CentreOfGravityFunction)
  {
    case 0: 
    {
      std::cout<<""<<std::endl;
      break;
    }
    case 1:
    {
      CentreOfGravity(comb->at(PadNo),CombinedPads);
      break;
    }
    case 2:
    {
      CentreOfGravity_blobs(comb->at(PadNo), CombinedPads);
      break;
    }
  }
  return CombinedPads;
}


std::vector<signal>* Match::CombinePads(std::vector< std::vector<signal> > *comb)
{

  if( comb->size()==0 ) return NULL;
  std::vector<signal>* CombinedPads=new std::vector<signal>;

  if( fTrace ) 
    {
      std::cout<<"Match::CombinePads comb size: "<<comb->size()<<"\t";
      std::cout<<"Using CentreOfGravityFunction: "<<CentreOfGravityFunction<<std::endl;
      std::cout<<"Match::CombinePads sssigv: ";
      if( fDebug ) {
        for( auto sigv=comb->begin(); sigv!=comb->end(); ++sigv )
          {
            std::cout<<sigv->size()<<" ";
          }
      }
      std::cout<<"\n";
    }
  
  switch(CentreOfGravityFunction) {
  case 0: 
    {
      std::cout<<""<<std::endl;
      break;
    }
  case 1: {
    for( auto sigv=comb->begin(); sigv!=comb->end(); ++sigv )
      {
	auto start = std::chrono::high_resolution_clock::now();
	CentreOfGravity(*sigv,CombinedPads);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
	if( fTrace ) 
	  std::cout << "Match::CombinePads Time taken CentreOfGravity: "
		    << duration.count() << " us" << std::endl; 
	if( diagnostic ) htimecog->Fill(duration.count());
      }
    break;}
  case 2: {
    std::vector<std::thread> cogthread;
    auto cogstart = std::chrono::high_resolution_clock::now();
    for( unsigned i=0; i<comb->size(); ++i)
      {
	cogthread.push_back( std::thread(&Match::CentreOfGravity_blobs,this,
					 std::ref(comb->at(i)), CombinedPads ) );
      }   
    for( auto th=cogthread.begin();th!=cogthread.end();++th)
      {
	th->join();
      }
    auto cogstop = std::chrono::high_resolution_clock::now();
    auto cogdura = std::chrono::duration_cast<std::chrono::microseconds>(cogstop-cogstart);
    if( diagnostic ) htimecog->Fill(cogdura.count());
    if( fTrace ) {
      std::cout<<"Match::CombinePads Time taken CentreOfGravity_blobs: "
	       << cogdura.count() << " us" << std::endl; 
    }
    break;}
  }
  return CombinedPads;
}

void Match::CombinePads(std::vector<signal>* padsignals)
{
  std::vector< std::vector<signal> > comb = CombPads( padsignals );
  CombinePads(&comb);
}


void Match::CentreOfGravity( std::vector<signal> &vsig, std::vector<signal>* CombinedPads )
{
  if(!vsig.size()) return;

  //Root's fitting routines are often not thread safe, lock globally
#ifdef MODULE_MULTITHREAD
  std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
#endif
  double time = vsig.begin()->t;
  short col = vsig.begin()->sec;
  TString hname = TString::Format("hhhhh_%d_%1.0f",col,time);
  //      std::cout<<hname<<std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  TH1D* hh = new TH1D(hname.Data(),"",int(ALPHAg::_padrow),-ALPHAg::_halflength,ALPHAg::_halflength);
  for( auto& s: vsig )
    {
      // s.print();
      double z = ( double(s.idx) + 0.5 ) * ALPHAg::_padpitch - ALPHAg::_halflength;
      //hh->Fill(s.idx,s.height);
      hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
    }

  // exploit wizard avalanche centroid (peak)
  TSpectrum spec(maxPadGroups);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  spec.Search(hh,1,"nodraw");
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
  int nfound = spec.GetNPeaks();

  if( diagnostic )
    {
      htimeblobs->Fill(duration.count());
      hcognpeaks ->Fill(nfound);
    }

  gErrorIgnoreLevel = error_level_save;

  if( fDebug )
    std::cout<<"Match::CentreOfGravity nfound: "<<nfound<<" @ t: "<<time<<std::endl;
  if( nfound > 1 && hh->GetRMS() < spectrum_width_min )
    {
      nfound = 1;
      if( fDebug )
	std::cout<<"\tRMS is small: "<<hh->GetRMS()<<" set nfound to 1"<<std::endl;
    }

  double peakx[nfound];
  double peaky[nfound];

  for(int i = 0; i < nfound; ++i)
    {
      peakx[i]=spec.GetPositionX()[i];
      peaky[i]=spec.GetPositionY()[i];
      TString ffname = TString::Format("fffff_%d_%1.0f_%d",col,time,i);
      TF1* ff = new TF1(ffname.Data(),"gaus(0)",peakx[i]-10.*padSigma,peakx[i]+10.*padSigma);
      // initialize gaussians with peak finding wizard
      ff->SetParameter(0,peaky[i]);
      ff->SetParameter(1,peakx[i]);
      ff->SetParameter(2,padSigma);

      start = std::chrono::high_resolution_clock::now();
      int r = hh->Fit(ff,"B0NQ","");
      stop = std::chrono::high_resolution_clock::now();
      duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      if( diagnostic ) htimefit->Fill(duration.count());
#ifdef RESCUE_FIT
      bool stat=true;
#endif
      if( r==0 ) // it's good
	{
	  // make sure that the fit is not crazy...
	  double sigma = ff->GetParameter(2);
	  double err = ff->GetParError(1);
	  double pos = ff->GetParameter(1);
	  double zix = ( pos + ALPHAg::_halflength ) / ALPHAg::_padpitch - 0.5;
	  int row = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));
	  double amp = ff->GetParameter(0);
	  double eamp = ff->GetParError(0);

	  if( diagnostic )
	    {
	      hcogsigma->Fill(sigma);
	      hcogerr->Fill(err);
	      int index = pmap.index(col,row);
	      hcogpadssigma->Fill(double(index),sigma);
	      hcogpadsamp->Fill(double(index),amp);
	      double totq = ff->Integral(pos-10.*sigma,pos+10.*sigma);
	      hcogpadsint->Fill(double(index),totq);
	      hcogpadsampamp->Fill(peaky[i],amp);
	    }
	  if( err < padFitErrThres &&
	      fabs(sigma-padSigma)/padSigma < padSigmaD )
	    //if( err < padFitErrThres && sigma > 0. )
	    {
	      // create new signal with combined pads
	      CombinedPads->emplace_back( col, row, time, amp, eamp, pos, err );
	      if( fDebug )
		std::cout<<"Combination Found! s: "<<col
			 <<" i: "<<row
			 <<" t: "<<time
			 <<" a: "<<amp
			 <<" z: "<<pos
			 <<" err: "<<err<<std::endl;
	    }
	  else // fit is crazy
	    {
	      if( fTrace )
		std::cout<<"Combination NOT found... position error: "<<err
			 <<" or sigma: "<<sigma<<std::endl;
#ifdef RESCUE_FIT
	      stat=false;
#endif
	    }
	}// fit is valid
      else
	{
	  if( fTrace )
	    std::cout<<"\tFit Not valid with status: "<<r<<std::endl;
#ifdef RESCUE_FIT
	  stat=false;
#endif
	}
      delete ff;

#ifdef RESCUE_FIT
      if( !stat )
	{
	  int b0 = hh->FindBin(peakx[i]);
	  int bmin = b0-5, bmax=b0+5;
	  if( bmin < 1 ) bmin=1;
	  if( bmax > int(_padrow) ) bmax=int(_padrow);
	  double zcoord=0.,tot=0.;
	  for( int ib=bmin; ib<=bmax; ++ib )
	    {
	      double bc = hh->GetBinContent(ib);
	      zcoord += bc*hh->GetBinCenter(ib);
	      tot += bc;
	    }
	  if( tot > 0. )
	    {
	      //double amp = tot/11.;
	      double amp = tot;
	      double pos = zcoord/tot;
	      double zix = ( pos + _halflength ) / _padpitch - 0.5;
	      int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	      // create new signal with combined pads
	      CombinedPads->emplace_back( col, index, time, amp, sqrt(amp), pos, zed_err );

	      if( fDebug )
		std::cout<<"at last Found! s: "<<col
			 <<" i: "<<index
			 <<" t: "<<time
			 <<" a: "<<amp
			 <<" z: "<<pos<<std::endl;
	      stat=true;
	    }
	  else
	    {
	      if( fTrace )
		std::cout<<"Failed last combination resort"<<std::endl;
	    }
	}
#endif
    } // wizard peak finding failed
  delete hh;
  if( fTrace )
    std::cout<<"-------------------------------"<<std::endl;
}


// TH1-independent method to find peaks in pad charge distribution
std::vector<std::pair<double, double> > Match::FindBlobs(const std::vector<signal> &sigs,
							 int ifirst, int ilast)
{
  if(ilast < 0) ilast = sigs.size()-1;
  std::vector<signal>::const_iterator first = std::next(sigs.begin(),ifirst);
  std::vector<signal>::const_iterator last = std::next(sigs.begin(),ilast);
  std::vector<std::pair<double, double> > blobs;

  signal::heightorder sigcmp_h;
  auto maxit = std::max_element(first, last, sigcmp_h);
  double maxpos = maxit->z;
  double max = maxit->height;

  if(maxit == first && first != sigs.begin()){ // if maximum is at left edge of dist,
    auto previt = std::prev(maxit);           // and there is another high signal nearby,
    if(previt->height >= max && maxit->z - previt->z <= 2.*ALPHAg::_padpitch) // don't count
      return blobs;
  }
  if(maxit == last && last != sigs.end()){   // if maximum is at right edge of dist,
    auto nextit = std::next(maxit);           // and there is another high signal nearby,
    if(nextit->height >= max && nextit->z - maxit->z <= 2.*ALPHAg::_padpitch) // don't count
      return blobs;
    // ilast = std::min(sigs.size(), ilast+5); // expand range and try again
    // return FindBlobs(sigs, ifirst, ilast, cumulBins);
  }

  double blobwidth = 5.;
  double minRMS = 2.;

  int padmask = 4;

  double mean,rms;
  SignalsStatistics(first, last, mean, rms);

  if(rms < blobwidth && abs(maxpos-mean) < blobwidth){ // small width, search no further peaks
    if(rms > minRMS){                                 // width is not too small for real peak
      blobs.emplace_back(maxpos, max);
    }
  } else {                  // large width, save this peak, then search for more
    blobs.emplace_back(maxpos, max);
    int maxbin = maxit-sigs.begin();
    int cutbin = maxbin-padmask;
    std::vector<std::pair<double, double> > subblobs;
    if(cutbin-ifirst > padsNmin){ // search left of found peak
      subblobs = FindBlobs(sigs, ifirst, cutbin);
      blobs.insert(blobs.end(), subblobs.begin(), subblobs.end());
    }
    cutbin = maxbin+padmask;
    if(ilast-cutbin > padsNmin){ // search right of found peak
      subblobs = FindBlobs(sigs, cutbin, ilast);
      blobs.insert(blobs.end(), subblobs.begin(), subblobs.end());
    }
  }
  return blobs;
}


void Match::CentreOfGravity_blobs( std::vector<signal>& vsig, std::vector<signal>* CombinedPads )
{
  int nPositions=0;
  if(int(vsig.size()) < padsNmin) return;
  double time = vsig.begin()->t;
  short col = vsig.begin()->sec;

  std::vector<signal> vsig_sorted(vsig);
  signal::indexorder sigcmp_z;
  auto start = std::chrono::high_resolution_clock::now();
  std::sort(vsig_sorted.begin(), vsig_sorted.end(), sigcmp_z);
  std::vector<std::pair<double, double> > blobs = FindBlobs(vsig_sorted, 0, -1);
  
  int nfound = blobs.size();
  if( fTrace )
    std::cout<<"MatchModule::CentreOfGravity_blobs nfound: "<<nfound<<" @ t: "<<time<<" in sec: "<<col<<std::endl;

  std::vector<double> peakx, peaky; // initiliaze CoG fit
  // cut grass
  for(int i = 0; i < nfound; ++i)
    {
      bool grass(false);
      for(int j = 0; j < i; j++)
	{
	  if(abs(blobs[i].first-blobs[j].first) < goodDist)
	    if(blobs[i].second < grassCut*blobs[j].second)
	      grass = true;
	}
      if(!grass)
	{
	  if( fDebug ) 
	    std::cout << blobs[i].first << '\t';
	  peakx.push_back(blobs[i].first);
	  peaky.push_back(blobs[i].second);
	}
      else
      	{
      	  if( fDebug ) std::cout << "OOOO cut grass peak at " << blobs[i].first << std::endl;
      	}
    }
  if( fDebug ) std::cout << "\n";

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
 
  nfound=int(peakx.size());
  if( fDebug )
    std::cout<<"Match::MatchModule::CentreOfGravity_blobs nfound after grass cut: "<<nfound<<" @ t: "<<time<<" in sec: "<<col<<std::endl;
    // assert(int(peakx.size())==nfound);

  if(diagnostic)
    {
      htimeblobs->Fill(duration.count());
      hcognpeaks->Fill(nfound);
      // hcognpeaksrms->Fill(rms, nfound);
      // hcognpeakswidth->Fill(width, nfound);
    }

  fitSignals ffs( vsig_sorted, nfound );
  for(int i = 0; i < nfound; ++i)
    {
      ffs.SetStart(3*i,peaky[i]);
      ffs.SetStart(1+3*i,peakx[i]);
      ffs.SetStart(2+3*i,padSigma);
    }

  //  const std::vector<double> init = ffs.GetStart();
  // std::cout<<"init: ";
  // for( auto it=init.begin(); it!=init.end(); ++it)
  //   std::cout<<*it<<", ";
  // std::cout<<"\n";

  start = std::chrono::high_resolution_clock::now();
  ffs.Fit();
  stop = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  if( diagnostic ) htimefit->Fill(duration.count());
  int r = ffs.GetStat();
  if( r==1 ) // it's good
    {
      if( fTrace ) ffs.Print();
      for(int i = 0; i < nfound; ++i)
	{
	  double amp = ffs.GetAmplitude(i);
	  double amp_err = ffs.GetAmplitudeError(i);

	  double pos = ffs.GetMean(i);
	  double zix = ( pos + ALPHAg::_halflength ) / ALPHAg::_padpitch - 0.5;
	  int row = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	  double err = ffs.GetMeanError(i);
	  double sigma = ffs.GetSigma(i);

	  if( diagnostic )
	    {
	      // mtx.lock();
	      hcogsigma->Fill(sigma);
	      hcogerr->Fill(err);
	      int index = pmap.index(col,row);
	      hcogpadssigma->Fill(double(index),sigma);
	      hcogpadsamp->Fill(double(index),amp);
	      // double totq = ff->Integral(pos-10.*sigma,pos+10.*sigma);
	      double totq = sqrt(2.*M_PI)*sigma*amp;
	      hcogpadsint->Fill(double(index),totq);
	      hcogpadsampamp->Fill(peaky[i],amp);
	      // mtx.unlock();
	    }

	  if( err < padFitErrThres &&
	      fabs(sigma-padSigma)/padSigma < padSigmaD )
	    {
	      if( fabs(pos) < ALPHAg::_halflength )
		{
		  mtx.lock();
		  // create new signal with combined pads
		  CombinedPads->emplace_back( col, row, time, amp, amp_err, pos, err );
		  mtx.unlock();
		  //signal pad_cog( col, row, time, amp, amp_err, pos, err );
		  //padcog.push_back(pad_cog);
		  ++nPositions;
		  if( fDebug )
		    std::cout<<"CoG_blobs Combination Found! s: "<<col
			     <<" i: "<<row
			     <<" t: "<<time
			     <<" a: "<<amp
			     <<" z: "<<pos
			     <<" err: "<<err<<std::endl;
		}
	      else
		{
		  if( fDebug )
		    std::cout<<"CoG_blobs Bad Combination Found! (z outside TPC) s: "<<col
			     <<" i: "<<row
			     <<" t: "<<time
			     <<" a: "<<amp
			     <<" z: "<<pos
			     <<" err: "<<err<<std::endl;
		}
	    }
	  else // fit is crazy
	    {
	      if( fTrace )
		std::cout<<"Combination NOT found... position error: "<<err
			 <<" or sigma: "<<sigma<<std::endl;
	    }
	} // loop over blobs and their fit
    }// fit is valid
  else
    {
      if( fTrace )
	std::cout<<"\tFit Not valid with status: "<<r<<std::endl;
    }

  if( fTrace )
    std::cout<<"-------------------------------"<<std::endl;

  //  return nPositions;
}


std::vector< std::pair<signal,signal> >* Match::MatchElectrodes(std::vector<signal>* awsignals, std::vector<signal>* CombinedPads )
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
						     awsignals->end());
  std::multiset<signal, signal::timeorder> pad_bytime(CombinedPads->begin(),
						      CombinedPads->end());

  std::vector< std::pair<signal,signal> >* spacepoints=new std::vector< std::pair<signal,signal> >;
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      if( iaw->t < 0. ) continue;
      short sector = short(iaw->idx/8);
      short secwire = short(iaw->idx%8);
      if( fTrace )
	std::cout<<"Match::Match aw: "<<iaw->idx
		 <<" t: "<<iaw->t<<" pad sector: "<<sector<<std::endl;
      for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
	{
	  if( ipd->t < 0. ) continue;
	  bool tmatch=false;
	  bool pmatch=false;

          bool ampCut = (charge_dist_scale==0);

	  double delta = fabs( iaw->t - ipd->t );
	  if( delta < fCoincTime ) tmatch=true;

	  if( sector == ipd->sec ) pmatch=true;

          if( !ampCut ){
              ampCut = (ipd->height > charge_dist_scale*padThr*relCharge[secwire]);
          }

	  if( tmatch && pmatch && ampCut )
	    {
	      spacepoints->push_back( std::make_pair(*iaw,*ipd) );
	      //pad_bytime.erase( ipd );
	      ++Nmatch;
	      if( fTrace )
		std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx
			 <<"\tpad err: "<<ipd->errz<<std::endl;
	    }
	}
    }
  if( fTrace )
    std::cout<<"Match::MatchElectrodes Number of Matches: "<<Nmatch<<std::endl;
  if( int(spacepoints->size()) != Nmatch )
    std::cerr<<"Match::MatchElectrodes ERROR: number of matches differs from number of spacepoints: "<<spacepoints->size()<<std::endl;
  return spacepoints;
}


std::vector< std::pair<signal,signal> >*  Match::FakePads(std::vector<signal>* awsignals)
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
						     awsignals->end());
  std::vector< std::pair<signal,signal> >* spacepoints=new std::vector<std::pair < signal, signal>>;
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      if( iaw->t < 0. ) continue;
      short sector = short(iaw->idx/8);
      //signal fake_pad( sector, 288, iaw->t, 1., 0.0 );
      //signal fake_pad( sector, 288, iaw->t, 1., 0.0, kUnknown);
      signal fake_pad( sector, 288, iaw->t, 1., 0.0, 0.0, zed_err);
      spacepoints->push_back( std::make_pair(*iaw,fake_pad) );
      ++Nmatch;
    }
  if( int(spacepoints->size()) != Nmatch )
    std::cerr<<"Match::FakePads ERROR: number of matches differs from number of spacepoints: "<<spacepoints->size()<<std::endl;
  std::cout<<"Match::FakePads Number of Matches: "<<Nmatch<<std::endl;
  return spacepoints;
}

void Match::SortPointsAW(  const std::pair<double,int>& pos,
			   std::vector<std::pair<signal,signal>*>& vec,
			   std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw )
{
  for(auto& s: vec)
    {
      if( 1 )
	std::cout<<"\ttime: "<<pos.first
		 <<" row: "<<pos.second
		 <<" aw: "<<s->first.idx
		 <<" amp: "<<s->first.height
		 <<"   ("<<s->first.t<<", "<<s->second.idx<<")"<<std::endl;
      spaw[s->first.idx].push_back( s );
    }// vector of sp with same time and row
}
void Match::SortPointsAW(  std::vector<std::pair<signal,signal>*>& vec,
			   std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw )
//void Match::SortPointsAW(  const std::pair<double,int>& pos,
//			   std::vector<std::pair<signal,signal>*>& vec,
//			   std::map<int,std::vector<std::pair<signal,signal>*>>& spaw )
{
  for(auto& s: vec)
    {
      spaw[s->first.idx].push_back( s );
    }// vector of sp with same time and row
}

void Match::CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw,
			 std::map<int,std::vector<std::pair<signal,signal>*>>& merger)
{
  int m=-1, aw = spaw.begin()->first, q=0;
  for( auto& msp: spaw )
    {
      if( fTrace )
	std::cout<<"Match::CombPointsAW: "<<msp.first<<std::endl;
      for( auto &s: msp.second )
	{
	  if( abs(s->first.idx-aw) <= 1 )
	    {
	      merger[q].push_back( s );
	      ++m;
	    }
	  else
	    {
	      ++q;
	      merger[q].push_back( s );
	      m=0;
	    }
	  if( fTrace )
	    std::cout<<"\t"<<m
		     <<" aw: "<<s->first.idx
		     <<" amp: "<<s->first.height
		     <<" phi: "<<s->first.phi
		     <<"   ("<<s->first.t<<", "<<s->second.idx<<", "
		     << ALPHAg::_anodepitch * ( double(s->first.idx) + 0.5 )
		     <<") {"
		     <<s->first.idx%8<<", "<<s->first.idx/8<<", "<<s->second.sec<<"}"
		     <<std::endl;
	  aw = s->first.idx;
	}// vector of sp with same time and row and decreasing aw number
    }// map of sp sorted by increasing aw number
}
void Match::CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>>& spaw,
			 std::map<int,std::vector<std::pair<signal,signal>*>>& merger)
{
  int m=-1, aw = spaw.begin()->first, q=0;
  // std::cout<<"Match::CombPoints() anode: "<<aw
  //          <<" pos: "<<_anodepitch * ( double(aw) + 0.5 )<<std::endl;
  for( auto& msp: spaw )
    {
      for( auto &s: msp.second )
	{
	  if( abs(s->first.idx-aw) <= 1 )
	    {
	      merger[q].push_back( s );
	      ++m;
	    }
	  else
	    {
	      ++q;
	      merger[q].push_back( s );
	      m=0;
	    }
	  if( 0 )
	    std::cout<<"\t"<<m
		     <<" aw: "<<s->first.idx
		     <<" amp: "<<s->first.height
		     <<" phi: "<<s->first.phi
		     <<"   ("<<s->first.t<<", "<<s->second.idx<<", "
		     << ALPHAg::_anodepitch * ( double(s->first.idx) + 0.5 )
		     <<") "
		     <<std::endl;
	  aw = s->first.idx;
	}// vector of sp with same time and row and increasing aw number
    }// map of sp sorted by increasing aw number
}

uint Match::MergePoints(std::map<int,std::vector<std::pair<signal,signal>*>>& merger,
			std::vector<std::pair<signal,signal>>& merged,
			uint& number_of_merged)
{
  uint np = 0;
  for( auto &mmm: merger )
    {
      double pos=0.,amp=0.;
      double maxA=amp, amp2=amp*amp;
      if( fTrace )
	std::cout<<"==="<<mmm.first<<std::endl;
      np+=mmm.second.size();
      uint j=0, idx=j;
      int wire=-1;
      for( auto &p: mmm.second )
	{
	  double A = p->first.height,
	    pphi = p->first.phi;
	  if( 0 )
	    std::cout<<" aw: "<<p->first.idx
		     <<" amp: "<<p->first.height
		     <<" phi: "<<p->first.phi
		     <<"   ("<<p->first.t<<", "<<p->second.idx<<", "
		     << ALPHAg::_anodepitch * ( double(p->first.idx) + 0.5 )
		     <<") "<<std::endl;
	  amp += A;
	  amp2 += (A*A);
	  pos += (pphi*A);
	  if( A > maxA )
	    {
	      idx = j;
	      maxA = A;
	      wire = p->first.idx;
	    }
	  ++number_of_merged;
	  ++j;
	}
      double phi = pos/amp,
	err = phi_err*sqrt(amp2)/amp,
	H = amp/double(mmm.second.size());
      if( fTrace )
	std::cout<<"\tpnt: "<<phi<<" +/- "<<err
		 <<" A: "<<H<<" # "<<mmm.second.size()
		 <<" wire: "<<wire<<" maxA: "<<maxA
		 <<std::endl;
      for( uint i=0; i<mmm.second.size(); ++i )
	{
	  if( i == idx )
	    {
	      mmm.second.at(i)->first.height = H;
	      mmm.second.at(i)->first.phi = phi;
	      mmm.second.at(i)->first.errphi = err;
	      merged.push_back( *mmm.second.at(i) );
	      --number_of_merged;
	    }
	}
    }
  return np;
}

std::vector< std::pair<signal,signal> >* Match::CombPoints(std::vector< std::pair<signal,signal> >* spacepoints)
{
  if( fTrace )
    std::cout<<"Match::CombPoints() spacepoints size: "<<spacepoints->size()<<std::endl;

  // sort sp by row and time
  std::map<std::pair<double,int>,std::vector<std::pair<signal,signal>*>> combsp;
  for(auto &sp: *spacepoints)
    {
      double time = sp.first.t;
      int row = sp.second.idx;
      std::pair<double,int> spid(time,row);
      combsp[spid].push_back( &sp );
    }

  if( fTrace )
    std::cout<<"Match::CombPoints() comb size: "<<combsp.size()<<std::endl;
  uint n=0;
  std::vector<std::pair<signal,signal>> merged;
  uint m=0;
  for(auto &k: combsp)
    {
      n+=k.second.size();
      if( k.second.size() > 1 )
	{
	  if( fTrace )
	    std::cout<<"Match::CombPoints() vec size: "<<k.second.size()
		     <<"\ttime: "<<k.first.first
		     <<"ns row: "<<k.first.second<<std::endl;

	  // sort sp by decreasing aw number
	  std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>> spaw;
	  //                  SortPointsAW( k.first, k.second, spaw );
	  SortPointsAW( k.second, spaw );

	  std::map<int,std::vector<std::pair<signal,signal>*>> merger;
	  CombPointsAW(spaw,merger);
	  if( 0 )
	    std::cout<<"Match::CombPoints() merger size: "<<merger.size()<<std::endl;

	  uint np = MergePoints( merger, merged, m );
	  if( np != k.second.size() )
	    std::cerr<<"Match::CombPoints() ERROR tot merger size: "<<np
		     <<" vec size: "<<k.second.size()<<std::endl;
	}// more than 1 sp at the same time in the same row
      else
	{
	  merged.push_back( *k.second.at(0) );
	}
    }// map of sp sorted by row and time

  if( n != spacepoints->size() )
    std::cerr<<"Match::CombPoints() ERROR total comb size: "<<n
	     <<"spacepoints size: "<<spacepoints->size()<<std::endl;
  if( (n-merged.size()) != m )
    std::cerr<<"Match::CombPoints() ERROR spacepoints merged diff size: "<<n-merged.size()
	     <<"\t"<<m<<std::endl;

  spacepoints->assign( merged.begin(), merged.end() );
  if( fTrace ) {
    std::cout<<"Match::CombPoints() spacepoints merged size: "<<merged.size()<<" (diff: "<<m<<")"<<std::endl;
    std::cout<<"Match::CombPoints() spacepoints size (after merge): "<<spacepoints->size()<<std::endl;
  }
  return spacepoints;
}
