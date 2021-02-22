#ifndef a2mc_CONST_FIELD_H
#define a2mc_CONST_FIELD_H

#include <TVirtualMagField.h>

class a2mcFieldConstant : public TVirtualMagField
{
public:
    a2mcFieldConstant(Double_t Bx, Double_t By, Double_t Bz, Double_t Brmax, Double_t Bzmin, Double_t Bzmax); 
    a2mcFieldConstant();
    virtual ~a2mcFieldConstant();
    
    virtual void Field(const Double_t* x, Double_t* B);
   
private:
    a2mcFieldConstant(const a2mcFieldConstant&);
    a2mcFieldConstant& operator=(const a2mcFieldConstant&);
    
    Double_t  fB[3]; ///< Magnetic field vector
    /** Field limits in local coordinate system **/
    Double_t fRmax;
    Double_t fZmin, fZmax;

    ClassDef(a2mcFieldConstant, 1)  // Uniform magnetic field        
};

#endif //a2mc_CONST_FIELD_H
