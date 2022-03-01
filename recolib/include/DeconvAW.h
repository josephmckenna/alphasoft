#ifndef __DECONV_AW__
#define __DECONV_AW__ 1

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

class DeconvAW
{
private:
   bool fTrace;
   bool fDiagnostic;
   bool fAged;

   const AnaSettings* ana_settings;

   // input
   std::vector<double> fAnodeFactors;

   std::vector<double> fAnodeResponse;

   std::vector<double> fAdcRescale;

   // anode mask
   std::vector<int> fAwMask;

   int fAWbinsize; //const me please

   const double fADCmax;
   const double fADCrange;

   double fADCdelay;

   int pedestal_length;
   double fScale;

   unsigned theAnodeBin;

   const double fADCThres;

   const double fADCpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

public:

   int ReadResponseFile(const int awbin);
   int ReadAWResponseFile( const int awbin );
   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
   int ReadRescaleFile();
   int ReadADCRescaleFile();

   void BuildWFContainer(
                       const Alpha16Event* anodeSignals,
                       std::vector<ALPHAg::wfholder>& AnodeWaves,
                       std::vector<ALPHAg::electrode>& AnodeIndex,
                       std::vector<ALPHAg::wf_ref>& wirewaveforms,  // to use in aged display
                       std::vector<ALPHAg::TWireSignal>& AdcPeaks // waveform max
                       ) const;
   void Deconvolution( std::vector<ALPHAg::wfholder>& subtracted,
                       const std::vector<ALPHAg::electrode> &fElectrodeIndex,
                       std::vector<ALPHAg::TWireSignal>& signal ) const;

   void Deconvolution( std::vector<ALPHAg::wfholder>& subtracted,
                       const std::vector<ALPHAg::electrode> &fElectrodeIndex,
                       std::vector<ALPHAg::TWireSignal>& signals,
                       const int start,
                       const int stop) const;

   void LogDeconvRemaineder( std::vector<ALPHAg::wfholder>& WireWaves );

   void SubtractAW(ALPHAg::wfholder* hist1,
                   std::vector<ALPHAg::wfholder>& wfmap,
                   const int b,
                   const double ne,
                   const std::vector<ALPHAg::electrode> &fElectrodeIndex) const;

   ALPHAg::comp_hist_t wf_comparator;
   std::vector<ALPHAg::wfholder*> wforder(std::vector<ALPHAg::wfholder>& subtracted, const unsigned b) const;
   
   //Take advantage that there are 256 anode wires
   inline bool IsAnodeNeighbour(const int w1,const int w2, const int dist) const
   {
      uint8_t c=w1-w2;
      return (Min(c,256-c)==dist);
   }

   inline uint8_t IsAnodeClose(const int w1,const int w2) const
   {
      uint8_t c=w1-w2;
      return Min(c,256-c);
   }
   
   inline uint8_t Min(uint8_t x, uint8_t y) const
   {
      return (x < y)? x : y;
   }

   inline bool MaskWires(int& aw) const
   {
      for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
         if( *it == aw ) return true;
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
      return GetADCpeak(i,y,ped);
   }

   inline double GetPeakTime(const std::vector<int>& adc_samples) const
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double peak_time = ( (double) std::distance(adc_samples.begin(),minit) + 0.5 );
      peak_time *= double(fAWbinsize);
      peak_time += fADCdelay;
      return peak_time;
   }

   inline double GetADCpeak(const int& i,const double& y,const double& ped) const
   {
      const double amp = fScale * y;
      if( amp < fADCrange )
         return  fAdcRescale[i] * fScale * ( y - ped );
      else
         return fADCmax;
   }

   // Calculate deconvolution error from residual (may change)
   inline double GetNeErr(const double /*ne (number of electrons)*/, const double res) const
   {
      return sqrt(res);
   }

   static TH2D* hADCped;
   static TProfile* hADCped_prox;

   // pads
   static TH1D* hAvgRMSPad;
   // anodes
   //   static TH1D* hAvgRMSBot;
   static TH1D* hAvgRMSTop;

   //std::vector<signal>* fAdcRange;

public:
   DeconvAW(double adc, double pwb, double aw, double pad);
   DeconvAW(std::string);
   DeconvAW(AnaSettings*);
   ~DeconvAW();

   void Setup();
   void SetupADCs(TFile* fout, int run, bool norm=false, bool diag=false);

#ifdef BUILD_AG_SIM
   int FindAnodeTimes(TClonesArray*);
#endif

   int FindAnodeTimes(const Alpha16Event*);
  
   inline void SetTrace(bool t)      { fTrace=t; }
   inline void SetDiagnostic(bool d) { fDiagnostic=d; }
   inline void SetDisplay(bool a)    { fAged=a; }

   inline double GetADCdelay() const { return fADCdelay; }
   inline void SetADCdelay(double d) { fADCdelay = d; }

   inline double GetADCthres() const { return fADCThres; }
   inline double GetAWthres() const  { return fADCpeak; }

   inline bool IsItAlpha16() const { return isalpha16; }
   inline void ItsAlpha16() { isalpha16=true; }

   inline int GetPedestalLength() const { return pedestal_length; }
   inline void SetPedestalLength(int l) { pedestal_length=l; }
   inline double GetScale() const { return fScale; }
   inline void SetScale(double s) { fScale=s; }

   void AWdiagnostic();

   void Reset();

   void PrintADCsettings();
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
