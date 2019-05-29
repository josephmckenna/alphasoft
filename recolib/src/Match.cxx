#include "Match.hh"

#include "TH1D.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "TCanvas.h"

#include <chrono>
Match::Match(std::string json)
{
  Match(new AnaSettings(json.c_str()));
}

Match::Match(AnaSettings* ana_set):fTrace(false),fDebug(false)
{
  ana_settings=ana_set;
  std::cout<<"Match::Loading AnaSettings from json"<<std::endl;
  fCoincTime = ana_settings->GetDouble("MatchModule","coincTime");
  maxPadGroups = ana_settings->GetDouble("MatchModule","maxPadGroups");
  padsNmin = ana_settings->GetInt("MatchModule","padsNmin");
  padSigma = ana_settings->GetDouble("MatchModule","padSigma");
  padSigmaD = ana_settings->GetDouble("MatchModule","padSigmaD");
  padFitErrThres = ana_settings->GetDouble("MatchModule","padFitErrThres");
  use_mean_on_spectrum=ana_settings->GetBool("MatchModule","use_mean_on_spectrum");
  spectrum_mean_multiplyer = ana_settings->GetDouble("MatchModule","spectrum_mean_multiplyer");
  spectrum_cut = ana_settings->GetDouble("MatchModule","spectrum_cut");
  spectrum_width_min = ana_settings->GetDouble("MatchModule","spectrum_width_min");


  TString CentreOfGravity=ana_settings->GetString("MatchModule","CentreOfGravityMethod");
  if ( CentreOfGravity.EqualTo("CentreOfGravity") ) CentreOfGravityFunction=0;
  if ( CentreOfGravity.EqualTo("CentreOfGravity_nofit") ) CentreOfGravityFunction=1;
  if ( CentreOfGravity.EqualTo("CentreOfGravity_nohisto") ) CentreOfGravityFunction=2;
  if ( CentreOfGravity.EqualTo("CentreOfGravity_single_peak") ) CentreOfGravityFunction=3;
  if ( CentreOfGravity.EqualTo("CentreOfGravity_multi_peak") ) CentreOfGravityFunction=4;
  if ( CentreOfGravityFunction < 0 )
  {
    std::cout<<"Match:No valid CentreOfGravityMethod function in json"<<std::endl;
    exit(1);
  }
  else
    std::cout<<"Using CentreOfGravity case:"<<CentreOfGravityFunction<<std::endl;

  phi_err = _anodepitch*_sq12;
  zed_err = _padpitch*_sq12;
  hsig = new TH1D("hpadRowSig","sigma of pad combination fit",1000,0,50);

}

Match::~Match()
{

}

void Match::Init()
{
  fCombinedPads=NULL;//new std::vector<signal>;
  spacepoints=NULL;//new std::vector< std::pair<signal,signal> >;
}

void Match::Setup(TFile* OutputFile)
{
    if( diagnostic )
      {
         OutputFile->cd(); // select correct ROOT directory
         gDirectory->mkdir("padmatch")->cd();
         hcognpeaks = new TH1D("hcognpeaks","CombPads CoG - Number of Avals",int(maxPadGroups+1.),
                              0.,maxPadGroups+1.);
         hcognpeaksrms = new TH2D("hcognpeaksrms","CombPads CoG - Number of Avals vs RMS", 500, 0., 50,int(maxPadGroups+1.),
                                  0.,maxPadGroups+1.);
         hcognpeakswidth = new TH2D("hcognpeakswidth","CombPads CoG - Number of Avals vs width", 20, 0., 20,int(maxPadGroups+1.),
                                  0.,maxPadGroups+1.);
         hcogsigma = new TH1D("hcogsigma","CombPads CoG - Sigma Charge Induced;[mm]",700,0.,70.);
         hcogerr = new TH1D("hcogerr","CombPads CoG - Error on Mean;[mm]",2000,0.,20.);
       }
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
      if (comb.size()==0) return;
      fCombinedPads=new std::vector<signal>;
      //std::cout<<"Using CentreOfGravityFunction"<<CentreOfGravityFunction<<std::endl;
      switch(CentreOfGravityFunction) {
         case 0: {
            for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
               CentreOfGravity(*sigv);
            break;
         }
         case 1: {
            for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
               CentreOfGravity_nofit(*sigv);
            break;
         }
         case 2: {
            for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
               CentreOfGravity_nohisto(*sigv);
            break;
         }
         case 3: {
            for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
               CentreOfGravity_single_peak(*sigv);
            break;
         }
         case 4: {
            for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
               CentreOfGravity_multi_peak(*sigv);
            break;
         }
      }
      for (uint i=0; i<comb.size(); i++)
         comb.at(i).clear();
      comb.clear();
      //return CombinedPads;
}

