#include "TSpill.h"


ClassImp(TSpill)
TSpill::TSpill(): RunNumber(-1)
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   Unixtime   =0;
}

// The first string may contain wildcard characters : * or ?
//https://www.geeksforgeeks.org/wildcard-character-matching/
bool TSpill::MatchWithWildCards(const char *first, const char * second) 
{ 
    // If we reach at the end of both strings, we are done 
    if (*first == '\0' && *second == '\0') 
        return true; 
  
    // Make sure that the characters after '*' are present 
    // in second string. This function assumes that the first 
    // string will not contain two consecutive '*' 
    if (*first == '*' && *(first+1) != '\0' && *second == '\0') 
        return false; 
  
    // If the first string contains '?', or current characters 
    // of both strings match 
    if (*first == '?' || *first == *second) 
        return MatchWithWildCards(first+1, second+1); 
  
    // If there is *, then there are two possibilities 
    // a) We consider current character of second string 
    // b) We ignore current character of second string. 
    if (*first == '*') 
        return MatchWithWildCards(first+1, second) || MatchWithWildCards(first, second+1); 
    return false; 
} 

//Always an exact match unless wild cards are used
bool TSpill::IsMatchForDumpName(const std::string& dumpname)
{
   std::string QuoteMarkFreeName;
   if (Name[0]=='"' && Name[Name.size()-1]=='"')
      QuoteMarkFreeName=Name.substr(1,Name.size()-2);
   else
      QuoteMarkFreeName=Name;
   return MatchWithWildCards(dumpname.c_str(),QuoteMarkFreeName.c_str());
}

TSpill::TSpill(int runno, uint32_t unixtime): RunNumber(runno)
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   Unixtime   =unixtime;
}
TSpill::TSpill(int runno, uint32_t unixtime, const char* format, ...): RunNumber(runno)
{
   Unixtime = unixtime;
   va_list args;
   va_start(args,format);
   InitByName(format,args);
   va_end(args);
}
void TSpill::InitByName(const char* format, va_list args)
{
   char buf[256];
   vsnprintf(buf,255,format,args);
   Name       =buf;
   IsDumpType =false; //By default, expect this to be a information if given a string at construction
   IsInfoType =false;
   std::cout<<"NewSpill:";
   Print();
}

TSpill::TSpill(const TSpill& a) : TObject(a)
{
   RunNumber    =a.RunNumber;
   Name         =a.Name;
   IsDumpType   =a.IsDumpType;
   IsInfoType   =a.IsInfoType;
   Unixtime     =a.Unixtime;
}

TSpill& TSpill::operator=(const TSpill& rhs)
{
   if (this == &rhs)
      return *this;
   RunNumber    =rhs.RunNumber;
   Name         =rhs.Name;
   IsDumpType   =rhs.IsDumpType;
   IsInfoType   =rhs.IsInfoType;
   Unixtime     =rhs.Unixtime;
   return *this;
}

void TSpill::Print()
{
   std::cout<<"RunNumber:"<<RunNumber<<"\t"
            <<"Name:"<<Name<<"\t"
            <<std::endl;
}

int TSpill::AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt)
{
   assert(!"Implement me!");
   db = db;
   stmt = stmt;
   return -1;
}

TString TSpill::Content(std::vector<int>*, int& )
{
   return Name;
}

bool TSpill::DumpHasMathSymbol() const
{
   char symbols[5]="+/*-";
   for (const char c: this->Name)
   {
      for (int i=0; i<4; i++)
      {
         //std::cout<<c<<"=="<<symbols[i]<<std::endl;
         if (c==symbols[i])
            return true;
      }
   }
   return false;
}

TSpill::~TSpill()
{

}


std::string TSpill::ContentCSVTitle() const
{
   std::string title = "RunNumber, Sequencer Start Time (Unix Time), Dump Name, Is Dump Type, Is Info Type,";
   return title;
}
std::string TSpill::ContentCSV() const
{
   std::string line = std::to_string(RunNumber) + "," +
                      std::to_string(Unixtime) + "," +
                      Name + ",";
                      if (IsDumpType)
                         line +="1,";
                      else
                         line +="0,";
                      if (IsInfoType)
                         line +="1,";
                      else
                         line +="0,";
   return line;
}