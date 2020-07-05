#include "PlotGetters.h"


void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<TH1D*> hh=Get_SIS(runNumber, SIS_Channel,tmin, tmax);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills)
{
   std::vector<TH1D*> hh=Get_SIS(runNumber, SIS_Channel,spills);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill*> s=Get_A2_Spills(runNumber,description,repetition);
   
   std::vector<TH1D*> hh=Get_SIS(runNumber,SIS_Channel,s);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}
extern Int_t gNbin;

void Plot_SVD(Int_t runNumber, std::vector<double> tmin, std::vector<double> tmax)
{
   //This function is complete crap... it should either use the SVDQOD 
   //integrator function... or work with a new 'TA2Plot class'

   assert(tmin.size()==tmax.size());

   double last_time=0;
   int n_times=tmin.size();
   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t<0) t=1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time<t ) last_time=t;
   }

   //Something smarter for the future?
   //TSVDQODIntegrator SVDCounts(TA2RunQOD* q,tmin[0], tmax[0]);
   
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* reader=Get_A2_SVD_Tree(runNumber);
   TTreeReaderValue<TSVD_QOD> SVDEvent(*reader, "OfficalTime");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   
   TH2D* XY=new TH2D("xyvtx", "X-Y Vertex;x [cm];y [cm]", gNbin, -5., 5., gNbin, -5., 5.);
   TH2D* ZY=new TH2D("zyvtx", "Z-Y Vertex;x [cm];y [cm]", gNbin, -5., 5., gNbin, -5., 5.);
   while (reader->Next())
   {
      double t=SVDEvent->t;
      if (t>last_time) break;
      
      //Loop over all time windows
      for (int j=0; j<n_times; j++)
      {
         if (t>tmin[j] && t< tmax[j])
         {
            if (SVDEvent->NPassedCuts)
            {
               //std::cout<<t<<"\t"<<tmin[j]<<"\t"<<t-tmin[j]<<std::endl;
               XY->Fill(SVDEvent->x,SVDEvent->y);
               ZY->Fill(SVDEvent->z,SVDEvent->y);
            }
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }
   XY->Draw("colz");
}

void Plot_SVD(Int_t runNumber, std::vector<TA2Spill*> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill->ScalerData)
      {
         tmin.push_back(spill->ScalerData->StartTime);
         tmax.push_back(spill->ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Plot_SVD(runNumber,tmin,tmax);
}

void Plot_SVD(Int_t runNumber, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill*> s=Get_A2_Spills(runNumber,description,repetition);
   return Plot_SVD(runNumber,s);
}
   
