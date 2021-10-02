#include "TA2Plot.h"
#include "TAPlot.h"

#include <vector>
#include <string>

std::vector<std::pair<std::string,std::string>> CheckForTrees(int runNumber)
{
   std::vector<std::pair<std::string,std::string>> errors;

   TTreeReader* SISTree = A2_SIS_Tree_Reader(runNumber);
   if (SISTree->GetEntries() <= 0 )
   {
      std::string announcement = "Alarm: No SIS data!";
      std::string full_message = "No SIS data in run, was fevme running?";
      errors.push_back({announcement, full_message});
   }
   delete SISTree;
   
   TTreeReader* SVDQODTree = Get_A2_SVD_Tree(runNumber);
   if (SVDQODTree->GetEntries() <= 0 )
   {
      std::string announcement = "Alarm: No SVD data!";
      std::string full_message = "Is the SVD on? Is it triggering?";
      errors.push_back({announcement, full_message});
   }
   delete SVDQODTree;
   return errors;
   
}


std::vector<std::pair<std::string,std::string>> CheckCosmicTriggerRates(TA2Plot* entire_run) // Must be a single time window
{
   std::vector<std::pair<std::string,std::string>> errors;

   TTimeWindows* t = entire_run->GetTimeWindows();
   //Assert that it really is just one time window
   assert (t->fRunNumber.size() == 1);
   assert (t->fMinTime.size() == 1);
   assert (t->fMaxTime.size() == 1);
   assert (t->fZeroTime.size() == 1); 
     
   double RunTime = t->fMaxTime.front() - t->fMinTime.front();
   
   //Too short to really do much
   if (RunTime < 20) return errors;
   
   assert(fTrigNobusy.size() == 1);
   
   int IO32_NOBUSY_CHANNEL = entire_run->GetTrigNoBusyChannel(t->fRunNumber.front());
   int IO32_CHANNEL = entire_run->GetTrigChannel(t->fRunNumber.front());
   
   int IO32_NOBUSY_Counts = entire_run->SISEvents.CountTotalCountsInChannel(IO32_NOBUSY_CHANNEL);
   int IO32_Counts = entire_run->SISEvents.CountTotalCountsInChannel(IO32_CHANNEL);
   
   double IO32_NOBUSY_RATE = IO32_NOBUSY_Counts / RunTime;
   double IO32_RATE = IO32_Counts / RunTime;
   
   if (IO32_NOBUSY_RATE<8)
   {
      std::cout<<"IO32_NOBUSY_RATE: " << IO32_NOBUSY_RATE << " < 8... Bad"<<std::endl;
      std::string announcement = "Warning: Silicon Detector trigger rate is low, check autoAnalysis log";
      std::string full_message = 
"*****************************************************\n\
|   WARNING! SILICON DETECTOR TRIGGER RATE LOW...   |\n\
*****************************************************\n\
|   There are two common causes for this...         |\n\
|                                                   |\n\
|  1. Cosmic trigger was slected in midas when      |\n\
|     starting run...                               |\n\
|  2. Incorrect trigger is selected (trigger should |\n\
|     be 2-1-1) - expert needed                     |\n\
|  3. Incorrect thresholds have been loaded in      |\n\
|     registers - expert needed                     |\n\
|                                                   |\n\
|If the problem is not [1], contact detector expert:|\n\
| Joe:       167337                                 |\n\
| Or anyone else...                                 |\n\
*****************************************************";
         errors.push_back({announcement , full_message});
   }
   else
   {
      system("Color E4");
      std::cout<<"IO32_NOBUSY_RATE: " << IO32_NOBUSY_RATE << " > 8... Good"<<std::endl;
   }
   
    
   if (IO32_NOBUSY_RATE>12 ) //do not announce on very short runs (it might be a dump we are looking at)
   {
      std::cout<<"IO32_NOBUSY_RATE: " << IO32_NOBUSY_RATE << " > 12... Bad!"<<std::endl;
      std::string announcement = "Warning: Silicon Detector trigger rate is high, check autoAnalysis  log";
      std::string full_message = 
"*****************************************************\n\
|   WARNING! SILICON DETECTOR TRIGGER RATE HIGH...  |\n\
*****************************************************\n\
| Contact detector expert:                          |\n\
| Joe:       167337                                 |\n\
| Or anyone else...                                 |\n\
*****************************************************\n"; 
         errors.push_back({announcement , full_message});
   }
   else
   {
      std::cout<<"IO32_NOBUSY_RATE: " << IO32_NOBUSY_RATE << " < 12... Good"<<std::endl;
   }
   
   if ((double)IO32_NOBUSY_Counts * 1.1 < IO32_Counts)
   {
      std::cout << "IO32_NOBUSY_Counts ( "<< IO32_NOBUSY_Counts << ") * 1.1 < IO32_Counts ( " << IO32_Counts << " )... Bad" << std::endl;
      std::string announcement = "Warning: Silicon Detector readouts high (or trigger low). Check autoAnalysis log";
      std::string full_message = 
"*****************************************************\n\
|   WARNING! SILICON DETECTOR DISCRIMINATOR MESS    |\n\
*****************************************************\n\
|The number of triggers for 'read' is higher than ..|\n\
|'trigger'... usually noise in discriminators       |\n\
| Contact detector expert:                          |\n\
| Joe:       167337                                 |\n\
| Or anyone else...                                 |\n\
*****************************************************\n";
   }
   else
   {
      std::cout << "IO32_NOBUSY_Counts ( "<< IO32_NOBUSY_Counts << ") * 1.1 >= IO32_Counts ( " << IO32_Counts << " )... Good" << std::endl;
   }
   if ((double)IO32_Counts * 1.1 < IO32_NOBUSY_Counts)
   {
      std::cout << "IO32_Counts ( "<< IO32_Counts << ") * 1.1 < IO32_NOBUSY_Counts ( " << IO32_NOBUSY_Counts << " )... Bad" << std::endl;
      std::string announcement = "Warning: Silicon Detector readouts low (or trigger high). Check autoAnalysis log";
      std::string full_message = 
"*****************************************************\n\
|   WARNING! SILICON DETECTOR DISCRIMINATOR MESS    |\n\
*****************************************************\n\
|The number of triggers for 'read' is higher than ..|\n\
|'trigger'... usually noise in discriminators       |\n\
| Contact detector expert:                          |\n\
| Joe:       167337                                 |\n\
| Or anyone else...                                 |\n\
*****************************************************\n";
      errors.push_back({announcement , full_message});
   }
   else
   {
      std::cout << "IO32_Counts ( "<< IO32_Counts << ") * 1.1 >= IO32_NOBUSY_Counts ( " << IO32_NOBUSY_Counts << " )... Good" << std::endl;
   }
   return errors;
}


