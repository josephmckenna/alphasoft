
void DumpCosmicData(int runNumber = 57181, const char* list_name = "Cosmic")
{
   TFile* f = Get_File(runNumber);
   if (!f) {
      std::cout <<"File doesn't exist..."<<std::endl;
      return;
   }
   std::vector<TA2Spill> spills = Get_A2_Spills(runNumber, {"*"},{-1});
   TSISChannels channels(runNumber);
   TSISChannel SVDTriggers = channels.GetChannel("IO32_TRIG_NOBUSY");
   //std::cout <<  SVDTriggers.toInt() <<std::endl;

   TA2Plot plot;
   //for now skip runs with spill in... in the future intelgently filter out pbar related names
   if (spills.size() )
   {
      std::cout<<"Too many dumps (" << spills.size() << ")... I'm probably not a cosmic run"<< std::endl;
      return;
   }
   double total_time = GetA2TotalRunTime(runNumber);
   if (total_time < 1000.)
   {
      std::cout<<"Run to short"<<std::endl;
      return;
   }

   plot.AddTimeGate(runNumber,0., total_time);
   
   plot.LoadData();
   if (plot.GetNVertexEvents()) {
      std::cout << "Adding "<< plot.GetNVertexEvents() << " Events to "<< list_name << ".list "<< std::endl;
      plot.WriteEventList("Cosmic");
   } else {
      std::cout << "No events to save in " << runNumber << std::endl;
   }
   return;
}
