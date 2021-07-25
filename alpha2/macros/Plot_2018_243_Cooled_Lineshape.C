#include "Plot_2018_243_Cooled_Lineshape.h"

#include "TA2Plot.h"

void Plot_243_Light_Lineshape(int runNumber, bool DrawVertices, bool ZeroTime)
{
   double zcut=10.;
   
   TA2Plot* VertexPlot[9][2];
   TA2Plot* DarkVertexPlot[9][2];
   std::vector<double> ActualFreq;
   if ( runNumber==57181 || runNumber==57195 )
      ActualFreq={-200,-100,   -50, -25,   0., 25,   50,100, 200};
   if ( runNumber==57208 )
      ActualFreq={-100, -25, -12.5, -0., 12.5, 25, 37.5, 50, 100};
         
   for (int i=0; i<9; i++)
   {
      VertexPlot[i][0]=new TA2Plot(-zcut,zcut,ZeroTime);
      VertexPlot[i][1]=new TA2Plot(-zcut,zcut,ZeroTime);
      DarkVertexPlot[i][0]=new TA2Plot(-zcut,zcut,ZeroTime);
      DarkVertexPlot[i][1]=new TA2Plot(-zcut,zcut,ZeroTime);

      
      std::cout<<"Populating frequency "<<i<<std::endl;

      for (int k=0; k<4; k++)
      {
         int Freq=i;
         if (k==1 || k==3)
            Freq=8-i;
         char buf[80];
         sprintf(buf,"243 List %u Freq %u",k,Freq);
         
         int IsCState=0;
         if ((k>1)&&(runNumber!=57208))
            IsCState=1;
         std::cout<<"Add all instances of dump:\t"<<buf<<"\t as Frequency "<<i;
         if (!IsCState)
            std::cout<<" D state ";
         else
            std::cout<<" C state ";
         std::cout<<"("<<ActualFreq[i]<<" offset)"<<std::endl;
         std::vector<TA2Spill> spills = Get_A2_Spills(runNumber,{buf},{-1});
         
         VertexPlot[i][IsCState]->AddDumpGates(spills);
         
         std::vector<std::pair<double,double>> DarkTimes;
         double last_tmin = -1;
         double last_tmax = -1;

         //Find all dark times between the above light times
         std::vector<TA2Spill> AllSpills = Get_A2_Spills(runNumber,{"*"},{-1});
         for (const TA2Spill& s: spills)
         {
            double tmin = s.GetStartTime();
            double tmax = s.GetStopTime();
            //std::cout<<"Window:\t"<<tmin<<"\t"<<tmax<<std::endl;
            for (const TA2Spill& all: AllSpills)
            {
               
               if (all.GetStartTime() > tmax && all.GetStartTime() > tmin)
               {
                  // Basic check for something wild going wrong
                  assert ( last_tmax < tmax );
                  // Check the dark period is shorter than 10 seconds
                  assert (  all.GetStartTime() - tmax  < 10 );
                  //std::cout<<"\tAdding "<<tmax<<"\t"<<all.GetStartTime() << std::endl;
                  DarkTimes.push_back({ tmax, all.GetStartTime()});
                  last_tmax = tmax;
                  break;
               }
            }
         }

         for (const std::pair<double,double>& d: DarkTimes)
         {
            //std::cout <<"DEBUG: Dark time:\t"<< d.first <<"\t"<<d.second<<"\t " << d.second - d.first <<std::endl;
            DarkVertexPlot[i][IsCState]->AddTimeGate( runNumber,  d.first, d.second);
         }


      }
   }

   std::cout<<"Loading data..."<<std::endl;
   // Note, at time of writing, TTreeReader is only using 1 thread... 
   // so this is slower than it could be
   /*for (int i=0; i<9; i++)
   {
      for (int j=0; j<2; j++) //IsCState?
      {
         // Load Data only makes one pass over each tree... 
         // so its reasonably fast... 
         VertexPlot[i][j]->LoadData();
         std::cout<<i*2+1+j<<"/"<<2*9<<" loaded..."<<std::endl;
      }
   }*/

   TA2Plot_Filler DataLoader;
   for (int i=0; i<9; i++)
   {
      for (int j=0; j<2; j++) //IsCState?
      {
         DataLoader.BookPlot(VertexPlot[i][j]);
         DataLoader.BookPlot(DarkVertexPlot[i][j]);
      }
   }
   DataLoader.SetLVChannel("D243",2);
   //Load all data in a single pass of each tree (I am aiming for efficiency)
   DataLoader.LoadData();

   // Optional 'Draw' command to show full vertex plots
   if (DrawVertices)
   {
      for (int i=0; i<9; i++)
      {
         for (int j=0; j<2; j++)
         {
            TString title="Freq";
            title+=i;
            title+="_State:";
            if (j==0)
               title+="D";
            if (j==1)
               title+="C";
            if (runNumber==57208 && j==1)
            {
               continue;
            }
            TCanvas* c1 = VertexPlot[i][j]->DrawCanvas(title);
            TString save_as = "R";
            save_as += runNumber;
            save_as += title;
            save_as += ".png";
            c1->SaveAs(save_as);

            TCanvas* c2 = DarkVertexPlot[i][j]->DrawCanvas(TString("Dark") + title);
            save_as = "R";
            save_as += runNumber;
            save_as += title;
            save_as += ".png";
            c2->SaveAs(save_as);

         }
      }
   }
   // In the near future you will be able to save these plots to file...
   // So the user only need use 'LoadData' once if they are smart


   TString title="R";
   title+=runNumber;
   title+="_Lineshape";
   
   //TH1D* hDState=new TH1D(title+"_DState",title+"_DState",9,0.,9.);
   //TH1D* hCState=new TH1D(title+"_CState",title+"_CState",9,0.,9.);
   std::vector<double> DStateCounts;
   std::vector<double> CStateCounts;

   std::vector<double> DarkDStateCounts;
   std::vector<double> DarkCStateCounts;
   for (int i=0; i<9; i++)
   {
      DStateCounts.push_back(VertexPlot[i][0]->GetNPassedCuts());
      CStateCounts.push_back(VertexPlot[i][1]->GetNPassedCuts());
      
      DarkDStateCounts.push_back(DarkVertexPlot[i][0]->GetNPassedCuts());
      DarkCStateCounts.push_back(DarkVertexPlot[i][1]->GetNPassedCuts());
      
      //hDState->Fill(i,VertexPlot[i][0]->GetNPassedCuts());

      //hCState->Fill(i,VertexPlot[i][1]->GetNPassedCuts());
   }
   TString canvasTitle="R";
   canvasTitle+=runNumber;
   TCanvas* c=new TCanvas(canvasTitle);
   c->Divide(2, 2);
   c->cd(1);
   TGraph* DState=new TGraph(9,ActualFreq.data(),DStateCounts.data());
   DState->SetTitle(canvasTitle+" D_State");
   DState->Draw("AP*");

   c->cd(2);
   TGraph* DarkDState=new TGraph(9,ActualFreq.data(),DarkDStateCounts.data());
   DarkDState->SetTitle(canvasTitle+" Dark D_State");
   DarkDState->Draw("AP*");

   //c->cd(2);
   //hDState->Draw("HIST");
   std::cout<<"RunNumber: "<<runNumber<<std::endl;
   std::cout <<"FrequencyOffset\tDStateCounts\tCStateCounts\tDarkDStateCounts\tDArkCStateCounts"<<std::endl;
   for (int i=0; i<9; i++)
   {
      std::cout<<ActualFreq[i];
      std::cout<<"\t"<<DStateCounts[i]<<"\t"<<CStateCounts[i];
      std::cout<<"\t"<<DarkDStateCounts[i]<<"\t"<<DarkCStateCounts[i];
   }

   c->cd(3);
   TGraph* CState=new TGraph(9,ActualFreq.data(),CStateCounts.data());
   CState->SetTitle(canvasTitle+" C_State");
   CState->Draw("AP*");
   
   c->cd(4);
   TGraph* DarkCState=new TGraph(9,ActualFreq.data(),DarkCStateCounts.data());
   DarkCState->SetTitle(canvasTitle+" Dark C_State");
   DarkCState->Draw("AP*");
   
   
   //c->cd(4);
   //hCState->Draw("HIST");
   c->Update();
   c->SaveAs(title + ".png");
   return;
}


int Plot_2018_243_Cooled_Lineshape(bool DrawVerticecs, bool zeroTime)
{
   
   
   Plot_243_Light_Lineshape(57181,DrawVerticecs, zeroTime);
   Plot_243_Light_Lineshape(57195,DrawVerticecs, zeroTime);
   Plot_243_Light_Lineshape(57208,DrawVerticecs, zeroTime);
   
   //Note: Missing in this macro:
   // 1. Dark Periods
   // 2. Any kind of MVA
   
   //Missing in TA2Plot:
   //Multithreaded TTreeReader
   //Proper plotting of SIS data (although it is in the object)
   //Saving TA2Plot to file (as an object, it contains the full detail,
   // all time stamps etc)
   
   return 0;
}

