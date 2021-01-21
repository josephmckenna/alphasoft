#include "TSpill.h"

ClassImp(TSpillScalerData);
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
TSpillScalerData::TSpillScalerData(int n_scaler_channels):DetectorCounts(n_scaler_channels,0),ScalerFilled(n_scaler_channels,0)
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
TSpillScalerData::TSpillScalerData(TSpillScalerData* a)
{
   StartTime         =a->StartTime;
   StopTime          =a->StopTime;
   DetectorCounts    =a->DetectorCounts;
   ScalerFilled      =a->ScalerFilled;
   
   FirstVertexEvent  =a->FirstVertexEvent;
   LastVertexEvent   =a->LastVertexEvent;
   VertexEvents      =a->VertexEvents;
   Verticies         =a->Verticies;
   PassCuts          =a->PassCuts;
   PassMVA           =a->PassMVA;
   VertexFilled      =a->VertexFilled;
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


ClassImp(TSpillSequencerData);


TSpillSequencerData::TSpillSequencerData()
{
   fSequenceNum=-1;
   fDumpID     =-1;
   fSeqName    ="";
   fStartState =-1;
   fStopState  =-1;
}
TSpillSequencerData::TSpillSequencerData(TSpillSequencerData* a)
{
   fSequenceNum  =a->fSequenceNum;
   fDumpID       =a->fDumpID;
   fSeqName      =a->fSeqName;
   fStartState   =a->fStartState;
   fStopState    =a->fStopState;
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


ClassImp(TSpill);
TSpill::TSpill(): RunNumber(-1)
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   Unixtime   =0;
}

bool TSpill::IsMatchForDumpName(std::string dumpname, bool exact)
{
   //Compare but ignore leading '"' mark
   if (Name.compare(1,dumpname.size(),dumpname.c_str())==0)
   {
      //If we need an exact match... make sure lengths match
      if (exact)
      {
         return Name.size() - 2 == dumpname.size();
      }
      else
      {
         return true;
      }
   }
   return false;
}

TSpill::TSpill(int runno): RunNumber(runno)
{
   IsDumpType =true; //By default, expect this to be a dump
   IsInfoType =false;
   Unixtime   =0;
}
TSpill::TSpill(int runno, const char* format, ...): RunNumber(runno)
{
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
   Unixtime   =0;
   IsDumpType =false; //By default, expect this to be a information if given a string at construction
   IsInfoType =false;
   std::cout<<"NewSpill:";
   Print();
}

TSpill::TSpill(TSpill* a): RunNumber(a->RunNumber)
{
   Name         =a->Name;
   IsDumpType   =a->IsDumpType;
   Unixtime     =a->Unixtime;
}

TSpill* TSpill::operator/(const TSpill* b)
{
   //Joe you need to set this up
   TSpill* a = new TSpill();
   return a;
}

void TSpill::Print()
{
   std::cout<<"RunNumber:"<<RunNumber<<"\t"
            <<"Name:"<<Name<<"\t"
            <<std::endl;
}

int TSpill::AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt)
{
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

