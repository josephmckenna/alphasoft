//
// calibration module v0.2
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
#include "TGraphErrors.h"

#include "manalyzer.h"
#include "AgFlow.h"

//#include "StraightTrack.hh"

//extern double gMinTime;   // trigger delay

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))


class CalibRun: public TARunObject
{
public:
   bool fTrace = false;
   int fSeparation;
   int fCosmicsFull;
   // Trigger delay
   //   double fTdelay = gMinTime;
   double fTdelay;
   TH2D* hRofT_straight;
   TF1* fit_func;
   TGraphErrors* str_raw;
   TGraph* str_err;
   TF1* str_fit;
   //TH2D* hRes;
   TGraph* gRes;

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
                                900, 109., 190. );
      fit_func = new TF1("fRofT","gaus(0)", 109., 190.);

      str_raw = new TGraphErrors();
      str_raw->SetName("STRraw");
      str_raw->SetTitle("STR;Drift Time [ns];TPC Radius [mm]");
      str_fit = new TF1("fSTR","pol3(0)", 0., 5000.);

      str_err = new TGraph();
      str_err->SetName("STRerr");
      str_err->SetTitle("STR radial error;Drift Time [ns];Radial Error [mm]");

      //hRes = new TH2D("hRes","STR residuals;t [ns];res [mm]",500,500,5000.,100,-50.,50.);
      gRes = new TGraph;
      gRes->SetName("STR residuals");
      gRes->SetTitle("STR residuals;t [ns];res [mm]");
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("CalibRun::EndRun, Full Cosmics Found: %d\n",fCosmicsFull);
      if( fCosmicsFull )
         {
            std::vector<double> time,rad,drad;
            CalculateSTR(time,rad,drad);

            if( time.size() > 0 && rad.size() > 0 && drad.size() > 0 )
               {
                  MakeLookUpTable( runinfo->fRunNo, time, rad, drad );
                  MakeLookUpTable( runinfo->fRunNo );
                  Residuals( time, rad );
            
                  runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
                  gDirectory->cd("Calibration");
                  str_raw->Write();
                  str_fit->Write();
                  str_err->Write();
                  gRes->Write();
               }
         }
      printf("CalibRun::EndRun, run %d\n", runinfo->fRunNo);      
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("CalibRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("CalibRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      printf("CalibRun::Analyze, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();
     
      if( !ef || !ef->fEvent || !ef->fEvent->a16)
         return flow;
  
      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
         return flow;

      printf("CalibRun::Analyze, N signals %d\n", int(SigFlow->awSig.size()));

      if( SigFlow->awSig.size() )
         AnalyzeSignals(&SigFlow->awSig);
     
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("CalibRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }


   void AnalyzeSignals(std::vector<signal>* awsignals)
   {      
      double aw_rad = 182.;
      std::vector<double> intersect;

      std::multiset<signal, signal::heightorder> byheight1, byheight2;
      std::multiset<signal, signal::timeorder> bytime(awsignals->begin(), 
                                                      awsignals->end());
      auto it = bytime.begin();

      double t1 = -1;
      double t2 = -1;
      double a0 = -1;
      double a1 = -1;

      double t_tol = 20.;
      while(it != bytime.end())
         {
            if(!byheight1.size()){
               if(abs(it->t - fTdelay) < t_tol)
                  {
                     byheight1.insert(*it);
                     t1 = it->t;
                     a0 = it->i;
                  }
            } 
            else 
               {
                  if(abs(it->i - a0) < fSeparation || abs(it->i - a0) > 255-fSeparation)
                     {
                        if(it->t == t1) 
                           byheight1.insert(*it);
                     } 
                  else 
                     {
                        if(t2 < 0. && (abs(it->t - fTdelay) < t_tol)) 
                           {
                              t2 = it->t;
                              a1 = it->i;
                              byheight2.insert(*it);
                           } 
                        else if(it->t == t2)
                           {
                              if(abs(it->i - a1) < fSeparation || abs(it->i - a1) > 255-fSeparation)
                                 byheight2.insert(*it);
                           } 
                        else break;
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
            //StraightTrack strack(intersect[0], intersect[1]);
            //		std::cout << "Full cosmic!" << std::endl;
            double phiT, d, phiRot=0.;
            StraightTrack(intersect[0], intersect[1], phiRot, phiT, d);
            ++fCosmicsFull;
            for(auto& s: *awsignals)
               {
                  //double r  = strack.GetR(s.i);
                  double phi=double(s.i)/256.*TMath::TwoPi();
                  phi+=phiRot;
                  double r = d/cos(phi-phiT);

                  // move pointless wire hit peak into overflow bin
                  if( hRofT_straight->GetYaxis()->FindBin(r) == 
                      hRofT_straight->GetYaxis()->FindBin(aw_rad) ) r = 1.e6;
                  
                  
                  hRofT_straight->Fill(s.t-fTdelay, r);
               }
         }
   }

   void StraightTrack(const double a0, const double a1, 
                      const double phi_rot,
                      double& phiT, double& d)
   {
    double R=182.;
    double a00 = double(a0);
    double a10 = double(a1);
    double a01 = double(a00 + 1);
    double a11 = double(a10 + 1);

    double ratio0 = a0 - a00;
    double ratio1 = a1 - a10;

    // Intersect aw positions
    double phiA=a00/256.*TMath::TwoPi(), phiB=a01/256.*TMath::TwoPi();
    phiA+=phi_rot; phiB+=phi_rot;
    double phi0 = ratio0*phiB + (1.0-ratio0)*phiA;

    phiA=a10/256.*TMath::TwoPi(), phiB=a11/256.*TMath::TwoPi();
    phiA+=phi_rot; phiB+=phi_rot;
    double phi1 = ratio1*phiB + (1-ratio1)*phiA;
    
    // std::cout << "anode phi positions: " << phi0*TMath::RadToDeg() << ", " 
    // << phi1*TMath::RadToDeg() << std::endl;

    // sagitta
    d = R*cos(0.5*(phi1-phi0));
    phiT = (phi0+phi1)*0.5;
    if( d < 0. )
       {
        phiT -= TMath::Pi();
        d *= -1.;
       }
    // std::cout << "d = " << d 
    // << ", phiT = " << phiT*TMath::RadToDeg() << std::endl;
   }

   void CalculateSTR(std::vector<double>& outtime,
                     std::vector<double>& outrad,
                     std::vector<double>& outdrad                     
                     )
   {
      TH2D* hh = (TH2D*) hRofT_straight->Clone();
      hh->RebinY(15); // <-- HARD-CODED: arbitrary

      ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(500);

      double aw_rad = 182.,
         cath_rad = 109.;    

      outdrad.clear();
      outrad.clear();
      outtime.clear();      

      outdrad.push_back(4.); // <-- HARD-CODED: arbitrary
      outrad.push_back( aw_rad );
      outtime.push_back(0.);
      int n=0; // number of points

      for(int b = 1; b <= hh->GetNbinsX(); ++b)
         {
            if( hh->GetXaxis()->GetBinCenter(b) < 0. )
               continue;

            // get me a slice of STR
            TString hname = TString::Format("py%04d",b);
            TH1D *h = hh->ProjectionY(hname.Data(), b, b);
            h->SetBinContent( h->FindBin( aw_rad ), 0. );

            // ignore slices with too few events
            if( h->Integral() < 100. ) // <-- HARD-CODED: arbitrary
               continue;

            // initialize gaus fit with peak finding results
            TSpectrum s(1, 0.001); // <-- HARD-CODED: arbitrary and irrelevant
            int error_level_save = gErrorIgnoreLevel;
            gErrorIgnoreLevel = kFatal;
            if( s.Search(h, 2., "nodraw") )
               {
                  //	    s.Print();
                  double *r = s.GetPositionX();
                  double *A = s.GetPositionY();
                  int rbin = h->FindBin( r[0] );

                  // estimate sigma by estimating FWHM
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
                  if( fTrace )
                     std::cout<<"CalibRun::CalculateSTR()  bin: "<<b
                              <<"  r: "<<r[0]
                              <<"mm    s: "<<sigma<<" mm"<<std::endl;
                  h->Fit(fit_func,"QME0","",r[0]-5.*sigma,r[0]+5.*sigma);

                  double time = hh->GetXaxis()->GetBinCenter(b),
                     radius = fit_func->GetParameter(1),
                     error = fit_func->GetParameter(2);
                  if( fTrace )
                     std::cout<<"CalibRun::CalculateSTR() fit slice result  t: "<<time
                              <<"ns   r: "<<radius
                              <<"mm   s: "<<error<<" mm"<<std::endl;

                  if( time < 0. || 
                      radius < cath_rad || radius > aw_rad || 
                      error < 0.1 || error > 15. ) // <-- HARD-CODED: arbitrary
                     continue;
	    
                  outdrad.push_back(error);
                  outrad.push_back(radius);
                  outtime.push_back(time);
                  str_raw->SetPoint(n,time,radius);
                  str_raw->SetPointError(n,16.,error);

                  str_err->SetPoint(n,time,error);
                 
                  ++n;
               }// peak found
            gErrorIgnoreLevel = error_level_save;
         }// bins loop

      if( n ) 
         {
            str_fit->FixParameter(0, 182.);
            str_raw->Fit(str_fit,"QME0");
            std::cout<<"CalibRun::CalculateSTR(...) STR function chi^2: "
                     <<str_fit->GetChisquare()/double(str_fit->GetNDF())<<std::endl;
         }
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
      flookup<<"# B = 0 T, TPC data (run "<<run<<"), "<<currentDateTime()<<std::endl;
      flookup<<"# t rmin r rmax phimin phi phimax"<<std::endl;
      double phi=0., phimix = 0.012;
      for( size_t it=0; it<time.size(); ++it)
         {
            flookup << time.at(it) << " "
                    << radius.at(it) - radius_error.at(it) << " "
                    << radius.at(it) << " "
                    << radius.at(it) + radius_error.at(it) << " "
                    << -phimix << " "
                    << phi << " "
                    << phimix << std::endl;
         }
      flookup.close();
   }

   void MakeLookUpTable( int run )
   {
      TString flookupname = TString::Format("LookUp_0.00T_STRR%d_fit.dat",run);
      std::ofstream flookup(flookupname.Data());
      flookup<<"# B = 0 T, TPC data (run "<<run<<"), "<<currentDateTime()<<std::endl;
      flookup<<"# t\tr\tphi"<<std::endl;
      double phi=0.;
      for(double t=0.; t<5000.; t+=8.)
         flookup<<t<<"\t"<<str_fit->Eval(t)<<"\t"<<phi<<std::endl;
      flookup.close();
   }

   void Residuals(std::vector<double> &time, std::vector<double> &radius)
   {
      int n=0;
      for(auto it=time.begin(); it!=time.end(); ++it)
         // hRes->Fill( *it, 
         //             str_fit->Eval(*it) - radius.at( std::distance(time.begin(),it) ) );
         gRes->SetPoint( n++, *it, 
                         str_fit->Eval(*it) - radius.at( std::distance(time.begin(),it) ) );
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
