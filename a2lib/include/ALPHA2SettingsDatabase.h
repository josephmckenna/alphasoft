#ifndef _ALPHA2SettingsDatabase_
#define _ALPHA2SettingsDatabase_
#include "TSettings.h"
#include <stdio.h>

// CMake build of software knows where 
// it will install the database, so can hardcode 
// its path... (no need for AGRELEASE var)... 
// ...perhaps we want relative directories in the future?
//ALPHASOFT_DB_INSTALL_PATH

namespace ALPHA2SettingsDatabase
{
   TSettings* GetTSettings();
   TSettings* GetTSettings(int runno);
}


#endif
