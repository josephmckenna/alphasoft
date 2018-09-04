#include "TStringGetters.h"





TString Get_Chrono_Name(Int_t runNumber, Int_t ChronoBoard, Int_t Channel)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(ChronoBoard);
   TString name=n->GetChannelName(Channel);
   delete n;
   return name;
}

TString Get_Chrono_Name(TSeq_Event* e)
{
   switch(e->GetSeqNum())
   {
      //CATCH SEQUENCER
      case 0: 
         if (e->GetEventName()=="startDump")
           return "CAT_START_DUMP";
         if (e->GetEventName()=="stopDump")
           return "CAT_STOP_DUMP";
      //BEAMLINE SEQUENCER
      case 1: 
         if (e->GetEventName()=="startDump")
           return "BL_START_DUMP";
         if (e->GetEventName()=="stopDump")
           return "BL_STOP_DUMP";
      //AG SEQUENCER
      case 2: 
         if (e->GetEventName()=="startDump")
           return "AG_START_DUMP";
         if (e->GetEventName()=="stopDump")
           return "AG_STOP_DUMP";
      //POS SEQUENCER
      case 3: 
         if (e->GetEventName()=="startDump")
           return "POS_START_DUMP";
         if (e->GetEventName()=="stopDump")
           return "POS_STOP_DUMP";
   }
   return "UNKNOWN_SEQUENCER";
         
}


TString SequenceQODDetectorLine(Int_t runNumber,Double_t tmin, Double_t tmax, Int_t* boards[], Int_t* channels[], Int_t nChannels)
{
   if (runNumber<0) return "CATCH OR\tCATCH_AND\tATOM_OR  \tATOM_AND\tCATCH_STICK\tIO32_NOBSY\tATOM_STICK";
   TString line="\t";
   //std::cout <<tmin<<":"<<tmax<<std::endl;
  
   //Add in SIS flags:
   for (Int_t i=0; i<nChannels; i++)
   {
      //std::cout <<i<<"\t"<<*boards[i]<<"-"<<*channels[i]<<std::endl;
      if (*channels[i]>-1)
        line+=GetCountsInChannel(runNumber, *boards[i], *channels[i], tmin, tmax);
      else
         line+="N/A";
      line+="\t";
   }
   return line;
}



TString MakeAutoPlotsFolder(TString subFolder)
{
  gSystem->mkdir("AutoPlots");
  // Make dated folder
  TDatime *TS1 = new TDatime;
  const unsigned int date = TS1->GetDate();
  TString savFolder("AutoPlots/");
  savFolder += date;
  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    savFolder = "AutoPlots/";
    savFolder += date;
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  savFolder += "/";
  savFolder += (subFolder);
  if (((gSystem->OpenDirectory(savFolder)) == 0)) //gSystem causesing problem when compiling marco... will fix tomorrow
  {
    gSystem->mkdir(savFolder);
    std::cout << "Plot output folder: " << savFolder << " created " << std::endl;
  }
  else
  {
    std::cout << "The folder " << savFolder << " already exists, saving plots here" << std::endl;
  }
  return savFolder;
}

