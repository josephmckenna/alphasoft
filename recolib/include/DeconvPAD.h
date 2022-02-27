#ifndef __DECONV_PAD__
#define __DECONV_PAD__ 1

#include <cassert>
#include <iostream>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "SignalsType.hh"
#include "AnaSettings.hh"

#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TFile.h>

#include "Alpha16.h"
#include "Feam.h"

class DeconvPAD
{
private:
   bool fTrace;
   bool fDiagnostic;
   bool fAged;

   AnaSettings* ana_settings;

   // input
   std::vector<double> fPadResponse;

   std::vector<double> fPwbRescale;

   // pad mask
   std::vector<int> fPadSecMask;
   std::vector<int> fPadRowMask;

   int fbinsize;
   int fPADbinsize;

   double fPWBmax;
   double fPWBrange;

   double fPWBdelay;

   int pedestal_length;
   double fScale;

   unsigned thePadBin;

   double fPWBThres;

   double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<ALPHAg::electrode> fPadIndex;

   std::vector<ALPHAg::TPadSignal> spad;


   int ReadResponseFile( const int padbin);
   int ReadPADResponseFile( const int padbin );
   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
   int ReadRescaleFile();
   int ReadPWBRescaleFile();

   std::vector<ALPHAg::TPadSignal> Deconvolution( std::vector<ALPHAg::wfholder>* subtracted,
                                       std::vector<ALPHAg::electrode> &fElectrodeIndex,
                                       std::vector<double> &fResponse, unsigned theBin);

   void SubtractPAD(ALPHAg::wfholder* hist1,
                   const int b,
                   const double ne,std::vector<ALPHAg::electrode> &fElectrodeIndex,
                   std::vector<double> &fResponse, const int theBin);
   
   ALPHAg::comp_hist_t wf_comparator;
   std::vector<ALPHAg::wfholder*>* wforder(std::vector<ALPHAg::wfholder>* subtracted, const unsigned b);
   
   std::map<int,ALPHAg::wfholder*>* wfordermap(std::vector<ALPHAg::wfholder*>* histset,
                                       std::vector<ALPHAg::electrode> &fElectrodeIndex);

   //Take advantage that there are 256 anode wires
   inline bool IsAnodeNeighbour(int w1, int w2, int dist)
   {
      uint8_t c=w1-w2;
      return (Min(c,256-c)==dist);
   }

   inline uint8_t IsAnodeClose(int w1, int w2)
   {
      uint8_t c=w1-w2;
      return Min(c,256-c);
   }
   
   inline uint8_t Min(uint8_t x, uint8_t y)
   {
      return (x < y)? x : y;
   }

   inline bool MaskPads(short& sec, int& row)
   {
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
         for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
            if( *it == sec && *jt == row ) return true;
      return false;
   }

   inline double CalculatePedestal(const std::vector<int>& adc_samples)
   {
      double ped(0.);
      for(int b = 0; b < pedestal_length; b++) ped += double(adc_samples[b]);
      if( pedestal_length > 0 )
         ped /= double(pedestal_length);
      // int temp=0;
      // std::accumulate(adc_samples.begin(),adc_samples.begin()+pedestal_length,temp);
      // double ped = double(temp)/double(pedestal_length);
      return ped;
   }
   
   inline double GetPeakHeight(std::vector<int>& adc_samples, int& i, double& ped)
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double y=double(*minit);
      return GetPWBpeak(i,y,ped);
   }

   inline double GetPeakTime(std::vector<int>& adc_samples)
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double peak_time = ( (double) std::distance(adc_samples.begin(),minit) + 0.5 );
      peak_time*=double(fPADbinsize);
      peak_time+=fPWBdelay;
      return peak_time;
   }

  inline double GetPWBpeak(int& i, double& y, double& ped)
   {
      double amp = fScale * y, max=-1.;
      if( amp < fPWBrange )
         max =  fPwbRescale.at(i) * fScale * ( y - ped );
      else
         max = fPWBmax;
      return max;
   }

   // Calculate deconvolution error from residual (may change)
   inline double GetNeErr(double /*ne (number of electrons)*/, double res)
   {
      return sqrt(res);
   }

   ALPHAg::padmap* pmap;

   static TH2D* hPWBped;
   static TProfile* hPWBped_prox;

   // pads
   static TH1D* hAvgRMSPad;
   // anodes
   //   static TH1D* hAvgRMSBot;
   static TH1D* hAvgRMSTop;

   // to use in aged display
   std::vector<ALPHAg::wf_ref> feamwaveforms;
   // waveform max
   std::vector<ALPHAg::TPadSignal> fPwbPeaks;
   // std::vector<signal>* fPwbRange;

public:
   DeconvPAD(double adc, double pwb, double aw, double pad);
   DeconvPAD(std::string);
   DeconvPAD(AnaSettings*);
   ~DeconvPAD();

   void Setup();
   void SetupPWBs(TFile* fout, int run, bool norm=false, bool diag=false);

#ifdef BUILD_AG_SIM
   int FindPadTimes(TClonesArray*);
#endif

   int FindPadTimes(const FeamEvent*);

   inline std::vector<ALPHAg::TPadSignal> GetPadSignal()  { return spad; }
//   inline const std::vector<ALPHAg::TPadSignal> GetPadSignal() const { return spad; }

   inline std::vector<ALPHAg::TPadSignal> GetPWBPeaks() { return fPwbPeaks; }
//   inline const std::vector<ALPHAg::TPadSignal> GetPWBPeaks() const { return fPwbPeaks; }

   inline std::vector<ALPHAg::wf_ref> GetPADwaveforms() { return feamwaveforms; }
//   inline const std::vector<ALPHAg::wf_ref> GetPADwaveforms() const  { return feamwaveforms; }

   inline void SetTrace(bool t)      { fTrace=t; }
   inline void SetDiagnostic(bool d) { fDiagnostic=d; }
   inline void SetDisplay(bool a)    { fAged=a; }

   // inline std::vector<double>* GetAnodeDeconvRemainder() { return &resRMS_a; }
   // inline std::vector<double>* GetPadDeconvRemainder() { return & resRMS_p; }

   inline double GetPWBdelay() const { return fPWBdelay; }
   inline void SetPWBdelay(double d) { fPWBdelay = d; }

   inline double GetPWBthres() const { return fPWBThres; }
   inline double GetPADthres() const { return fPWBpeak; }

   inline bool IsItAlpha16() const { return isalpha16; }
   inline void ItsAlpha16() { isalpha16=true; }

   inline int GetPedestalLength() const { return pedestal_length; }
   inline void SetPedestalLength(int l) { pedestal_length=l; }
   inline double GetScale() const { return fScale; }
   inline void SetScale(double s) { fScale=s; }

   void PADdiagnostic();

   void Reset();

   void PrintPWBsettings();
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
