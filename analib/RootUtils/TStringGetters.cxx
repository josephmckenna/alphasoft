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
