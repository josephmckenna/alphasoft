///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include <iostream>
#include <limits>

#include "a2mcPrimaryVertex.h"

ClassImp(a2mcPrimaryVertex)

//_____________________________________________________________________________
a2mcPrimaryVertex::a2mcPrimaryVertex() {
/// Default constructor
    Reset();
}

//_____________________________________________________________________________
a2mcPrimaryVertex::~a2mcPrimaryVertex()
{
/// Destructor
}

//_____________________________________________________________________________
void a2mcPrimaryVertex::Reset()
{
    fPdgCode    = 0;
    fVx         = std::numeric_limits<double>::quiet_NaN();
    fVy         = std::numeric_limits<double>::quiet_NaN();
    fVz         = std::numeric_limits<double>::quiet_NaN();
    fPx         = std::numeric_limits<double>::quiet_NaN();
    fPy         = std::numeric_limits<double>::quiet_NaN();
    fPz         = std::numeric_limits<double>::quiet_NaN();
    fE          = std::numeric_limits<double>::quiet_NaN();
}

//_____________________________________________________________________________
void a2mcPrimaryVertex::Print(const Option_t* /*opt*/) const
{
/// Printing

  	std::cout << "  Primary particle PDG Code " << fPdgCode << std::endl; 
	std::cout << "  Vertex: (" << fVx << ", " << fVy << ", " << fVz << ") cm" << std::endl;
	std::cout << "  Mom: (" << fPx << ", " << fPy << ", " << fPz << ")  MeV/c" << std::endl;

}

