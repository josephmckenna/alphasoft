#include "DoubleGetters.h"



Double_t GetTotalRunTime(Int_t runNumber)
{
   TTree* t=Get_Chrono_Tree(runNumber,0,CHRONO_CLOCK_CHANNEL);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   t->GetEntry(t->GetEntries()-1);
   Double_t RunTime=e->GetRunTime();
   delete e;
   return RunTime;
}


Double_t GetRunTimeOfCount(Int_t runNumber, Int_t Board, Int_t Channel, Int_t repetition=1)
{
   TTree* t=Get_Chrono_Tree(runNumber,Board,Channel);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   t->GetEntry(repetition-1);
   Double_t RunTime=e->GetRunTime();
   delete e;
   return RunTime;
}
Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e)
{
   TString ChronoChannelName=Get_Chrono_Name(e);
   Int_t board=0;
   Int_t chan=0;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
      chan=Get_Chrono_Channel(runNumber,board,ChronoChannelName,kTRUE);
   }
   Double_t RunTime=GetRunTimeOfCount(runNumber, board, chan,e->GetID());
   delete e;
   return RunTime;
}



Double_t MatchEventToTime(Int_t runNumber,TString description, Bool_t IsStart, Int_t repetition, Int_t offset, Bool_t ExactMatch)
{
   TSeq_Event* e=Get_Seq_Event(runNumber, description, IsStart, repetition); //Creates new TSeq_Event
   Double_t RunTime=GetRunTimeOfEvent(runNumber, e);
  delete e;
  return RunTime;

}