std::vector<std::pair<double, double> > FindBlobs(TH1D *h){
   std::vector<std::pair<double, double> > blobs;
   double blobwidth = 8.;
   double minRMS = 3.;
   Double_t stats[4];
   h->GetStats(stats);
   double mean = stats[2]/stats[0];
   double rms = sqrt(abs(stats[3]/stats[0] - mean*mean));
   // double rms = h->GetRMS(); // This is slower, as it contains a bunch of ifs and recomputes mean
   std::cout << "OOOOOOOOOOOOOO RMS: " << rms << std::endl;
   if(rms < blobwidth){
      if(rms > minRMS)
         blobs.emplace_back(mean, h->GetMaximum());
   } else {
      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();
      h->GetXaxis()->SetRangeUser(xmin, mean);
      std::vector<std::pair<double, double> > subblobs = FindBlobs(h);
      for(auto b: subblobs){
         blobs.push_back(b);
      }
      h->GetXaxis()->SetRangeUser(mean, xmax);
      subblobs = FindBlobs(h);
      for(auto b: subblobs){
         blobs.push_back(b);
      }
   }
   return blobs;
}

void Match::CentreOfGravity( std::vector<signal> &vsig )
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
      TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);
      for( auto& s: vsig )
         {
            // s.print();
            double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
            //hh->Fill(s.idx,s.height);
            hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
         }

      // exploit wizard avalanche centroid (peak)
      TSpectrum spec(maxPadGroups);
      int error_level_save = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      spec.Search(hh,1,"nodraw");
      int nfound = spec.GetNPeaks();
      if( diagnostic )
         hcognpeaks ->Fill(nfound);

      gErrorIgnoreLevel = error_level_save;

      if( fTrace )
         std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;
      if( nfound > 1 && hh->GetRMS() < spectrum_width_min )
         {
            nfound = 1;
            if( fTrace )
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

            int r = hh->Fit(ff,"B0NQ","");
#ifdef RESCUE_FIT
            bool stat=true;
#endif
            if( r==0 ) // it's good
               {
                  // make sure that the fit is not crazy...
                  double sigma = ff->GetParameter(2);
                  double err = ff->GetParError(1);
                  if( diagnostic )
                     {
                        hcogsigma->Fill(sigma);
                        hcogerr->Fill(err);
                     }
                  if( err < padFitErrThres &&
                      fabs(sigma-padSigma)/padSigma < padSigmaD )
                     //if( err < padFitErrThres && sigma > 0. )
                     {
                        double amp = ff->GetParameter(0);
                        double pos = ff->GetParameter(1);
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        fCombinedPads->emplace_back( col, index, time, amp, pos, err );

                        if( fTrace )
                           std::cout<<"Combination Found! s: "<<col
                                    <<" i: "<<index
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
                        double amp = tot/11.;
                        double pos = zcoord/tot;
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        CombinedPads->emplace_back( col, index, time, amp, pos, zed_err );

                        if( fTrace )
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


void Match::CentreOfGravity_blob( std::vector<signal> &vsig )
{
    if(vsig.size() < (unsigned int)padsNmin) return;
  double time = vsig.begin()->t;
  short col = vsig.begin()->sec;
  TString hname = TString::Format("hhhhh_%d_%1.0f",col,time);

  TCanvas c;
  //      std::cout<<hname<<std::endl;
  TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);
  for( auto& s: vsig )
    {
      // s.print();
      double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
      //hh->Fill(s.idx,s.height);
      hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
    }

  // exploit wizard avalanche centroid (peak)
  auto startSpec = std::chrono::steady_clock::now();
  TSpectrum spec(maxPadGroups);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  spec.Search(hh,1,"nodraw");
  int nfound = spec.GetNPeaks();
  auto endSpec = std::chrono::steady_clock::now();
  std::vector<std::pair<double, double> > blobs = FindBlobs(hh);
  auto endBlobs = std::chrono::steady_clock::now();
  std::cout << "OOOOOOOOOOOOOOOOOOOOOOOOOO " << nfound << " ---------- " << blobs.size() << std::endl;
  static int specTime(0), blobTime(0);
  specTime += std::chrono::duration_cast<std::chrono::nanoseconds>(endSpec - startSpec).count();
  blobTime += std::chrono::duration_cast<std::chrono::nanoseconds>(endBlobs - endSpec).count();
  std::cout << "OOOOOOOOOOO cumulative time for spec: " << specTime << std::endl;
  std::cout << "OOOOOOOOOOO cumulative time for blob: " << blobTime << std::endl;
  std::cout << "OOOOOOOOOOO ratio: " << double(blobTime)/double(specTime) << std::endl;

  gErrorIgnoreLevel = error_level_save;

  nfound = blobs.size();
  if( fTrace )
    std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;
  if( nfound > 1 && hh->GetRMS() < spectrum_width_min )
    {
      nfound = 1;
      if( fTrace )
	std::cout<<"\tRMS is small: "<<hh->GetRMS()<<" set nfound to 1"<<std::endl;
    }

  // double peakx[nfound];
  // double peaky[nfound];
  // for(int i = 0; i < nfound; ++i)
  //   {
  //     peakx[i]=spec.GetPositionX()[i];
  //     peaky[i]=spec.GetPositionY()[i];
  //   }

  double peakx[nfound];
  double peaky[nfound];
  for(int i = 0; i < nfound; ++i)
    {
       peakx[i]=blobs[i].first;
       peaky[i]=blobs[i].second;
    }

  if(nfound > 1 && fDebug){
      std::cout << "XXXXXXXXXXXXXXXXXXXX " << nfound << " peaks for col " << col << " time " << time << std::endl;
  }
  for(int i = 0; i < nfound; ++i)
    {
      TString ffname = TString::Format("fffff_%d_%1.0f_%d",col,time,i);
      TF1* ff = new TF1(ffname.Data(),"gaus(0)",peakx[i]-10.*padSigma,peakx[i]+10.*padSigma);
      // initialize gaussians with peak finding wizard
      ff->SetParameter(0,peaky[i]);
      ff->SetParameter(1,peakx[i]);
      ff->SetParameter(2,padSigma);

      int r = hh->Fit(ff,"B0NQ","");
#ifdef RESCUE_FIT
      bool stat=true;
#endif
      if( r==0 ) // it's good
	{
	  // make sure that the fit is not crazy...
	  double sigma = ff->GetParameter(2);
          hsig->Fill(sigma);
	  double err = ff->GetParError(1);
	  if( err < padFitErrThres &&
	      fabs(sigma-padSigma)/padSigma < padSigmaD )
	    //if( err < padFitErrThres && sigma > 0. )
	    {
	      double amp = ff->GetParameter(0);
	      double pos = ff->GetParameter(1);
	      double zix = ( pos + _halflength ) / _padpitch - 0.5;
	      int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	      // create new signal with combined pads
	      fCombinedPads->emplace_back( col, index, time, amp, pos, err );

              // if(abs(pos) > 2.)
              //     std::cout << "XXXXXXXXXXXXXXXXXXXX far away from track, z = " << pos << " for col " << col << " time " << time << ", error " << err << std::endl;
	      if( fTrace )
		std::cout<<"Combination Found! s: "<<col
			 <<" i: "<<index
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
	      double amp = tot/11.;
	      double pos = zcoord/tot;
	      double zix = ( pos + _halflength ) / _padpitch - 0.5;
	      int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	      // create new signal with combined pads
	      fCombinedPads->emplace_back( col, index, time, amp, pos );

	      if( fTrace )
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

void Match::CentreOfGravity_nofit( std::vector<signal> &vsig )
{
  if(!vsig.size()) return;
  double time = vsig.begin()->t;
  short col = vsig.begin()->sec;
  TString hname = TString::Format("hhhhh_%d_%1.0f",col,time);
  //      std::cout<<hname<<std::endl;
  TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);

  for( auto& s: vsig )
    {
      // s.print();
      double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
      //hh->Fill(s.idx,s.height);
      hh->Fill(z,s.height);
    }

  // exploit wizard avalanche centroid (peak)
  TSpectrum spec(maxPadGroups);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  spec.Search(hh,1,"nodraw");
  int nfound = spec.GetNPeaks();
  gErrorIgnoreLevel = error_level_save;

  if( fTrace )
    std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;
  if( nfound > 1 && hh->GetRMS() < spectrum_width_min )
    {
      nfound = 1;
      if( fTrace )
	std::cout<<"\tRMS is small: "<<hh->GetRMS()<<" set nfound to 1"<<std::endl;
    }

  double peakx[nfound];
  //double peaky[nfound];

  for(int i = 0; i < nfound; ++i)
    {
      peakx[i]=spec.GetPositionX()[i];
      //peaky[i]=spec.GetPositionY()[i];
      TString hname = TString::Format("hhhhhh_%d_%1.0f",col,time);
      double min=peakx[i]-5.*padSigma;
      double max=peakx[i]+5.*padSigma;
      int bins=(max-min)/_padpitch;
      TH1D* hhh = new TH1D(hname.Data(),"",bins,min,max);
      for( auto& s: vsig )
	{
	  // s.print();
	  double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
	  if (z<min) continue;
	  if (z>max) continue;
	  hhh->Fill(z,s.height);
	}

      bool stat=true;
      double sigma = hhh->GetRMS();
      double err = hhh->GetMeanError();
      if( sigma == 0. || err == 0. ) stat=false;
      if( err < padFitErrThres &&
	  fabs(sigma-padSigma)/padSigma < padSigmaD && stat )
	{
	  double amp = hhh->GetBinContent(hhh->GetMaximumBin());
	  double pos = hhh->GetMean();
	  double zix = ( pos + _halflength ) / _padpitch - 0.5;
	  int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

	  // create new signal with combined pads
	  fCombinedPads->emplace_back( col, index, time, amp, pos, err );

	  if( fTrace )
	    std::cout<<"Combination Found! s: "<<col
		     <<" i: "<<index
		     <<" t: "<<time
		     <<" a: "<<amp
		     <<" z: "<<pos
		     <<" err: "<<err<<std::endl;
	}
      delete hhh;
    } // wizard peak finding failed
  delete hh;
  if( fTrace )
    std::cout<<"-------------------------------"<<std::endl;
}


void Match::CentreOfGravity_single_peak( std::vector<signal> &vsig )
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
      TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);
      for( auto& s: vsig )
         {
            // s.print();
            double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
            //hh->Fill(s.idx,s.height);
            hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
         }

      int nfound=1;

      double peakx[nfound];
      double peaky[nfound];

      for(int i = 0; i < nfound; ++i)
         {
            peakx[i]=hh->GetMean();
            peaky[i]=hh->GetMaximum();
            TString ffname = TString::Format("fffff_%d_%1.0f_%d",col,time,i);
            TF1* ff = new TF1(ffname.Data(),"gaus(0)",peakx[i]-10.*padSigma,peakx[i]+10.*padSigma);
            // initialize gaussians with peak finding wizard
            ff->SetParameter(0,peaky[i]);
            ff->SetParameter(1,peakx[i]);
            ff->SetParameter(2,padSigma);

            int r = hh->Fit(ff,"B0NQ","");
#ifdef RESCUE_FIT
            bool stat=true;
#endif
            if( r==0 ) // it's good
               {
                  // make sure that the fit is not crazy...
                  double sigma = ff->GetParameter(2);
                  double err = ff->GetParError(1);
                  if( diagnostic )
                     {
                        hcogsigma->Fill(sigma);
                        hcogerr->Fill(err);
                     }
                  if( err < padFitErrThres &&
                      fabs(sigma-padSigma)/padSigma < padSigmaD )
                     //if( err < padFitErrThres && sigma > 0. )
                     {
                        double amp = ff->GetParameter(0);
                        double pos = ff->GetParameter(1);
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        fCombinedPads->emplace_back( col, index, time, amp, pos, err );

                        if( fTrace )
                           std::cout<<"Combination Found! s: "<<col
                                    <<" i: "<<index
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
                        double amp = tot/11.;
                        double pos = zcoord/tot;
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        fCombinedPads->emplace_back( col, index, time, amp, pos, zed_err );

                        if( fTrace )
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

void Match::CentreOfGravity_multi_peak( std::vector<signal> &vsig )
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
      TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);
      double total_height=0;
      for( auto& s: vsig ) //signal
         {
            // s.print();
            double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
            //hh->Fill(s.idx,s.height);
            hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
            total_height+=s.height;
         }
      double threshold=total_height/(double)vsig.size();
      int min_peak_spacing=4;
      std::vector<double> peakx;
      std::vector<double> peaky;
      int nfound=0;
      double lasty=hh->GetBinContent(1);
      int lastx=0;
      //int lastx=0;
      // Loop over all bins, skip the first bin (as lasty is already set
      for (int i=2; i<int(_padrow); i++)
      {

         double h=hh->GetBinContent(i);
         //If above threshold and lower than last bin
         if (h>threshold && h<lasty )
         {
            //std::cout<<i<<"\t"<<h<<">"<<threshold<<" at "<<hh->GetBinCenter(i)<<std::endl;
            //std::cout<<"fabs("<<hh->GetBinCenter(i)<<"-"<<hh->GetBinCenter(lastx)<<")<"<<padSigma<<std::endl;
            if (fabs(hh->GetBinCenter(i)-hh->GetBinCenter(lastx))<10.*padSigma)
            {
               //std::cout<<"Too soon after last bin"<<std::endl;
               continue;
            }
            lastx=i-1;
            peakx.push_back(hh->GetBinCenter(lastx));
            peaky.push_back(hh->GetBinContent(lastx));
            //std::cout<<"x:"<<hh->GetBinCenter(lastx)<<"\ty:"<<hh->GetBinContent(lastx)<<std::endl;
            nfound++;
         }
       
         lasty=h;
      }
      //if (nfound) std::cout<<"Nfound:"<<nfound<<std::endl;

      for(int i = 0; i < nfound; ++i)
         {
            TString ffname = TString::Format("fffff_%d_%1.0f_%d",col,time,i);
            TF1* ff = new TF1(ffname.Data(),"gaus(0)",peakx[i]-10.*padSigma,peakx[i]+10.*padSigma);
            // initialize gaussians with peak finding wizard
            ff->SetParameter(0,peaky[i]);
            ff->SetParameter(1,peakx[i]);
            ff->SetParameter(2,padSigma);

            int r = hh->Fit(ff,"B0NQ","");
#ifdef RESCUE_FIT
            bool stat=true;
#endif
            if( r==0 ) // it's good
               {
                  // make sure that the fit is not crazy...
                  double sigma = ff->GetParameter(2);
                  double err = ff->GetParError(1);
                  if( fTrace )
                     {
                        hcogsigma->Fill(sigma);
                        hcogerr->Fill(err);
                     }
                  if( err < padFitErrThres &&
                      fabs(sigma-padSigma)/padSigma < padSigmaD )
                     //if( err < padFitErrThres && sigma > 0. )
                     {
                        double amp = ff->GetParameter(0);
                        double pos = ff->GetParameter(1);
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        fCombinedPads->emplace_back( col, index, time, amp, pos, err );

                        if( fTrace )
                           std::cout<<"Combination Found! s: "<<col
                                    <<" i: "<<index
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
                        double amp = tot/11.;
                        double pos = zcoord/tot;
                        double zix = ( pos + _halflength ) / _padpitch - 0.5;
                        int index = (zix - floor(zix)) < 0.5 ? int(floor(zix)):int(ceil(zix));

                        // create new signal with combined pads
                        fCombinedPads->emplace_back( col, index, time, amp, pos, zed_err );

                        if( fTrace )
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
	  fCombinedPads->emplace_back( col, index, time, amp, pos, err );

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
  std::multiset<signal, signal::timeorder> pad_bytime(fCombinedPads->begin(),
						      fCombinedPads->end());
  if (spacepoints) delete spacepoints;
  spacepoints=new std::vector< std::pair<signal,signal> >;
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      if( iaw->t < 0. ) continue;
      short sector = short(iaw->idx/8);
      if( fTrace )
	std::cout<<"MatchModule::Match aw: "<<iaw->idx
		 <<" t: "<<iaw->t<<" pad sector: "<<sector<<std::endl;
      for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
	{
      if( ipd->t < 0. ) continue;
	  bool tmatch=false;
	  bool pmatch=false;

	  double delta = fabs( iaw->t - ipd->t );
	  if( delta < fCoincTime ) tmatch=true;

	  if( sector == ipd->sec ) pmatch=true;

	  if( tmatch && pmatch )
	    {
	      spacepoints->push_back( std::make_pair(*iaw,*ipd) );
	      //pad_bytime.erase( ipd );
	      ++Nmatch;
	      if( fTrace )
		std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx<<std::endl;
	    }
	}
    }
  //  if( fTrace )
  std::cout<<"MatchModule::Match Number of Matches: "<<Nmatch<<std::endl;
  if( int(spacepoints->size()) != Nmatch )
    std::cerr<<"Match::MatchElectrodes ERROR: number of matches differs from number of spacepoints: "<<spacepoints->size()<<std::endl;
}

void Match::FakePads(std::vector<signal>* awsignals)
{
  std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
						     awsignals->end());
  if (spacepoints) spacepoints->clear();
  spacepoints=new std::vector<std::pair < signal, signal>>;
  int Nmatch=0;
  for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
    {
      short sector = short(iaw->idx/8);
      //signal fake_pad( sector, 288, iaw->t, 1., 0.0 );
      signal fake_pad( sector, 288, iaw->t, 1., 0.0, kUnknown);
      spacepoints->push_back( std::make_pair(*iaw,fake_pad) );
      ++Nmatch;
    }
  std::cout<<"MatchModule::FakePads Number of Matches: "<<Nmatch<<std::endl;
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
               std::cout<<"MatchModule::CombPointsAW: "<<msp.first<<std::endl;
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
                              << _anodepitch * ( double(s->first.idx) + 0.5 )
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
  // std::cout<<"MatchModule::CombPoints() anode: "<<aw
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
		     << _anodepitch * ( double(s->first.idx) + 0.5 )
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
		     << _anodepitch * ( double(p->first.idx) + 0.5 )
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

void Match::CombPoints()
{
  std::cout<<"MatchModule::CombPoints() spacepoints size: "<<spacepoints->size()<<std::endl;

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
    std::cout<<"MatchModule::CombPoints() comb size: "<<combsp.size()<<std::endl;
  uint n=0;
  std::vector<std::pair<signal,signal>> merged;
  uint m=0;
  for(auto &k: combsp)
    {
      n+=k.second.size();
      if( k.second.size() > 1 )
	{
	  if( fTrace )
	    std::cout<<"MatchModule::CombPoints() vec size: "<<k.second.size()
		     <<"\ttime: "<<k.first.first
		     <<"ns row: "<<k.first.second<<std::endl;

	    // sort sp by decreasing aw number
	    std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>> spaw;
	    //                  SortPointsAW( k.first, k.second, spaw );
	    SortPointsAW( k.second, spaw );

	  std::map<int,std::vector<std::pair<signal,signal>*>> merger;
	  CombPointsAW(spaw,merger);
	  if( 0 )
	    std::cout<<"MatchModule::CombPoints() merger size: "<<merger.size()<<std::endl;

	  uint np = MergePoints( merger, merged, m );
	  if( np != k.second.size() )
	    std::cerr<<"MatchModule::CombPoints() ERROR tot merger size: "<<np
		     <<" vec size: "<<k.second.size()<<std::endl;
	}// more than 1 sp at the same time in the same row
      else
	{
	  merged.push_back( *k.second.at(0) );
	}
    }// map of sp sorted by row and time

  if( n != spacepoints->size() )
    std::cerr<<"MatchModule::CombPoints() ERROR total comb size: "<<n
	     <<"spacepoints size: "<<spacepoints->size()<<std::endl;
  if( (n-merged.size()) != m )
    std::cerr<<"MatchModule::CombPoints() ERROR spacepoints merged diff size: "<<n-merged.size()
	     <<"\t"<<m<<std::endl;

  //if( fTrace )
  std::cout<<"MatchModule::CombPoints() spacepoints merged size: "<<merged.size()<<" (diff: "<<m<<")"<<std::endl;

  spacepoints->assign( merged.begin(), merged.end() );
  std::cout<<"MatchModule::CombPoints() spacepoints size (after merge): "<<spacepoints->size()<<std::endl;
}
