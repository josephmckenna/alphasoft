#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.h"
#include <set>
#include <iostream>

#include "AnalysisTimer.h"
#include "AnaSettings.h"

class MatchFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fTimeCut = false;
   bool fUseSpec = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   AnaSettings* ana_settings=NULL;
   bool fDiag = false;
   MatchFlags() // ctor
   { }

   ~MatchFlags() // dtor
   { }
};

class MatchModule: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;

private:
   double fCoincTime; // ns

   int maxPadGroups = 10; // max. number of separate groups of pads coincident with single wire signal
   double padSigma = 7.; // width of single avalanche charge distribution = 2*(pad-aw)/2.34
   double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   double padFitErrThres = 10.; // max. accepted error on pad gaussian fit mean
   bool use_mean_on_spectrum=false;
   double spectrum_mean_multiplyer = 0.33333333333; //if use_mean_on_spectrum is true, this is used.
   double spectrum_cut = 10.;              //if use_mean_on_spectrum is false, this is used.
   double spectrum_width_min = 10.;
   int minNpads = 4;            // min number of pads to attempt centre-of-gravity
   double grassCut = 0.1;       // don't consider peaks smaller than grassCut factor of a
   double goodDist = 40.;       // neighbouring peak, if that peak is closer than goodDist

   std::vector<signal> fCombinedPads;
   std::vector< std::pair<signal,signal> > spacepoints;

   double phi_err = _anodepitch*_sq12;
   double zed_err = _padpitch*_sq12;

   bool diagnostic;

   TH1D* hcognpeaks;
   TH2D* hcognpeaksrms;
   TH2D* hcognpeakswidth;
   TH1D* hcogsigma;
   TH1D* hcogerr;

