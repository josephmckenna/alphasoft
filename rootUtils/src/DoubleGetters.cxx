#include "DoubleGetters.h"

#ifdef BUILD_AG
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, Int_t Board)
{
   TTree* t=Get_Chrono_Tree(runNumber,TChronoChannel(Board,CHRONO_CLOCK_CHANNEL).GetBranchName());
   TCbFIFOEvent* e=new TCbFIFOEvent();
   t->SetBranchAddress("FIFOData", &e);
   t->GetEntry(t->GetEntries()-1);
   Double_t RunTime = e->GetRunTime();
   std::cout << "End time from CB0" << Board << ":" << RunTime << std::endl;
   delete e;
   return RunTime;
}
#endif
#ifdef BUILD_AG
Double_t GetTotalRunTimeFromTPC(Int_t runNumber)
{
   Double_t OfficialTime;
   TTree* t=Get_StoreEvent_Tree(runNumber, OfficialTime);
   TStoreEvent* e = new TStoreEvent();
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
Double_t GetRunTimeOfChronoCount(Int_t runNumber, TChronoChannel chan, Int_t event_index)
{
   TTree* t=Get_Chrono_Tree(runNumber,chan.GetBranchName());
   TCbFIFOEvent* e=new TCbFIFOEvent();
   t->SetBranchAddress(chan.GetBranchName().c_str(), &e);
   if (event_index > t->GetEntries()) return -1;
   t->GetEntry(event_index);
   Double_t RunTime = e->GetRunTime();
   delete e;
   return RunTime;
}
#endif
#ifdef BUILD_AG
Double_t GetRunTimeOfChronoCount(Int_t runNumber, const char* ChannelName, Int_t event_number)
{
   TChronoChannel chan = Get_Chrono_Channel(runNumber, ChannelName);
   return GetRunTimeOfChronoCount(runNumber, chan,  event_number);
}
#endif
#ifdef BUILD_AG
Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset)
{
   TString ChronoChannelName=Get_Chrono_Name(e);
   //   std::cout <<"Channel Name:"<<ChronoChannelName<<std::endl;
   TChronoChannel chan = Get_Chrono_Channel(runNumber,ChronoChannelName,kTRUE);
   if (chan.IsValidChannel())
      return GetRunTimeOfChronoCount(runNumber,  chan,e->GetID()+1 + offset);
   else
      return -1.;
   //   std::cout <<"Looking for TS in board:"<<board <<" channel: "<<chan<<" event: "<<e->GetID()<<std::endl;
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
   double t = -1;
   for (int sis_module_no = 0; sis_module_no < NUM_SIS_MODULES; sis_module_no++)
   {
      TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber, sis_module_no);
      if (!SISReader->GetTree())
         continue;
      TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
      SISReader->SetEntry(SISReader->GetEntries(false) -1 );
      if (SISEvent->GetRunTime() > t)
         t = SISEvent->GetRunTime();
   }
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
