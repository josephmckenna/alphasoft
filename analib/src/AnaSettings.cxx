#include "AnaSettings.h"

AnaSettings::AnaSettings(const char* name)
{
//using json = nlohmann::json;
//std::ifstream file("Config.json");
//json object(file);
   if (strcmp(name,"default")==0)
      name="ana/ana_settings.json";

   filename=name;
   json_file.open(name, std::ifstream::binary);
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
   return settings.at(module).at(var);
}

bool AnaSettings::GetBool(const char* module, const char* var)
{
   return settings.at(module).at(var);
}

void AnaSettings::Print()
{
   std::cout<<"JSON Settings:"<< filename<<std::endl;
   std::cout<<settings<<std::endl;
}
