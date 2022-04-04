#include "TAGSpillScalerData.h"

#if BUILD_AG

ClassImp(TAGSpillScalerData)
TAGSpillScalerData::TAGSpillScalerData():
   TSpillScalerData(CHRONO_N_BOARDS*CHRONO_N_CHANNELS)
{

}

TAGSpillScalerData::~TAGSpillScalerData()
{

}

TAGSpillScalerData::TAGSpillScalerData(int n_scaler_channels):
   TSpillScalerData(n_scaler_channels)
{

}

TAGSpillScalerData::TAGSpillScalerData(const TAGSpillScalerData& a): TSpillScalerData(a)
{

}

TAGSpillScalerData::TAGSpillScalerData(TDumpMarkerPair<TAGDetectorEvent,TChronoBoardCounter,CHRONO_N_BOARDS>* d):
   TAGSpillScalerData()
{
   for (int i=0; i<CHRONO_N_BOARDS; i++)
   {
      const TChronoBoardCounter& e = d->fIntegratedScalerCounts[i];
      for (size_t j = 0; j < e.fCounts.size(); j++)
         fDetectorCounts.at(i*CHRONO_N_CHANNELS + j) = e.fCounts[j];
      ScalerFilled[i] = true;
   }

   if (d->fStartDumpMarker)
      fStartTime = d->fStartDumpMarker->fRunTime;
   if (d->fStopDumpMarker)
      fStopTime = d->fStopDumpMarker->fRunTime;

   /*FirstVertexEvent  =d->fIntegratedVertexCounts.fFirstEventID;
   LastVertexEvent   =d->fIntegratedVertexCounts.fLastEventID;
   VertexEvents      =d->fIntegratedVertexCounts.fEvents;
   fVerticies         =d->fIntegratedVertexCounts.fVerticies;
   fPassCuts          =d->fIntegratedVertexCounts.fPassCuts;
   fPassMVA           =d->fIntegratedVertexCounts.fPassMVA;*/
   VertexFilled      =true;
}

void TAGSpillScalerData::Print()
{
   std::cout<<"StartTime: "<<fStartTime << " StopTime: "<<fStopTime <<std::endl;
   std::cout<<"ChronoFilled: ";
   for (size_t i=0; i<ScalerFilled.size(); i++)
   {
      std::cout<<ScalerFilled.at(i);
   }
   std::cout  << " BVFilled: "<<VertexFilled <<std::endl;
   int sum = 0;
   for (size_t i = 0; i <  fDetectorCounts.size(); i++)
      sum+=fDetectorCounts[i];
   std::cout<<"ChronoEntries:"<< sum <<std::endl;
   for (size_t i = 0; i < fDetectorCounts.size(); i++)
   {
      std::cout<<fDetectorCounts[i]<<"\t";
   }
}

#endif