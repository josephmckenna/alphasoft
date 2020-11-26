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

    // methods
    virtual void Print(const Option_t* option = "") const;

    //
    // set methods
    
    /// Set track Id
    void SetTrackID  (Int_t track)  { fTrackID = track; };

    /// Set particle PDG Code
    void SetPdgCode  (Int_t pdg)  { fPdgCode = pdg; };

    /// Set Mother ID
    void SetMotherID  (Int_t mid)  { fMotherID = mid; };

    /// Set track Id
    void SetEventNumber  (Int_t ievt)  { fEvent = ievt; };
    
    void SetHalfID(Int_t id)   { fHalfID  = id; };
    void SetLayerID(Int_t id)  { fLayerID = id; };  
    void SetModuleID(Int_t id)  { fModuleID = id; };  

    /// Set energy deposit
    void SetEdep     (Double_t de)  { fEdep = de; };

    /// Set hit time 
    void SetTime    (Double_t dt)  { fTime = dt; };
    
    /// Set step length 
    void SetStep    (Double_t dl)  { fStep = dl; };
    
    /// Set momentum
    void SetMom      (TVector3 xyz) { fMom = xyz; fMomX = xyz.X(); fMomY = xyz.Y(); fMomZ = xyz.Z();};
      
    /// Set hit coordinates
    void SetPos      (TVector3 xyz) { fPos = xyz; fPosX = xyz.X(); fPosY = xyz.Y(); fPosZ = xyz.Z();};
      
    //
    // get methods
    
    /// \return The track Id
    Int_t GetTrackID()   { return fTrackID; };

    Int_t GetPdgCode()  { return fPdgCode; };
    /// \return The event number
    Int_t GetEventNumber()   { return fEvent; };

    Int_t GetHalfID()  { return fHalfID;  };
    Int_t GetLayerID() { return fLayerID; };
    Int_t GetModuleID() { return fModuleID; };

    /// \return The energy deposit
    Double_t GetEdep()   { return fEdep; };      

    /// \return The hit time
    Double_t GetTime()   { return fTime; };      

    /// \return The step length
    Double_t GetStep()   { return fStep; };      

    // /// \return The track position
    TVector3 GetMom()    { return fMom; };
      
    // // return Hit coordinates (at the entrance of the detector)
    TVector3 GetPos()    { return fPos; };

  private:
    Int_t       fTrackID;    // Track Id
    Int_t       fPdgCode;    // Particle PDG Code
    Int_t       fMotherID;   // Particle mother ID (-1 = primary, 0 = secondary, etc..)
    Int_t       fEvent;      // Event Number
    Int_t       fHalfID;     // Telescope 
    Int_t       fLayerID;    // Layer
    Int_t       fModuleID;    // Fiber
    Double_t    fEdep;       // Energy deposit
    Double_t    fTime;       // Hit time
    Double_t    fStep;       // Step Length
    TVector3	fPos;
    TVector3    fMom;
    Double_t    fPosX;		///< Hit coordinates (at the entrance of the detector)
    Double_t    fPosY;
    Double_t    fPosZ;
    Double_t    fMomX;		///< Track momentum when releasing the hit
    Double_t    fMomY;
    Double_t    fMomZ;
    
  ClassDef(a2mcSilHit,1) //a2mcSilHit  
};

#endif //a2mc_SilHIT_H


