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

#include "TClonesArray.h"

class Deconv
{
private:
  bool fTrace;

  AnaSettings* ana_settings;

  // input
  std::vector<double> fAnodeFactors;

  std::vector<double> fAnodeResponse;
  std::vector<double> fPadResponse;

  int fbinsize;
  int fAWbinsize;
  int fPADbinsize;

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

  std::vector<signal> sanode;
  std::vector<signal> spad;

  std::set<double> aTimes;
  std::set<double> pTimes;
  
  // check
  std::vector<double> resRMS_a;
  std::vector<double> resRMS_p;

  // anode mask
  std::vector<int> fAwMask;
  // pad mask
  std::vector<int> fPadSecMask;
  std::vector<int> fPadRowMask;

  void Init();

  int ReadResponseFile(const double awbin, const double padbin);
  std::vector<double> Rebin(const std::vector<double> &in, int binsize, double ped = 0.);

  int Deconvolution( std::vector<std::vector<double>*>* subtracted, 
		     std::vector<signal> &fSignals, std::set<double> &fTimes,
		     std::vector<electrode> &fElectrodeIndex, 
		     std::vector<double> &fResponse, int theBin, bool isanode );
  void Subtract(std::map<int,wfholder*>* wfmap,
		const unsigned i, const int b,
		const double ne,std::vector<electrode> &fElectrodeIndex, 
		std::vector<double> &fResponse, int theBin, bool isanode);
  std::set<wfholder*,comp_hist_t>* wforder(std::vector<std::vector<double>*>* subtracted, const int b);
  std::map<int,wfholder*>* wfordermap(std::set<wfholder*,comp_hist_t>* histset,std::vector<electrode> &fElectrodeIndex);

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

   padmap* pmap;

public:
  Deconv(double adc, double pwb, double aw, double pad);
  Deconv(std::string);
  ~Deconv();

  int FindAnodeTimes(TClonesArray*);
  int FindPadTimes(TClonesArray*);

  std::vector<signal>* GetAnodeSignal() { return &sanode; }
  std::vector<signal>* GetPadSignal() { return &spad; }

  void SetTrace(bool t) { fTrace=t; }

  std::vector<double>* GetAnodeDeconvRemainder() { return &resRMS_a; }
  std::vector<double>* GetPadDeconvRemainder() { return & resRMS_p; }

  double GetADCdelay() const { return fADCdelay; }
  double GetPWBdelay() const { return fPWBdelay; }

  double GetADCthres() const { return fADCThres; }
  double GetPWBthres() const { return fPWBThres; }
  double GetAWthres() const  { return fADCpeak; }
  double GetPADthres() const { return fPWBpeak; }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
