#include "Plot_2018_243_Cooled_Lineshape.h"

#include "TA2Plot.h"

std::vector<TA2Plot*> Plot_243_Light_And_Dark_Lineshape(int runNumber, bool DrawVertices, bool ZeroTime)
{
  double zcut=10.;
   
  TA2Plot* VertexPlot[9][2];
  TA2Plot* DarkVertexPlot[9][2];
  std::vector<double> ActualFreq;
  if ( runNumber==57181 || runNumber==57195 )
    ActualFreq={-200,-100,   -50, -25,   0., 25,   50,100, 200};
  if ( runNumber==57208 )
    ActualFreq={-100, -25, -12.5, -0., 12.5, 25, 37.5, 50, 100};
  std::vector<TA2Spill> spills;
  std::vector<std::pair<double,double>> DarkTimes[9];       
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
          spills = Get_A2_Spills(runNumber,{buf},{-1});
         
          VertexPlot[i][IsCState]->AddDumpGates(spills);
         
          //Find all dark times between the above light times

          double last_tmin = -1;
          double last_tmax = -1;

          //Note the user of the wild card to get all dumps!
          const std::vector<TA2Spill>& AllSpills = Get_A2_Spills(runNumber,{"*"},{-1});
          //Loop over our light windows
          for (const TA2Spill& s: spills)
            {
              double tmin = s.GetStartTime();
              double tmax = s.GetStopTime();
              //std::cout<<"Window:\t"<<tmin<<"\t"<<tmax<<std::endl;

              // The loop over all windows to find the first start dump after our light (to calculate the dark)
              for (const TA2Spill& all: AllSpills)
                {
                  if (all.GetStartTime() > tmax && all.GetStartTime() > tmin)
                    {
                      // Basic check for something wild going wrong
                      assert ( last_tmax < tmax );
                      // Check the dark period is shorter than 100 seconds
                      //   assert (  all.GetStartTime() - tmax  < 100 );
                      //std::cout<<"\tAdding "<<tmax<<"\t"<<all.GetStartTime() << std::endl;
                      DarkTimes[i].push_back({ tmax, all.GetStartTime()});
                      last_tmax = tmax;
                      break;
                    }
                }
            }

          for (const std::pair<double,double>& d: DarkTimes[i])
            {
              //std::cout <<"DEBUG: Dark time:\t"<< d.first <<"\t"<<d.second<<"\t " << d.second - d.first <<std::endl;
              DarkVertexPlot[i][IsCState]->AddTimeGate( runNumber,  d.first, d.second);
            }
          // std::cout<<"  allspillsize "<<AllSpills.size()<<std::endl;
          // std::cout<<"freq#  "<<i<<"  spillsize "<<spills.size()<<std::endl;


        }
    }
  // std::vector<TA2Plot*> tmp;
  // return tmp;
  std::cout<<"Loading data..."<<std::endl;
  // Note, at time of writing, TTreeReader is only using 1 thread... but hey,
  // it only makes one pass over all files so its pretty quick
  // so this is slower than it could be
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
              title+="_State";
              if (j==0)
                title+="D";
              if (j==1)
                title+="C";
              if (runNumber==57208 && j==1)
                {
                  continue;
                }
              TCanvas* c1 = VertexPlot[i][j]->DrawCanvas(title, true,2);
              TString save_as = "R";
              save_as += runNumber;
              save_as += title;
              save_as += ".png";
              c1->SaveAs(save_as.Data());

              TCanvas* c2 = DarkVertexPlot[i][j]->DrawCanvas(TString("Dark") + title,true,2);
              save_as = "R";
              save_as += runNumber;
              save_as += "Dark";
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
      DStateCounts.push_back(VertexPlot[i][0]->GetNPassedType(2));
      CStateCounts.push_back(VertexPlot[i][1]->GetNPassedType(2));
      
      DarkDStateCounts.push_back(DarkVertexPlot[i][0]->GetNPassedType(2));
      DarkCStateCounts.push_back(DarkVertexPlot[i][1]->GetNPassedType(2));
      
    }
  std::cout<<"RunNumber: "<<runNumber<<std::endl;
  std::cout <<"FrequencyOffset\tDStateCounts\tCStateCounts\tDarkDStateCounts\tDArkCStateCounts"<<std::endl;
  int sumdl(0);
  int sumdd(0);
  int sumcl(0);
  int sumcd(0);
  for (int i=0; i<9; i++)
    {  
      sumdl +=DStateCounts[i];
      sumdd+=DarkDStateCounts[i];
      sumcl +=CStateCounts[i];
      sumcd+=DarkCStateCounts[i];
      std::cout<<ActualFreq[i];
      std::cout<<"\t\t"<<DStateCounts[i]<<"\t\t"<<CStateCounts[i];
      std::cout<<"\t\t"<<DarkDStateCounts[i]<<"\t\t"<<DarkCStateCounts[i]<<std::endl;
    }
  std::cout<<"Total \t\t"<<sumdl<<"\t\t"<<sumcl<<"\t\t"<<sumdd<<"\t\t"<<sumcd<<std::endl;
  TString hfilename("hlaserft");
  hfilename+=runNumber;
  hfilename+=".root";
  TFile* f=new TFile(hfilename,"RECREATE");
  TString canvasTitle="R";
  canvasTitle+=runNumber;
  TCanvas* c=new TCanvas(canvasTitle);
  c->Divide(2, 2);
  c->cd(1);
  TGraph* DState=new TGraph(9,ActualFreq.data(),DStateCounts.data());
  DState->SetNameTitle("gfreqdd",canvasTitle+" D_State");
  DState->Draw("AP*");
  DState->Write();

  c->cd(2);
  TGraph* DarkDState=new TGraph(9,ActualFreq.data(),DarkDStateCounts.data());
  DarkDState->SetNameTitle("gdarkdd",canvasTitle+" Dark D_State");
  DarkDState->Draw("AP*");
  DarkDState->Write();
  //c->cd(2);
  //hDState->Draw("HIST");
 
  c->cd(3);
  TGraph* CState=new TGraph(9,ActualFreq.data(),CStateCounts.data());
  CState->SetNameTitle("gfreqcc",canvasTitle+" C_State");
  CState->Draw("AP*");
  CState->Write();   
  c->cd(4);
  TGraph* DarkCState=new TGraph(9,ActualFreq.data(),DarkCStateCounts.data());
  DarkCState->SetNameTitle("gdarkcc",canvasTitle+" Dark C_State");
  DarkCState->Draw("AP*");
  DarkCState->Write();
  c->Update();
  c->SaveAs(title + ".png");
  // scheme for freqvtime vertex plots

  // get tstart and Tstop from  VertexPlot[i][0] times
  const TVertexEvents* kVertexEvents= VertexPlot[0][0]->GetVertexEvents();
  double tstart= kVertexEvents->fRunTimes[0]-1;
  double tend=kVertexEvents->fRunTimes[kVertexEvents->size()-1]+20;
  std::cout<<"\ntstart\t"<<tstart<<"\ttend "<<tend<<"\tendindex\t"<<kVertexEvents->size()<<std::endl;
  int nwindows=(runNumber==57208)?200:100;
  TString hTitle="Rep vs Freq_DStateR";
  hTitle+=runNumber;
  TH2D* hFreqTimeD = new TH2D("hFTDL",hTitle,9,-0.5,8.5,nwindows,0.5, nwindows+.5);
  hTitle="Time vs Freq_CStateR";
  hTitle+=runNumber;
  TH2D* hFreqTimeC = new TH2D("hFTCL",hTitle,9,-0.5,8.5,nwindows,0.5,nwindows+0.5);
  hTitle="Time vs Dark_DStateR";
  hTitle+=runNumber;
  TH2D* hDarkTimeD = new TH2D("hFTDD",hTitle,9,-0.5,8.5,nwindows,0.5,nwindows+0.5);
  hTitle="Time vs Dark_CStateR";
  hTitle+=runNumber;
  TH2D* hDarkTimeC = new TH2D("hFTCD",hTitle,9,-0.5,8.5,nwindows,0.5,nwindows+0.5);
  TTimeWindows* timeWindows = new TTimeWindows();
  // We need separate loops over 4 kvertexevents loops, 
  for (int ifreq=0;ifreq<9;ifreq++)
    {
      kVertexEvents =VertexPlot[ifreq][0]->GetVertexEvents();
      timeWindows =VertexPlot[ifreq][0]->GetTimeWindows();
      int lastRep(0);
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          //hFreqTimeD->Fill( double(ifreq),time);
          if (cutsResult &2) {
            //              hFreqTimeD->Fill( double(ifreq),time);
            hFreqTimeD->Fill(double(ifreq),GetRep(time,timeWindows,lastRep) );
            //std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<"\tlastrep "<<lastRep<<std::endl;
          }
        }
      lastRep=0;
      kVertexEvents =VertexPlot[ifreq][1]->GetVertexEvents(); 
      timeWindows =VertexPlot[ifreq][1]->GetTimeWindows();
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            hFreqTimeC->Fill( double(ifreq),GetRep(time,timeWindows,lastRep));
          }
        }
      lastRep=0;
      kVertexEvents =DarkVertexPlot[ifreq][0]->GetVertexEvents(); 
      timeWindows =DarkVertexPlot[ifreq][0]->GetTimeWindows();
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            hDarkTimeD->Fill( double(ifreq),GetRep(time,timeWindows,lastRep));
            //std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<std::endl;
          }
        }
      lastRep=0;
      kVertexEvents =DarkVertexPlot[ifreq][1]->GetVertexEvents(); 
      timeWindows =DarkVertexPlot[ifreq][1]->GetTimeWindows();
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            hDarkTimeC->Fill( double(ifreq),GetRep(time,timeWindows,lastRep));
            //std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<std::endl;
          }
        }
      
    }
  // create the cycle number Tgraphs here from the 2d projection.
  TGraph* gkoutdd = new TGraph(hFreqTimeD->ProjectionY());
  gkoutdd->SetName("gkoutdd");
  gkoutdd->GetXaxis()->SetTitle("Repetition");
  gkoutdd->Write();
   TGraph* gkoutcc = new TGraph(hFreqTimeC->ProjectionY());
  gkoutcc->SetName("gkoutcc");
  gkoutcc->GetXaxis()->SetTitle("Repetition");
  gkoutcc->Write();
  TGraph* gkoutddark = new TGraph(hDarkTimeD->ProjectionY());
  gkoutddark->SetName("gkoutddark");
  gkoutddark->GetXaxis()->SetTitle("Repetition");
  gkoutddark->Write();
  TGraph* gkoutcdark = new TGraph(hDarkTimeC->ProjectionY());
  gkoutcdark->SetName("gkoutcdark");
  gkoutcdark->GetXaxis()->SetTitle("Repetition");
  gkoutcdark->Write();
  

  canvasTitle="FTR";
  canvasTitle+=runNumber;
  TCanvas* c3=new TCanvas(canvasTitle);
  c3->Divide(2, 2);
  c3->cd(1);
  hFreqTimeD->Draw("BOX");
  hFreqTimeD->Write();
  c3->cd(2);
  hFreqTimeC->Draw("BOX");
  hFreqTimeC->Write();
  c3->cd(3);
  hDarkTimeD->Draw("BOX");
  hDarkTimeD->Write();
  c3->cd(4);
  hDarkTimeC->Draw("BOX");
  hDarkTimeC->Write();
  c3->Update();
  c3->SaveAs(canvasTitle+".png");
   
  // Open a root file and write >HF2D's to it;
  //  f->Write();  
  f->Close();
 

  // Return all plots in case anyone wants do do post-post processing
  std::vector<TA2Plot*> plots;
  for (int i=0; i<9; i++)
    {
      for (int j=0; j<1; j++)
        {
          plots.push_back(VertexPlot[i][j]);
          plots.push_back(DarkVertexPlot[i][j]);
        }
    }
  return plots;
}
double GetRep(double time, TTimeWindows* timeWindows, int& lastRep)
{
  //  std::cout<<"nspills\t"<<timeWindows->fMinTime.size()<<std::endl;
  //Get vector of starttimes need to add this to TAPlot.h TTWindows getter.
  for (int i=lastRep;i<timeWindows->fMinTime.size();i++)
    {    if(time<timeWindows->fMinTime[i])
        {lastRep =i-1;
          // std::cout<< "Rep: "<< i<<" spillno-time"<< timeWindows->fMinTime[i]-time<<" lastrep "<<lastRep<<"\n";
          return double(i);
        }
           // std::cout<<" i "<<i<<" Spillno-time "<< timeWindows->fMinTime[i]-time<<"\t ";
    }
  return 0.; 
}

