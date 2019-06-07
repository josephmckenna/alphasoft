#include "fitSignals.h"

#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
//#include "Minuit2/MnUserParameters.h"
//#include "Minuit2/MnMigrad.h"

#include <cassert>

double GaussFcn::operator()(const std::vector<double>& par) const 
{
  assert( par.size() == 3 );
  GaussFunction gauss(par[0], par[1], par[2]);
  
  double chi2 = 0.,d;
  for(const signal &s: fSignals)
    {
       
       d = ( gauss(s.z) - s.height ) / s.errh;
       chi2+=(d*d);
    }
  return chi2;
}

fitSignals::fitSignals(std::vector<signal> s):theFCN(s),fNpar(3),
                                              fStep(3,1.e-3),fStart(3,0.0),
					      print_level(-1),
                                              fStat(-1),fchi2(-1.)
                                              
{
   CalculateDoF();

   fAmplitude = fMean = fSigma = fAmplitudeError = fMeanError = fSigmaError = -1.;
}

void fitSignals::CalculateDoF()
{
   fDoF = std::count_if(theFCN.GetData()->begin(), theFCN.GetData()->end(), [](signal s){ return s.height != 0.; }) - fNpar;
}

void fitSignals::Fit()
{
   // // create the parameters
   // ROOT::Minuit2::MnUserParameters upar;
   // upar.Add("amplitude", fStart[0], fStep[0]);
   // upar.Add("mean",      fStart[1], fStep[1]);
   // upar.Add("sigma",     fStart[1], fStep[2]);

   // // create MIGRAD minimizer
   // ROOT::Minuit2::MnMigrad migrad(theFCN, upar);

   // // minimize
   // ROOT::Minuit2::FunctionMinimum min = migrad();

   // create minimizer (default constructor)
   ROOT::Minuit2::VariableMetricMinimizer theMinimizer; 
   theMinimizer.Builder().SetPrintLevel(print_level);

   // minimize
   ROOT::Minuit2::FunctionMinimum min = theMinimizer.Minimize(theFCN, fStart, fStep);
   
   // output
   fStat = min.IsValid();
   if( print_level > 0 )
      std::cout<<"fitSignals::Fit() status: "<<fStat<<std::endl;

   ROOT::Minuit2::MnUserParameterState state = min.UserState();

   fAmplitude = state.Value(0); fAmplitudeError = state.Error(0);
   fMean      = state.Value(1); fMeanError      = state.Error(1);
   fSigma     = state.Value(2); fSigmaError     = state.Error(2);
 
   fchi2 = min.Fval();
}


#ifdef TEST_STLFIT

#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TApplication.h>

