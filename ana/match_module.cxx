#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TFitResult.h"
//#include "Math/MinimizerOptions.h"

#include "SignalsType.h"
#include <set>
#include <iostream>

// #include <future>
// #include <atomic>         // std::atomic
// #include <thread>         // std::thread

class MatchFlags
{
   // public:
   //    bool fExportWaveforms = false;

public:
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
   int fCounter = 0;

private:
   // pads
   TH1D* hNhitPad;
   TH1D* hOccRow;
   TH1D* hOccCol;
   TH2D* hOccPad;

   TH1D* hAmpPad;
   TH1D* hTimePad;

   TH2D* hTimeAmpPad;

   TH2D* hTimePadCol;
   TH2D* hTimePadRow;
   TH2D* hAmpPadCol;
   TH2D* hAmpPadRow;

   TH1D* hNcpads;   

   // match
   TH2D* hawcol;
   // TH2D* hawcol_timecut;
   // TH2D* hawcol_colcut;
   // TH2D* hawcol_timecolcut;
   TH2D* hamprow_timecolcut;

   TH1D* hNmatch;

   TH2D* hawcol_time;
   TH2D* hawcol_sector_time;
   TH2D* hawcol_deltat_sec;

   TH2D* hawcol_match;
   TH2D* hawcol_match_amp;
   TH2D* hawcol_match_time;
   
   double fCoincTime; // ns

   int maxPadGroups = 10; // max. number of separate groups of pads coincident with single wire signal
   // double padSigma = 1.5; // width of single avalanche charge distribution
   // double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   // //   double padSigmaD = 0.25;
   // double padFitErrThres = 1.; // max. accepted error on pad gaussian fit mean
   double padSigma = 7.; // width of single avalanche charge distribution
   double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   double padFitErrThres = 10.; // max. accepted error on pad gaussian fit mean

   std::vector<signal> combpad;
   std::vector< std::pair<signal,signal> > spacepoints;
   
public:

