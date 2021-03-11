#include "ALPHA2SettingsDatabase.h"

namespace ALPHA2SettingsDatabase
{
   TSettings* GetTSettings()
   {
      char dbName[255]; 
#ifdef ALPHASOFT_DB_INSTALL_PATH
      sprintf(dbName,"%s/main.db",ALPHASOFT_DB_INSTALL_PATH);
#else
      sprintf(dbName,"%s/a2lib/main.db",getenv ("AGRELEASE"));
#endif
      return new TSettings(dbName); 
   }

   TSettings* GetTSettings(int runno)
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

