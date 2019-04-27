#include "AnaSettings.h"

AnaSettings::AnaSettings(const char* name)
{
//using json = nlohmann::json;
//std::ifstream file("Config.json");
//json object(file);
   if (strcmp(name,"default")==0)
     filename=TString::Format("%s/ana/ana_settings.hjson",getenv("AGRELEASE"));
   else
     filename=name;
   //   std::cout<<"AnaSettings::AnaSettings Configuration file:"<<filename<<std::endl;
   std::ifstream json_file(filename.Data());

   
   std::stringstream strStream;
   strStream << json_file.rdbuf(); //read the file
   std::string str = strStream.str(); //str holds the content of the file

   if (filename.EndsWith(".hjson"))
   {
      std::cout<<"Trying to convert hjson to json on the fly..."<<std::endl;
      str=removeComments(str);
   }
   settings =json::parse(str);
   std::cout<<"Json parsing success!"<<std::endl;
   json_file.close();
}



//Modified from https://www.geeksforgeeks.org/remove-comments-given-cc-program/
std::string AnaSettings::removeComments(std::string prgm) 
{ 
   int n = prgm.length(); 
   std::string res; 
  
   // Flags to indicate that single line and multpile line comments 
   // have started or not. 
   bool s_cmt = false; 
   bool m_cmt = false; 

   bool quote_open = false; //Track quote open or close
   bool new_line = false;
   //bool colon_counter = false;
   bool blank_line = true;
   //char last_char=0;

   // Traverse the given program 
   for (int i=0; i<n; i++) 
   {

      /* Look for sections to comment out */
      // If single line comment flag is on, then check for end of it 
      if (s_cmt == true && ((prgm[i] == '\n') || (prgm[i] == '\r')) ) 
         s_cmt = false; 
      // If multiple line comment is on, then check for end of it 
      if  (m_cmt == true && prgm[i] == '*' && prgm[i+1] == '/') 
         m_cmt = false,  i++; 
      // If this character is in a comment, ignore it 
      if (s_cmt || m_cmt) 
         continue; 

      //Remove white space
      if (prgm[i] == ' ') continue;
      
      if (blank_line && isgraph(prgm[i]))
         blank_line=false;
      //Remove blank lines
      if (blank_line && new_line && prgm[i] == '\n') continue;
      if (blank_line && new_line && prgm[i] == '\r') continue;
      //I am a new line, reset bools, move to next character
      if (((prgm[i] == '\n') || (prgm[i] == '\r')) && !(s_cmt || m_cmt)  ) 
      {
         //colon_counter = false;
         new_line = true;
         blank_line= true;
      }
      /*//A colon has been written in the line
      if (prgm[i] == ':' )
      {
         colon_counter = true;
      }*/
      // Check for beginning of comments and set the approproate flags 
      if (prgm[i] == '/' && prgm[i+1] == '/') 
      {
         s_cmt = true;//, i++;
         continue; 
      }
      if (prgm[i] == '/' && prgm[i+1] == '*') 
      {
         m_cmt = true,  i++; 
         continue;
      }

      //Place quote marks around variables
      if (new_line && std::isgraph(prgm[i] ))
      {
         new_line=false;
         if (std::isalpha(prgm[i]))
         {
            quote_open=true;
            if (res.back()!='{')
              res+=",";
            res+='\n';
            res+='"';
         }
      }
      if (prgm[i] == ':' || prgm[i] == ' ')
         if (quote_open)
         {
            res+='"';
            quote_open=false;
         }

        //if (std::isgraph(prgm[i]))
        //   last_char=prgm[i];
        // If current character is a non-comment character, append it to res 
        if (prgm[i]!='\n')
           res += prgm[i]; 
        

    } 
    
    return res; 
} 

AnaSettings::~AnaSettings()
{}

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

TObjString AnaSettings::GetSettingsString()
{
  std::stringstream ss;
  ss<<settings<<std::endl;
  TObjString sobj(ss.str().c_str());
  return sobj;
}
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
