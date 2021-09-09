
#include "TGraph.h"

void PMT_Comparison(int runNumber)
{
  gStyle->SetOptFit(111);
  gStyle->SetStatY(0.4);                
  // Set y-position (fraction of pad size)
  gStyle->SetStatX(0.85);                
  // Set x-position (fraction of pad size)
  gStyle->SetStatW(0.2);                
  // Set width of stat-box (fraction of pad size)
  gStyle->SetStatH(0.2);

   //All detectors that could be interesting
   std::vector<std::string> detector_channels = {
         "SIS_PMT_CATCH_OR",
         "SIS_PMT_CATCH_AND",
         "CT_SiPM1",
         "CT_SiPM2",
         "CT_SiPM_OR",
         "CT_SiPM_AND"};

   //The two detectors we are actually interested in comparing
   TSISChannels* sisch = new TSISChannels(runNumber);
   int PMT_CHAN = sisch->GetChannel("SIS_PMT_CATCH_OR");
   int SiPM_CHAN = sisch->GetChannel("CT_SiPM_OR");
   delete sisch;
   
   //Lets compare while there are lots of counts in the detectors... the hot dumps
   std::vector<TA2Spill> spills =  Get_A2_Spills(runNumber,{"Hot Dump"},{-1});
   std::cout<<spills.size() << " dumps"<<std::endl;
   
   //2D array of plots... Dimension 1: SIS channel, Dimension 2: Dump number
   std::vector<std::vector<TH1D*>> plots = Get_SIS(runNumber, {PMT_CHAN, SiPM_CHAN}, spills);
   
   //Show example of the histograms we want to compare
   TCanvas* example = new TCanvas();
   example->Divide(1,2);
   example->cd(1);
   plots.at(0).at(0)->Draw("HIST");
   example->cd(2);
   plots.at(1).at(0)->Draw("HIST");
   example->Draw();

   

   TCanvas *cgraph = new TCanvas("cgraph","cgraph",2000,1500);
   cgraph->Divide(TMath::Sqrt(spills.size())+1,TMath::Sqrt(spills.size()));
 
   //When that works... do it again for the next Hot Dumps
   for (int i = 0; i < spills.size(); i++)
   {
      //plots.at(0).at(i) vs plots.at(1).at(i)
      TH1D *hPMT = plots.at(0).at(i);
      TH1D *hSiPM = plots.at(1).at(i);

      if(hPMT->GetNbinsX() != hSiPM->GetNbinsX()){
        std::cout<<"hPMT and hSiPM have different number of bins!!"<<std::endl;
        return;
        }
      Float_t nBins = hPMT->GetNbinsX();
      //Ok, now make a scatter graph (TGraph) of the counts in first histogram vs second histogram
       // i.e (Bin0 of Histo1 vs Bin0 of Histo2)
       // i.e (Bin1 of Histo1 vs Bin1 of Histo2)
       // i.e (Bin2 of Histo2 vs Bin1 of Histo2)
       // etc
      TGraph *Scattgraph = new TGraph(nBins);
      Scattgraph->SetName(Form("ScattGraph_PMT_vs_hSiPM_%d",i) );
      Scattgraph->SetTitle(Form("run %d ScattGraph_PMT_vs_hSiPM_%d",runNumber,i) );
      Scattgraph->SetMarkerStyle(3);
      Scattgraph->SetMarkerSize(1);
      Scattgraph->SetMarkerColor(kBlue);
      Scattgraph->GetXaxis()->SetTitle("PMT counts");
      Scattgraph->GetYaxis()->SetTitle("SiPM counts");
      for(Int_t j=1; j<= nBins; j++)
      {
        Scattgraph->SetPoint(j,hPMT->GetBinContent(j),hSiPM->GetBinContent(j));
      }
      cgraph->cd(i+1);
      gPad->SetLeftMargin(0.15);
      Scattgraph->Draw("AP");
         
      //The TGraph should look like a stright line... can you fit a straight line please?
      TF1 *l1 = new TF1(Form("l1_%d",i),"pol1",0, hPMT->GetBinContent(hPMT->GetMaximumBin()));
      Scattgraph->Fit(l1);
   }
   
   //When that works... try some more runs from the last week
}
