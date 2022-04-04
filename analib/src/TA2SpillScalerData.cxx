
#include "TA2SpillScalerData.h"

ClassImp(TA2SpillScalerData)
/*TA2SpillScalerData::TA2SpillScalerData()
{

}*/

TA2SpillScalerData::~TA2SpillScalerData()
{

}

TA2SpillScalerData::TA2SpillScalerData(int n_scaler_channels):
   TSpillScalerData(n_scaler_channels)
{

}

TA2SpillScalerData::TA2SpillScalerData(const TA2SpillScalerData& a): TSpillScalerData(a)
{

}

TA2SpillScalerData::TA2SpillScalerData(TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d):
   TA2SpillScalerData()
{
   for (int i=0; i<NUM_SIS_MODULES; i++)
   {
      const TSISEvent& e = d->fIntegratedScalerCounts[i];
      for (int j=i*NUM_SIS_CHANNELS; j<(i+1)*NUM_SIS_CHANNELS; j++)
      {
         fDetectorCounts.at(j) = e.GetCountsInChannel(j);
         ScalerFilled[j]=true;
      }
   }

   if (d->fStartDumpMarker)
      fStartTime=d->fStartDumpMarker->fRunTime;
   if (d->fStopDumpMarker)
      fStopTime=d->fStopDumpMarker->fRunTime;

   FirstVertexEvent  =d->fIntegratedVertexCounts.fFirstEventID;
   LastVertexEvent   =d->fIntegratedVertexCounts.fLastEventID;
   VertexEvents      =d->fIntegratedVertexCounts.fEvents;
   fVerticies        =d->fIntegratedVertexCounts.fVerticies;
   fPassCuts         =d->fIntegratedVertexCounts.fPassCuts;
   fPassMVA          =d->fIntegratedVertexCounts.fPassMVA;
   VertexFilled      =true;
}



void TA2SpillScalerData::Print()
{
   std::cout<<"StartTime: "<<fStartTime << " StopTime: "<<fStopTime <<std::endl;
   std::cout<<"SISFilled: ";
   for (size_t i=0; i<ScalerFilled.size(); i++)
   {
      std::cout<<ScalerFilled.at(i);
   }
   std::cout  << " SVDFilled: "<<VertexFilled <<std::endl;
   int sum=0;
   for (int i = 0; i < NUM_SIS_MODULES*NUM_SIS_CHANNELS; i++)
      sum+=fDetectorCounts[i];
   std::cout<<"SISEntries:"<< sum << "\tSVD Events:"<<VertexEvents<<std::endl;
   for (int i = 0; i < NUM_SIS_MODULES*NUM_SIS_CHANNELS; i++)
   {
      std::cout<<fDetectorCounts[i]<<"\t";
   }
}