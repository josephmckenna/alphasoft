#ifndef __DECONV__
#define __DECONV__ 1

#include <cassert>
#include <iostream>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "SignalsType.h"
#include "AnaSettings.h"

#include <TClonesArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>

#include "Alpha16.h"
#include "Feam.h"

class Deconv
{
private:
   bool fTrace;
   bool fDiagnostic;
   bool fAged;

   AnaSettings* ana_settings;

   // input
   std::vector<double> fAnodeFactors;

   std::vector<double> fAnodeResponse;
   std::vector<double> fPadResponse;

   std::vector<double> fAdcRescale;
   std::vector<double> fPwbRescale;

   // anode mask
   std::vector<int> fAwMask;
   // pad mask
   std::vector<int> fPadSecMask;
   std::vector<int> fPadRowMask;

   int fbinsize;
   int fAWbinsize;
   int fPADbinsize;

   double fADCmax;
   double fADCrange;
   double fPWBmax;
   double fPWBrange;

   double fADCdelay;
   double fPWBdelay;

   int pedestal_length;
   double fScale;

   int theAnodeBin;
   int thePadBin;

   double fADCThres;
   double fPWBThres;

   double fAvalancheSize;
   double fADCpeak;
   double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<electrode> fAnodeIndex;
   std::vector<electrode> fPadIndex;

   std::vector<signal>* sanode;
   std::vector<signal>* spad;

   // check
   std::vector<double> resRMS_a;
   std::vector<double> resRMS_p;

   void Setup();

   int ReadResponseFile(const double awbin, const double padbin);
   int ReadAWResponseFile( const double awbin );
   int ReadPADResponseFile( const double padbin );
   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
   int ReadRescaleFile();
   int ReadADCRescaleFile();
   int ReadPWBRescaleFile();

   std::vector<signal>* Deconvolution( std::vector<wfholder*>* subtracted,
                                       std::vector<electrode> &fElectrodeIndex,
                                       std::vector<double> &fResponse, int theBin, bool isanode);

   void SubtractAW(wfholder* hist1,
                   std::vector<wfholder*>* wfmap,
                   const int b,
                   const double ne,std::vector<electrode> &fElectrodeIndex,
                   std::vector<double> &fResponse, int theBin);
   
   void SubtractPAD(wfholder* hist1,
                    std::vector<wfholder*>* wfmap,
                    const int b,
                    const double ne,std::vector<electrode> &fElectrodeIndex,
                    std::vector<double> &fResponse, int theBin);
   
   comp_hist_t wf_comparator;
   std::vector<wfholder*>* wforder(std::vector<wfholder*>* subtracted, const int b);
   
   std::map<int,wfholder*>* wfordermap(std::vector<wfholder*>* histset,
                                       std::vector<electrode> &fElectrodeIndex);

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

   inline bool MaskWires(int& aw)
   {
      for(auto it=fAwMask.begin(); it!=fAwMask.end(); ++it)
         if( *it == aw ) return true;
      return false;
   }

   inline bool MaskPads(short& sec, int& row)
   {
      for(auto it=fPadSecMask.begin(); it!=fPadSecMask.end(); ++it)
         for(auto jt=fPadRowMask.begin(); jt!=fPadRowMask.end(); ++jt)
            if( *it == sec && *jt == row ) return true;
      return false;
   }

   inline double CalculatePedestal(std::vector<int>& adc_samples)
   {
      double ped(0.);
      for(int b = 0; b < pedestal_length; b++) ped += double(adc_samples.at( b ));
      ped /= double(pedestal_length);
      return ped;
   }
   
   inline double GetPeakHeight(std::vector<int>& adc_samples, int& i, double& ped, bool isanode)
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double max=-1.,y=double(*minit);
      if( isanode ) max = GetADCpeak(i,y,ped);
      else max = GetPWBpeak(i,y,ped);
      return max;
   }

   inline double GetPeakTime(std::vector<int>& adc_samples, bool isanode)
   {
      auto minit = std::min_element(adc_samples.begin(), adc_samples.end());
      double peak_time = ( (double) std::distance(adc_samples.begin(),minit) + 0.5 );
      if( isanode )
         {
            peak_time*=double(fAWbinsize);
            peak_time+=fADCdelay;
         }
      else
         {
            peak_time*=double(fPADbinsize);
            peak_time+=fPWBdelay;
         }
      return peak_time;
   }

   inline double GetADCpeak(int& i, double& y, double& ped)
   {
      double amp = fScale * y, max=-1.;
      if( amp < fADCrange )
         max =  fAdcRescale.at(i) * fScale * ( y - ped );
      else
         max = fADCmax;
      return max;
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

   padmap* pmap;

   TH2D* hADCped;
   TProfile* hADCped_prox;
   TH2D* hPWBped;
   TProfile* hPWBped_prox;

   // pads
   TH1D* hAvgRMSPad;
   // anodes
   TH1D* hAvgRMSBot;
   TH1D* hAvgRMSTop;

   // to use in aged display
   std::vector<wf_ref>* wirewaveforms;
   std::vector<wf_ref>* feamwaveforms;

   // waveform max
   std::vector<signal>* fAdcPeaks;
   //std::vector<signal>* fAdcRange;
   std::vector<signal>* fPwbPeaks;
   // std::vector<signal>* fPwbRange;

public:
   Deconv(double adc, double pwb, double aw, double pad);
   Deconv(std::string);
   Deconv(AnaSettings*);
   ~Deconv();

   void SetupADCs(int run, bool norm=false, bool diag=false);
   void SetupPWBs(int run, bool norm=false, bool diag=false);

   int FindAnodeTimes(TClonesArray*);
   int FindPadTimes(TClonesArray*);

   int FindAnodeTimes(const Alpha16Event*);
   int FindPadTimes(const FeamEvent*);

   inline std::vector<signal>* GetAnodeSignal() { return sanode; }
   inline std::vector<signal>* GetPadSignal()  { return spad; }

   inline std::vector<signal>* GetAdcPeaks() { return fAdcPeaks; }
   inline std::vector<signal>* GetPWBPeaks() { return fPwbPeaks; }

   inline std::vector<wf_ref>* GetAWwaveforms()  { return wirewaveforms; }
   inline std::vector<wf_ref>* GetPADwaveforms() { return feamwaveforms; }

   inline void SetTrace(bool t)      { fTrace=t; }
   inline void SetDiagnostic(bool d) { fDiagnostic=d; }
   inline void SetDisplay(bool a)    { fAged=a; }

   inline std::vector<double>* GetAnodeDeconvRemainder() { return &resRMS_a; }
   inline std::vector<double>* GetPadDeconvRemainder() { return & resRMS_p; }

   inline double GetADCdelay() const { return fADCdelay; }
   inline double GetPWBdelay() const { return fPWBdelay; }
   inline void SetADCdelay(double d) { fADCdelay = d; }
   inline void SetPWBdelay(double d) { fPWBdelay = d; }

   inline double GetADCthres() const { return fADCThres; }
   inline double GetPWBthres() const { return fPWBThres; }
   inline double GetAWthres() const  { return fADCpeak; }
   inline double GetPADthres() const { return fPWBpeak; }

   inline bool IsItAlpha16() const { return isalpha16; }
   inline void ItsAlpha16() { isalpha16=true; }

   void AWdiagnostic();
   void PADdiagnostic();

   void Reset();

   void PrintADCsettings();
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
