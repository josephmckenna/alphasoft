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

  double fPos; // digitized hit position = phi [rad]
  int fBar;    // bar hit
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

  ClassDef(TScintDigi,1)
};

#endif
