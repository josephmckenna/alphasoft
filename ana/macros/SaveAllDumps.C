#include "RootUtils.h"
#include "PlotGetters.h"

#include "TChronoChannelName.h"
#include "TChronoChannelGetters.h"

void SaveAllDumps(int runNumber)
{
   std::vector<TAGSpill> all = Get_AG_Spills(runNumber,{"*"},{-1});
   //Get this list from the root file! (or ODB)
   
   //List of channels for the AutoAnalysis to look at
   std::vector<std::string> detector_channels = {
      //channel name, descriptive name
      "PMT_CATCH_OR",
      "SiPM_CATCH_OR",
      "SiPM_A","SiPM_D","SiPM_A_OR_D",
      "SiPM_E",
      "SiPM_C","SiPM_F","SiPM_C_OR_F",
      "SiPM_B",
      "ADC_TRG",
      //Are these gone? yep
      "TPC_TRIG",
      "SiPM_A_AND_D",
      "SiPM_C_AND_F",
      "SiPM A_OR_C-AND-D_OR_F",
   };
   std::vector<TChronoChannel> chans;
   std::vector<std::string> valid_channels;
   for (std::string channel_name: detector_channels)
   {
      TChronoChannel c = Get_Chrono_Channel(runNumber,channel_name.c_str());
      if (c.IsValidChannel())
      {
         chans.push_back(c);
         valid_channels.push_back(channel_name);
      }
   }

   std::vector<std::vector<TH1D*>> Histos = Get_Chrono(runNumber,chans,all);
   std::cout<<Histos.size() <<"\t"<<Histos.at(0).size()<<std::endl;
   
   //All sis channels
   for (int i = 0; i< valid_channels.size(); i++)
   {
      std::string channel_name = valid_channels.at(i);
      
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



   //Make compound plots

   std::map<std::string,int> dumpIndex_counter;
   for (int j = 0; j < all.size(); j++)
   {
      TCanvas* c = new TCanvas("cSiPM","cSiPM", 1800, 1500);
      c->Divide(2,4);

      const TAGSpill s = all.at(j);

      std::string dump_name = s.GetSequenceName() + "_" + s.Name + "_" + std::to_string(dumpIndex_counter[s.GetSequenceName() + "_" + s.Name]++);
      // Remove quote marks... they upset uploading to elog
      dump_name.erase(std::remove(dump_name.begin(), dump_name.end(), '"'), dump_name.end());

      for (int i = 0; i< valid_channels.size(); i++)
      {
         // List of channels for the 8 plot combined canvas
         std::vector<std::string> channels {"SiPM_A","SiPM_D","SiPM_A_OR_D","SiPM_E",
                                           "SiPM_C","SiPM_F","SiPM_C_OR_F","ADC_TRG"};
         std::string channel_name = valid_channels.at(i);
         for ( int k = 0; k < channels.size(); k++ )
         {
            if (channels.at(k) == channel_name)
            {
               c->cd(k + 1);
               TH1D* h = Histos.at(i).at(j);
               h->GetXaxis()->SetTitle("Time [s]");
               h->GetYaxis()->SetTitle("Counts"); 
               h->Draw("HIST");
            }
         }
      }
      
      std::string folder = "AutoChronoPlots/";
      gSystem->mkdir(folder.c_str());
      folder += std::to_string(runNumber) + "/";
      gSystem->mkdir(folder.c_str());
      folder += "ChronoScintialltors/";
      gSystem->mkdir(folder.c_str());
      std::string filename = folder + "R" + std::to_string(runNumber) + std::string("_") + dump_name + ".png";
      c->Draw();
      c->SaveAs(filename.c_str());
      delete c;
   }
}


