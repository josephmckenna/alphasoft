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
        void SetEnePMT1(Double_t e)  { fEnePMT1 = e; };
        void SetEnePMT2(Double_t e)  { fEnePMT2 = e; };

        // -------> GET METHODS

        // Get element ID
        Int_t GetElemID() { return fElemID; };

        // Get energy
        Double_t GetEnergy()   	{ return fEnergy; };
        Double_t GetEnePMT1()   { return fEnePMT1; };
        Double_t GetEnePMT2()   { return fEnePMT2; };


        // -------> PRIVATE VARIABLES
    private:
        // For the Silillators: Element ID = Silillator number
        Int_t      fElemID;     // Element ID 
        Double_t   fEnergy;     // Energy released in the element
        Double_t   fEnePMT1;    // Energy released and collected at one side (PMT1)
        Double_t   fEnePMT2;    // Energy released and collected on the other side (PMT2)

        ClassDef(a2mcSilDIGI,1) //a2mcSilDIGI  
};

#endif //a2mcSilDIGI_H


