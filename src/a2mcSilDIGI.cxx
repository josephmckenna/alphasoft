///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcSilDIGI.h"

ClassImp(a2mcSilDIGI)

using namespace std;

//_____________________________________________________________________________
a2mcSilDIGI::a2mcSilDIGI() 
  : fElemID(-1),
    fEnergy(0.)
{
/// Default constructor
}

//_____________________________________________________________________________
a2mcSilDIGI::~a2mcSilDIGI() 
{
/// Destructor
}
//_____________________________________________________________________________
void a2mcSilDIGI::Print(const Option_t* /*opt*/) const
{
/// Printing

  cout << "  SilDet number: " << fElemID 
       << "  SilDet released energy:  " << fEnergy
       << endl;
}

