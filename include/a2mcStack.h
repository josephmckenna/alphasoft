#ifndef a2mc_STACK_H
#define a2mc_STACK_H

#include <stack>


#include <TVirtualMCStack.h>
#include <TClonesArray.h>

#include "a2mcMCTrack.h"

class TParticle;
class TClonesArray;
class a2mcMCTrack;

/// \ingroup E03
/// \ingroup E06
/// \brief Implementation of the TVirtualMCStack interface
///
/// (Taken from the E03 example)
///
/// \date 16/05/2005
/// \author I. Hrivnacova; IPN, Orsay

class a2mcStack : public TVirtualMCStack
{
  public:
    a2mcStack(Int_t size);
    a2mcStack();
    virtual ~a2mcStack();     

    // methods
    static a2mcStack* Instance(); 
    virtual void  PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
  	              Double_t px, Double_t py, Double_t pz, Double_t e,
  		      Double_t vx, Double_t vy, Double_t vz, Double_t tof,
		      Double_t polx, Double_t poly, Double_t polz,
		      TMCProcess mech, Int_t& ntr, Double_t weight,
		      Int_t is) ;
    virtual TParticle* PopNextTrack(Int_t& track);
    virtual TParticle* PopPrimaryForTracking(Int_t i); 
    virtual void Print(Option_t* option = "") const;   
    virtual void RegisterMCTrack();
    void Reset();   
   
    // set methods
    virtual void  SetCurrentTrack(Int_t track);                           

    // get methods
    virtual Int_t  GetNtrack() const;
    virtual Int_t  GetNprimary() const;
    virtual TParticle* GetCurrentTrack() const;   
    virtual Int_t  GetCurrentTrackNumber() const;
    virtual Int_t  GetCurrentParentTrackNumber() const;
    TParticle*     GetParticle(Int_t id) const;
    virtual a2mcMCTrack*  GetTrack(Int_t id) const;
    
  private:
    // data members
    static  a2mcStack* fgInstance; ///< Singleton instance
    std::stack<TParticle*>  fStack;       //!< The stack of particles (transient)
    TClonesArray*           fParticles;   ///< The array of particle (persistent)
    TClonesArray*           fTracks;      ///< The array of tracks (persistent)
    Int_t                   fTracksLim;   ///< Limit the tracks to be stored in the stack
    Int_t                   fCurrentTrack;///< The current track number
    Int_t                   fNPrimary;    ///< The number of primaries

    ClassDef(a2mcStack,1) // a2mcStack
};

#endif //a2mc_STACK_H   
   

