
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TError.h"
#include <iostream>

#ifndef _FileGetters_
#define _FileGetters_

void SetFileNamePatter(const std::string pattern = "output%5d.root");
std::string GetFileNamePattern();

TFile* Get_File(Int_t run_number, Bool_t die=kFALSE);



#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