   MatchModule(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo), fCoincTime(16.)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;   
   }

   ~MatchModule()
   {
      if (fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // anodes histograms
      gDirectory->mkdir("match_el")->cd();

      hNhitPad = new TH1D("hNhitcPad","Number of Hits Combined Pad;N",500,0.,5000.);
      hOccRow = new TH1D("hOcccRow","Number of Hits Combined Pad Rows;N",576,0.,576.);
      hOccCol = new TH1D("hOcccCol","Number of Hits Combined Pad Cols;N",32,0.,32.);
      hOccPad = new TH2D("hOcccPad","Number of Hits Combined Pads;N",576,0.,576.,32,0.,32.);

      hAmpPad = new TH1D("hAmpcPad","Reconstructed Avalanche Size Combined Pad",200,0.,10000.);
      hTimePad = new TH1D("hTimecPad","Reconstructed Avalanche Time Combined Pad",375,0.,6000.);

      hTimeAmpPad = new TH2D("hTimeAmpcPad",
                             "Reconstructed Avalanche Time Vs Size - Combined Pad",
                             40,0.,6000.,20,0.,10000.);
      
      hTimePadCol = new TH2D("hTimecPadCol",
                             "Reconstructed Avalanche Time Vs Combined Pad Cols",
                             32,0.,32.,40,0.,6000.);
      hTimePadRow = new TH2D("hTimecPadRow",
                             "Reconstructed Avalanche Time Vs Combined Pad Rows",
                             576,0.,576,40,0.,6000.);
      hAmpPadCol = new TH2D("hAmpcPadCol",
                            "Reconstructed Avalanche Size Vs Combined Pad Cols",
                            32,0.,32.,20,0.,10000.);
      hAmpPadRow = new TH2D("hAmpcPadRow",
                            "Reconstructed Avalanche Size Vs Combined Pad Rows",
                            576,0.,576,20,0.,10000.);

      hNcpads = new TH1D("hNcpads","Number of Combined Pads",500,0.,5000.);

      hawcol = new TH2D("hawcol","Match Electrodes;AW;PAD COL",256,0.,256.,32,0.,32.);
      // hawcol_timecut = new TH2D("hawcol_timecut","Match Electrodes Time Cut;AW;PAD COL",
      //   			256,0.,256.,32,0.,32.);
      // hawcol_colcut = new TH2D("hawcol_colcut",
      //                          "Match Electrodes Sector Cut;AW;PAD COL",
      //                          256,0.,256.,32,0.,32.);
      // hawcol_timecolcut = new TH2D("hawcol_timecolcut",
      //                              "Match Electrodes Time && Sector Cut;AW;PAD COL",
      //                              256,0.,256.,32,0.,32.);
      hamprow_timecolcut = new TH2D("hamprow_timecolcut",
                                    "Pad Amplitude By Row - Matched Electrodes by Time && Sector Cut;PAD ROW",
                                    576,0.,576.,300,0.,6000.);

      hNmatch = new TH1D("hNmatch","Number of AW*PAD matches",500,0.,5000.);  

      hawcol_time = new TH2D("hawcol_time","AW vs PAD Time;AW [ns];PAD [ns]",375,0.,6000.,375,0.,6000.);
      hawcol_sector_time = new TH2D("hawcol_sector_time","AW vs PAD Time with Matching Sector;AW [ns];PAD [ns]",375,0.,6000.,375,0.,6000.);
      hawcol_deltat_sec = new TH2D("hawcol_deltat_sec","AW vs PAD col with Matching Time;AW;PAD COL",256,0.,256.,32,0.,32.);
      
      hawcol_match = new TH2D("hawcol_match",
                              "Match Electrodes Time && Sector Cut;AW;PAD COL",
                              256,0.,256.,32,0.,32.);
      hawcol_match_amp = new TH2D("hawcol_match_amp",
                                  "Amplitude of Matching Electrodes Time && Sector Cut;AW;PAD COL",
                                  200,0.,2000.,200,0.,10000.);   
      hawcol_match_time = new TH2D("hawcol_match_time",
                                   "Time of Matching Electrodes Time && Sector Cut;AW [ns];PAD [ns]",
                                   375,0.,6000.,375,0.,6000.);
   }

   void EndRun(TARunInfo* runinfo)
   {
      //    if (fTrace)
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
      printf("MatchModule::Analyze, run %d, counter %d\n", 
             runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();
     
      if (!ef || !ef->fEvent)
         return flow;
     
      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;

      //      if( fTrace )
      printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
      if( ! SigFlow->awSig.size() ) return flow;

      //      if( fTrace )
      printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));
      if( ! SigFlow->pdSig.size() ) return flow;

      //      PlotMatch(&SigFlow->awSig, &SigFlow->pdSig);

      CombinePads(&SigFlow->pdSig);
      //if( fTrace )
      printf("MatchModule::Analyze, combined pads # %d\n", int(combpad.size()));

      if( combpad.size() > 0 )
         {
            CombinedPADdiagnostic();
            SigFlow->AddPadSignals(combpad);
            Match( &SigFlow->awSig );
            //            PlotMatch( &SigFlow->awSig, &combpad);
         }

      if( spacepoints.size() > 0 )
         SigFlow->AddMatchSignals( spacepoints );

      ++fCounter;
      return flow;
   }

   std::set<short> PartionBySector(std::vector<signal>* padsignals, std::vector< std::vector<signal> >& pad_bysec)
   {
      std::set<short> secs;
      pad_bysec.resize(32);
      pad_bysec.clear();
      for( auto ipd=padsignals->begin(); ipd!=padsignals->end(); ++ipd )
         {
            //ipd->print();
            secs.insert( ipd->sec );
            pad_bysec[ipd->sec].push_back(*ipd);
         }
      return secs;
   }
   
   std::vector< std::vector<signal> > PartitionByTime( std::vector<signal> sig )
   {     
      std::multiset<signal, signal::timeorder> sig_bytime(sig.begin(), 
                                                          sig.end());
      double temp=-999999.;
      std::vector< std::vector<signal> > pad_bytime;
      for( auto isig = sig_bytime.begin(); isig!=sig_bytime.end(); ++isig )
         {
            // if( isig->t==temp )
            //    {   
            //       pad_bytime.back().push_back( *isig );
            //    }
            // else
            //    {
            //       temp=isig->t;
            //       pad_bytime.emplace_back();
            //       pad_bytime.back().push_back( *isig );
            //    }
            if( isig->t > temp )
               {
                  temp=isig->t;
                  pad_bytime.emplace_back();
                  pad_bytime.back().push_back( *isig );
               }
            else
               pad_bytime.back().push_back( *isig );
         }
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
         }
      return comb;
   }
   
   void CombinePads(std::vector<signal>* padsignals)
   {      
      //     ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
      std::vector< std::vector<signal> > comb = CombPads( padsignals );
      //std::vector< std::future<std::vector<signal>> > ccpf;
      // std::vector<std::thread> threads;
      combpad.clear();
      for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
         {
            std::vector<signal> time_slice = *sigv;
            // ccpf.push_back( std::async( std::launch::async,
            //                             &MatchModule::CentreOfGravity, 
            //                             this, std::ref(time_slice) ) );
            // ccpf.push_back( std::async( std::launch::deferred,
            //                             &MatchModule::CentreOfGravity, 
            //                             this, std::ref(time_slice) ) );

            // threads.push_back( std::thread(&MatchModule::CentreOfGravity, 
            //                                this, std::ref(time_slice) ) );

            CentreOfGravity(time_slice);
         }

      // combpad.clear();
      // for(unsigned n=0;n<ccpf.size();n++) 
      //    {
      //       std::vector<signal> csv = ccpf[n].get();
      //       for( auto& cs: csv )
      //          combpad.push_back( cs );
      //    }
      //for (auto& th : threads) th.join();
      hNcpads->Fill( double(combpad.size()) );
   }

   

   //std::vector<signal> CentreOfGravity( std::vector<signal> vsig )
   void CentreOfGravity( std::vector<signal> &vsig )
   {
      //std::vector<signal> cpad;
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
      int nfound = spec.Search(hh,1,"nodraw");
      gErrorIgnoreLevel = error_level_save;

      if( fTrace )
         std::cout<<"MatchModule::CombinePads nfound: "<<nfound<<" @ t: "<<time<<std::endl;
      if( nfound > 1 && hh->GetRMS() < 10. )
         {
            nfound = 1;
            if( fTrace )
               std::cout<<"\tRMS is small: "<<hh->GetRMS()<<" set nfound to 1"<<std::endl;
         }

      double *peakx = spec.GetPositionX();
      double *peaky = spec.GetPositionY();
      
      for(int i = 0; i < nfound; ++i)
         {
            TString ffname = TString::Format("fffff_%d_%1.0f_%d",col,time,i);
            TF1* ff = new TF1(ffname.Data(),"gaus(0)",peakx[i]-10.*padSigma,peakx[i]+10.*padSigma);
            // initialize gaussians with peak finding wizard
            ff->SetParameter(0,peaky[i]);
            ff->SetParameter(1,peakx[i]);
            ff->SetParameter(2,padSigma);
            TFitResultPtr r = hh->Fit(ff,"BS0NQ",""); // CHECK ME!!!
            bool stat=true;
            if( r->IsValid() )
               {
                  // make sure that the fit is not crazy...
                  double sigma = ff->GetParameter(2);
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
                        //cpad.emplace_back( col, index, time, amp, pos, err );
                        combpad.emplace_back( col, index, time, amp, pos, err );
                                    
                        if( fTrace )
                           std::cout<<"Combination Found! s: "<<col
                                    <<" i: "<<index
                                    <<" t: "<<time
                                    <<" a: "<<amp
                                    <<" z: "<<pos
                                    <<" err: "<<err<<std::endl;
                        //
                     }
                  else // fit is crazy
                     {
                        if( fTrace )
                           std::cout<<"Combination NOT found... position error: "<<err
                                    <<" or sigma: "<<sigma<<std::endl;
                        stat=false;
                     }
               }// fit is valid
            else
               {
                  if( fTrace )
                     std::cout<<"\tFit Not valid"<<std::endl;
                  stat=false;
               }
            delete ff;

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
                        //cpad.emplace_back( col, index, time, amp, pos );
                        combpad.emplace_back( col, index, time, amp, pos );

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
         } // wizard peak finding failed
      delete hh;
      if( fTrace )
         std::cout<<"-------------------------------"<<std::endl;
      //return cpad;
   }


   void Match(std::vector<signal>* awsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
                                                         awsignals->end());
      std::multiset<signal, signal::timeorder> pad_bytime(combpad.begin(), 
                                                          combpad.end());
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

                  hawcol->Fill(iaw->idx,ipd->sec);
                  hawcol_time->Fill( iaw->t , ipd->t );

                  double delta = fabs( iaw->t - ipd->t );
                  if( delta < fCoincTime ) 
                     {
                        tmatch=true;
                        hawcol_deltat_sec->Fill(iaw->idx,ipd->sec);
                     }
                  
                  if( sector == ipd->sec ) 
                     {
                        pmatch=true;
                        hawcol_sector_time->Fill( iaw->t , ipd->t );
                     }
                  // if( pmatch )
                  //    {
                  //       std::cout<<"\t dt = "<<delta<<"  pad col: "<<ipd->sec;
                  //       if( tmatch ) 
                  //          std::cout<<" ok"<<std::endl;
                  //       else
                  //          std::cout<<"\n";
                  //    }
                  if( tmatch && pmatch ) 
                     {
                        hawcol_match->Fill(iaw->idx,ipd->sec);
                        hawcol_match_amp->Fill(iaw->height,ipd->height);
                        hawcol_match_time->Fill(iaw->t,ipd->t);
                        hamprow_timecolcut->Fill(ipd->idx,ipd->height);
                        spacepoints.push_back( std::make_pair(*iaw,*ipd) );
                        //pad_bytime.erase( ipd );
                        ++Nmatch;
                        if( fTrace )
                           std::cout<<"\t"<<Nmatch<<")  pad col: "<<ipd->sec<<" pad row: "<<ipd->idx<<std::endl;
                     }
               }
         }
      if( fTrace )
         std::cout<<"MatchModule::Match Number of Matches: "<<Nmatch<<std::endl;
      if( Nmatch ) hNmatch->Fill( double(Nmatch) );
   }

   void CombinedPADdiagnostic()
   {
      int nhit=0;
      for( auto iSig=combpad.begin(); iSig!=combpad.end(); ++iSig )
         { 
            hOccCol->Fill(iSig->sec);
            hOccRow->Fill(iSig->idx);
            hOccPad->Fill(iSig->idx,iSig->sec);
            hAmpPad->Fill(iSig->height);
            hTimePad->Fill(iSig->t);
            hTimeAmpPad->Fill(iSig->t,iSig->height);

            hTimePadCol->Fill(iSig->sec,iSig->t);
            hAmpPadCol->Fill(iSig->sec,iSig->height);
            hTimePadRow->Fill(iSig->idx,iSig->t);
            hAmpPadRow->Fill(iSig->idx,iSig->height);
            ++nhit;
            //std::cout<<"\t"<<nhit<<" "<<iSig->sec<<" "<<iSig->idx<<" "<<iSig->height<<" "<<iSig->t<<std::endl;
         }
      hNhitPad->Fill(nhit);
   }
   
