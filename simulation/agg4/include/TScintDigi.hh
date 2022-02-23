// Scintillating Bars Digitization Class Definition
//-------------------------------------------------
// Author: A.Capra   Jan 2016
//-------------------------------------------------

#ifndef __TScintDigi__
#define __TScintDigi__ 1

#include <TObject.h>

class TScintDigi: public TObject
{
private:
  int fID;  // G4 track ID
  int fPDG; // PDG code

  double fp; // MC phi position [rad]
  double ft; // MC time [ns]
  double ft_in; // MC time incoming hit [ns]
  double ft_out; // MC time outgoing hit [ns]

  double fPos; // digitized hit position = phi [rad]
  
  int fBar;    // bar hit
  int fNhits; // number of hits in the track
  bool fMultiTrack; // not used
  int fMotherID;
  double fpx_in; // [mm]
  double fpy_in; // [mm]
  double fpz_in; // [mm]
  double fpx_out; // [mm]
  double fpy_out; // [mm]
  double fpz_out; // [mm]


  double fSigmaPhi; // [rad]
  double fz; // [mm]
  double fr;  
  double fSmearZ;
  double fSigmaZ;
  double fCentreR;

  double fEnergy; // MeV

  int fMultiplicity;
  
public:
  TScintDigi() {};
  TScintDigi(int, int, double, double);
  TScintDigi(int, int, double, double, double, double);
  TScintDigi(int, int, double, double, double, double, double);

  // determine the bar hit from the MC phi position
  void Digi();

  inline int GetTrackID()  { return fID; }
  inline int GetTrackPDG() { return fPDG; }

  inline double GetPhi()  const { return fp; }
  inline double GetTime() const { return ft; }
  inline double GetTimeIn() const { return ft_in; }
  inline double GetTimeOut() const { return ft_out; }

  inline int GetBar()     const { return fBar; }
  inline double GetPos()  const { return fPos; }
  inline double GetSigmaPhi() const { return fSigmaPhi; }

  inline double GetMCR() const {return fr;}
  inline double GetMCZ() const {return fz;}

  inline double GetZ() const {return fSmearZ;}
  inline void SetSigmaZ(double sig) {fSigmaZ=sig;}
  inline double GetSigmaZ() const {return fSigmaZ;}

  inline double GetR() const {return fCentreR;}

  inline int GetMultiplicity() const {return fMultiplicity;}

  inline double GetEnergy() const {return fEnergy;}

  bool IsSameDigi(TScintDigi*);
  void PrintChannel();

  void SetBarID(int barid) {fBar = barid;}
  void SetNhits(int nhits) {fNhits = nhits;}
  void SetMultitracks(bool bmultitrack) {fMultiTrack = bmultitrack;}
  void SetTrackID(int trkid) {fID = trkid;}
  void SetMotherID(int mthrid) {fMotherID = mthrid;}
  void SetEnergy(double energy) { fEnergy=energy; }
  void SetPos_in(double p[3]) {fpx_in = p[0]; fpy_in = p[1]; fpz_in = p[2];}
  void SetPos_out(double p[3]) {fpx_out = p[0]; fpy_out = p[1]; fpz_out = p[2];}
  void SetTimeIn(double tIN) { ft_in = tIN; }
  void SetTimeOut(double tOUT) { ft_out = tOUT; }


  int GetNhits() {return fNhits;}
  int GetMotherID() {return fMotherID;}
  void GetPos_in(double* p) {p[0]=fpx_in; p[1]=fpy_in; p[2]=fpz_in; }
  void GetPos_out(double* p) {p[0]=fpx_out; p[1]=fpy_out; p[2]=fpz_out;}
   ClassDef(TScintDigi,1)
};

#endif
