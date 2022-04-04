#include "DoubleGetters.h"

#ifdef BUILD_AG
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, const std::string& Board)
{
   TTree* t=Get_Chrono_Tree(runNumber,TChronoChannel(Board,CHRONO_CLOCK_CHANNEL).GetBranchName());
   if (!t)
      return -1;
   TCbFIFOEvent* e=new TCbFIFOEvent();
   t->SetBranchAddress("FIFOData", &e);
   if (!t->GetEntries())
      return -1;
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
   TTree* t=Get_StoreEvent_Tree(runNumber);
   TStoreEvent* e = new TStoreEvent();
   t->SetBranchAddress("StoredEvent", &e);
   t->GetEntry(t->GetEntries()-1);
   Double_t RunTime=e->GetTimeOfEvent();
   delete e;
   //We may want to choose to only use official time once its well calibrated
   std::cout<<"End time from TPC (official time):"<<RunTime<<std::endl;
   return RunTime;
}
#endif
#ifdef BUILD_AG
Double_t GetAGTotalRunTime(Int_t runNumber)
{
   double tmax=-999.;
   double tmp;
   for (const std::pair<std::string,int>& board: TChronoChannel::CBMAP)
   {
      tmp=GetTotalRunTimeFromChrono(runNumber, board.first);
      if (tmp>tmax) tmax=tmp;
   }
   tmp=GetTotalRunTimeFromTPC(runNumber);
   if (tmp>tmax) tmax=tmp;
   return tmax;
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
      SISReader->SetEntry(SISReader->GetEntries(false) - 1 );
      std::cout <<"mod: " << sis_module_no << "\t" <<SISReader->GetEntries(true) -1 <<std::endl;
      std::cout << SISEvent->GetRunTime() << std::endl;
      if (SISEvent->GetRunTime() > t)
         t = SISEvent->GetRunTime();
      delete SISReader;
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
