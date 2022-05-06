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

   const AnaSettings* ana_settings;

   // input
   std::vector<double> fPadResponse;

   std::vector<double> fPwbRescale;

   // pad mask
   std::vector<int> fPadSecMask;
   std::vector<int> fPadRowMask;

   const int fPADbinsize;

   const double fPWBmax;
   const double fPWBrange;

   double fPWBdelay;

   int pedestal_length;
   double fScale;

   unsigned thePadBin;

   const double fPWBThres;

   const double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   ALPHAg::padmap pmap;

   static TH2D* hPWBped;
   static TProfile* hPWBped_prox;

   // pads
   static TH1D* hAvgRMSPad;
   // anodes
   //   static TH1D* hAvgRMSBot;
   static TH1D* hAvgRMSTop;

   // std::vector<signal>* fPwbRange;

public:

   int ReadResponseFile( const int padbin);
   int ReadPADResponseFile( const int padbin );
   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
   int ReadRescaleFile();
   int ReadPWBRescaleFile();


   void Deconvolution( std::vector<ALPHAg::wfholder>& subtracted,
                       const std::vector<ALPHAg::electrode> &fElectrodeIndex,
                       std::vector<ALPHAg::TPadSignal>& signal,
                       const int thread_no,
                       const int total_threads ) const;

   void DeconvolutionByRange( std::vector<ALPHAg::wfholder>& subtracted,
                       const std::vector<ALPHAg::electrode> &fElectrodeIndex,
                       std::vector<ALPHAg::TPadSignal>& signals,
                       const int start,
                       const int stop) const;

   void LogDeconvRemaineder( std::vector<ALPHAg::wfholder>& PadWaves );

   void SubtractPAD(ALPHAg::wfholder* hist1,
                   const int b,
                   const double ne,
                   const std::vector<ALPHAg::electrode> &fElectrodeIndex) const;
   
   ALPHAg::comp_hist_t wf_comparator;
   std::vector<ALPHAg::wfholder*> wforder(std::vector<ALPHAg::wfholder>& subtracted, const unsigned b) const;

   //Take advantage that there are 256 anode wires
   inline bool IsAnodeNeighbour(const int w1, const int w2, int dist) const
   {
      uint8_t c=w1-w2;
      return (Min(c,256-c)==dist);
   }

   inline uint8_t IsAnodeClose(const int w1, const int w2) const
   {
      uint8_t c=w1-w2;
      return Min(c,256-c);
   }
   
   inline uint8_t Min(uint8_t x, uint8_t y) const
   {
      return (x < y)? x : y;
   }

   inline bool MaskPads(const short& sec, const int& row) const
   {
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
         for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
            if( *it == sec && *jt == row ) return true;
      return false;
   }

   inline double CalculatePedestal(const std::vector<int>& adc_samples) const
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
   
   inline double GetPeakHeight(const std::vector<int>& adc_samples,const int& i, const double& ped) const
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      const double y = double(*minit);
      return GetPWBpeak(i,y,ped);
   }

   inline double GetPeakTime(const std::vector<int>& adc_samples) const
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double peak_time = ( (double) std::distance(adc_samples.begin(),minit) + 0.5 );
      peak_time *= double(fPADbinsize);
      peak_time += fPWBdelay;
      return peak_time;
   }

   inline double GetPWBpeak(const int& i, const double& y, const double& ped) const
   {
      const double amp = fScale * y;
      if( amp < fPWBrange )
         return fPwbRescale[i] * fScale * ( y - ped );
      else
         return fPWBmax;
   }

   // Calculate deconvolution error from residual (may change)
   inline double GetNeErr(const double /*ne (number of electrons)*/, const double res) const
   {
      return sqrt(res);
   }


public:
   DeconvPAD( double pwb,  double pad);
   DeconvPAD(std::string);
   DeconvPAD(AnaSettings*);
   ~DeconvPAD();

   void Setup();
   void SetupPWBs(TFile* fout, int run, bool norm=false, bool diag=false);

#ifdef BUILD_AG_SIM
   int FindPadTimes(TClonesArray*, std::vector<ALPHAg::TPadSignal>& signals);
#endif

void BuildWFContainer(
    const FeamEvent* padSignals,
    std::vector<ALPHAg::wfholder>& PadWaves,
    std::vector<ALPHAg::electrode>& PadIndex,
    std::vector<ALPHAg::wf_ref>& feamwaveforms,  // to use in aged display
    std::vector<ALPHAg::TPadSignal>& PwbPeaks // waveform max
) const;
 

   inline void SetTrace(bool t)      { fTrace=t; }
   inline void SetDiagnostic(bool d) { fDiagnostic=d; }
   inline void SetDisplay(bool a)    { fAged=a; }


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
