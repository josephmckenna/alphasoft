#include "RootUtils.h"
#include "PlotGetters.h"

#include "TChronoChannelName.h"
#include "TChronoChannelGetters.h"

void SaveAllDumps(int runNumber)
{
   std::vector<TAGSpill> all = Get_AG_Spills(runNumber,{"*"},{-1});
   //Get this list from the root file! (or ODB)
   std::vector<std::string> detector_channels = {
      //channel name, descriptive name
      "CATCH_OR",
      "SiPM_B",
      "SiPM_E",
      "TPC_TRIG",
      "SiPM_A_AND_D",
      "SiPM_C_AND_F",
      "SiPM A_OR_C-AND-D_OR_F",
      "SiPM_C",
      "SiPM_D",
      "SiPM_F"
   };
   std::vector<TChronoChannel> chans;
   for (std::string channel_name: detector_channels)
   {
      TChronoChannel c = Get_Chrono_Channel(runNumber,channel_name.c_str());
      chans.push_back(c);
   }

   std::vector<std::vector<TH1D*>> Histos = Get_Chrono(runNumber,chans,all);
   std::cout<<Histos.size() <<"\t"<<Histos.at(0).size()<<std::endl;
   
   //All sis channels
   for (int i = 0; i< detector_channels.size(); i++)
   {
      std::string channel_name = detector_channels.at(i);
      
      std::map<std::string,int> dumpIndex_counter;
    
      TChronoChannel ch = Get_Chrono_Channel(runNumber,channel_name.c_str());
      chans.push_back(ch);

      std::cout<<channel_name <<"\tis sis channel\t"<<ch<<std::endl;
      if (!ch.IsValidChannel())
         continue;
      
      for (int j = 0; j < all.size(); j++)
      {
         const TAGSpill s = all.at(j);
         if (s.GetStartTime() == s.GetStopTime())
         {
            std::cout<<s.Name << " has no time duration (its a pulse...) skipping plot" <<std::endl;
            continue;
         }
         if (!s.ScalerData)
         {
            std::cout<<s.Name << " has no scaler data (its an information dump...) skipping plot" <<std::endl;
            continue;
         }
         if (!s.ScalerData->DetectorCounts.at(ch.GetIndex()))
         {
            std::cout<<s.Name << " has no counts in channel " << ch<<" ... skipping plot" <<std::endl;
            continue;
         }
         std::string dump_name = s.GetSequenceName() + "_" + s.Name + "_" + std::to_string(dumpIndex_counter[s.GetSequenceName() + "_" + s.Name]++);
         // Remove quote marks... they upset uploading to elog
         dump_name.erase(std::remove(dump_name.begin(), dump_name.end(), '"'), dump_name.end());
         std::cout << "dump_name:"<< channel_name << "/"<< dump_name <<std::endl;

         TCanvas* c = new TCanvas();
         Histos.at(i).at(j)->Draw("HIST");

         std::string folder = "AutoChronoPlots/";
         gSystem->mkdir(folder.c_str());
         folder += std::to_string(runNumber) + "/";
         gSystem->mkdir(folder.c_str());
         folder += std::string(channel_name.c_str()) + "/";
         gSystem->mkdir(folder.c_str());
         
         std::string filename = folder + "R" + std::to_string(runNumber) + std::string("_") + dump_name + ".png";
         c->Draw();
         c->SaveAs(filename.c_str());
         c->Close();
         delete c;
      }
   }
}


