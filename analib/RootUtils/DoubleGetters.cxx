#include "DoubleGetters.h"


Double_t GetTotalRunTimeFromChrono(Int_t runNumber, Int_t Board)
{
   Double_t OfficialTime;
   TTree* t=Get_Chrono_Tree(runNumber,Board,CHRONO_CLOCK_CHANNEL,OfficialTime);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   t->GetEntry(t->GetEntries()-1);
   Double_t RunTime=e->GetRunTime();
   std::cout<<"End time from CB0"<<Board<<" (official time):"<<RunTime<<" ("<<OfficialTime<<")"<<std::endl;
   delete e;
   if (RunTime>OfficialTime)
      return RunTime;
   return OfficialTime;
}
Double_t GetTotalRunTimeFromTPC(Int_t runNumber)
{
   Double_t OfficialTime;
   TTree* t=Get_StoreEvent_Tree(runNumber, OfficialTime);
   TStoreEvent* e=new TStoreEvent();
   t->SetBranchAddress("StoredEvent", &e);
   t->GetEntry(t->GetEntries()-1);
   Double_t RunTime=e->GetTimeOfEvent();
   delete e;
   //We may want to choose to only use official time once its well calibrated
   std::cout<<"End time from TPC (official time):"<<RunTime<<" ("<<OfficialTime<<")"<<std::endl;
   if (RunTime>OfficialTime)
      return RunTime;
   return OfficialTime;
}

Double_t GetTotalRunTime(Int_t runNumber)
{
   double tmax=-999.;
   double tmp;
   for (int i=0; i<CHRONO_N_BOARDS; i++)
   {
      tmp=GetTotalRunTimeFromChrono(runNumber, i);
      if (tmp>tmax) tmax=tmp;
   }
   tmp=GetTotalRunTimeFromTPC(runNumber);
   if (tmp>tmax) tmax=tmp;
   return tmax;
}



Double_t GetRunTimeOfCount(Int_t runNumber, Int_t Board, Int_t Channel, Int_t repetition, Int_t offset)
{
   double official_time;
   TTree* t=Get_Chrono_Tree(runNumber,Board,Channel,official_time);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   if (repetition+offset>t->GetEntries()) return -1;
   t->GetEntry(repetition-1+offset);
   //Double_t RunTime=e->GetRunTime();
   delete e;
   return official_time;
}

Double_t GetRunTimeOfCount(Int_t runNumber, const char* ChannelName, Int_t repetition, Int_t offset)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return GetRunTimeOfCount(runNumber, board, chan,  repetition,  offset);
}

Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset)
{
   TString ChronoChannelName=Get_Chrono_Name(e);
   std::cout <<"Channel Name:"<<ChronoChannelName<<std::endl;
   Int_t board=0;
   Int_t chan=0;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
      chan=Get_Chrono_Channel(runNumber,board,ChronoChannelName,kTRUE);
      if (chan>-1) break;
   }
   std::cout <<"Looking for TS in board:"<<board <<" channel: "<<chan<<" event: "<<e->GetID()<<std::endl;
   Double_t RunTime=GetRunTimeOfCount(runNumber, board, chan,e->GetID()+1, offset);
   return RunTime;
}


Double_t MatchEventToTime(Int_t runNumber,const char* description, const char* name, Int_t repetition, Int_t offset)//, Bool_t ExactMatch)
{
   TSeq_Event* e=Get_Seq_Event(runNumber, description, name, repetition); //Creates new TSeq_Event
   Double_t RunTime=GetRunTimeOfEvent(runNumber, e, offset);
   delete e;
   return RunTime;

}


Double_t MatchEventToTime(Int_t runNumber,const char* description, Bool_t IsStart, Int_t repetition, Int_t offset)//, Bool_t ExactMatch)
{
   TSeq_Event* e=Get_Seq_Event(runNumber, description, IsStart, repetition); //Creates new TSeq_Event
   Double_t RunTime=GetRunTimeOfEvent(runNumber, e, offset);
   delete e;
   return RunTime;

}
