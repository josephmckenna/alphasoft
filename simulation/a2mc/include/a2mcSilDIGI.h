#ifndef a2mc_SilDIGI_H
#define a2mc_SilDIGI_H

#include <iostream>
#include <TObject.h>

class a2mcSilDIGI : public TObject
{
    public:
        a2mcSilDIGI();
        virtual ~a2mcSilDIGI();

        // -------> PUBLIC FUNCTIONS
        virtual void Print(const Option_t* option = "") const;

        // -------> SET METHODS

        // Set element ID 
        void SetElemID(Int_t id)   { fElemID = id; };  

        // Set energy 
        void SetEnergy(Double_t e)  { fEnergy = e; };

        // -------> GET METHODS

        // Get element ID
        Int_t GetElemID() { return fElemID; };

        // Get energy
        Double_t GetEnergy()   	{ return fEnergy; };


        // -------> PRIVATE VARIABLES
    private:
        // For the Silillators: Element ID = Silillator number
        Int_t      fElemID;     // Element ID 
        Double_t   fEnergy;     // Energy released in the element

        ClassDef(a2mcSilDIGI,1) //a2mcSilDIGI  
};

#endif //a2mcSilDIGI_H