std::vector<std::pair<std::string,std::string>> CheckOccupancy(TA2AnalysisReport* r) // Must be a single time window
{
   std::vector<std::pair<std::string,std::string>> errors;
   std::vector<int> good_channels;
   std::vector<int> dead_channels;
   std::vector<int> quiet_channels;
   int total_N_counts = 0;
   int total_P_counts = 0;
   std::cout<<"Occupancy N side: ";
   for (int i = 0; i < 72; i++)
   {
      int NOcc = r->GetHybridNSideOccupancy(i);
      total_N_counts +=  NOcc;
      std::cout << NOcc<<"\t";
   }
   std::cout << "\n";
   std::cout<<"Occupancy P side: ";
   for (int i = 0; i < 72; i++)
   {
      int POcc = r->GetHybridNSideOccupancy(i);
      total_P_counts +=  POcc;
      std::cout << POcc<<"\t";
   }
   std::cout << "\n";
   for (int i = 0; i < 72; i++)
   {
      int NOcc = r->GetHybridNSideOccupancy(i);
      int POcc = r->GetHybridNSideOccupancy(i);
      if (NOcc == 0 || POcc == 0)
      {
         dead_channels.push_back(i);
      }
      else if ( (double)NOcc / (double)total_N_counts < 0.0005 ||
                  (double)POcc / (double)total_P_counts < 0.0005 )
      {
         quiet_channels.push_back(i);
      }
      else
      {
         good_channels.push_back(i);
      }
   }
   if (dead_channels.size() || quiet_channels.size())
   {
      std::string announcement = 
         "Warning: Silicon Detector has bad modules, (";
      for (const int& dead: dead_channels)
      {
         announcement += std::to_string(dead) + std::string(" ");
      }
      if (dead_channels.size())
         announcement += " are dead";
      for (const int& quiet: quiet_channels)
      {
         announcement += std::to_string(quiet) + std::string(" ");
      }
      if (quiet_channels.size())
         announcement += " are quiet";
      announcement += ")";
      std::string error = 
"*****************************************************\n\
|WARNING! SILICON HYBRID BADNESS. Broken FRC boards?|\n\
*****************************************************\n\
| Contact detector expert:                          |\n\
| Joe:       167337                                 |\n\
| Or anyone else...                                 |\n\
*****************************************************";
      errors.push_back({announcement,error});
   }
   if (good_channels.size() == 72*2)
   {
      std::cout << "Occupancy fairly even. All channels looking good! " <<std::endl;
   }
   return errors;
}

void RunFullTestSuite(int runNumber)
{
   TA2AnalysisReport report = Get_A2Analysis_Report(runNumber);
   report.Print();
   std::cout<<"\n\n";

   std::vector<std::pair<std::string,std::string>> TreeErrors = CheckForTrees(runNumber);
   if (TreeErrors.size())
   {
      std::cout<<"There are missing Trees in this data file... so more tests will likely fail, I continue to test anyway...\n\n";
      for (const std::pair<std::string,std::string>& e: TreeErrors)
      {
         std::cout << e.first << std::endl;
         std::cout << e.second << std::endl;
      }
   }

   TA2Plot a;
   a.AddTimeGate(runNumber,0,GetA2TotalRunTime(runNumber));
   a.LoadData();
   std::vector<std::pair<std::string,std::string>> TriggerErrors = CheckCosmicTriggerRates(&a);
   for (const std::pair<std::string,std::string>& e: TriggerErrors)
   {
     std::cout << e.first << std::endl;
     std::cout << e.second << std::endl;
   }

   
   std::vector<std::pair<std::string,std::string>>  OccupancyErrors = CheckOccupancy(&report);
   for (const std::pair<std::string,std::string>& e: OccupancyErrors)
   {
     std::cout << e.first << std::endl;
     std::cout << e.second << std::endl;
   }

}

void Test(int runNumber = 58687)
{
  return RunFullTestSuite(runNumber);
}

