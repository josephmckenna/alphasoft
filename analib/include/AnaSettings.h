
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
   std::ifstream json_file;
   TString filename;
   json settings;
 public: 
   AnaSettings(const char* filename);
   virtual ~AnaSettings();
   bool HasVar(char* module, const char* var);
   
   double GetDouble(const char* Module, const char* Variable);
   int GetInt(const char* Module, const char* Variable);
   bool GetBool(const char* module, const char* var);
   std::string GetString(const char* Module, const char* Variable);
   virtual void Print();

   const json* GetSettings() const { return &settings; }
   TObjString GetSettingsString();
};

#endif
