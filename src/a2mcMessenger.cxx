///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcMessenger.h"

ClassImp(a2mcMessenger)
a2mcMessenger* a2mcMessenger::fgInstance = 0;

//_____________________________________________________________________________
a2mcMessenger::a2mcMessenger()
{
    // Default constuctor
    if (fgInstance) {
      Fatal("a2mcMessenger", "Singleton instance already exists.");
      return;
    }  

    fgInstance = this;

}

//_____________________________________________________________________________
a2mcMessenger::~a2mcMessenger()
{
    /// Destructor
  fgInstance = 0;

}

//
// static methods
//_____________________________________________________________________________
a2mcMessenger* a2mcMessenger::Instance()
{
/// \return The singleton instance.

  return fgInstance;
}  

void a2mcMessenger::TitleFrame(const std::string &s) {
    Int_t l = s.length();
    Int_t w = 40;
    std::cout << std::left << std::setw(w) << std::setfill('-') << "|||" << std::right << std::setw(w) << std::setfill('-') << "|||" << std::endl; 
    
    std::cout << std::left << std::setw(w-l) << std::setfill(' ') << "|||" << s.c_str() << std::right << std::setw(w) << std::setfill(' ') << "|||" << std::endl; 

    std::cout << std::left << std::setw(w) << std::setfill('-') << "|||" << std::right << std::setw(w) << std::setfill('-') << "|||" << std::endl;     
}

void a2mcMessenger::toRight(UInt_t w) {
    std::cout << std::right << std::setw(w) << std::setfill(' ') << "| "; 
}

void a2mcMessenger::printLeft(UInt_t w, const std::string &s) {
    std::cout << std::left << std::setw(w) << std::setfill(' ') << s.c_str() << std::right << std::setw(1) << std::setfill(' ') << "| "; 
}
