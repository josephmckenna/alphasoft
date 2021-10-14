#include "DoubleGetters.h"

#ifdef BUILD_AG
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, Int_t Board)
{
   Double_t OfficialTime;
   TTree* t=Get_Chrono_Tree(runNumber,{Board,CHRONO_CLOCK_CHANNEL},OfficialTime);
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
#endif
#ifdef BUILD_AG
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
#endif
#ifdef BUILD_AG
Double_t GetAGTotalRunTime(Int_t runNumber)
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
#endif
#ifdef BUILD_AG
Double_t GetRunTimeOfChronoCount(Int_t runNumber, Int_t Board, Int_t Channel, Int_t dumpIndex, Int_t offset)
{
   double official_time;
   TTree* t=Get_Chrono_Tree(runNumber,{Board,Channel},official_time);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   if (dumpIndex+offset>t->GetEntries()) return -1;
   t->GetEntry(dumpIndex+offset);
   //Double_t RunTime=e->GetRunTime();
   delete e;
   return official_time;
}
#endif
#ifdef BUILD_AG
Double_t GetRunTimeOfChronoCount(Int_t runNumber, const char* ChannelName, Int_t dumpIndex, Int_t offset)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return GetRunTimeOfChronoCount(runNumber, board, chan,  dumpIndex,  offset);
}
#endif
#ifdef BUILD_AG
Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset)
{
   TString ChronoChannelName=Get_Chrono_Name(e);
   //   std::cout <<"Channel Name:"<<ChronoChannelName<<std::endl;
   Int_t board=0;
   Int_t chan=0;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
      chan=Get_Chrono_Channel(runNumber,board,ChronoChannelName,kTRUE);
      if (chan>-1) break;
   }
   //   std::cout <<"Looking for TS in board:"<<board <<" channel: "<<chan<<" event: "<<e->GetID()<<std::endl;
   Double_t RunTime=GetRunTimeOfChronoCount(runNumber, board, chan,e->GetID()+1, offset);
   return RunTime;
}
#endif

#ifdef BUILD_AG
Double_t MatchEventToTime(Int_t runNumber,const char* description, const char* name, Int_t dumpIndex, Int_t offset)//, Bool_t ExactMatch)
{
   TSeq_Event* e=Get_Seq_Event(runNumber, description, name, dumpIndex); //Creates new TSeq_Event
   Double_t RunTime=GetRunTimeOfEvent(runNumber, e, offset);
   delete e;
   return RunTime;
}
Double_t MatchEventToTime(Int_t runNumber,const char* description, Bool_t IsStart, Int_t dumpIndex, Int_t offset)//, Bool_t ExactMatch)
{
   TSeq_Event* e=Get_Seq_Event(runNumber, description, IsStart, dumpIndex); //Creates new TSeq_Event
   Double_t RunTime=GetRunTimeOfEvent(runNumber, e, offset);
   delete e;
   return RunTime;

}
#endif

#ifdef BUILD_AG
Double_t GetTrigTimeBefore(Int_t runNumber, Double_t mytime)
{   
  double official_time;
  TStoreEvent *store_event = new TStoreEvent();
  TTree *t0 = Get_StoreEvent_Tree(runNumber, official_time);
  t0->SetBranchAddress("StoredEvent", &store_event);
  int event_id = -1;
  for( Int_t i = 0; i < t0->GetEntries(); ++i )
    {
      store_event->Reset();
      t0->GetEntry(i);
      if( !store_event )
	{
	  std::cout<<"NULL TStore event: Probably more OfficialTimeStamps than events"<<std::endl;
	  break;
	}
      if( official_time > mytime )
	{
	  event_id = i-1;
	  store_event->Reset();
	  break;
	}
      store_event->Reset();
   }
  t0->GetEntry(event_id);
  double runtime=store_event->GetTimeOfEvent();
  delete store_event;
  return runtime;
}
#endif
#ifdef BUILD_AG
Double_t GetTrigTimeAfter(Int_t runNumber, Double_t mytime)
{   
  double official_time;
  TStoreEvent *store_event = new TStoreEvent();
  TTree *t0 = Get_StoreEvent_Tree(runNumber, official_time);
  t0->SetBranchAddress("StoredEvent", &store_event);
  int event_id = -1;
  for( Int_t i = 0; i < t0->GetEntries(); ++i )
    {
      store_event->Reset();
      t0->GetEntry(i);
      if( !store_event )
	{
	  std::cout<<"NULL TStore event: Probably more OfficialTimeStamps than events"<<std::endl;
	  break;
	}
      if( official_time > mytime )
	{
	  event_id = i;
	  store_event->Reset();
	  break;
	}
      store_event->Reset();
   }
  t0->GetEntry(event_id);
  double runtime=store_event->GetTimeOfEvent();  
  delete store_event;
  return runtime;
}
#endif

#if BUILD_A2
Double_t GetTotalRunTimeFromSIS(Int_t runNumber)
{
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   if (!SISReader->GetTree())
      return -1.;
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   SISReader->SetEntry(SISReader->GetEntries(false) -1 );
   double t = SISEvent->GetRunTime();
   return t;   
}
Double_t GetTotalRunTimeFromSVD(Int_t runNumber)
{
      //More performance is maybe available if we use DataFrames...
   TTreeReader* SVDReader=Get_A2_SVD_Tree(runNumber);
   if (!SVDReader->GetTree())
      return -1.;
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficialTime");
   SVDReader->SetEntry(SVDReader->GetEntries(false) -1 );
   double t = SVDEvent->t;
   return t;
}
Double_t GetA2TotalRunTime(Int_t runNumber)
{
   double SISTime = GetTotalRunTimeFromSIS(runNumber);
   double SVDTime = GetTotalRunTimeFromSVD(runNumber);
   if (SISTime > SVDTime)
      return SISTime;
   else
      return SVDTime;
}


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
