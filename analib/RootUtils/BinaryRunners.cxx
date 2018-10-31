#include "BinaryRunners.h"

void RunEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax)
{
   //Convert TMIN and TMAX from official time to TPC time
   Int_t first=GetTPCEventNoBeforeOfficialTime(runNumber,tmin);
   Int_t last=GetTPCEventNoBeforeOfficialTime(runNumber,tmax)+1;
   std::cout<<"THIS IS ONLY A PLACEHOLDER"<<std::endl;
   std::cout<<"THIS IS ONLY A PLACEHOLDER"<<std::endl;
   std::cout<<"THIS IS ONLY A PLACEHOLDER"<<std::endl;
   TString a="./agana.exe MIDASFILES -- --aged --useeventrange ";
   a+=first; a+=" "; a+=last;
   gSystem->Exec("echo big bells");
   gSystem->Exec(a);
   return;
}

void RunEventViewerInTime(Int_t runNumber,  const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return RunEventViewerInTime(runNumber, tmin, tmax);
}
