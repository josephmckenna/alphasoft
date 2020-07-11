#include "TA2Plot.h"

ClassImp(TA2Plot);

TA2Plot::TA2Plot(): TAPlot()
{
   NSIS_Channels=0;
}
TA2Plot::~TA2Plot()
{
}

void TA2Plot::AddEvent(TSVD_QOD* event, double time_offset)
{
   VertexEvent Event;
   Event.runNumber    =event->RunNumber;
   Event.EventNo      =event->VF48NEvent;
   // Encode Passed cuts and online MVA (CutsResult&1 is passed cuts, 
   // CutsResult&2 is online MVA)
   Event.CutsResult   =event->NPassedCuts;
   Event.CutsResult  +=event->MVA*2;
   Event.VertexStatus =event->NVertices;
   Event.x            =event->x;
   Event.y            =event->y;
   Event.z            =event->z;
   Event.t            =event->t-time_offset; //Plot time (based off offical time)
   Event.EventTime    =event->VF48Timestamp; //TPC time stamp
   Event.RunTime      =event->t; //Official Time
   Event.nHelices     =-1; // helices used for vertexing
   Event.nTracks      =event->NTracks; // reconstructed (good) helices
   AddVertexEvent(Event);

}

void TA2Plot::AddEvent(TSISEvent* event, int channel, double time_offset)
{
   SISPlotEvent Event;
   Event.runNumber    =event->GetRunNumber(); // I don't get set yet...
   //int clock
   Event.t            =event->GetRunTime()-time_offset; //Plot time (based off offical time)
   Event.OfficialTime =event->GetRunTime();
   Event.Counts       =event->GetCountsInChannel(channel);
   Event.SIS_Channel  =channel;

   SISEvents.push_back(Event);
}

void TA2Plot::LoadRun(int runNumber)
{
   double last_time=0;
   //Calculate our list time... so we can stop early
   for (auto& t: GetTimeWindows())
   {
      if (t.runNumber==runNumber)
      {
         if (t.tmax<0) last_time=1E99;
         if (last_time<t.tmax)
         {
            last_time=t.tmax;
         }
      }
   }

   //Something smarter for the future?
   //TSVDQODIntegrator SVDCounts(TA2RunQOD* q,tmin[0], tmax[0]);
   
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SVDReader=Get_A2_SVD_Tree(runNumber);
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficalTime");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SVDReader->Next())
   {
      double t=SVDEvent->t;
      if (t>last_time) break;
      
      //Loop over all time windows
      for (auto& window: GetTimeWindows())
      {
         //If inside the time window
         if ( (t>window.tmin && t< window.tmax) ||
         //Or if after tmin and tmax is invalid (-1)
              (t>window.tmin && window.tmax<0) )
         {
            AddEvent(&(*SVDEvent),window.tmin);
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SISReader->Next())
   {
      double t=SISEvent->GetRunTime();
      if (t>last_time) break;

      //Loop over all time windows
      for (auto& window: GetTimeWindows())
      {
         //If inside the time window
         if ( (t>window.tmin && t< window.tmax) ||
         //Or if after tmin and tmax is invalid (-1)
              (t>window.tmin && window.tmax<0) )
         {
            for (int i=0; i<NSIS_Channels; i++)
            {
               int counts=SISEvent->GetCountsInChannel(SIS_Channels[i]);
               if (counts)
               {
                  AddEvent(&(*SISEvent),SIS_Channels[i],window.tmin);
               }
            }
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }
}



void TA2Plot::AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition )
{
   std::vector<TA2Spill*> spills=Get_A2_Spills(runNumber,description,repetition);
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
   return AddTimeGates(runNumber,tmin,tmax);

}