#if 0
   void PlotMatch(std::vector<signal>* awsignals, std::vector<signal>* padsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
                                                         awsignals->end());
      std::multiset<signal, signal::timeorder> pad_bytime(padsignals->begin(), 
                                                          padsignals->end());
      int Nmatch=0;
      for( auto iaw : aw_bytime )
         {
            for( auto ipd : pad_bytime )
               {
                  hawcol->Fill(iaw.idx,ipd.sec);

                  bool tmatch=false;
                  bool pmatch=false;

                  double delta = fabs( iaw.t - ipd.t );
                  if( delta < fCoincTime )
                     {
                        hawcol_timecut->Fill(iaw.idx,ipd.sec);
                        tmatch=true;
                     }

                  short sector = short(iaw.idx/8-1);
                  if( sector < 0 ) sector=31;
                  if( sector == ipd.sec )
                     {
                        hawcol_colcut->Fill(iaw.idx,ipd.sec);
                        pmatch=true;
                     }

                  if( tmatch && pmatch )
                     {
                        hawcol_timecolcut->Fill(iaw.idx,ipd.sec);
                        hamprow_timecolcut->Fill(ipd.idx,ipd.height);
                        ++Nmatch;
                     }
               }
         }
      if( fTrace )
         std::cout<<"MatchModule::PlotMatch Number of Matches: "<<Nmatch<<std::endl;
   }
#endif
};


class MatchModuleFactory: public TAFactory
{
public:
   MatchFlags fFlags;
   
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("MatchModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         // if (args[i] == "--wfexport")
         //    fFlags.fExportWaveforms = true;
      }
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

