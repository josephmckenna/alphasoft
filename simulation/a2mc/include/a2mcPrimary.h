#ifndef a2mc_PRIM_H
#define a2mc_PRIM_H

#include <TObject.h>

class a2mcPrimary : public TObject
{
    public:
        a2mcPrimary();
        virtual ~a2mcPrimary();

        // -------> PUBLIC FUNCTIONS
        virtual void Print(const Option_t* option = "") const;

        // -------> SET METHODS

		void SetPdgCode(Int_t code) { fPdgCode = code; };

        void SetVox(Double_t v) { fVox = v; };
        void SetVoy(Double_t v) { fVoy = v; };
        void SetVoz(Double_t v) { fVoz = v; };

        void SetPox(Double_t v) { fPox = v; };  
        void SetPoy(Double_t v) { fPoy = v; };  
        void SetPoz(Double_t v) { fPoz = v; };  

        void SetEo(Double_t v)  { fEo = v; };
        
        void SetVdx(Double_t v) { fVdx = v; };  
        void SetVdy(Double_t v) { fVdy = v; };  
        void SetVdz(Double_t v) { fVdz = v; };  

		void SetGenMode(Int_t mode) { fGenMode = mode; };

void Reset();

    private:
        // -------> PRIVATE VARIABLES
		Int_t          fPdgCode;              // PDG code of the particle
///<    Origin vertex
  		Double_t       fVox;                   // x of origin vertex
  		Double_t       fVoy;                   // y of origin vertex
  		Double_t       fVoz;                   // z of origin vertex
 		Double_t       fPox;                   // x component of momentum
  		Double_t       fPoy;                   // y component of momentum
  		Double_t       fPoz;                   // z component of momentum
  		Double_t       fEo;                    // Energy
///<    Decay vertex
  		Double_t       fVdx;                   // x of decay vertex
  		Double_t       fVdy;                   // y of decay vertex
  		Double_t       fVdz;                   // z of decay vertex
        Int_t          fGenMode;               // Type of generation (e.g. for muons, over a flat sky or a sphere)

        ClassDef(a2mcPrimary,1) //a2mcPrimary  
};

#endif //a2mc_PRIM_H


