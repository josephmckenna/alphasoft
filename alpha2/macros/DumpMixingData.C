#include "DumpMixingData.h"

void DumpMixingData(int runNumber = 57181, const char* list_name = "Mixing")
{
   TFile* f = Get_File(runNumber);
   if (!f) {
      std::cout <<"File doesn't exist..."<<std::endl;
      return;
   }
   std::vector<TA2Spill> spills = Get_A2_Spills(runNumber, {"Mixing"},{-1});
   TSISChannels channels(runNumber);
   TSISChannel SVDTriggers = channels.GetChannel("IO32_TRIG_NOBUSY");
   //std::cout <<  SVDTriggers.toInt() <<std::endl;

   TA2Plot plot;
   //std::cout<<spills.size()<<std::endl;
   //for (const TA2Spill& s: spills) {
   for (int i = 0; i < spills.size(); i++){
      const TA2Spill& s = spills.at(i);

      double dump_length = s.GetStopTime() - s.GetStartTime();

      // Limit dump length
      if ( dump_length < 1.0 ) {
         continue;
      }
      if ( dump_length > 1.5 ) {
         continue;
      }
      
      // Require 300 pass cuts
      if (s.ScalerData) {
         if ( s.ScalerData->PassCuts <= 300) {
            continue;
         }
      }
      
      // Require 10k triggers
      if (s.ScalerData) {
         if ( SVDTriggers.toInt() >= 0 && s.ScalerData->DetectorCounts.size() ) {
            if ( s.ScalerData->DetectorCounts.at(SVDTriggers.toInt()) < 10000) {
               continue;
            }
         }
      }
      plot.AddDumpGates(runNumber,{TA2Spill(s)});
   }
   plot.LoadData();
   if (plot.GetNVertexEvents()) {
      plot.WriteEventList("Mixing");
   } else {
      std::cout << "No events to save in " << runNumber << std::endl;
   }
   return;
}