public:

   MatchModule(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo), fCoincTime(16.)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModule()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
      if (fFlags->ana_settings)
         {
            std::cout<<"MatchModule::Loading AnaSettings from json"<<std::endl;
            fCoincTime = fFlags->ana_settings->GetDouble("MatchModule","coincTime");
            maxPadGroups = fFlags->ana_settings->GetDouble("MatchModule","maxPadGroups");
            padSigma = fFlags->ana_settings->GetDouble("MatchModule","padSigma");
            padSigmaD = fFlags->ana_settings->GetDouble("MatchModule","padSigmaD");
            padFitErrThres = fFlags->ana_settings->GetDouble("MatchModule","padFitErrThres");
            use_mean_on_spectrum=fFlags->ana_settings->GetBool("MatchModule","use_mean_on_spectrum");
            spectrum_mean_multiplyer = fFlags->ana_settings->GetDouble("MatchModule","spectrum_mean_multiplyer");
            spectrum_cut = fFlags->ana_settings->GetDouble("MatchModule","spectrum_cut");
            spectrum_width_min = fFlags->ana_settings->GetDouble("MatchModule","spectrum_width_min");
         }

      if( diagnostic )
         {
            runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
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
   void EndRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("MatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
         return flow;

      if(fTrace)
         printf("MatchModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      if (fFlags->fTimeCut)
         {
            if (ef->fEvent->time<fFlags->start_time)
               return flow;
            if (ef->fEvent->time>fFlags->stop_time)
               return flow;
         }

      if (fFlags->fEventRangeCut)
         {
            if (ef->fEvent->counter<fFlags->start_event)
               return flow;
            if (ef->fEvent->counter>fFlags->stop_event)
               return flow;
         }

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      if( fTrace )
         printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
      if( ! SigFlow->awSig.size() ) return flow;

      if( fTrace )
         printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));
      if( SigFlow->pdSig.size() ) //return flow;
         {
            CombinePads(&SigFlow->pdSig);
#ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module(CombinePads)");
#endif
            //if( fTrace )
            printf("MatchModule::Analyze, combined pads # %d\n", int(fCombinedPads.size()));
         }
      // allow events without pwbs
      if( fCombinedPads.size() > 0 )
         {
            SigFlow->AddPadSignals(fCombinedPads);
            Match( &SigFlow->awSig );
            CombPoints();
         }
      else
         {
            FakePads( &SigFlow->awSig );
         }

      printf("MatchModule::Analyze, Spacepoints # %d\n", int(spacepoints.size()));
      if( spacepoints.size() > 0 )
         SigFlow->AddMatchSignals( spacepoints );

      fCombinedPads.clear();

      ++fCounter;
#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module");
#endif
      return flow;
   }

   std::set<short> PartionBySector(std::vector<signal>* padsignals, std::vector< std::vector<signal> >& pad_bysec)
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

   std::vector< std::vector<signal> > PartitionByTime( std::vector<signal>& sig )
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

   std::vector<std::vector<signal>> CombPads(std::vector<signal>* padsignals)
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

   void CombinePads(std::vector<signal>* padsignals)
   {
      //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
      std::vector< std::vector<signal> > comb = CombPads( padsignals );
      fCombinedPads.clear();
      for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
         {
            CentreOfGravity(*sigv);
            //New function without fitting (3.5x faster...
            //... but does it fit well enough?):
            //CentreOfGravity_nofit(*sigv);
            //CentreOfGravity_nohisto(*sigv);
         }

      for (uint i=0; i<comb.size(); i++)
         comb.at(i).clear();
      comb.clear();
   }

   std::vector<std::pair<double, double> > FindBlobs(TH1D *h, const std::vector<int> &cumulBins){
      std::vector<std::pair<double, double> > blobs;
      double blobwidth = 5.;
      double minRMS = 2.;

      int binmask = 4;
      int minNbins = 7;

      TAxis *ax = h->GetXaxis();
      int firstbin = ax->GetFirst();
      int lastbin = ax->GetLast();

      double mean = 0.;
      double rms = 0.;
      Double_t stats[4];
      h->GetStats(stats);
      if(stats[0]){
         mean = stats[2]/stats[0];
         rms = sqrt(abs(stats[3]/stats[0] - mean*mean));
      } else {
         return blobs;
      }
      int maxbin = h->GetMaximumBin();
      double maxpos = h->GetXaxis()->GetBinCenter(maxbin);
      double max = h->GetMaximum();

      // double rms = h->GetRMS(); // This is slower, as it contains a bunch of ifs and recomputes mean
      // std::cout << "OOOOOOOOOOOOOO RMS: " << rms << " , mean: " << mean << std::endl;
      if(rms < blobwidth && abs(maxpos-mean) < blobwidth){
         if(rms > minRMS){
            // std::cout << "OOOOOOOOOOOOOO rms ok" << std::endl;
            blobs.emplace_back(maxpos, max);
         } else {
            // std::cout << "OOOOOOOOOOOOOO rms too small" << std::endl;
         }
      } else {
         bool badmax = false;
         if((lastbin < h->GetNbinsX()) && (maxbin == lastbin)){
            badmax = (h->GetBinContent(lastbin+1) > max);
         }
         if((firstbin > 1) && (maxbin == firstbin)){
            badmax = (h->GetBinContent(firstbin-1) > max);
         }
         if(!badmax){
            blobs.emplace_back(maxpos, max);
         }

         int cutbin = maxbin-binmask;
         std::vector<std::pair<double, double> > subblobs;
         if(cutbin-firstbin > minNbins){
            if(cumulBins[cutbin]-cumulBins[std::min(firstbin,0)] > minNpads){
               ax->SetRange(firstbin, cutbin);
               subblobs = FindBlobs(h, cumulBins);
               blobs.insert(blobs.end(), subblobs.begin(), subblobs.end());
            }
         }
         cutbin = maxbin+binmask;
         if(lastbin-cutbin > minNbins){
            if(cumulBins[lastbin]-cumulBins[cutbin] > minNpads){
               ax->SetRange(cutbin, lastbin);
               subblobs = FindBlobs(h, cumulBins);
               blobs.insert(blobs.end(), subblobs.begin(), subblobs.end());
            }
         }
      }
      return blobs;
   }

   void CentreOfGravity( std::vector<signal> &vsig )
   {
      if(int(vsig.size()) < minNpads) return;
      double time = vsig.begin()->t;
      short col = vsig.begin()->sec;
      TString hname = TString::Format("hhhhh_%d_%1.0f",col,time);
      //      std::cout<<hname<<std::endl;

      //////////// Make histo only as big as necessary, does this save time or cost time?
      signal::indexorder sigcmp_i;
      auto padBounds = std::minmax_element(vsig.begin(), vsig.end(), sigcmp_i);
      int p1 = padBounds.first->idx;
      int p2 = padBounds.second->idx;
      TH1D* hh = new TH1D(hname.Data(),"",p2-p1+1,p1*_padpitch-_halflength,(p2+1)*_padpitch-_halflength);
      //////////// Alternatively work with fixed size histo
      // TH1D* hh = new TH1D(hname.Data(),"",int(_padrow),-_halflength,_halflength);
      ////////////////////////
      // signal::heightorder sigcmp_h;
      // double max = std::max_element(vsig.begin(), vsig.end(), sigcmp_h)->height;
      for( auto& s: vsig )
         {
            // s.print();
            double z = ( double(s.idx) + 0.5 ) * _padpitch - _halflength;
            //hh->Fill(s.idx,s.height);
            hh->SetBinContent(hh->GetXaxis()->FindBin(z),s.height);
         }

      // TCanvas ctmp("ctmp");
      // hh->Draw();
      // ctmp.Draw();
      // ctmp.WaitPrimitive();

      // exploit wizard avalanche centroid (peak)
      int nfound = 0;
      std::vector<double> peakx, peaky;

      double rms = hh->GetRMS();
      double width = hh->FindLastBinAbove(0.2*hh->GetMaximum()) -
         hh->FindFirstBinAbove(0.2*hh->GetMaximum());
      if(fFlags->fUseSpec){
         // std::cout << "useSpec" << std::endl;
         TSpectrum spec(maxPadGroups);
         int error_level_save = gErrorIgnoreLevel;
         gErrorIgnoreLevel = kFatal;
         spec.Search(hh,1,"nodraw");
         nfound = spec.GetNPeaks();
         gErrorIgnoreLevel = error_level_save;
         for(int i = 0; i < nfound; ++i)
            {
               peakx.push_back(spec.GetPositionX()[i]);
               peaky.push_back(spec.GetPositionY()[i]);
            }
      } else {
         // std::cout << "noSpec" << std::endl;
         std::vector<int> cumulBins;
         cumulBins.push_back(0);
         for(int i = 1; i <= hh->GetNbinsX(); i++){
            cumulBins.push_back(cumulBins.back()+int(hh->GetBinContent(i)>0));
         }
         std::vector<std::pair<double, double> > blobs = FindBlobs(hh, cumulBins);
         nfound = blobs.size();
         for(int i = 0; i < nfound; ++i)
            {
               bool grass(false);
               for(int j = 0; j < i; j++){
                  if(abs(blobs[i].first-blobs[j].first) < goodDist)
                     if(blobs[i].second < grassCut*blobs[j].second)
                        grass = true;
               }
               if(!grass){
                  // std::cout << blobs[i].first << '\t';
                  peakx.push_back(blobs[i].first);
                  peaky.push_back(blobs[i].second);
               } else {
                  // std::cout << "OOOO cut grass peak at " << blobs[i].first << std::endl;
               }
            }
         // std::cout << std::endl;
      }

      hh->GetXaxis()->SetRange(0,-1);
      if(false){
         if(nfound > 1){
            TCanvas c("ctmp");
            hh->Draw();
            c.Draw();
            c.WaitPrimitive();
         }
      }
      if(diagnostic){
         hcognpeaksrms->Fill(rms, nfound);
         hcognpeakswidth->Fill(width, nfound);
         if(rms > 12 && nfound == 1){
            static int n(0);
            if(n++ < 20){
               TCanvas csav;
               hh->GetXaxis()->SetRange(hh->FindFirstBinAbove(0)-2, hh->FindLastBinAbove(0)+2);
               hh->Draw("hist");
               csav.SaveAs(TString::Format("h_1peak_%02d_rms%.3f.png", n, rms));
               hh->GetXaxis()->UnZoom();
            }
         } else if(rms < 5 && nfound > 1){
            static int n(0);
            if(n++ < 20){
               TCanvas csav;
               hh->GetXaxis()->SetRange(hh->FindFirstBinAbove(0)-2, hh->FindLastBinAbove(0)+2);
               hh->Draw("hist");
               csav.SaveAs(TString::Format("h_%dpeak_%02d_rms%.3f.png", nfound, n, rms));
               hh->GetXaxis()->UnZoom();
            }
         }
      }
      if( fTrace )
         std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;
      if( nfound > 1 && rms < spectrum_width_min )
         {
            nfound = 1;
            if( fTrace )
               std::cout<<"\tRMS is small: "<<hh->GetRMS()<<" set nfound to 1"<<std::endl;
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
                        fCombinedPads.emplace_back( col, index, time, amp, pos, err );

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
                        fCombinedPads.emplace_back( col, index, time, amp, pos, zed_err );

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

   void CentreOfGravity_nohisto( std::vector<signal> &vsig )
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
            double sigma = TMath::Sqrt(sq_sum / n - mean * mean);
            if (nfound>1 && sigma<spectrum_width_min) continue;
            //N Effective entries (TH1 style)
            double neff=((double)n)*((double)n)/((double)n2);
            double err = TMath::Sqrt(sq_sum / n - mean*mean)/TMath::Sqrt(neff);
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

   void CentreOfGravity_nofit( std::vector<signal> &vsig )
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
                  fCombinedPads.emplace_back( col, index, time, amp, pos, err );

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


   void Match(std::vector<signal>* awsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
                                                         awsignals->end());
      std::multiset<signal, signal::timeorder> pad_bytime(fCombinedPads.begin(),
                                                          fCombinedPads.end());
      spacepoints.clear();
      int Nmatch=0;
      for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
         {
            if( iaw->t < 0. ) continue;
            short sector = short(iaw->idx/8);
            //int wsec = iaw->idx%8;
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
                  //else if( abs( sector - ipd->sec ) <=1 && (wsec==0 || wsec == 7) ) pmatch=true;

                  if( tmatch && pmatch )
                     {
                        spacepoints.push_back( std::make_pair(*iaw,*ipd) );
                        ++Nmatch;
                        if( fTrace )
                           std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx<<std::endl;
                        //break;
                     }
               }
         }
      if( fTrace )
         std::cout<<"MatchModule::Match Number of Matches: "<<Nmatch<<std::endl;
   }

   void FakePads(std::vector<signal>* awsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
                                                         awsignals->end());
      spacepoints.clear();
      int Nmatch=0;
      for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
         {
            short sector = short(iaw->idx/8);
            signal fake_pad( sector, 288, iaw->t, 1., 0.0, zed_err);
            spacepoints.push_back( std::make_pair(*iaw,fake_pad) );
            ++Nmatch;
         }
      std::cout<<"MatchModule::FakePads Number of Matches: "<<Nmatch<<std::endl;
   }

   void SortPointsAW(  const std::pair<double,int>& pos,
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

   void SortPointsAW(  std::vector<std::pair<signal,signal>*>& vec,
                       std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw )
   {
      for(auto& s: vec)
         {
            spaw[s->first.idx].push_back( s );
         }// vector of sp with same time and row
   }

   void CombPointsAW(std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>>& spaw,
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

   uint MergePoints(std::map<int,std::vector<std::pair<signal,signal>*>>& merger,
                    std::vector<std::pair<signal,signal>>& merged,
                    uint& number_of_merged)
   {
      uint np = 0;
      for( auto &mmm: merger )
         {
            double pos=0.,amp=0.;
            double maxA=amp, amp2=amp*amp;
            if( fTrace )
               std::cout<<"MatchModule::MergePoints  "<<mmm.first<<std::endl;
            np+=mmm.second.size();
            uint j=0, idx=j;
            int wire=-1;
            for( auto &p: mmm.second )
               {
                  double A = p->first.height,
                     pphi = p->first.phi;
                  if( fTrace )
                     std::cout<<" aw: "<<p->first.idx
                              <<" amp: "<<p->first.height
                              <<" phi: "<<p->first.phi
                              <<"   ("<<p->first.t<<", "<<p->second.idx<<", "
                              << _anodepitch * ( double(p->first.idx) + 0.5 )
                              <<") {"
                              <<p->first.idx%8<<", "<<p->first.idx/8<<", "<<p->second.sec<<"}"
                              <<std::endl;
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

   void CombPoints()
   {
      if( fTrace )
         std::cout<<"MatchModule::CombPoints() spacepoints size: "<<spacepoints.size()<<std::endl;

      // sort sp by row and time
      std::map<std::pair<double,int>,std::vector<std::pair<signal,signal>*>> combsp;
      for(auto &sp: spacepoints)
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
                              <<" ns row: "<<k.first.second<<std::endl;

                  // sort sp by decreasing aw number
                  std::map<int,std::vector<std::pair<signal,signal>*>,std::greater<int>> spaw;
                  //                  SortPointsAW( k.first, k.second, spaw );
                  SortPointsAW( k.second, spaw );

                  std::map<int,std::vector<std::pair<signal,signal>*>> merger;
                  CombPointsAW(spaw,merger);
                  if( fTrace )
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

      if( n != spacepoints.size() )
         std::cerr<<"MatchModule::CombPoints() ERROR total comb size: "<<n
                  <<"spacepoints size: "<<spacepoints.size()<<std::endl;
      if( (n-merged.size()) != m )
         std::cerr<<"MatchModule::CombPoints() ERROR spacepoints merged diff size: "<<n-merged.size()
                  <<"\t"<<m<<std::endl;

      if( fTrace )
         std::cout<<"MatchModule::CombPoints() spacepoints merged size: "<<merged.size()
                  <<" (diff: "<<m<<")"<<std::endl;

      spacepoints.assign( merged.begin(), merged.end() );
      std::cout<<"MatchModule::CombPoints() spacepoints size (after merge): "<<spacepoints.size()<<std::endl;
   }
};


class MatchModuleFactory: public TAFactory
{
public:
   MatchFlags fFlags;

public:

   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("MatchModuleFactory::Init!\n");
      for(unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--usetimerange" )
               {
                  fFlags.fTimeCut=true;
                  i++;
                  fFlags.start_time=atof(args[i].c_str());
                  i++;
                  fFlags.stop_time=atof(args[i].c_str());
                  printf("Using time range for reconstruction: ");
                  printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
               }
            if( args[i] == "--useeventrange" )
               {
                  fFlags.fEventRangeCut=true;
                  i++;
                  fFlags.start_event=atoi(args[i].c_str());
                  i++;
                  fFlags.stop_event=atoi(args[i].c_str());
                  printf("Using event range for reconstruction: ");
                  printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
               }
            if (args[i] == "--recoff")
               fFlags.fRecOff = true;
            if (args[i] == "--useSpec")
               fFlags.fUseSpec = true;
            if( args[i] == "--diag" )
               fFlags.fDiag = true;
            if (args[i] == "--anasettings")
               {
                  i++;
                  json=args[i];
                  i++;
               }
         }
      fFlags.ana_settings=new AnaSettings(json);
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new MatchModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
