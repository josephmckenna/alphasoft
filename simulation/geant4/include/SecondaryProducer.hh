// -------------------------------------------------------------------
//
// Class description:
// Empirical model from p-pbar data for pions production in 
// Antihydrogen-Nuclei annhilation
//
// A. Capra     Sep. 2013
//
// -------------------------------------------------------------------

#ifndef SecondaryProducer_h
#define SecondaryProducer_h 1

//#include "globals.hh"

#include "TGenPhaseSpace.h"
#include "TLorentzVector.h"

#include "G4ThreeVector.hh"
#include <vector>

class G4PrimaryParticle;

class SecondaryProducer//: public G4VPrimaryGenerator
{
public:

  SecondaryProducer();
  ~SecondaryProducer();

  G4int Produce();
  std::vector<G4PrimaryParticle*> GetSecondaries() {return fsecondaries;}
  G4PrimaryParticle* GetSecondary(G4int n);
  void ClearSecondaries();

  inline void SetVerbose(G4int level) {fVerbose=level;}

private:

  G4int GetChannel();
  G4double GetWeight(G4int ch);

  TGenPhaseSpace pspi; // phase space for pions
  TLorentzVector Ppi;  // total 4-momentum of ppbar
  TGenPhaseSpace psph; // phase space for pions
  TLorentzVector Pph;  // total 4-momentum of e-e+

  G4int NFS;
  G4double* BraRatCum;

  std::vector<G4PrimaryParticle*> fsecondaries;

  G4int fVerbose;

};

#endif











/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
