// TPC Digitization Class
// simple method via arithmetics calculations
// saved to ROOT file (output of the simulation)
// used in the analysis
//------------------------------------------------
// Author: A.Capra   Nov 2014
//------------------------------------------------

#ifndef __TDIGI__
#define __TDIGI__

#include <TObject.h>

class TDigi: public TObject
{
private:
  double fPadZed;
  double fPadRphi;
  double fPadTime;

  int fCharge;
  
  int fID;
  int fPDG;

  int fChannelZed;
  int fChannelRphi;
  int fChannelTime;

  int fChannelZedOffset;
  int fChannelRphiOffset;

  double fHitZed;
  double fHitRphi;
  double fHitTime;
  
  double fDigiZed;
  double fDigiRphi;
  double fDigiTime;
 
  double fHitRadius;

public:
  TDigi();
  TDigi(int, int, double, double, double);
  TDigi(double, double, double);
  ~TDigi() {};

  inline int GetTrackID()  { return fID; }
  inline int GetTrackPDG() { return fPDG; }

  inline int GetCharge() { return fCharge; }

  inline void   SetPadTime(double t)   { fPadTime=t; }
  inline double GetPadTime()           { return fPadTime; }
  inline void   SetPadZ(double z)      { fPadZed=z; }
  inline double GetPadZ()              { return fPadZed; }
  inline void   SetPadRPhi(double rp)  { fPadRphi = rp; }
  inline double GetPadRPhi()           { return fPadRphi; }

  inline int GetChannelZed()  { return fChannelZed; }
  inline int GetChannelRphi() { return fChannelRphi; }
  inline int GetChannelTime() { return fChannelTime; }
  
  inline void SetChannelZed(int ch)  { fChannelZed=ch; }
  inline void SetChannelRphi(int ch) { fChannelRphi=ch; }
  inline void SetChannelTime(int ch) { fChannelTime=ch; }

  inline int GetChannelOffsetZed()  { return fChannelZedOffset; }
  inline int GetChannelOffsetRphi() { return fChannelRphiOffset; }
  
  inline void SetChannelOffsetZed(int off)  { fChannelZedOffset=off; }
  inline void SetChannelOffsetRphi(int off) { fChannelRphiOffset=off; }

  inline double GetDigiZed()  { return fDigiZed; }  
  inline double GetDigiRphi() { return fDigiRphi; }
  inline double GetDigiTime() { return fDigiTime; }

  void Digitization();
  void DigiPosition();

  bool IsSameDigi(TDigi*);
  bool IsSamePad(int,int,int);
  bool IsSamePad(TDigi*);

  void PrintChannel();

  inline bool IsSortable() const { return true; }
  int Compare(const TObject*) const;


  ClassDef(TDigi,1)
};

#endif
