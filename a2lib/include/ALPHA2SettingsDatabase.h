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
   static TSettings* GetTSettings()
   {
      char dbName[255]; 
#ifdef ALPHASOFT_DB_INSTALL_PATH
      sprintf(dbName,"%s/main.db",ALPHASOFT_DB_INSTALL_PATH);
#else
      sprintf(dbName,"%s/a2lib/main.db",getenv ("AGRELEASE"));
#endif
      return new TSettings(dbName); 
   }

   static TSettings* GetTSettings(int runno)
   {
      char dbName[255]; 
#ifdef ALPHASOFT_DB_INSTALL_PATH
      sprintf(dbName,"%s/main.db",ALPHASOFT_DB_INSTALL_PATH);
#else
      sprintf(dbName,"%s/a2lib/main.db",getenv ("AGRELEASE"));
#endif
      return new TSettings(dbName,runno); 
   }
}


#endif