#include "TStringGetters.h"

#include "Sequencer2.h"


#ifdef BUILD_AG
TString Get_Chrono_Name(Int_t runNumber, TChronoChannel chan)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(chan.GetBoard());
   TString name=n->GetChannelName(chan.GetChannel());
   delete n;
   return name;
}
#endif
#ifdef BUILD_AG
TString Get_Chrono_Name(TSeq_Event* e)
{

   if (e->GetEventName()=="startDump")
      return StartDumpName[e->GetSeqNum()];
   if (e->GetEventName()=="stopDump")
      return StopDumpName[e->GetSeqNum()];
   return "UNKNOWN_SEQUENCER";
         
}
#endif

#ifdef BUILD_A2
TString Get_SIS_Name(Int_t runNumber, Int_t SIS_Channel)
{
   TSISChannels ch(runNumber);
   TString name(ch.GetDescription(SIS_Channel, runNumber));
   return name;
}
#endif

#ifdef BUILD_AG
TString SequenceAGQODDetectorLine(Int_t runNumber,Double_t tmin, Double_t tmax, std::vector<TChronoChannel> chans)
{
   if (runNumber<0) return "CATCH_OR\tTPC TRIG\tSiPM_B\tSiPM_E\tSiPM_A_AND_D\tSiPM_C_AND_F";
   TString line="\t";
   //std::cout <<tmin<<":"<<tmax<<std::endl;
   if (tmin<0 && tmax<0) return "\tINVALID TIME RANGE";
   //Add in SIS flags:
   for (Int_t i=0; i<chans.size(); i++)
   {
      //std::cout <<i<<"\t"<<*boards[i]<<"-"<<*channels[i]<<std::endl;
      if (chans[i].IsValidChannel())
        line+=GetCountsInChannel(runNumber, chans[i], tmin, tmax);
      else
         line+="N/A";
      line+="\t";
   }
   return line;
}
#endif

TString MakeAutoPlotsFolder(TString subFolder)
{
  gSystem->mkdir("AutoPlots");
  // Make dated folder
  TDatime *TS1 = new TDatime;
  const unsigned int date = TS1->GetDate();
  TString savFolder("AutoPlots/");
  savFolder += date;
  if ( subFolder.CompareTo("time")==0 )
  {
     subFolder="";
     savFolder+="-"; // Date - time separation character
     //Present time as characters (HHMM)
     Int_t H=TS1->GetHour();
     if (H<10) savFolder+=0;
     savFolder+=H;
     Int_t M=TS1->GetMinute();
     if (M<10) savFolder+=0;
     savFolder+=M;
  }

  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  savFolder += "/";
  savFolder += (subFolder);
  //savFolder += "/";
  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  else
  {
    std::cout << "The folder " << savFolder << " already exists, saving plots here" << std::endl;
  }
  delete TS1;
  return savFolder;
}

TString MakeAutoPlotsFolder(TString subFolder,TString rootdir)
{
  if( !IsPathExist(rootdir) ) return MakeAutoPlotsFolder(subFolder);
  gSystem->mkdir(rootdir+"AutoPlots");
  // Make dated folder
  TDatime *TS1 = new TDatime;
  const unsigned int date = TS1->GetDate();
  TString savFolder(rootdir+"AutoPlots/");
  savFolder += date;
  if ( subFolder.CompareTo("time")==0 )
  {
     subFolder="";
     savFolder+="-"; // Date - time separation character
     //Present time as characters (HHMM)
     Int_t H=TS1->GetHour();
     if (H<10) savFolder+=0;
     savFolder+=H;
     Int_t M=TS1->GetMinute();
     if (M<10) savFolder+=0;
     savFolder+=M;
  }

  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  savFolder += "/";
  savFolder += (subFolder);
  //savFolder += "/";
  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  else
  {
    std::cout << "The folder " << savFolder << " already exists, saving plots here" << std::endl;
  }
  delete TS1;
  return savFolder;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
