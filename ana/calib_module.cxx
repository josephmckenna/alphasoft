//
// calibration module v0.1
//
// STR for ALPHA-g TPC
//
// Author: A. Capra
// Based on RofT by L. Martin
// 

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <ctime>

#include "TH2D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "Math/MinimizerOptions.h"
#include "TFitResult.h"

#include "manalyzer.h"
#include "AgFlow.h"

#include "Signals.hh"
#include "StraightTrack.hh"

extern double gMinTime;   // trigger delay

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;


class CalibRun: public TARunObject
{
public:
   int fSeparation;
   int fCosmicsFull;
   // Trigger delay
   //   double fTdelay = gMinTime;
   double fTdelay;
   TH2D* hRofT_straight;
   TF1* fit_func;

public:

   CalibRun(TARunInfo* runinfo)
      : TARunObject(runinfo),fSeparation(32),fCosmicsFull(0),
        //fTdelay(gMinTime)//,
        fTdelay(0.)
   {
      printf("CalibRun::ctor!\n");
   }

   ~CalibRun()
   {
      printf("CalibRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("CalibRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      TDirectory* dir = gDirectory->mkdir("Calibration");
      dir->cd();

      hRofT_straight = new TH2D("hRofT_straight","straight track r vs t;t in ns;r in mm", 
                                550, -500., 5000., 
                                900, 
                                TPCBase::TPCBaseInstance()->GetCathodeRadius(true), 
                                TPCBase::TPCBaseInstance()->GetROradius(true) );
      fit_func = new TF1("fSTR","gaus(0)", 109., 190.);
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("CalibRun::EndRun, run %d\n", runinfo->fRunNo);

      printf("CalibRun::EndRun, Full Cosmics%d\n",fCosmicsFull);

      TH2D* hh = (TH2D*) hRofT_straight->Clone();
      hh->RebinY(15); // <-- HARD-CODED: arbitrary

      // TString ofname = TString::Format("RofTrun%d.dat", runinfo->fRunNo);
      // std::ofstream ofs(ofname.Data());
      // ofs << "t\tr0\tdr0" << endl;

      ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(10000);
    
      vector<double> outdrad, outrad, outtime;
      outdrad.push_back(4.); // <-- HARD-CODED: arbitrary
      outrad.push_back( TPCBase::TPCBaseInstance()->GetAnodeWiresRadius(true) );
      outtime.push_back(0.);

      for(int b = 1; b <= hh->GetNbinsX(); ++b)
         {
            if( hh->GetXaxis()->GetBinCenter(b) < 0. )
               continue;

            TString hname = TString::Format("py%04d",b);
            TH1D *h = hh->ProjectionY(hname.Data(), b, b);
            h->SetBinContent( h->FindBin( TPCBase::TPCBaseInstance()->GetAnodeWiresRadius(true) ), 
                              0. );

            if( h->Integral() < 100. ) // <-- HARD-CODED: arbitrary
               continue;

            TSpectrum s(1, 0.001); // <-- HARD-CODED: arbitrary and irrelevant
            if(s.Search(h))
               {
                  //	    s.Print();
                  double *r = s.GetPositionX();
                  double *A = s.GetPositionY();
                  int rbin = h->FindBin( r[0] );

                  double fwhm = h->GetBinWidth( rbin );
                  for(int ib=rbin; ib<h->GetNbinsX(); ++ib)
                     {
                        double bc = h->GetBinContent( ib );
                        if( bc < 0.5*A[0] )
                           {
                              fwhm = 2.* TMath::Abs( r[0] - h->GetBinCenter( ib-1 ) );
                              break;
                           }
                     }
                  double sigma = fwhm/2.355;
	  
                  fit_func->SetParameter(0, A[0]);
                  fit_func->SetParameter(1, r[0]);
                  fit_func->SetParameter(2, sigma);
                  //h->Fit(fit_func,"Q");
                  //h->Fit("fSTR","QEM");
                  //	    h->Fit(fit_func,"QEM");
                  h->Fit(fit_func,"QME0","",r[0]-5.*sigma,r[0]+5.*sigma);

                  double time = hh->GetXaxis()->GetBinCenter(b),
                     radius = fit_func->GetParameter(1),
                     error = fit_func->GetParameter(2);

                  if( radius < TPCBase::TPCBaseInstance()->GetCathodeRadius(true) ) break;

                  //	    if( time < 0. || radius < 0. || radius > 190. || error > 7. )  
                  if( time < 0. || radius < 0. || 
                      radius > TPCBase::TPCBaseInstance()->GetAnodeWiresRadius(true) || 
                      error > 30. ) // <-- HARD-CODED: arbitrary
                     continue;

                  //	    ofs << time << '\t' << radius << '\t' << error << endl;
	    
                  outdrad.push_back(error);
                  outrad.push_back(radius);
                  outtime.push_back(time);

               }// peak found
         }// bins loop
      //    ofs.close();

      MakeLookUpTable( runinfo->fRunNo, outtime, outrad, outdrad );

      time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("CalibRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("CalibRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   //   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //     printf("CalibRun::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      printf("CalibRun::Analyze, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();
     
      if( !ef || !ef->fEvent )
         return flow;
      AgEvent* age = ef->fEvent;
    
      if( !age->feam || !age->a16 )
         return flow;
      // if( !age->feam->complete || !age->a16->complete || age->feam->error || age->a16->error )
      //    return flow;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
         return flow;
      
      if( SigFlow->awSig.size() )
         {
            std::vector<Signals::signal> awsignals(SigFlow->awSig);
            vector<double> intersect;
            //            vector<double> intersect = sig->FindAnodeIntersect(fTdelay, fSeparation);

            std::multiset<Signals::signal, Signals::signal::heightorder> byheight1, byheight2;
            std::multiset<Signals::signal, Signals::signal::timeorder> bytime(awsignals.begin(), 
                                                                              awsignals.end());
            auto it = bytime.begin();

            double t1 = -1;
            double t2 = -1;
            double a0 = -1;
            double a1 = -1;

            double t_tol = 20;
            while(it != bytime.end()){
               if(!byheight1.size()){
                  if(fTdelay == kUnknown || abs(it->t - fTdelay) < t_tol){
                     byheight1.insert(*it);
                     t1 = it->t;
                     a0 = it->i;
                  }
               } else {
                  if(abs(it->i - a0) < fSeparation || abs(it->i - a0) > 255-fSeparation){
                     if(it->t == t1) byheight1.insert(*it);
                  } else {
                     if(t2 < 0 && (fTdelay == kUnknown || abs(it->t - fTdelay) < t_tol)) {
                        t2 = it->t;
                        a1 = it->i;
                        byheight2.insert(*it);
                     } else if(it->t == t2){
                        if(abs(it->i - a1) < fSeparation || abs(it->i - a1) > 255-fSeparation)
                           byheight2.insert(*it);
                     } else break;
                  }
               }
               it++;
            }

            double totheight = 0;
            if(byheight1.size()){
               a0 = 0;
               for(auto s: byheight1){
                  a0 += s.height*s.i;
                  totheight += s.height;
               }
               a0 /= totheight;
               intersect.push_back(a0);
            }
            if(byheight2.size()){
               a1 = 0;
               totheight = 0;
               for(auto s: byheight2){
                  a1 += s.height*s.i;
                  totheight += s.height;
               }
               a1 /= totheight;
               intersect.push_back(a1);
            }

            if(intersect.size() == 2)
               {
                  StraightTrack strack(intersect[0], intersect[1]);
                  //		cout << "Full cosmic!" << endl;
                  ++fCosmicsFull;
                  for(auto s: awsignals)
                     {
                        double r  = strack.GetR(s.i);
	     
                        // move pointless wire hit peak into overflow bin
                        if( hRofT_straight->GetYaxis()->FindBin(r) == 
                            hRofT_straight->GetYaxis()->FindBin( TPCBase::TPCBaseInstance()->GetAnodeWiresRadius(true)) ) r = 1e6;
                        hRofT_straight->Fill(s.t-fTdelay, r);
                     }
               }
         }
     
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("CalibRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

   // Get current date/time
   const std::string currentDateTime() 
   {
      time_t     now = time(0);
      struct tm  tstruct;
      char       buf[80];
      tstruct = *localtime(&now);
      // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
      // for more information about date/time format
      //    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
      strftime(buf, sizeof(buf), "%d %B %Y, %X", &tstruct);
      return buf;
   }
  
   void MakeLookUpTable( int run,
                         std::vector<double> &time, std::vector<double> &radius, 
                         std::vector<double> &radius_error )
   {
      TString flookupname = TString::Format("LookUp_0.00T_STRR%d.dat",run);
      std::ofstream flookup(flookupname.Data());
      flookup<<"# B = 0T, real prototype data (run "<<run<<"), "<<currentDateTime()<<endl;
      flookup<<"t rmin r rmax phimin phi phimax"<<endl;
      double phi=0., phimix = 0.012;
      for( size_t it=0; it<time.size(); ++it)
         {
            flookup << time.at(it) << " "
                    << radius.at(it) - radius_error.at(it) << " "
                    << radius.at(it) << " "
                    << radius.at(it) + radius_error.at(it) << " "
                    << -phimix << " "
                    << phi << " "
                    << phimix << endl;
         }
      flookup.close();
   }
};


class CalibModuleFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("CalibModuleFactory::Init!\n");
    
      for (unsigned i=0; i<args.size(); i++) { }
   }

   void Finish()
   {
      printf("CalibModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {  
      printf("CalibModuleFactory::NewRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new CalibRun(runinfo);
   }
};

static TARegister tar(new CalibModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
