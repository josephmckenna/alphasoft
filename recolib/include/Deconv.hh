#ifndef __DECONV__
#define __DECONV__ 1

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

   // double fAvalancheSize; //Not used
   double fADCpeak;
   double fPWBpeak;

   bool isalpha16; // flag to distinguish 100Ms/s from 62.5 Ms/s ADCs

   // output
   std::vector<ALPHAg::electrode> fAnodeIndex;
   std::vector<ALPHAg::electrode> fPadIndex;

   std::vector<ALPHAg::signal>* sanode;
   std::vector<ALPHAg::signal>* spad;

   // // check
   // std::vector<double> resRMS_a;
   // std::vector<double> resRMS_p;


   int ReadResponseFile(const double awbin, const double padbin);
   int ReadAWResponseFile( const double awbin );
   int ReadPADResponseFile( const double padbin );
   std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);
   int ReadRescaleFile();
   int ReadADCRescaleFile();
   int ReadPWBRescaleFile();

   std::vector<ALPHAg::signal>* Deconvolution( std::vector<ALPHAg::wfholder*>* subtracted,
                                       std::vector<ALPHAg::electrode> &fElectrodeIndex,
                                       std::vector<double> &fResponse, int theBin, bool isanode);

   void SubtractAW(ALPHAg::wfholder* hist1,
                   std::vector<ALPHAg::wfholder*>* wfmap,
                   const int b,
                   const double ne,std::vector<ALPHAg::electrode> &fElectrodeIndex,
                   std::vector<double> &fResponse, int theBin);
   
   void SubtractPAD(ALPHAg::wfholder* hist1,
                    std::vector<ALPHAg::wfholder*>* wfmap,
                    const int b,
                    const double ne,std::vector<ALPHAg::electrode> &fElectrodeIndex,
                    std::vector<double> &fResponse, int theBin);
   
   ALPHAg::comp_hist_t wf_comparator;
   std::vector<ALPHAg::wfholder*>* wforder(std::vector<ALPHAg::wfholder*>* subtracted, const int b);
   
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
      if( pedestal_length > 0 )
         ped /= double(pedestal_length);
      // int temp=0;
      // std::accumulate(adc_samples.begin(),adc_samples.begin()+pedestal_length,temp);
      // double ped = double(temp)/double(pedestal_length);
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

   ALPHAg::padmap* pmap;

   static TH2D* hADCped;
   static TProfile* hADCped_prox;
   static TH2D* hPWBped;
   static TProfile* hPWBped_prox;

   // pads
   static TH1D* hAvgRMSPad;
   // anodes
   //   static TH1D* hAvgRMSBot;
   static TH1D* hAvgRMSTop;

   // to use in aged display
   std::vector<ALPHAg::wf_ref>* wirewaveforms;
   std::vector<ALPHAg::wf_ref>* feamwaveforms;

   // waveform max
   std::vector<ALPHAg::signal>* fAdcPeaks;
   //std::vector<signal>* fAdcRange;
   std::vector<ALPHAg::signal>* fPwbPeaks;
   // std::vector<signal>* fPwbRange;

public:
   Deconv(double adc, double pwb, double aw, double pad);
   Deconv(std::string);
   Deconv(AnaSettings*);
   ~Deconv();

   void Setup();
   void SetupADCs(TFile* fout, int run, bool norm=false, bool diag=false);
   void SetupPWBs(TFile* fout, int run, bool norm=false, bool diag=false);

   int FindAnodeTimes(TClonesArray*);
   int FindPadTimes(TClonesArray*);

   int FindAnodeTimes(const Alpha16Event*);
   int FindPadTimes(const FeamEvent*);

   inline std::vector<ALPHAg::signal>* GetAnodeSignal() { return sanode; }
   inline std::vector<ALPHAg::signal>* GetPadSignal()  { return spad; }

   inline std::vector<ALPHAg::signal>* GetAdcPeaks() { return fAdcPeaks; }
   inline std::vector<ALPHAg::signal>* GetPWBPeaks() { return fPwbPeaks; }

   inline std::vector<ALPHAg::wf_ref>* GetAWwaveforms()  { return wirewaveforms; }
   inline std::vector<ALPHAg::wf_ref>* GetPADwaveforms() { return feamwaveforms; }

   inline void SetTrace(bool t)      { fTrace=t; }
   inline void SetDiagnostic(bool d) { fDiagnostic=d; }
   inline void SetDisplay(bool a)    { fAged=a; }

   // inline std::vector<double>* GetAnodeDeconvRemainder() { return &resRMS_a; }
   // inline std::vector<double>* GetPadDeconvRemainder() { return & resRMS_p; }

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

   inline int GetPedestalLength() const { return pedestal_length; }
   inline void SetPedestalLength(int l) { pedestal_length=l; }
   inline int GetScale() const { return fScale; }
   inline void SetScale(double s) { fScale=s; }

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
