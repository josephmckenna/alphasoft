#include "TChronoChannelName.h"

ClassImp(TChronoChannelName)

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
            if (Name[i].BeginsWith(ChannelName)==0) return i;
         }
      }
      else
      {
         for (int i=0; i<CHRONO_N_CHANNELS; i++)
         {
            if (Name[i].CompareTo(ChannelName)==0) return i;
         }
      }
      return -1;
   }
