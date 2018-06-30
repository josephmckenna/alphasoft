#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "TFitResult.h"

#include "SignalsType.h"
#include <set>
//#include <map>
#include <iostream>
#include <sstream>      // std::ostringstream


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
   double padSigma = 1.5; // width of single avalanche charge distribution
   double padSigmaD = 0.75; // max. rel. deviation of fitted sigma from padSigma
   //   double padSigmaD = 0.25;
   double padFitErrThres = 1.; // max. accepted error on pad gaussian fit mean

   std::vector<signal> combpad;
   
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

      if( fTrace )
         printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
      if( ! SigFlow->awSig.size() ) return flow;

      if( fTrace )
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
      double temp=-1.;
      std::vector< std::vector<signal> > pad_bytime;
      for( auto isig = sig.begin(); isig!=sig.end(); ++isig )
         {
            if( isig->t==temp )
               {   
                  pad_bytime.back().push_back( *isig );
               }
            else
               {
                  temp=isig->t;
                  pad_bytime.emplace_back();
                  pad_bytime.back().push_back( *isig );
               }
         }
      return pad_bytime;
   }

   void CombinePads(std::vector<signal>* padsignals)
   {
      combpad.clear();
      // combine pads in the same column only
      std::vector< std::vector<signal> > pad_bysec;
      std::set<short> secs = PartionBySector( padsignals, pad_bysec ) ;

      for( auto isec=secs.begin(); isec!=secs.end(); ++isec )
         {
            short sector = *isec;
            if( fTrace )
               std::cout<<"MatchModule::CombinePads sec: "<<sector
                        <<" sector: "<<pad_bysec[sector].at(0).sec
                        <<" size: "<<pad_bysec[sector].size()<<std::endl;
            
            // slice the signal by time (avalanche creation time)
            std::vector< std::vector<signal> > pad_bytime = PartitionByTime( pad_bysec[sector] );

            for( auto it=pad_bytime.begin(); it!=pad_bytime.end(); ++it )
               {
                  if( it->size() <= 2 ) continue;
                  TH1D* hh = new TH1D("hhhhh","",576,0.,576.);
                  double time = it->begin()->t;
                  for( auto s: *it )
                     {
                        // s.print();
                        hh->Fill(s.idx,s.height);
                     }
                  
                  // exploit wizard avalanche centroid (peak)
                  TSpectrum spec(maxPadGroups);
                  int error_level_save = gErrorIgnoreLevel;
                  gErrorIgnoreLevel = kFatal;
                  int nfound = spec.Search(hh,1,"nodraw");
                  gErrorIgnoreLevel = error_level_save;
                  
                  if(nfound)
                     {
                        // Sometimes TSpectrum gets overeager and finds
                        // multiple peaks in a narrow distribution.
                        if( hh->GetRMS() < 1. )
                           nfound = 1;
                        
                        // Require at least 4 data points per peak to justify multiple peaks
                        if(nfound > 0.25*hh->GetEntries())
                           {  
                              int nf = int(0.25*hh->GetEntries());
                              if(nf) nfound = nf;
                              else nfound = 1;
                           }

                        // fit a number of gaussians
                        std::ostringstream oss;
                        for(int i = 0; i < nfound; i++)
                           {
                              oss << (i?" + ":"") << "gaus(" << 3*i << ")";
                           }

                        TF1* ff = new TF1("fffff",oss.str().c_str(),0.,576.);
                        // initialize gaussians with peak finding wizard
                        double *peakx = spec.GetPositionX();
                        double *peaky = spec.GetPositionY();
                        for(int i = 0; i < nfound; i++)
                           {
                              ff->SetParameter(3*i,peaky[i]);
                              ff->SetParameter(3*i+1,peakx[i]);
                              ff->SetParameter(3*i+2,padSigma);
                           }

                        TFitResultPtr r = hh->Fit(ff,"BWS0NQ",""); // CHECK ME!!!
                        if( r->IsValid() )
                           {
                              for(int i = 0; i < nfound; ++i)
                                 {
                                    // make sure that the fit is not crazy...
                                    double sigma = ff->GetParameter(3*i+2);
                                    if( ff->GetParError(3*i+1) < padFitErrThres && 
                                        abs(sigma-padSigma)/padSigma < padSigmaD )
                                       {
                                          double amp = ff->GetParameter(3*i);
                                          double pos = ff->GetParameter(3*i+1);                                    
                                          int index = (pos - floor(pos)) < 0.5 ? int(floor(pos)):int(ceil(pos));

                                          // create new signal with combined pads
                                          combpad.emplace_back( sector, index, time, amp );

                                          if( fTrace )
                                             std::cout<<"Combination Found! s: "<<sector
                                                      <<" i: "<<index
                                                      <<" t: "<<time
                                                      <<" a: "<<amp<<std::endl;
                                          //
                                       }
                                    // else // fit is crazy
                                    //    std::cout<<"Combination NOT found... position error: "<<ff->GetParError(3*i+1)
                                    //            <<" or sigma: "<<sigma<<std::endl;
                                 } // calcute centroid for each peak found
                           }// fit is valid
                        // else
                        //    std::cout<<"Fit Not valid"<<std::endl;
                        delete ff;
                     } // wizard peak finding failed
                  // else
                  //    std::cout<<"Peaks not found"<<std::endl;

                  delete hh;
                  if( fTrace )
                     std::cout<<"-------------------------------"<<std::endl;
               } // loop over time slices
            if( fTrace )
               std::cout<<"==============================="<<std::endl;
         }// loop over columns (or sectors)

      hNcpads->Fill( double(combpad.size()) );

   }

   void Match(std::vector<signal>* awsignals)
   {
      std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
                                                         awsignals->end());
      std::multiset<signal, signal::timeorder> pad_bytime(combpad.begin(), 
                                                          combpad.end());
      int Nmatch=0;
      for( auto& iaw : aw_bytime )
         {
            if( fTrace )
               std::cout<<"MatchModule::Match aw: "<<iaw.idx<<" t: "<<iaw.t<<std::endl;
            for( auto& ipd : pad_bytime )
               {
                  bool tmatch=false;
                  bool pmatch=false;

                  hawcol->Fill(iaw.idx,ipd.sec);
                  hawcol_time->Fill( iaw.t , ipd.t );

                  double delta = fabs( iaw.t - ipd.t );
                  if( delta <= fCoincTime ) 
                     {
                        tmatch=true;
                        hawcol_deltat_sec->Fill(iaw.idx,ipd.sec);
                     }
                  // short sector = short(iaw.idx/8-1);
                  // if( sector < 0 ) sector=31;
                  short sector = short(iaw.idx/8);
                  if( sector == ipd.sec ) 
                     {
                        pmatch=true;
                        hawcol_sector_time->Fill( iaw.t , ipd.t );
                     }
                  // if( pmatch )
                  //    std::cout<<"\t dt = "<<delta<<"  pad col: "<<ipd.sec<<std::endl;
                  if( tmatch && pmatch ) 
                     {
                        hawcol_match->Fill(iaw.idx,ipd.sec);
                        hawcol_match_amp->Fill(iaw.height,ipd.height);
                        hawcol_match_time->Fill(iaw.t,ipd.t);
                        hamprow_timecolcut->Fill(ipd.idx,ipd.height);
                        //pad_bytime.erase( ipd );
                        ++Nmatch;
                     }
               }
         }
      //      if( fTrace )
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
            // std::cout<<"\t"<<nhit<<" "<<iSig->sec<<" "<<iSig->i<<" "<<iSig->height<<" "<<iSig->t<<std::endl;
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
                  if( delta <= fCoincTime )
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

