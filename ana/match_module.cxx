#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.h"
#include <set>
#include <iostream>

#include "AnalysisTimer.h"
// #include <future>
// #include <atomic>         // std::atomic
// #include <thread>         // std::thread

class MatchFlags
{
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
   //bool fTrace = true;
   int fCounter = 0;

private:
   double fCoincTime; // ns

   int maxPadGroups = 10; // max. number of separate groups of pads coincident with single wire signal
   // double padSigma = 1.5; // width of single avalanche charge distribution
   // double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   // //   double padSigmaD = 0.25;
   // double padFitErrThres = 1.; // max. accepted error on pad gaussian fit mean
   double padSigma = 7.; // width of single avalanche charge distribution
   double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   double padFitErrThres = 10.; // max. accepted error on pad gaussian fit mean

   std::vector<signal> fCombinedPads;
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

      CombinePads(&SigFlow->pdSig);
#ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module(CombinePads)");
#endif
      //if( fTrace )
      printf("MatchModule::Analyze, combined pads # %d\n", int(fCombinedPads.size()));

      if( fCombinedPads.size() > 0 )
         {
            SigFlow->AddPadSignals(fCombinedPads);
            Match( &SigFlow->awSig );
         }

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
      ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
      std::vector< std::vector<signal> > comb = CombPads( padsignals );
      //std::vector< std::future<std::vector<signal>> > ccpf;
      // std::vector<std::thread> threads;
      fCombinedPads.clear();
      for( auto sigv=comb.begin(); sigv!=comb.end(); ++sigv )
         {
            // std::vector<signal> time_slice = *sigv;
            // ccpf.push_back( std::async( std::launch::async,
            //                             &MatchModule::CentreOfGravity, 
            //                             this, std::ref(time_slice) ) );
            // ccpf.push_back( std::async( std::launch::deferred,
            //                             &MatchModule::CentreOfGravity, 
            //                             this, std::ref(time_slice) ) );

            // threads.push_back( std::thread(&MatchModule::CentreOfGravity, 
            //                                this, std::ref(time_slice) ) );
            // threads.push_back( std::thread(&MatchModule::CentreOfGravity, 
            //                                this, std::ref(*sigv) ) );

            //            CentreOfGravity(time_slice);
            //            time_slice.clear();
            CentreOfGravity(*sigv);
         }

      // fCombinedPads.clear();
      // for(unsigned n=0;n<ccpf.size();n++) 
      //    {
      //       std::vector<signal> csv = ccpf[n].get();
      //       for( auto& cs: csv )
      //          fCombinedPads.push_back( cs );
      //    }
      //for (auto& th : threads) th.join();
      for (uint i=0; i<comb.size(); i++)
         comb.at(i).clear();
      comb.clear();
   }

   

   //std::vector<signal> CentreOfGravity( std::vector<signal> vsig )
   void CentreOfGravity( std::vector<signal> &vsig )
   {
      if( !vsig.size() ) return;
      //std::vector<signal> cpad;
      double time = vsig.begin()->t;
      if( time < 0. ) return;
      short col = vsig.begin()->sec;
      if( col < 0. ) return;
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
      if( nfound > 1 && hh->GetRMS() < 10. )
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
            //TFitResultPtr r = hh->Fit(ff,"BS0NQ",""); // CHECK ME!!!
            int r = hh->Fit(ff,"B0NQ",""); // CHECK ME!!!
            bool stat=true;
            //if( r->IsValid() )
            if( r==0 ) // it's good
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
                        fCombinedPads.emplace_back( col, index, time, amp, pos, err );
                                    
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
                     std::cout<<"\tFit Not valid, status: "<<r<<std::endl;
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
                        fCombinedPads.emplace_back( col, index, time, amp, pos );

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
      //delete peakx; delete peaky;
      if( fTrace )
         std::cout<<"-------------------------------"<<std::endl;
      //return cpad;
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
      if( fTrace )
         std::cout<<"MatchModule::Match Number of Matches: "<<Nmatch<<std::endl;
   }

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

