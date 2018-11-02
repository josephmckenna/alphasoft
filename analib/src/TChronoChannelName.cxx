#include "TChronoChannelName.h"

ClassImp(TChronoChannelName)

TChronoChannelName::TChronoChannelName(VirtualOdb* Odb, Int_t b, Int_t BoxIndex)
{
   SetBoardIndex(b+1);
   SetBoxIndex(BoxIndex);
   for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
      {
         TString OdbPath="/Equipment/cbms0";
         OdbPath+=b+1;
         OdbPath+="/Settings/ChannelNames";
         //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
         if (Odb->odbReadString(OdbPath.Data(),chan))
            SetChannelName(Odb->odbReadString(OdbPath.Data(),chan),chan);
      }
}

TChronoChannelName::TChronoChannelName()
{
   fChronoBoxIndex=-1;
   fChronoBoardIndex=-1;
   for (int i=0; i<CHRONO_N_CHANNELS; i++)
     Name[i]="";
}

TChronoChannelName::TChronoChannelName(TString json, Int_t b)
{
   TString ChannelJSON;
   int last;
   if (json.CountChar('.')==2)
      for (int i=0; i<2; i++) //Strip 2 sets of '.' (ie .boarno.json)
         {
            last=json.Last('.');
            json=json(0,last);
         }
   //Re-add extension for this module
   json+=".";
   json+=b;
   json+=".json";
   
   std::cout <<"Loading Channel name list from file: "<<json<<std::endl;
   std::ifstream file;
   file.open(json);
   if (!file)
   {
      std::cerr<<"TChronoChannelName cannot find json file: "<<json<<std::endl;
      exit(12);
   }
   ChannelJSON.ReadFile(file);
   file.close();
   std::cout <<ChannelJSON<<std::endl;
   TChronoChannelName* me=NULL;//new TChronoChannelName();
   TBufferJSON::FromJSON(me,ChannelJSON);
   for (int i=0; i<CHRONO_N_CHANNELS; i++)
      Name[i]=me->GetChannelName(i);
   fChronoBoxIndex=me->GetBoxIndex();
   fChronoBoardIndex=me->GetBoardIndex();
   //me->Print();
   delete me;
   //Print();
}

TChronoChannelName::~TChronoChannelName()
{
}

void TChronoChannelName::DumpToJson(int runno)
{
   TString json = TBufferJSON::ToJSON(this);
   std::ofstream stream;
   TString fname="chrono/R";
   fname+=runno;
   fname+=".";
   fname+=fChronoBoardIndex-1;
   fname+=".json";
   stream.open(fname);
   if( !stream )
      std::cout << "Opening file failed" << std::endl;
   // use operator<< for clarity
   stream << json<<std::endl;
   stream.close();
}

void TChronoChannelName::Print()
{

  std::cout<<"Box Index:\t"<<fChronoBoxIndex<<std::endl;;
  std::cout<<"Board Index:\t"<<fChronoBoardIndex<<std::endl;
  for (int i=0; i<CHRONO_N_CHANNELS; i++)
     std::cout<<i<<": "<<Name[i] <<std::endl;
}

Int_t TChronoChannelName::GetChannel(TString ChannelName, Bool_t exact_match)
{
   if (!exact_match)
   {
      for (int i=0; i<CHRONO_N_CHANNELS; i++)
      {
         //std::cout <<Name[i]<<std::endl;
         if (Name[i].BeginsWith(ChannelName)) return i;
      }
   }
   else
   {
      for (int i=0; i<CHRONO_N_CHANNELS; i++)
      {
         //std::cout <<Name[i]<<std::endl;
         if (Name[i].CompareTo(ChannelName)==0) return i;
      }
   }
   return -1;
}
