#include "fitSignals.hh"

#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"

#include <TError.h>

double GaussFcn::operator()(const std::vector<double>& par) const 
{
   // assert( par.size() == 3 );
   // GaussFunction gauss(par[0], par[1], par[2]);
   //GaussFunction gauss(par);
   MultiGaussFunction gauss(par);

   double chi2 = 0.,d;
   for(const signal &s: fSignals)
      {
         d = ( gauss(s.z) - s.height ) / s.errh;
         chi2+=(d*d);
      }
   return chi2;
}

void GaussFcn::TestSignals() const
{
  double mean,rms;
  SignalsStatistics(fSignals.begin(), fSignals.end(), mean, rms);
  signal::heightorder sigcmp_h;
  auto maxit = std::max_element(fSignals.begin(), fSignals.end(), sigcmp_h);
  double maxpos = maxit->z;
  double max = maxit->height;
  std::cout<<"GaussFcn::TestSignals() size: "<<fSignals.size()<<" max: "<<max<<" mean: "<<mean<<" rms: "<<rms<<" pos: "<<maxpos<<std::endl;
  
}

fitSignals::fitSignals(std::vector<signal> s, int n):theFCN(s),fNpar(3*n),
                                                     fStep(3*n,1.e-3),fStart(3*n,0.0),
                                                     print_level(-1),
                                                     fStat(-1),fchi2(-1.),
                                                     fAmplitude(n,-1.),fMean(n,-1.),fSigma(n,-1.),
                                                     fAmplitudeError(n,-1.), fMeanError(n,-1.), fSigmaError(n,-1.)
                                              
{
   //  std::cout<<"fitSignals::fitSignals # of signals: "<<s.size()<<" # of peaks: "<<n<<std::endl;
   CalculateDoF();
   // theFCN.TestSignals();
   // for(auto it = s.begin(); it != s.end(); ++it)
   //    it->print();
   
}



void fitSignals::CalculateDoF()
{
   fDoF = std::count_if(theFCN.GetData()->begin(), theFCN.GetData()->end(), 
                        [](signal s){ return s.height > 0.; }) - fNpar;
}

void fitSignals::Fit()
{
   // see:
   // https://root.cern.ch/root/htmldoc/guides/minuit2/Minuit2.html

   // create minimizer (default constructor)
   ROOT::Minuit2::VariableMetricMinimizer theMinimizer; 
   theMinimizer.Builder().SetPrintLevel(print_level);

   int error_level_save = gErrorIgnoreLevel;
   gErrorIgnoreLevel = kFatal;
   // minimize
   ROOT::Minuit2::FunctionMinimum min = theMinimizer.Minimize(theFCN, fStart, fStep);
   gErrorIgnoreLevel = error_level_save;

   // output
   fStat = min.IsValid();
   if( print_level > 0 )
      std::cout<<"fitSignals::Fit() status: "<<fStat<<std::endl;

   ROOT::Minuit2::MnUserParameterState state = min.UserState();

   for( uint i=0; i<fNpar/3; ++i )
      {
         fAmplitude[i] = state.Value(3*i);   fAmplitudeError[i] = state.Error(3*i);
         fMean[i]      = state.Value(1+3*i); fMeanError[i]      = state.Error(1+3*i);
         fSigma[i]     = state.Value(2+3*i); fSigmaError[i]     = state.Error(2+3*i);
      }

   fchi2 = min.Fval();
   if( print_level > 0 )
      std::cout<<"fitSignals::Fit() chi2 = "<<fchi2<<std::endl;
}

void SignalsStatistics(std::vector<signal>::const_iterator first,
		       std::vector<signal>::const_iterator last, 
		       double& mean, double& rms)
{
  int dimension = last-first;
  // den = sum_i yi --> sum of weights    //signal(ss,ii,tt,hh,eh,zz,(ez))
  signal res = std::accumulate(first, last, signal(0,0,0.,0.,0.,0.), 
			       [](const signal &a, const signal &b)
			       { return signal(0,0,0.,a.height+b.height,0.,a.z+b.z*b.height); });
  double den = res.height;
  double num = res.z;
  double norm = (double(dimension)-1.)*den/double(dimension);
    
  mean = num/den;
    
  std::vector<double> temp(dimension);
  // calculate the difference from the mean: xi-m
  std::transform(first, last, temp.begin(),[mean](const signal &s){ return s.z - mean; });
  // square it: (xi-m)*(xi-m)
  std::transform(temp.begin(),temp.end(),temp.begin(),temp.begin(),std::multiplies<double>());
  // multiply by the weights: yi*(xi-m)*(xi-m)
  std::transform(temp.begin(),temp.end(),first,temp.begin(),[](const double &d, const signal &s){ return d*s.height; });
    
  // rms = sqrt( (sum_i yi*(xi-m)*(xi-m))/ norm )
  rms = sqrt( std::accumulate(temp.begin(), temp.end(), 0.) / norm); // rms
}

#ifdef TEST_STLFIT

#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TApplication.h>

#include <iostream>
#include <fstream>
#include <chrono>

int main(int argc, char** argv)
{
   double amp=1.,mean=0.,stddev=1.;
   bool draw = false;
   int Nevents=1000000, Nbins=100000;
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

   double startvals[3];
   startvals[0] = 0.8*amp;
   startvals[1] = mean+0.5*stddev;
   startvals[2] = 1.2*stddev;
   for(int i=0; i<3; ++i)
      cout<<"startval["<<i<<"] = "<<startvals[i]<<endl;
   cout<<"----------------------------"<<endl;
   double avg,rms;
   SignalsStatistics(sigs.begin(),sigs.end(),avg,rms);
   cout<<"Signal Statistics --> Mean: "<<avg<<"   RMS: "<<rms<<endl;
   cout<<"----------------------------"<<endl;

   fitSignals myfit( sigs );
   high_resolution_clock::time_point t1_mine = high_resolution_clock::now();
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
