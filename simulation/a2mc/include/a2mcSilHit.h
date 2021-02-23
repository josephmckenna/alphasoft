#ifndef a2mc_SilHIT_H
#define a2mc_SilHIT_H

#include <iostream>

#include <TObject.h>
#include <TVector3.h>

class a2mcSilHit : public TObject
{
  public:
    a2mcSilHit();
    virtual ~a2mcSilHit();

    ///< general methods
    virtual void Print(const Option_t* option = "") const;

    ///< set methods
    void SetTrackID(Int_t track)    {fTrackID   = track; };
    void SetPdgCode(Int_t pdg)      {fPdgCode   = pdg; };
    void SetMotherID(Int_t mid)     {fMotherID  = mid; };
    void SetEventNumber(Int_t ievt) {fEvent     = ievt; };
    void SetSilID(Int_t id)         {fSilID     = id; };
    void SetLayN(Int_t id)          {fLayN      = id; };
    void SetModN(Int_t id)          {fModN      = id; };
    void SetNStrip(Int_t n)         {fnStrp     = n; };
    void SetPStrip(Int_t n)         {fpStrp     = n; };
    void SetEdep(Double_t de)       {fEdep      = de; };
    void SetPos(TVector3 xyz)       {fPosX      = xyz.X(); fPosY = xyz.Y(); fPosZ = xyz.Z();};
    void SetMom(TVector3 xyz)       {fMomX      = xyz.X(); fMomY = xyz.Y(); fMomZ = xyz.Z();};
      
    ///< get methods    
    Int_t GetTrackID()      {return fTrackID;};
    Int_t GetPdgCode()      {return fPdgCode;};
    Int_t GetEventNumber()  {return fEvent;};

    Int_t GetSilID()        {return fSilID;};
    Int_t GetLayN()         {return fLayN;};
    Int_t GetModN()         {return fModN;};
    Int_t GetNStrip()       {return fnStrp;};
    Int_t GetPStrip()       {return fpStrp;};
    Double_t GetEdep()      {return fEdep;};

  private:
    Int_t       fTrackID;   ///< Track Id
    Int_t       fPdgCode;   ///< Particle PDG Code
    Int_t       fMotherID;  ///< Particle mother ID (-1 = primary, 0 = secondary, etc..)
    Int_t       fEvent;     ///< Event Number
    Int_t       fSilID;     ///< Silicon module ID [0-71]
    Int_t       fLayN;      ///< Layer number [0-5] (3 layers for each of the two half)
    Int_t       fModN;      ///< Module number [0-9] for the first layer, [0-11] for the 2nd, [0-13] for the third
    Int_t       fnStrp;     ///< n-Strip that fired
    Int_t       fpStrp;     ///< p-Strip that fired
    Double_t    fEdep;      ///< Energy deposit
    Double_t    fPosX;      ///< Hit coordinates (at the entrance of the detector)
    Double_t    fPosY;
    Double_t    fPosZ;
    Double_t    fMomX;      ///< Track momentum when releasing the hit
    Double_t    fMomY;
    Double_t    fMomZ;
    
  ClassDef(a2mcSilHit,1)
};

#endif //a2mc_SilHIT_H


