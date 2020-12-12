///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcFieldConstant.h"

ClassImp(a2mcFieldConstant)

//______________________________________________________________________________
a2mcFieldConstant::a2mcFieldConstant(Double_t Bx, Double_t By, Double_t Bz, Double_t Brmax, Double_t Bzmin, Double_t Bzmax)
  : TVirtualMagField("a2mc magnetic field")
{
/// Standard constructor
/// \param Bx   The x component of the field value (in kiloGauss)
/// \param By   The y component of the field value (in kiloGauss)
/// \param Bz   The z component of the field value (in kiloGauss)

    fB[0] = Bx;
    fB[1] = By;
    fB[2] = Bz;
    fRmax = Brmax;
    fZmin = Bzmin;
    fZmax = Bzmax;
}

//______________________________________________________________________________
a2mcFieldConstant::a2mcFieldConstant()
  : TVirtualMagField()
{
/// Default constructor
      fB[0] = 0.;
      fB[1] = 0.;
      fB[2] = 0.;
}

//______________________________________________________________________________
a2mcFieldConstant::~a2mcFieldConstant()
{
/// Destructor
}


//______________________________________________________________________________
void a2mcFieldConstant::Field(const Double_t* x, Double_t* B) 
{
///< Fill in the field value B in the given position at x.
///< (In case of a uniform magnetic field the value B does not depend on 
///< the position x ) 
///< x   The position
///< B   the field value (in kiloGauss) 

    if(sqrt(pow(x[0],2.)+pow(x[1],2.))<fRmax) {
        B[0] = fB[0]; 
        B[1] = fB[1]; 
    } else {
        B[0] = 0.;
        B[1] = 0.;
    }
    if(fZmin<x[2]&&x[2]<fZmax) {B[2] = fB[2];} else { B[2] = 0.;} 
}
