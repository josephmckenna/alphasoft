#include "Plot_2018_243_Cooled_Lineshape.h"

#include "TA2Plot.h"
const Int_t nfreq(9), ncyc(200);
std::vector<TA2Plot*> Plot_243_Light_And_Dark_Lineshape(int runNumber, bool DrawVertices, bool ZeroTime)
{
  const double zcut=10.;
   
  TA2Plot* VertexPlot[nfreq][2];
  TA2Plot* DarkVertexPlot[nfreq][2];
  std::vector<double> ActualFreq;
  if ( runNumber==57181 || runNumber==57195 )
    ActualFreq={-200,-100,   -50, -25,   0., 25,   50,100, 200};
  if ( runNumber==57208 )
    ActualFreq={-100, -25, -12.5, -0., 12.5, 25, 37.5, 50, 100};
  std::vector<TA2Spill> spills;
  std::vector<std::pair<double,double>> DarkTimes[nfreq];       
  const std::vector<TA2Spill>& LaserSpills = Get_A2_Spills(runNumber,{"243 List*"},{-1});
    for (int i=0; i<nfreq; i++)
    {
      for (int j=0; j<2; j++)
        {
          char tit[80];
          VertexPlot[i][j]=new TA2Plot(-zcut,zcut,ZeroTime);
          sprintf(tit,"VertexPlot_%u_%u",i,j);
          VertexPlot[i][j]->SetTAPlotTitle(tit);
          DarkVertexPlot[i][j]=new TA2Plot(-zcut,zcut,ZeroTime);
          sprintf(tit,"DarkVertexPlot_%u_%u",i,j);
          DarkVertexPlot[i][j]->SetTAPlotTitle(tit);
        }
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
          std::cout<<"Add all instances of dump:\t"<<buf<<"\t as Frequency "<<i<<std::endl;
          spills = Get_A2_Spills(runNumber,{buf},{-1});
          //std::cout<<" 1st Light spill: "<<spills[0].GetStartTime()<<"\t"<<spills[0].GetStopTime()<<std::endl;
          VertexPlot[i][IsCState]->AddDumpGates(spills);
         
          //Find all dark times between the above light times

          double last_tmin = -1;
          double last_tmax = -1;

          //Note the user of the wild card to get all dumps!
          //std::cout<<"Dark Time windows \n";
         //Loop over our light windows
          for (const TA2Spill& s: spills)
            {
              double tmin = s.GetStartTime();
              double tmax = s.GetStopTime();
              //std::cout<<"Window: "<<i<<" Freq "<<Freq<<"\t"<<tmin<<"\t"<<tmax<<std::endl;

              // The loop over all windows to find the first start dump after our light (to calculate the dark)
              for (const TA2Spill& all: LaserSpills)
                {
                  if (all.GetStartTime() > tmax && all.GetStartTime() > tmin)
                    {
                      // Basic check for something wild going wrong
                      assert ( last_tmax < tmax );
                      // Check the dark period is shorter than 100 seconds
                         assert (  all.GetStartTime() - tmax  < 100 );
                         //  if(i==0)
                         //std::cout<<"\tAdding i "<<i<<"\t"<<tmax<<"\t"<<all.GetStartTime() << " Duration\t"<<all.GetStartTime()-tmax<< std::endl;
                      DarkTimes[i].push_back({ tmax, all.GetStartTime()});
                      last_tmax = tmax;
                      break;
                    }
                }
            }
          //std::cout<<i<<" dark# "<<DarkTimes[i].size()<<std::endl;
          for (const std::pair<double,double>& d: DarkTimes[i])
            {
              // std::cout <<"DEBUG: index "<<i<<" Dark time:\t"<< d.first <<"\t"<<d.second<<"\t " << d.second - d.first <<std::endl;
              DarkVertexPlot[i][IsCState]->AddTimeGate( runNumber,  d.first, d.second);
            }
          // std::cout<<"  LaserSpillsize "<<LaserSpills.size()<<std::endl;

        }
    }
  // std::vector<TA2Plot*> tmp;
  // return tmp;
  std::cout<<"Loading data..."<<std::endl;
  // Note, at time of writing, TTreeReader is only using 1 thread... but hey,
  // it only makes one pass over all files so its pretty quick
  // so this is slower than it could be
  TA2Plot_Filler DataLoader;
  for (int i=0; i<nfreq; i++)
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
      for (int i=0; i<nfreq; i++)
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

  std::vector<double> DarkStateCounts;
  std::vector<double> DStateErrors;
  std::vector<double> CStateErrors;

  std::vector<double> DarkStateErrors;
  std::vector<double> XErrors;
  for (int i=0; i<nfreq; i++)
    {
      DStateCounts.push_back(VertexPlot[i][0]->GetNPassedType(2));
      CStateCounts.push_back(VertexPlot[i][1]->GetNPassedType(2));
      
      DarkStateCounts.push_back(DarkVertexPlot[i][0]->GetNPassedType(2)
      +DarkVertexPlot[i][1]->GetNPassedType(2));

      DStateErrors.push_back(sqrt(VertexPlot[i][0]->GetNPassedType(2)));
      CStateErrors.push_back(sqrt(VertexPlot[i][1]->GetNPassedType(2)));
      
      DarkStateErrors.push_back(sqrt(DarkVertexPlot[i][0]->GetNPassedType(2)
      +DarkVertexPlot[i][1]->GetNPassedType(2)));
      XErrors.push_back( 0.);
    }
  
  std::cout<<"RunNumber: "<<runNumber<<std::endl;
  std::cout <<"FrequencyOffset\tDState\tCState\tDark"<<std::endl;
  int sumdl(0);
  int sumdk(0);
  int sumcl(0);
  for (int i=0; i<nfreq; i++)
    {  
      sumdl +=DStateCounts[i];
      sumdk+=DarkStateCounts[i];
      sumcl +=CStateCounts[i];
       std::cout<<ActualFreq[i];
      std::cout<<"\t\t"<<DStateCounts[i]<<"\t\t"<<CStateCounts[i];
      std::cout<<"\t\t"<<DarkStateCounts[i]<<std::endl;
    }
  std::cout<<"Total \t\t"<<sumdl<<"\t\t"<<sumcl<<"\t\t"<<sumdk<<"\t\t"<<std::endl;
  TString hfilename("hlaser");
  hfilename+=runNumber;
  hfilename+=".root";
  TFile* f=new TFile(hfilename,"RECREATE");
  TString canvasTitle="R";
  canvasTitle+=runNumber;
  TCanvas* c=new TCanvas(canvasTitle);
  c->Divide(1, 3);
  c->cd(1);
  TGraphErrors* DState=new TGraphErrors(nfreq,ActualFreq.data(),DStateCounts.data(), XErrors.data(),DStateErrors.data());
  DState->SetNameTitle("gfreqdd",canvasTitle+" D_State");
  DState->Draw("AP*");
  DState->Write();

  c->cd(3);
  TGraphErrors* DarkState=new TGraphErrors(nfreq,ActualFreq.data(),DarkStateCounts.data(), XErrors.data(),DarkStateErrors.data());
  DarkState->SetNameTitle("gdark",canvasTitle+" Dark Windows");
  DarkState->Draw("AP*");
  DarkState->Write();
  //c->cd(2);
  //hDState->Draw("HIST");
 
  c->cd(2);
  TGraphErrors* CState=new TGraphErrors(nfreq,ActualFreq.data(),CStateCounts.data(), XErrors.data(),CStateErrors.data());
  CState->SetNameTitle("gfreqcc",canvasTitle+" C_State");
  CState->Draw("AP*");
  CState->Write();   
  c->Update();
  c->SaveAs(title + ".png");
  // scheme for freqvtime vertex plots

  // get tstart and Tstop from  VertexPlot[i][0] times
  const TVertexEvents* kVertexEvents= VertexPlot[0][0]->GetVertexEvents();
  double tstart= kVertexEvents->fRunTimes[0]-2;
  double tend=kVertexEvents->fRunTimes[kVertexEvents->size()-1]+25;
  //std::cout<<"\ntstart\t"<<tstart<<"\ttend "<<tend<<"\tendvertexindex\t"<<kVertexEvents->size()<<std::endl;
  int nwindows=(runNumber==57208)?200:100;
  TString hTitle="Rep vs Freq_DStateR";
  hTitle+=runNumber;
  TH2D* hFreqRepD = new TH2D("hFTDL",hTitle,nfreq,-0.5,nfreq-0.5,nwindows,0.5, nwindows+.5);
  hTitle="Rep vs Freq_CStateR";
  hTitle+=runNumber;
  TH2D* hFreqRepC = new TH2D("hFTCL",hTitle,nfreq,-0.5,nfreq-0.5,nwindows,0.5,nwindows+0.5);
  hTitle="Rep vs Dark_StateR";
  hTitle+=runNumber;
  TH2D* hDarkRep = new TH2D("hFTDK",hTitle,nfreq,-0.5,nfreq-0.5,200,0.5,200+0.5);
  TH1D* hLongDumps=new TH1D("hLong","DarkDumpTimes", 500,0.,10.4);
  //Dark time durations histo
  TH2D* hDarkTimes= new TH2D("hDarkTimes","Dark Times vs freq vs cycle",nfreq,-.5, nfreq-.5,ncyc, .5,ncyc+.5);
  for (int i=0;i<LaserSpills.size()-1;i++)
    hDarkTimes->Fill( double(((i/nfreq)%2)?nfreq-1-i%nfreq:i%nfreq),double(i/nfreq+1), LaserSpills[i+1].GetStartTime()-LaserSpills[i].GetStopTime());
  
  bool IsLight;
  // We need separate loops over 4 kvertexevents loops, 
  for (int ifreq=0;ifreq<nfreq;ifreq++)
    {
      kVertexEvents =VertexPlot[ifreq][0]->GetVertexEvents();
      int lastSpill(0);
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            if (runNumber==57208) { 
              hFreqRepD->Fill(double(ifreq),GetRep(time,LaserSpills,lastSpill,IsLight) );
            }
            else {
              hFreqRepD->Fill(double(ifreq),GetRep1(time,LaserSpills,lastSpill,IsLight) );
            }
            hLongDumps->Fill(time-LaserSpills[lastSpill].GetStopTime());
            // std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<"\tlastspill "<<lastSpill<<" IsLight "<<IsLight<<" rep "<<GetRep1(time,LaserSpills,lastSpill,IsLight)<<std::endl;
          }
        }
     
      lastSpill=0;
      kVertexEvents =VertexPlot[ifreq][1]->GetVertexEvents(); 
      for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            if (!(runNumber==57208))
            hFreqRepC->Fill( double(ifreq),GetRep1(time,LaserSpills,lastSpill,IsLight) );
            hLongDumps->Fill(time-LaserSpills[lastSpill].GetStopTime());
          }
        }
      lastSpill=0;
      kVertexEvents =DarkVertexPlot[ifreq][0]->GetVertexEvents(); 
       for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          //std::cout<< "time  "<<time<<"\t rep "<<GetRep(time,LaserSpills,lastSpill,IsLight)<<std::endl;
          if (cutsResult &2) {
            hDarkRep->Fill( double(ifreq),GetRep(time,LaserSpills,lastSpill,IsLight) );
            hLongDumps->Fill(time-LaserSpills[lastSpill].GetStopTime());
            // std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<" lastSpill "<<lastSpill<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<" rep "<<GetRep(time,LaserSpills,lastSpill,IsLight)<<" IsLight "<<IsLight<<std::endl;
          }
        }
      lastSpill=0;
      kVertexEvents =DarkVertexPlot[ifreq][1]->GetVertexEvents(); 
       for (size_t ivt=0; ivt<kVertexEvents->fXVertex.size(); ivt++)
        {
          double time = kVertexEvents->fRunTimes[ivt];
          int cutsResult = kVertexEvents->fCutsResults[ivt];
          if (cutsResult &2) {
            hDarkRep->Fill( double(ifreq),GetRep(time,LaserSpills,lastSpill,IsLight) );
            hLongDumps->Fill(time-LaserSpills[lastSpill].GetStopTime());
            //std::cout<<"ifreq\t"<<ifreq<<"\t time "<<time<<"\tz\t"<<kVertexEvents->fZVertex[ivt]<<" spill "<<lastSpill<<std::endl;
          }
        }
 
    }
  // create the cycle number Tgraphs here from the 2d projection.
  TGraphErrors* gkoutdd = new TGraphErrors(hFreqRepD->ProjectionY());
  gkoutdd->SetName("gkoutdd");
  gkoutdd->GetXaxis()->SetTitle("Repetition");
  gkoutdd->Write();
   TGraphErrors* gkoutcc = new TGraphErrors(hFreqRepC->ProjectionY());
  gkoutcc->SetName("gkoutcc");
  gkoutcc->GetXaxis()->SetTitle("Repetition");
  gkoutcc->Write();
  TGraphErrors* gkoutdark = new TGraphErrors(hDarkRep->ProjectionY());
  gkoutdark->SetName("gkoutdark");
  gkoutdark->GetXaxis()->SetTitle("Repetition");
  gkoutdark->Write();
  hLongDumps->Write();
  hDarkTimes->Write();

  canvasTitle="FTR";
  canvasTitle+=runNumber;
  TCanvas* c3=new TCanvas(canvasTitle);
  c3->Divide(3, 1);
  c3->cd(1);
  hFreqRepD->Draw("BOX");
  hFreqRepD->Write();
  c3->cd(2);
  hFreqRepC->Draw("BOX");
  hFreqRepC->Write();
  c3->cd(3);
  hDarkRep->Draw("BOX");
  hDarkRep->Write();
  c3->Update();
  c3->SaveAs(canvasTitle+".png");
  f->Close();
  std::cout<<"Histograms written to "<<hfilename<<std::endl;
 

  // Return all plots in case anyone wants do do post-post processing
  std::vector<TA2Plot*> plots;
  for (int i=0; i<nfreq; i++)
    {
      for (int j=0; j<1; j++)
        {
          plots.push_back(VertexPlot[i][j]);
          plots.push_back(DarkVertexPlot[i][j]);
        }
    }
  return plots;
}
double GetRep(double time, const std::vector<TA2Spill>& LaserSpills, int& lastSpill, bool& IsLight)
{
    // std::cout<<"nspills\t"<<LaserSpills.size()<<std::endl;
  //Get vector of starttimes need to add this to TAPlot.h TTWindows getter.
  for (int i=lastSpill;i<LaserSpills.size();i++)
    {    if(time<LaserSpills[i].GetStartTime())
        { lastSpill =i-1;
          IsLight=time<LaserSpills[lastSpill].GetStopTime();
          // std::cout<< "Rep: "<< lastSpill/nfreq+1<<" spillno-time"<< LaserSpills[i].GetStartTime()-time<<" lastspill "<<lastSpill<<" IsLight ",IsLight,"\n";
          return double(lastSpill/nfreq)+1;
        }
     }
  return 0.; 
}
double GetRep1(double time, const std::vector<TA2Spill>& LaserSpills, int& lastSpill, bool& IsLight)
{
    // std::cout<<"nspills\t"<<LaserSpills.size()<<std::endl;
  //For run 57181, 57195 light plots
  for (int i=lastSpill;i<LaserSpills.size();i++)
    {    if(time<LaserSpills[i].GetStartTime())
        { lastSpill =i-1;
          IsLight=time<LaserSpills[lastSpill].GetStopTime();
          // std::cout<< "Rep: "<<(lastSpill/4/nfreq)*2+(lastSpill/nfreq)%2+1 <<" spillno-time"<< LaserSpills[i].GetStartTime()-time<<" lastspill "<<lastSpill<<" IsLight ",IsLight,"\n";
          return double((lastSpill/4/nfreq)*2+(lastSpill/nfreq)%2+1);
        }
     }
  return 0.; 
}

