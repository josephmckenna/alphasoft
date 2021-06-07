
// Other headers are going to need this flag set...
// bit hacky and this should just load from BuildConfig.h
#ifndef BUILD_A2 
#define BUILD_A2
#endif

#include "LyAlpha.h"
#include "TA2Plot.h"
#include "TA2Plot_Filler.h"



void PlotLyAlphaScan(int runNumber = 57019, int n_dumps = -1)
{

    TA2Plot* freq[9];

    for (int i=0; i<9; i++)
    {
        std::cout<<"Adding Freq "<<i<<std::endl;
        freq[i] = new TA2Plot(-10.,10.);
    
    
        std::string dump="Lyman Alpha Freq "+ std::to_string(i);
        std::vector<TA2Spill> dumps=Get_A2_Spills(runNumber,{dump},{n_dumps});
        std::cout<<dumps.size() << " dumps found"<<std::endl;
        std::vector<std::pair<double,int>> qpulses=GetSISTimeAndCounts(
           runNumber,  //Run Number
           "QPULSE",   //SIS CHannel name (you can also use the number, in this case "QPULSE" == 53)
           dumps
        );
        std::cout<<qpulses.size()<< " pulses found"<<std::endl;
        for (std::pair<double,int>& pulse: qpulses)
        {
            //std::cout<<pulse.first-0.01<<std::endl;
            freq[i]->AddTimeGate(
                runNumber,
                pulse.first-0.01, //Min time (before pulse)
                pulse.first + 0.09, //Max time (after pulse)
                pulse.first); //Definition of 'zero' time
        }
    }

    TA2Plot_Filler DataLoader;
    for (int i=0; i<9; i++)
    {
        DataLoader.BookPlot(freq[i]);
    }
    DataLoader.SetLVChannel("121P",6);
    std::cout<<"Loading data"<<std::endl;
    //Load all data in a single pass of each tree (I am aiming for efficiency)
    DataLoader.LoadData();

    std::vector<double> ActualFreq={1.,2.,3.,4.,5.,6.,7.,8.};
    std::vector<double> Counts;
    for (int i=0; i<9; i++)
    {
        Counts.push_back(freq[i]->GetNPassedCuts());
    }
    TString canvasTitle="R";
    canvasTitle+=runNumber;
    TCanvas* c1=new TCanvas(canvasTitle);
    TGraph* shape=new TGraph(9,ActualFreq.data(),Counts.data());
    shape->SetTitle(canvasTitle+" ");
    shape->Draw("AP*");

    TCanvas* c[9];
    for (int i=0; i<9; i++)
    {
        c[i]=freq[i]->DrawCanvas();
        TString name="R";
        name+=runNumber;
        name+="_Freq";
        name+=i;
        name+=".png";
        c[i]->SaveAs(name);
    }
    return;
}


int LyAlpha()
{
    PlotLyAlphaScan();
    return 0;
}