int Plot_2018_243_Cooled_Lineshape(bool DrawVerticecs, bool zeroTime)
{
   
   
   std::vector<TA2Plot*> R57181plots = Plot_243_Light_And_Dark_Lineshape(57181,DrawVerticecs, zeroTime);
   std::vector<TA2Plot*> R57195plots = Plot_243_Light_And_Dark_Lineshape(57195,DrawVerticecs, zeroTime);
   std::vector<TA2Plot*> R57208plots = Plot_243_Light_And_Dark_Lineshape(57208,DrawVerticecs, zeroTime);
   
   //Example of summing all plots together (not sure why you might want to do it... but its possible)
   TA2Plot Light(zeroTime);
   TA2Plot Dark(zeroTime);
   for ( size_t i = 0; i< R57181plots.size(); i++)
   {
      if ( i % 2 == 0)
         // Operator overloads dont expect a pointer, de-reference the TA2Plot with *
         Light += *R57181plots.at(i);
      else
         Dark += *R57181plots.at(i);
   }
   
   //Re-bin new histograms to desired granularity
   Light.SetBinNumber(40);
   Dark.SetBinNumber(40);

   TCanvas* c1 = Light.DrawCanvas("All Light Vertex for R57181");
   c1->SaveAs("All_Light_Vertex_for_R57181.png");

   TCanvas* c2 = Dark.DrawCanvas("All Dark Vertex for R57181");
   c2->SaveAs("All_Dark_Vertex_for_R57181.png");
   

   //Note: Missing in this macro:
   // 2. Any kind of MVA
   
   //Missing in TA2Plot:
   //Multithreaded TTreeReader?
   
   return 0;
}

