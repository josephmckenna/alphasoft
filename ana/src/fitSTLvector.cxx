#include <TMinuit.h>
#include <TObject.h>
#include <vector>
#include <iostream>
#include "SignalsType.h"

bool IsEmpty(double y) { return (y==0.); }

class fitSTLvector: public TObject
{
protected:
   std::vector<signal> *fSignals;
   int fNpar;
   double* fStep;
   double* fStart;
   double fchi2;
   int fStat;
   int fDoF;
   TMinuit* fFitter;
   double up;
   int max_calls;
   int ierflg;
   int print_level;


public:
   fitSTLvector():TObject(),fSignals(0),fNpar(0),fStep(0),fStart(0),
                  fchi2(-1.),fStat(-1),fDoF(0),fFitter(0),
                  up(1.0),max_calls(500),ierflg(0),print_level(-1)
   {   }

   fitSTLvector(std::vector<signal> *s):TObject(),
                              fSignals(s),fNpar(0),fStep(0),fStart(0),
                              fchi2(-1.),fStat(-1),fDoF(0),fFitter(0),
                              up(1.0),max_calls(500),ierflg(0),print_level(-1)
   {
   }

   void CalculateDoF()
   {
      fDoF = std::count_if(fSignals->begin(), fSignals->end(), [](signal s){ return s.height == 0; }) - fNpar;
   }

   inline const std::vector<signal>* GetData() const { return fSignals; }

   inline double GetChi2() const { return fchi2; }
   inline int GetStat()    const { return fStat; }
   inline int GetDoF()     const { return fDoF; }

   inline void SetStart(double* s) { for(int i=0; i<fNpar; ++i) fStart[i]=s[i]; }
   inline const double* GetStart() const { return fStart; }

   // virtual void Initialize()=0;
   virtual int Fit()=0;
};

static TMinuit* _Fitter;
void GausFit(int&, double*, double& chi2, double* p, int)
{
   const std::vector<signal>* v = ((fitSTLvector*) _Fitter->GetObjectFit())->GetData();
   chi2=0.;
   double t,y,d;
   for(const signal &s: *v)
      {
         if( s.height == 0. ) continue;
         t = (s.z-p[1])/p[2];
         y = p[0]*exp(-0.5*t*t);
         d = (s.height-y)/s.errh;
         chi2 += (d*d);
      }
   return;
}

class fitGaussSTLvector: public fitSTLvector
{
public:
   fitGaussSTLvector(std::vector<signal>* v):fitSTLvector(v)
   {
      fNpar=3;
      fStep = new double[fNpar];
      fStep[0] = fStep[1] = fStep[2] = 1.e-3;
      fStart = new double[fNpar];
      fStart[0] = fStart[1] = fStart[2] = 0.0;
      fFitter = new TMinuit(fNpar);
      Setup();
      _Fitter = fFitter;
   }

   fitGaussSTLvector():fitSTLvector()
   {
      fNpar=3;
      fStep = new double[fNpar];
      fStep[0] = fStep[1] = fStep[2] = 1.e-3;
      fStart = new double[fNpar];
      fStart[0] = fStart[1] = fStart[2] = 0.0;
      fFitter = new TMinuit(fNpar);
      Setup();
      _Fitter = fFitter;
   }

   ~fitGaussSTLvector()
   {
      delete[] fStep;
      delete[] fStart;
      delete fFitter;
   }

   inline double GetAmplitude() const      { return fAmplitude; }
   inline double GetMean() const           { return fMean; }
   inline double GetSigma() const          { return fSigma; }
   inline double GetAmplitudeError() const { return fAmplitudeError; }
   inline double GetMeanError() const      { return fMeanError; }
   inline double GetSigmaError() const     { return fSigmaError;}

private:
   double fAmplitude;
   double fMean;
   double fSigma;
   double fAmplitudeError;
   double fMeanError;
   double fSigmaError;


public:
   void Setup()
   {
      fFitter->SetPrintLevel(print_level);
      fFitter->SetObjectFit(this);
      fFitter->SetFCN(GausFit);
      fFitter->SetErrorDef(up);
      fFitter->SetMaxIterations( max_calls );
   }

