
#include "json.hpp"
#include <fstream>
#include <string>
#include "TString.h"
#include "TObjString.h"

#ifndef _AnaSettings_
#define _AnaSettings_

using json = nlohmann::json;
class AnaSettings
{
 private:

   TString filename;
   json settings;
 public: 
   AnaSettings(const char* filename);
   virtual ~AnaSettings();
   std::string removeComments(std::string prgm); //Convert hjson to json in memory
   
   bool HasVar(char* module, const char* var) const;
   
   double GetDouble(const char* Module, const char* Variable) const;
   int GetInt(const char* Module, const char* Variable) const;
   bool GetBool(const char* module, const char* var) const;
   std::string GetString(const char* Module, const char* Variable) const;
   virtual void Print();

   const json* GetSettings() const { return &settings; }
   TObjString GetSettingsString();

   TString GetFilename() { return filename; }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
