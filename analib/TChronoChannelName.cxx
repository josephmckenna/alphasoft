#include "TChronoChannelName.h"

ClassImp(TChronoChannelName)

TChronoChannelName::TChronoChannelName(VirtualOdb* Odb, Int_t b)
{
   SetBoardIndex(b+1);
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

TChronoChannelName::~TChronoChannelName()
{
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
