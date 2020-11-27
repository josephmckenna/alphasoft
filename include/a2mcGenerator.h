#ifndef a2mcGenerator_H
#define a2mcGenerator_H


#include <TPDGCode.h>
#include <TDatabasePDG.h>

#include "a2mcSettings.h"
#include "a2mcApparatus.h"

class TVirtualMCStack;
class a2mcApparatus;

class a2mcGenerator : public TObject
{
public:
    a2mcGenerator(TVirtualMCStack* stack,a2mcApparatus* fDetConstruction);
    virtual ~a2mcGenerator();
    virtual void Generate();
    virtual void DumpGenInfo();
    
    Int_t       GetnGens()      {return nGens;};
    Int_t       GetnTrks()      {return nTrks;};
    Int_t       GetPdgCode()    {return fPDG;};
    Double_t    GetVx()         {return fGenPos[0];};
    Double_t    GetVy()         {return fGenPos[1];};
    Double_t    GetVz()         {return fGenPos[2];};
    Double_t    GetPx()         {return fGenMom[0];};
    Double_t    GetPy()         {return fGenMom[1];};
    Double_t    GetPz()         {return fGenMom[2];};

    Double_t EquivalentTimeMin();
    TVector3 LineExtrapolation(TVector3, TVector3, Double_t);
    Bool_t CheckSilDetCrossing(TVector3, TVector3);
    Bool_t CheckHemisphereBase(TVector3, TVector3, Double_t, Double_t);
    //static access method
    static a2mcGenerator* Instance();
    
private:
    static a2mcGenerator* fgInstance;   ///< Singleton instance
    a2mcSettings a2mcConf{};            ///< Configuration settings (a2MC.ini)
    TVirtualMCStack*  fStack;           ///< VMC stack
    a2mcApparatus* fDetConstruction;    ///< a2mcApparatus
    ///< Single particle variables
    Int_t       fPDG;                   ///< pdg code of the particle (see $ROOTSYS/include/TPDGCode.h)
    Double_t    fKinE;                  ///< kinetic energy (at generation)
    Double_t    fTotE;                  ///< total energy
    TVector3    fGenPos;                ///< Origin 
    TVector3    fGenMom;                ///< Momentum at the origin 
    ///< Overall generator parameters
    Double_t    fSkyDx, fSkyDz, fSkyY;  ///< Parameters to generate over an horizontal plane 
    Int_t       nGens;                  ///< Number of generated particles 
    Int_t       nTrks;                  ///< Number of particles sent to tracking (maybe different from nGens)
    Double_t p_min[500];                ///< muon momentum cumulative distribution from ext file
    
    ///< Private methods
    Bool_t      GenPbars();             ///< 
    Bool_t      GenMuons();             ///< 
    Double_t    Maxwell(Double_t, Double_t);

ClassDef(a2mcGenerator,1) // a2mc MC generator
 
};
#endif
