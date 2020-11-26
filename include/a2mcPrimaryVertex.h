#ifndef a2mc_PVTX_H
#define a2mc_PVTX_H

#include <TObject.h>

class a2mcPrimaryVertex : public TObject
{
    public:
        a2mcPrimaryVertex();
        virtual ~a2mcPrimaryVertex();

        // -------> PUBLIC FUNCTIONS
        virtual void Print(const Option_t* option = "") const;

        // -------> SET METHODS

		void SetPdgCode(Int_t code) { fPdgCode = code; };

        void SetVx(Double_t Vx)   { fVx = Vx; };  
        void SetVy(Double_t Vy)   { fVy = Vy; };  
        void SetVz(Double_t Vz)   { fVz = Vz; };  

        void SetPx(Double_t Px)   { fPx = Px; };  
        void SetPy(Double_t Py)   { fPy = Py; };  
        void SetPz(Double_t Pz)   { fPz = Pz; };  

        void SetE(Double_t E)  { fE = E; };

        void Reset();
    private:
        // -------> PRIVATE VARIABLES
		Int_t          fPdgCode;              // PDG code of the particle

  		Double_t       fVx;                   // x of production vertex
  		Double_t       fVy;                   // y of production vertex
  		Double_t       fVz;                   // z of production vertex

 		Double_t       fPx;                   // x component of momentum
  		Double_t       fPy;                   // y component of momentum
  		Double_t       fPz;                   // z component of momentum
  		Double_t       fE;                    // Energy


        ClassDef(a2mcPrimaryVertex,1) //a2mcPrimaryVertex  
};

#endif //a2mc_PVTX_H


