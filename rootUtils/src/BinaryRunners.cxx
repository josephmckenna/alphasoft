#include "BinaryRunners.h"


Bool_t gAnnounce = kTRUE;
#ifdef BUILD_AG
void RunAGEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax)
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

void RunAGEventViewerInTime(Int_t runNumber,  const char* description, Int_t dumpIndex)
{
   std::vector<TAGSpill> s = Get_AG_Spills(runNumber,{description},{dumpIndex});
   if (s.empty())
   {
      std::cout<<"No matching dump name"<<std::endl;
      return;
   }
   Double_t tmin=s.front().GetStartTime();
   Double_t tmax=s.front().GetStopTime();
   return RunAGEventViewerInTime(runNumber, tmin, tmax);
}
#endif


void AnnounceOnSpeaker(Int_t runNumber, TString Phrase)
{
   if (gAnnounce == kFALSE)
      return;
   std::cout << gSystem->HostName() << std::endl;
   if (strcmp(gSystem->HostName(), "alphadaq.cern.ch") == 0 || 
       strcmp(gSystem->HostName(), "alphacpc04.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc09.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc39.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic2") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic2.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphadaq") == 0 || 
       strcmp(gSystem->HostName(), "alphacpc04") == 0 || 
       strcmp(gSystem->HostName(), "alphacpc09") == 0 || 
       strcmp(gSystem->HostName(), "alphacpc39") == 0)
      {
         TString CurrentRunNo = gSystem->GetFromPipe("ssh -x agdaq@alphagdaq \'odbedit -d \"/Runinfo/Run number\" -c ls \' | grep -Eo [0-9]{5}");
         Int_t CurrentNo = CurrentRunNo.Atoi();
         if (CurrentNo < runNumber + 50)
            {
               AnnounceOnSpeaker(Phrase);
            }
         else
            {
               std::cout << "RunNumber too old to announce in control room (" << runNumber << " <= " << CurrentNo << " - 50)" << std::endl;
            }
      }
   return;
}

void AnnounceOnSpeaker(TString Phrase)
{
   if (gAnnounce == kFALSE)
      return;
   std::cout << gSystem->HostName() << std::endl;
   if (strcmp(gSystem->HostName(), "alphadaq.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc04.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc09.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc39.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic2") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphaautomagic2.cern.ch") == 0 ||
       strcmp(gSystem->HostName(), "alphadaq") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc04") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc09") == 0 ||
       strcmp(gSystem->HostName(), "alphacpc39") == 0)
      {
         TString result = "ssh -x alpha@alphadaq \"echo \'";
         result += Phrase;
         result += "\' | ~/online/bin/google_speech/speech_google.py \"";
         gSystem->Exec(result);
         std::cout << Phrase << std::endl;
      }
   return;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
