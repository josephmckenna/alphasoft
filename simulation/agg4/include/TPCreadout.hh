// TPC Readout class definition
//------------------------------------------------
// Author: A.Capra   Apr. 2016
//------------------------------------------------

#ifndef __TTPCREADOUT__
#define __TTPCREADOUT__ 1

#include <TObject.h>
#include <TH1D.h>
#include <TMath.h>
#include <TClonesArray.h>

#include "TAnode.hh"
#include "TPads.hh"

#include "Alpha16.h"
#include "GrifC.h"

class TPCreadout
{
public:
  TPCreadout();
  TPCreadout(int);
  TPCreadout(double);
  TPCreadout(const char* );
  virtual ~TPCreadout();
  TPCreadout( const TPCreadout& );

  TPCreadout& operator=( const TPCreadout& );

  inline int GetEventNumber() const {return fEventNumber;}
  inline void SetEventNumber(int e) {fEventNumber=e;}

  inline TAnode* GetWire(int w) { return (TAnode*) fAnodesArray->At(w); }
  inline TPads* GetPad(int p)   { return (TPads*) fPadsArray->At(p);   }

  void AddHit(const double t, const double z, const double phi);
  void AddHit(const int id, const int pdg, const double t, const double z, const double phi);

  int FindPad(const double z, const double phi);
  int FindAnode(const double phi);

  void AddInduction(const double t, const int pad, const int anode);
  void AddInduction(const double t, const int anode);
  void AddInduction(const double t, const int pad, const TF1*);

  virtual void Reset();

  int GetNumberOfWiresHit();
  int GetNumberOfPadsHit();

  inline double GetSignalThreshold() const { return fSignalThreshold; }
  inline void SetSignalThreshold(double thres) { fSignalThreshold = thres; }
  Alpha16Event* GetALPHA16Event(const TClonesArray*);
  GrifCEvent_t* GetGrifCEvent(const TClonesArray*);

private:
  // int SectorAndPad2Index(const int padsec, const int pad);
  // std::pair<int,int> Index2SectorAndPad(const int padindex);
  int fEventNumber;

  TClonesArray* fAnodesArray;
  TClonesArray* fPadsArray;

  //  GarfieldAlpha16EVB fEventBuilder;
  // Alpha16EVB fEventBuilder;
  double fSignalThreshold;

  double fPadsChargeSigma;
  std::vector<int> fAWch;
};

#endif
