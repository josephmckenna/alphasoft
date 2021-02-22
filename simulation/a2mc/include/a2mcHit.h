#ifndef a2mc_HIT_H
#define a2mc_HIT_H

#include <TObject.h>
#include <TVector3.h>

class a2mcHit : public TObject
{
  public:
    a2mcHit();
    virtual ~a2mcHit();

    // methods
    virtual void Print(const Option_t* option = "") const;
    //
    // set methods
    
    void SetEventNumber (Int_t ievt)    { fEvent = ievt;    }; //0
    void SetTrackID     (Int_t track)   { fTrackID = track; }; //1
    void SetPdgCode     (Int_t pdg)     { fPdgCode = pdg;   }; //2
    void SetMotherID    (Int_t mid)     { fMotherID = mid;  }; //3
    void SetVolCopyNo   (Int_t no)      { fVolCopyNo = no;  }; //4
    void SetVolLSD      (Double_t lsd)  { fVolLSD = lsd;    }; //5
    void SetMom         (TVector3 xyz)  { fMom = xyz; fMomX = xyz.X(); fMomY = xyz.Y(); fMomZ = xyz.Z();}; //6     
    void SetPos         (TVector3 xyz)  { fPos = xyz; fPosX = xyz.X(); fPosY = xyz.Y(); fPosZ = xyz.Z();}; //7
    void SetIsEntering  (Bool_t Is)     { fIsEntering = Is; }; //8
    void SetIsExiting   (Bool_t Is)     { fIsExiting = Is;  }; //9
    //
    // get methods
    
    Int_t GetEventNumber()  { return fEvent;        }; //0
    Int_t GetTrackID()      { return fTrackID;      }; //1
    Int_t GetPdgCode()      { return fPdgCode;      }; //2
    Int_t GetMotherID()     { return fMotherID;     }; //3
    Int_t GetVolCopyNo()    { return fVolCopyNo;    }; //4
    Double_t GetVolLSD()    { return fVolLSD;       }; //5
    TVector3 GetMom()       { return fMom;          }; //6
    TVector3 GetPos()       { return fPos;          }; //7
    Bool_t IsEntering()     { return fIsEntering;   }; //8
    Bool_t IsExiting()      { return fIsExiting;    }; //9

  private:
    Int_t       fEvent;      //0 Event Number
    Int_t       fTrackID;    //1 Track Id
    Int_t       fPdgCode;    //2 Particle PDG Code
    Int_t       fMotherID;   //3 Particle mother ID (-1 = primary, 0 = secondary, etc..)
    Int_t       fVolCopyNo;  //4 Volume copy ID (to identify where the hit is)
    Double_t    fVolLSD;     //5 Volume Linear Scattering Density (1/Rad. Length)
    TVector3    fMom;        //6 Track momentum when releasing the hit
    TVector3    fPos;        //7 Hit coordinates (at the entrance of the detector)
    Double_t    fMomX;
    Double_t    fMomY;
    Double_t    fMomZ;
    Double_t    fPosX;
    Double_t    fPosY;
    Double_t    fPosZ;    
    Bool_t      fIsEntering; //8 IsEntering?
    Bool_t      fIsExiting;  //9 IsExiting?
    

    ClassDef(a2mcHit,1) //a2mcHit  
};

#endif //a2mc_HIT_H


