#include "RootUtils.h"
#include "PlotGetters.h"
#include "TSISChannels.h"

void SaveAllDumps(int runNumber)
{
   std::vector<TA2Spill> all = Get_A2_Spills(runNumber,{"*"},{-1});
   //Get this list from the root file! (or ODB)
   std::vector<std::string> detector_channels = {
         "SIS_PMT_CATCH_OR",
         "SIS_PMT_CATCH_AND",
         "CT_SiPM1",
         "CT_SiPM2",
         "CT_SiPM_OR",
         "CT_SiPM_AND"};
   std::vector<int> chans;
   for (std::string channel_name: detector_channels)
   {
      TSISChannels* sisch = new TSISChannels(runNumber);
      chans.push_back(sisch->GetChannel(channel_name.c_str()));
      delete sisch;
   }

   std::vector<std::vector<TH1D*>> Histos = Get_SIS(runNumber,chans,all);
   std::cout<<Histos.size() <<"\t"<<Histos.at(0).size()<<std::endl;
   
   //All sis channels
   for (int i = 0; i< detector_channels.size(); i++)
   {
      std::string channel_name = detector_channels.at(i);
      
      std::map<std::string,int> repetition_counter;
    
      TSISChannels* sisch = new TSISChannels(runNumber);
      int ch = sisch->GetChannel(channel_name.c_str());
      delete sisch;

      std::cout<<channel_name <<"\tis sis channel\t"<<ch<<std::endl;
      if (ch<0)
         continue;
      
      for (int j = 0; j < all.size(); j++)
      {
         TA2Spill s = all.at(j);
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
         if (!s.ScalerData->DetectorCounts.at(ch))
         {
            std::cout<<s.Name << " has no counts in channel " << ch<<" ... skipping plot" <<std::endl;
            continue;
         }
         std::string dump_name = s.Name + "_" + std::to_string(repetition_counter[s.Name]++);
         // Remove quote marks... they upset uploading to elog
         dump_name.erase(std::remove(dump_name.begin(), dump_name.end(), '"'), dump_name.end());
         std::cout << "dump_name:"<< channel_name << "/"<< dump_name <<std::endl;

         TCanvas* c = new TCanvas();
         Histos.at(i).at(j)->Draw("HIST");

         std::string folder = "AutoSISPlots/";
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


void SaveAllDumpsSVD(int runNumber, bool SaveEmpty = false)
{
   std::vector<TA2Spill> all = Get_A2_Spills(runNumber,{"*"},{-1});
   
   std::vector<TA2Plot*> plots;
   std::vector<std::string> plot_names;
   TA2Plot_Filler DataLoader;
   
   
   std::map<std::string,int> repetition_counter;
   if (all.empty())
   {
      double svd = GetTotalRunTimeFromSVD(runNumber);
      double sis = GetTotalRunTimeFromSIS(runNumber);
      double tmax;
      if ( sis > svd)
         tmax = sis;
      else
         tmax = svd;
      TA2Plot* a = new TA2Plot();
      a->AddTimeGate(runNumber, 0, tmax);
      plots.push_back(a);
      DataLoader.BookPlot(a);
   }
   else
   {
      for (int j = 0; j < all.size(); j++)
      {
         TA2Spill s = all.at(j);
 
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

         std::string dump_name = s.Name + "_" + std::to_string(repetition_counter[s.Name]++);
         // Remove quote marks... they upset uploading to elog
         dump_name.erase(std::remove(dump_name.begin(), dump_name.end(), '"'), dump_name.end());
         std::cout << "dump_name:"<< dump_name <<std::endl;

         TA2Plot* a = new TA2Plot();
         a->AddDumpGates(runNumber, { s });
         DataLoader.BookPlot(a);

         plots.push_back(a);
         plot_names.push_back(dump_name);
      }
   }
   DataLoader.LoadData();
   
   std::string folder = "AutoSISPlots/";
   gSystem->mkdir(folder.c_str());
   folder += std::to_string(runNumber) + "/";
   gSystem->mkdir(folder.c_str());
   folder += "SVD/";
   gSystem->mkdir(folder.c_str());
      
   if (all.empty())
   {
      std::cout <<"No dumps found... Save as:";
      std::string filename = folder + "R" + std::to_string(runNumber) + std::string("_EntireRun.png");
      std::cout <<filename<<std::endl;
      plots.front()->DrawCanvas()->SaveAs(filename.c_str());
      std::cout<<"Done"<<std::endl;
   }
   else
   {
      for (int j = 0; j < plots.size(); j++)
      {   
         std::string filename = folder + "R" + std::to_string(runNumber) + std::string("_") + plot_names.at(j) + ".png";
         if (SaveEmpty)
         {
            plots.at(j)->DrawCanvas()->SaveAs(filename.c_str());
         }
         else
         {
            if (plots.at(j)->GetVertexEvents()->CountPassedCuts(1))
               plots.at(j)->DrawCanvas()->SaveAs(filename.c_str());
         }
      }
   }
}