#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char** argv)
{
   double amp = 1.,mean=0.,stddev=1.;
   bool draw = false;
   int Nevents = 1000000, Nbins=100000;
   cout<<" --> "<<argv[0]<<" <--"<<endl;
   if( argc == 4 )
      {
         amp = atof(argv[1]);
         mean = atof(argv[2]);
         stddev = atof(argv[3]);
      }
   if( argc == 5 )
      {
         amp = atof(argv[1]);
         mean = atof(argv[2]);
         stddev = atof(argv[3]);
         Nevents = atoi(argv[4]);
      }
   if( argc == 6 )
      {
         amp = atof(argv[1]);
         mean = atof(argv[2]);
         stddev = atof(argv[3]);
         Nevents = atoi(argv[4]);
         Nbins = atoi(argv[5]);
      }
  if( argc == 7 )
      {
         amp = atof(argv[1]);
         mean = atof(argv[2]);
         stddev = atof(argv[3]);
         Nevents = atoi(argv[4]);
         Nbins = atoi(argv[5]);
         draw = bool(atoi(argv[6]));
      }
   
   double xmin = mean-7.*stddev, xmax = mean+7.*stddev;
   TF1* fg = new TF1("fg","gaus(0)", xmin, xmax);
   fg->SetParameters(amp,mean,stddev);
   cout<<"----------------------------"<<endl;
   cout<<"Original parameters"<<endl;
   cout<<"Amplitude: "<<amp<<endl;
   cout<<"Mean: "<<mean<<endl;
   cout<<"Sigma: "<<stddev<<endl;
   cout<<"----------------------------"<<endl;
   cout<<"Histogram parameters"<<endl;
   cout<<"Number of Entries: "<<Nevents<<endl;
   cout<<"Number of Bins: "<<Nbins<<endl;
   cout<<"----------------------------"<<endl;

   // --------------------------------------------------------------------------------
   TH1D* h1 = new TH1D("h1", "histo from a gaussian", Nbins, xmin, xmax);
   h1->SetStats(kFALSE);
   for(int n=0; n<Nevents;++n)
      {
         double x = fg->GetRandom();
         double y = fg->Eval(x);
         // if( h1->GetBinContent(h1->FindBin(x)) > 0. )
         h1->AddBinContent(h1->FindBin(x),y);
         // else
         //    h1->SetBinContent(h1->FindBin(x),y);
         //h1->Fill(x,y);
      }

   TApplication app("Test STL FIT",&argc,argv);
   TCanvas* c1 = 0;
   if( draw )
      {
         c1 = new TCanvas("gaussian_fit_test","gaussian fit test");
         c1->cd();
         h1->Draw("E1");
         //h1->Draw("HIST");
      }

   // --------------------------------------------------------------------------------

   high_resolution_clock::time_point t1_root = high_resolution_clock::now();
   h1->Fit("gaus","0Q");

   high_resolution_clock::time_point t2_root = high_resolution_clock::now();
   auto fit_only_root = duration_cast<microseconds>( t2_root - t1_root ).count();
   cout<<"ROOT time (fit only) "<<fit_only_root<<" us"<<endl;
   TF1* f1 = h1->GetFunction("gaus");
   cout<<"ROOT fit parameters"<<endl;
   cout<<"Amplitude: "<<f1->GetParameter(0)<<"\tError: "<<f1->GetParError(0)<<endl;
   cout<<"Mean: "<<f1->GetParameter(1)<<"\tError: "<<f1->GetParError(1)<<endl;
   cout<<"Sigma: "<<f1->GetParameter(2)<<"\tError: "<<f1->GetParError(2)<<endl;
   cout<<"chi^2: "<<f1->GetChisquare()<<"\tDegrees Of Freedom: "<<f1->GetNDF()<<endl;
   high_resolution_clock::time_point t3_root = high_resolution_clock::now();
   auto access_root = duration_cast<microseconds>( t3_root - t1_root ).count();
   cout<<"ROOT time (fit + access) "<<access_root<<" us"<<endl;
   cout<<"----------------------------"<<endl;

   if( draw )
      f1->Draw("same");

   // --------------------------------------------------------------------------------

   std::vector<signal> sigs;
   for(int b=1; b<=h1->GetNbinsX(); ++b)
      {
         if(h1->GetBinContent(b)){
            sigs.emplace_back(0,0,0.,h1->GetBinContent(b),h1->GetBinError(b),h1->GetBinCenter(b));
         }
      }

   // --------------------------------------------------------------------------------
   fitSignals myfit( sigs );
   high_resolution_clock::time_point t1_mine = high_resolution_clock::now();

   double startvals[3];
   startvals[0] = 0.8*amp;
   startvals[1] = mean+0.5*stddev;
   startvals[2] = 1.2*stddev;
   myfit.SetStart(startvals);

   myfit.Fit();
   high_resolution_clock::time_point t2_mine = high_resolution_clock::now();
   auto fit_only_mine = duration_cast<microseconds>( t2_mine - t1_mine ).count();
   cout<<"Mine time (fit only) "<<fit_only_mine<<" us"<<endl;
   cout<<"My Fit status: "<<myfit.GetStat()<<endl;
   cout<<"My fit parameters"<<endl;
   cout<<"Amplitude: "<<myfit.GetAmplitude()<<"\tError: "<<myfit.GetAmplitudeError()<<endl;
   cout<<"Mean: "<<myfit.GetMean()<<"\tError: "<<myfit.GetMeanError()<<endl;
   cout<<"Sigma: "<<myfit.GetSigma()<<"\tError: "<<myfit.GetSigmaError()<<endl;
   cout<<"chi^2: "<<myfit.GetChi2()<<"\tDegrees Of Freedom: "<<myfit.GetDoF()<<endl;
   high_resolution_clock::time_point t3_mine = high_resolution_clock::now();
   auto access_mine = duration_cast<microseconds>( t3_mine - t1_mine ).count();
   cout<<"Mine time (fit + access) "<<access_mine<<" us"<<endl;
   cout<<"----------------------------"<<endl;

   if( draw )
      {
         TF1* myf = new TF1("myf","gaus(0)", xmin, xmax);
         myf->SetLineColor(kBlack);
         myf->SetLineStyle(2);
         myf->SetParameters(myfit.GetAmplitude(),myfit.GetMean(),myfit.GetSigma());
         c1->cd();
         myf->Draw("same");
      }


   // -------------------------------------------------------------------------------
   high_resolution_clock::time_point t1_acc = high_resolution_clock::now();
   std::vector<double> heights(sigs.size());
   std::transform(sigs.begin(), sigs.end(), heights.begin(), [](const signal &s){ return s.height; });
   std::vector<double> zs(sigs.size());
   std::transform(sigs.begin(), sigs.end(), zs.begin(), [](const signal &s){ return s.z; });
   high_resolution_clock::time_point t1_acc_tr = high_resolution_clock::now();
   double den1 = std::accumulate(heights.begin(), heights.end(), 0.0);
   double num1 = std::inner_product( zs.begin(), zs.end(), heights.begin(), 0.0);
   high_resolution_clock::time_point t2_acc = high_resolution_clock::now();
   signal res2 = std::accumulate(sigs.begin(), sigs.end(), signal(0,0,0,0,0,0), [](const signal &a, const signal &b){ return signal(0,0,0,a.height+b.height,0,a.z+b.z*b.height); });
   double den2 = res2.height;
   double num2 = res2.z;
   high_resolution_clock::time_point t3_acc = high_resolution_clock::now();
   std::cout << "Method 1: " << duration_cast<microseconds>( t2_acc - t1_acc ).count() << ", result: " << den1 << '\t' << num1 << std::endl;
   std::cout << "vector extraction part: " << duration_cast<microseconds>( t1_acc_tr - t1_acc ).count() << std::endl;
   std::cout << "Method 2: " << duration_cast<microseconds>( t3_acc - t2_acc ).count() << ", result: " << den2 << '\t' << num2 << std::endl;
   
   if( draw ) app.Run();

   return 0;
}

#endif

//    g++ -DTEST_STLFIT -O3 -Wall -Wuninitialized `root-config --cflags` `root-config --glibs` -lMinuit2 -I${AGRELEASE}/ana/include -I${AGRELEASE}/analib/include/ -I${AGRELEASE}/recolib/include/ -o testfitSignals.exe fitSignals.cxx

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
