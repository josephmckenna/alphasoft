#include "TChronoChannelName.h"

ClassImp(TChronoChannelName)

#ifdef INCLUDE_VirtualOdb_H
//Old manalyzer uses VirtualODB (before Jan 2020)
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
#endif
#ifdef INCLUDE_MVODB_H
//New manalyzer uses VirtualODB (after Jan 2020)
TChronoChannelName::TChronoChannelName(MVOdb* Odb, Int_t b, Int_t BoxIndex)
{
   SetBoardIndex(b+1);
   SetBoxIndex(BoxIndex);
   for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
      {
         TString OdbPath="/Equipment/cbms0";
         OdbPath+=b+1;
         OdbPath+="/Settings/ChannelNames";
         //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
         //if (Odb->odbReadString(OdbPath.Data(),chan))
         std::string tmp;
         Odb->RSAI(OdbPath.Data(),chan,&tmp);
         SetChannelName(tmp.c_str(),chan);
      }
}
#endif
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
   
   #if ROOT_VERSION_CODE > ROOT_VERSION(6,14,0)
   TBufferJSON::FromJSON(me,ChannelJSON);
   #else
   std::cout <<"Please update to atleast root 6.14 so that TBufferJSON is supported"<<std::endl;
   exit(1);
   #endif
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
   #if ROOT_VERSION_CODE > ROOT_VERSION(6,14,0)
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
   #else
   std::cout <<"Please update to atleast root 6.14 so that TBufferJSON is supported"<<std::endl;
   exit(1);
   #endif
   
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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
