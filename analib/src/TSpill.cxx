#include "TSpill.h"

ClassImp(TSpillScalerData)
TSpillScalerData::TSpillScalerData()
{
   StartTime         =-1;
   StopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   Verticies         =0;
   PassCuts          =0;
   PassMVA           =0;
   VertexFilled      =false;
}

TSpillScalerData::~TSpillScalerData()
{

}

TSpillScalerData::TSpillScalerData(int n_scaler_channels) : DetectorCounts(n_scaler_channels,0),ScalerFilled(n_scaler_channels,0)
{
   StartTime         =-1;
   StopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   Verticies         =0;
   PassCuts          =0;
   PassMVA           =0;
   VertexFilled      =false;
}

TSpillScalerData::TSpillScalerData(const TSpillScalerData& a) : TObject(a)
{
   StartTime         =a.StartTime;
   StopTime          =a.StopTime;
   DetectorCounts    =a.DetectorCounts;
   ScalerFilled      =a.ScalerFilled;
   
   FirstVertexEvent  =a.FirstVertexEvent;
   LastVertexEvent   =a.LastVertexEvent;
   VertexEvents      =a.VertexEvents;
   Verticies         =a.Verticies;
   PassCuts          =a.PassCuts;
   PassMVA           =a.PassMVA;
   VertexFilled      =a.VertexFilled;
}

TSpillScalerData& TSpillScalerData::operator=(const TSpillScalerData& rhs)
{
   if (this == &rhs)
      return *this;
   StartTime         =rhs.StartTime;
   StopTime          =rhs.StopTime;
   DetectorCounts    =rhs.DetectorCounts;
   ScalerFilled      =rhs.ScalerFilled;
   
   FirstVertexEvent  =rhs.FirstVertexEvent;
   LastVertexEvent   =rhs.LastVertexEvent;
   VertexEvents      =rhs.VertexEvents;
   Verticies         =rhs.Verticies;
   PassCuts          =rhs.PassCuts;
   PassMVA           =rhs.PassMVA;
   VertexFilled      =rhs.VertexFilled;
   return *this;
}


bool TSpillScalerData::Ready(bool have_vertex_detector)
{
   bool SomeScalerFilled=false;
   for (auto chan: ScalerFilled)
   {
      if (chan)
      {
         SomeScalerFilled=true;
         break;
      }
   }
   if (StartTime>0 &&
        StopTime>0 &&
        //Some SIS channels filled test
       SomeScalerFilled )
   {  
      if ( VertexFilled || !have_vertex_detector)
         return true;
      //If there is no SVD data, then SVDFilled is false... 
      //wait for the SIS data to be atleast 'data_buffer_time' seconds
      //ahead to check there is no SVD data...
      //if ( !SVDFilled && T>StopTime+data_buffer_time )
      //   return true;
      return false;
   }
   else
   {
      return false;
   }
}


ClassImp(TSpillSequencerData)


TSpillSequencerData::TSpillSequencerData()
{
   fSequenceNum=-1;
   fDumpID     =-1;
   fSeqName    ="";
   fStartState =-1;
   fStopState  =-1;
}
TSpillSequencerData::TSpillSequencerData(const TSpillSequencerData& a) : TObject(a)
{
   fSequenceNum  =a.fSequenceNum;
   fDumpID       =a.fDumpID;
   fSeqName      =a.fSeqName;
   fStartState   =a.fStartState;
   fStopState    =a.fStopState;
}


TSpillSequencerData::~TSpillSequencerData()
{
}

void TSpillSequencerData::Print()
{
   std::cout<<"SeqName:"       <<fSeqName
            <<"\tSeq:"         <<fSequenceNum
            <<"\tDumpID:"      <<fDumpID
            <<"\tstartState:"  <<fStartState
            <<"\tstopState:"   <<fStopState
            <<std::endl;
}


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
