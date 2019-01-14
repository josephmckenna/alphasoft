#include "AnaSettings.h"

AnaSettings::AnaSettings(const char* name)
{
//using json = nlohmann::json;
//std::ifstream file("Config.json");
//json object(file);
   if (strcmp(name,"default")==0)
     filename=TString::Format("%s/ana/ana_settings.json",getenv("AGRELEASE"));
   else
     filename=name;
   //   std::cout<<"AnaSettings::AnaSettings Configuration file:"<<filename<<std::endl;
   json_file.open(filename.Data(), std::ifstream::binary);
   json_file>>settings;
}

AnaSettings::~AnaSettings()
{
   json_file.close();
}

//Test function
bool AnaSettings::HasVar(char* module, const char* var)
{
   std::cout<<"hi"<<std::endl;
   std::cout<<settings.at(module)<<std::endl;
   std::cout<<settings.at(module).at(var)<<std::endl;
   return false;
}

double AnaSettings::GetDouble(const char* module, const char* var)
{
  return double(settings.at(module).at(var));
}   

int AnaSettings::GetInt(const char* mod, const char* var)
{
  return int(settings.at(mod).at(var));
}

bool AnaSettings::GetBool(const char* module, const char* var)
{
  return bool(settings.at(module).at(var));
}

std::string AnaSettings::GetString(const char* mod, const char* var)
{
  std::string s = settings.at(mod).at(var);
  return s;
}

void AnaSettings::Print()
{
   std::cout<<"JSON Settings:"<< filename<<std::endl;
   std::cout<<settings<<std::endl;
}
