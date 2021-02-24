///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include <iostream>
#include <limits>
#include <cmath>

#include "a2mcPrimary.h"

ClassImp(a2mcPrimary)

//_____________________________________________________________________________
a2mcPrimary::a2mcPrimary() {
/// Default constructor
    Reset();
}

//_____________________________________________________________________________
a2mcPrimary::~a2mcPrimary()
{
/// Destructor
}

//_____________________________________________________________________________
void a2mcPrimary::Reset()
{
    fPdgCode    = 0;
    fVox         = std::numeric_limits<double>::quiet_NaN();
    fVoy         = std::numeric_limits<double>::quiet_NaN();
    fVoz         = std::numeric_limits<double>::quiet_NaN();
    fPox         = std::numeric_limits<double>::quiet_NaN();
    fPoy         = std::numeric_limits<double>::quiet_NaN();
    fPoz         = std::numeric_limits<double>::quiet_NaN();
    fEo          = std::numeric_limits<double>::quiet_NaN();
    fVdx         = std::numeric_limits<double>::quiet_NaN();
    fVdy         = std::numeric_limits<double>::quiet_NaN();
    fVdz         = std::numeric_limits<double>::quiet_NaN();
    fGenMode     = -1;
}

//_____________________________________________________________________________
void a2mcPrimary::Print(const Option_t* /*opt*/) const
{
/// Printing
  	std::cout << "  Primary particle PDG Code " << fPdgCode << std::endl; 
	std::cout << "  Origin: (" << fVox << ", " << fVoy << ", " << fVoz << ") cm" << std::endl;
	std::cout << "  Mom: (" << fPox << ", " << fPoy << ", " << fPoz << ")  MeV/c" << std::endl;
	if(!std::isnan(fVdx)) std::cout << "  Decay: (" << fVdx << ", " << fVdy << ", " << fVdz << ") cm" << std::endl;

}

