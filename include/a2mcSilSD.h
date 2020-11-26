#ifndef a2mcSilSD_H
#define a2mcSilSD_H

#include <TNamed.h>
#include <TClonesArray.h>
#include <TParticle.h>

#include "a2mcSilHit.h"
#include "a2mcSilDIGI.h"
#include "a2mcApparatus.h"

class a2mcSilHit;
class a2mcSilDIGI;
class a2mcApparatus;

class a2mcSilSD : public TNamed
{
    public:
        a2mcSilSD(const char* name);
        a2mcSilSD();
        virtual ~a2mcSilSD();

       // -------> PUBLIC FUNCTIONS
        void    Initialize();
        void    Register();
        void    BeginOfEvent();
        Bool_t  ProcessHits();
        void    Digitalize();
        void    EndOfEvent();
        virtual void  Print(const Option_t* option = 0) const;

        // -------> SET METHODS
        void SetVerboseLevel(Int_t level);

    private:
        // -------> PRIVATE FUNCTIONS
        a2mcSilHit* AddHit();             // Add the hit to the collection
        a2mcSilHit* GetHit(Int_t);        // Get the hit from collection

        a2mcSilDIGI* AddDIGI();      // Add the digit to the collection
        a2mcSilDIGI* GetDIGI(Int_t); // Get the digit from the DIGI collection

        Int_t GetHitCollectionSize();   // Get the hits collection size
        Int_t GetDIGICollectionSize();  // Get the DIGI collection size


        // -------> PRIVATE VARIABLES
        // REMEMBER TO CHANGE SIZE IF MORE SilS
        TClonesArray*  fHitCollection;   //  Hits collection    
        TClonesArray*  fDIGICollection;  //  Digits collection
        std::vector<Int_t>  fSensitiveID;
        Int_t          fVerboseLevel;    // Verbosity level

        ClassDef(a2mcSilSD,1) //a2mcSilSD 

};

/// Set verbose level
/// \param level The new verbose level value
inline void a2mcSilSD::SetVerboseLevel(Int_t level) 
{ fVerboseLevel = level; }


#endif //a2mcSilSD_H

