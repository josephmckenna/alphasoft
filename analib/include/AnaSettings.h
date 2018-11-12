
#include "json.hpp"
#include <fstream>
#include "TString.h"


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
   TString GetString(const char* Module, const char* Variable);
   virtual void Print();
};

#endif