   // void Initialize()
   // {
   //    // den = sum_i yi --> sum of weights
   //    double den = std::accumulate( fH->y.begin(), fH->y.end(), 0.0);
   //    // norm = (N-1)/N * sum_i yi
   //    double norm = (double(fH->dimension)-1.)*den/double(fH->dimension);
   //    if( norm > 0. && den > 0. )
   //       fStart[0] = *std::max_element(fH->y.begin(),fH->y.end());
   //    else return; // kill me if my normalization is 0

   //    // num = sum_i xi*yi
   //    double num = std::inner_product( fH->x.begin(), fH->x.end(), fH->y.begin(), 0.0);
   //    fStart[1] = num/den;

   //    std::vector<double> temp(fH->dimension);
   //    // calculate the difference from the mean: xi-m
   //    std::transform(fH->x.begin(), fH->x.end(), temp.begin(),bind2nd(std::minus<double>(), fStart[1]));
   //    // square it: (xi-m)*(xi-m)
   //    std::transform(temp.begin(),temp.end(),temp.begin(),temp.begin(),std::multiplies<double>());
   //    // multiply by the weights: yi*(xi-m)*(xi-m)
   //    std::transform(temp.begin(),temp.end(),fH->y.begin(),temp.begin(),std::multiplies<double>());

   //    // rms = sqrt( (sum_i yi*(xi-m)*(xi-m))/ norm )
   //    fStart[2] = sqrt( std::accumulate(temp.begin(), temp.end(), 0.) / norm); // rms
   // }

   int Fit()
   {
      CalculateDoF();
      if( fDoF <= 0 )
         {
            std::cerr<<"fitSTLvector::Fit() ERROR: degrees of Freedom is "<<fDoF<<std::endl;
            return fStat;
         }

      fFitter->mnparm(0, "Amplitude", fStart[0], fStep[0], 0,0,ierflg);
      fFitter->mnparm(1, "Mean",      fStart[1], fStep[1], 0,0,ierflg);
      fFitter->mnparm(2, "Sigma",     fStart[2], fStep[2], 0,0,ierflg);

      fFitter->Migrad();

      double nused;
      int npar;
      fFitter->mnstat(fchi2,nused,nused,npar,npar,fStat);

      fFitter->GetParameter(0, fAmplitude, fAmplitudeError);
      fFitter->GetParameter(1, fMean,      fMeanError);
      fFitter->GetParameter(2, fSigma,     fSigmaError);

      // status integer indicating how good is the covariance
      //   0= not calculated at all
      //   1= approximation only, not accurate
      //   2= full matrix, but forced positive-definite
      //   3= full accurate covariance matrix
      return fStat;
   }
};

#ifdef TEST_STLFIT
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TApplication.h>

#include <fstream>
#include <chrono>
using namespace std;
using namespace std::chrono;