int Plot_2018_243_Cooled_Lineshape(bool DrawVerticecs, bool zeroTime)
{   
   std::vector<TA2Plot*> R57181plots = Plot_243_Light_And_Dark_Lineshape(57181, DrawVerticecs, zeroTime);
   std::vector<TA2Plot*> R57195plots = Plot_243_Light_And_Dark_Lineshape(57195,DrawVerticecs, zeroTime);
   std::vector<TA2Plot*> R57208plots = Plot_243_Light_And_Dark_Lineshape(57208,DrawVerticecs, zeroTime);
   
   //Example of summing all plots together (not sure why you might want to do it... but its possible)
   // TA2Plot Light(zeroTime);
   // TA2Plot Dark(zeroTime);
   // for ( size_t i = 0; i< R57181plots.size(); i++)
   // {
   //    if ( i % 2 == 0)
   //       // Operator overloads dont expect a pointer, de-reference the TA2Plot with *
   //       Light += *R57181plots.at(i);
   //    else
   //       Dark += *R57181plots.at(i);
   // }
   
   // //Re-bin new histograms to desired granularity
   // Light.SetBinNumber(40);
   // Dark.SetBinNumber(40);

   // TCanvas* c1 = Light.DrawCanvas("All Light Vertex for R57181");
   // c1->SaveAs("All_Light_Vertex_for_R57181.png");

   // TCanvas* c2 = Dark.DrawCanvas("All Dark Vertex for R57181");
   // c2->SaveAs("All_Dark_Vertex_for_R57181.png");
   

   //Note: Missing in this macro:
   // 2. Any kind of MVA
   
   //Missing in TA2Plot:
   //Multithreaded TTreeReader?
   
   return 0;
}