int main(int argc, char** argv)
{
   double amp = 1.,mean=0.,stddev=1.;
   bool draw = true;
   int Nevents = 10000;
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
         draw = bool(atoi(argv[5]));
      }
   double xmin = mean-7.*stddev, xmax = mean+7.*stddev;
   TF1* fg = new TF1("fg","gaus(0)", xmin, xmax);
   fg->SetParameters(amp,mean,stddev);
   cout<<"Original parameters"<<endl;
   cout<<"Amplitude: "<<amp<<endl;
   cout<<"Mean: "<<mean<<endl;
   cout<<"Sigma: "<<stddev<<endl;
   cout<<"----------------------------"<<endl;

   TH1D* h1 = new TH1D("h1", "histo from a gaussian", 100, xmin, xmax);
   h1->SetStats(kFALSE);
   for(int n=0; n<Nevents;++n)
      {
         double x = fg->GetRandom();
         double y = fg->Eval(x);
         //h1->Fill(x,y);
         //if( h1->GetBinContent(h1->FindBin(x)) > 0. )
         h1->AddBinContent(h1->FindBin(x),y);
         //else
         // h1->SetBinContent(h1->FindBin(x),y);
      }

   TApplication app("Test STL FIT",&argc,argv);
   TCanvas* c1 = 0;
   if( draw )
      {
         c1 = new TCanvas("gaussian_fit_test","gaussian fit test");
         h1->Draw("E1");
      }

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

   // std::vector<double> z;
   // std::vector<double> a;
   // std::vector<double> s;
   // for(int b=1; b<=h1->GetNbinsX(); ++b)
   //    {
   //       z.push_back(h1->GetBinCenter(b));
   //       a.push_back(h1->GetBinContent(b));
   //       s.push_back(h1->GetBinError(b));
   //    }
   // fitGaussSTLvector myfit(&z,&a,&s);

   std::vector<signal> sigs;
   for(int b=1; b<=h1->GetNbinsX(); ++b)
      {
         sigs.emplace_back(0,0,0.,h1->GetBinContent(b),h1->GetBinError(b),h1->GetBinCenter(b));
      }
   fitGaussSTLvector myfit(&sigs);

   high_resolution_clock::time_point t1_mine = high_resolution_clock::now();
   // myfit.Initialize();

   double startvals[3];
   startvals[0] = 0.8*amp;
   startvals[1] = mean+0.5*stddev;
   startvals[2] = 1.2*stddev;
   myfit.SetStart(startvals);

   int stat = myfit.Fit();
   high_resolution_clock::time_point t2_mine = high_resolution_clock::now();
   auto fit_only_mine = duration_cast<microseconds>( t2_mine - t1_mine ).count();
   cout<<"Mine time (fit only) "<<fit_only_mine<<" us"<<endl;
   cout<<"My Fit status: "<<stat<<endl;//<<"\t"<<myfit.GetStat()<<endl;
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

   // const double* start = myfit.GetStart();
   // const std::vector<double> init {h1->GetBinContent(h1->GetMaximumBin()), h1->GetMean(), h1->GetRMS()};
   // cout<<"Initialization Values"<<endl;
   // for(int i=0; i<3; ++i) cout<<"ROOT: "<<init[i]<<"\t Mine: "<<start[i]<<endl;

   if( draw ) app.Run();
   else
      {
         std::ofstream fout("testfitresults.csv",std::fstream::app);
         // //   fout<<"Amplitude,Mean,Std.Dev.,N events,ROOT Amp, ROOT Amp Err,ROOT Mean,ROOT Mean Err,ROOT Sigma,ROOT Sigma Err,ROOT chi2,ROOT NDF,ROOT Fit Time,ROOT Fit+Access Time,Mine Amp, Mine Amp Err,Mine Mean,Mine Mean Err,Mine Sigma,Mine Sigma Err,Mine chi2,Mine NDF,Mine Fit Time,Mine Fit+Access Time"<<std::endl;
         fout<<amp<<","<<mean<<","<<stddev<<","<<Nevents<<","
             <<f1->GetParameter(0)<<","<<f1->GetParError(0)<<","
             <<f1->GetParameter(1)<<","<<f1->GetParError(1)<<","
             <<f1->GetParameter(2)<<","<<f1->GetParError(2)<<","
             <<f1->GetChisquare()<<","<<f1->GetNDF()<<","
             <<fit_only_root<<","<<access_root<<","
             <<myfit.GetAmplitude()<<","<<myfit.GetAmplitudeError()<<","
             <<myfit.GetMean()<<","<<myfit.GetMeanError()<<","
             <<myfit.GetSigma()<<","<<myfit.GetSigmaError()<<","
             <<myfit.GetChi2()<<","<<myfit.GetDoF()<<","
             <<fit_only_mine<<","<<access_mine<<std::endl;
         fout.close();
      }
   return 0;
}

#endif

//    g++ -DTEST_STLFIT -O3 -Wall -Wuninitialized `root-config --cflags` `root-config --glibs` -lMinuit -I../include -I../../analib/include/ -I../../recolib/include/ -o fitSTLvector.exe fitSTLvector.cxx

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
