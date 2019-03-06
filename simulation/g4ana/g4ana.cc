#include <iostream>
#include <string>
#include <sstream>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TAxis.h"

#include "TH1D.h"
#include "TH2D.h"

#include "Deconv.hh"
#include "Match.hh"
#include "Reco.hh"

#include "Utils.hh"

using namespace std;

int main(int argc, char** argv)
{
  if( argc == 1 ) 
    {
      cerr<<"Please provide rootfile from command line"<<endl;
      return 1;
    }
  
  TFile* fin = TFile::Open(argv[1],"READ");
  if( !fin->IsOpen() )
    {
      cerr<<"rootfile not open"<<endl;
      return 1;
    }

  TTree* tGarf = (TTree*) fin->Get("Garfield");
  TClonesArray* garfpp_hits = new TClonesArray("TMChit");
  tGarf->SetBranchAddress("GarfHits",&garfpp_hits);

  TTree* tSig =  (TTree*) fin->Get("Signals");
  if( !tSig )
    {
      cerr<<"rootfile does not contain proper simulation data"<<endl;
      return 1;
    }

  TClonesArray* AWsignals = new TClonesArray("TWaveform");
  tSig->SetBranchAddress("AW",&AWsignals);

  TClonesArray* PADsignals = new TClonesArray("TWaveform");
  tSig->SetBranchAddress("PAD",&PADsignals);

  double ADCThres=5000., PWBThres=100., ADCpeak=1000., PWBpeak=10.;
  Deconv d(ADCThres, PWBThres, ADCpeak, PWBpeak);

  string json_file = "ana_settings.json";
  ostringstream json_filepath;
  json_filepath<<getenv("AGRELEASE")<<"/ana/"<<json_file;
  cout<<"[main]# Loading Ana settings from: "<<json_filepath.str()<<endl;
  Match m(json_filepath.str());

  double B=1.;
  Reco r(json_filepath.str(),B);

  TApplication app("g4ana",&argc,argv);

  TCanvas* csig = new TCanvas("csig","csig",1400,1400);
  csig->Divide(2,2);
  double tmax = 4500.;

  TCanvas* creco = new TCanvas("creco","creco",1400,1400);
  creco->Divide(2,2);

  for( int i=0; i<tSig->GetEntries(); ++i )
    {
      tSig->GetEntry(i);

      // anode deconv
      int nsig = d.FindAnodeTimes( AWsignals );
      cout<<"[main]# "<<i<<"\tFindAnodeTimes: "<<nsig<<endl;
      // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      PrintSignals( d.GetAnodeSignal() );
      TH1D* haw = PlotSignals( d.GetAnodeSignal(), "anodes" );
      haw->Scale(1./haw->Integral());
      haw->SetLineColor(kRed);
      cout<<"[main]# "<<i<<"\tPlotAnodeTimes: "<<haw->GetEntries()<<endl;
      csig->cd(1);
      haw->Draw("hist");
      haw->SetTitle("Deconv Times");
      haw->GetXaxis()->SetRangeUser(0.,tmax);
     
      // pad deconv
      nsig = d.FindPadTimes( PADsignals );
      cout<<"[main]# "<<i<<"\tFindPadTimes: "<<nsig<<endl;
      PrintSignals( d.GetPadSignal() );
      // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      TH1D* hpads = PlotSignals( d.GetPadSignal(), "pads" );
      hpads->Scale(1./hpads->Integral());
      hpads->SetLineColor(kBlue);
      csig->cd(1);
      hpads->Draw("histsame");

      m.Init();

      // combine pads
      m.CombinePads( d.GetPadSignal() );
      cout<<"[main]# "<<i<<"\tCombinePads: "<<m.GetCombinedPads()->size()<<endl;
      // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      
      PrintSignals( m.GetCombinedPads() );
      TH1D* hcombpads = PlotSignals( m.GetCombinedPads(), "combinedpads" );
      hcombpads->Scale(1./hcombpads->Integral());
      hcombpads->SetLineColor(kBlue);
      csig->cd(2);
      haw->Draw("hist");
      hcombpads->Draw("histsame");

      TH2D* hmatch = PlotSignals( d.GetAnodeSignal(), m.GetCombinedPads(), "sector");
      //TH2D* hmatch = PlotSignals( d.GetAnodeSignal(), d.GetPadSignal(), "sector");
      csig->cd(3);
      hmatch->Draw();
      hmatch->GetXaxis()->SetRangeUser(0.,tmax);
      hmatch->GetYaxis()->SetRangeUser(0.,tmax);

      TH1D* hoccaw = PlotOccupancy( d.GetAnodeSignal(), "anodes" );
      hoccaw->Scale(1./hoccaw->Integral());
      hoccaw->SetLineColor(kRed);
      TH1D* hocccombpads = PlotOccupancy( m.GetCombinedPads(), "pads" );
      hocccombpads->Scale(1./hocccombpads->Integral());
      hocccombpads->SetLineColor(kBlue);
      csig->cd(4);
      hoccaw->Draw("hist");
      hocccombpads->Draw("histsame");

      // match electrodes
      m.MatchElectrodes( d.GetAnodeSignal() );
      cout<<"[main]# "<<i<<"\tMatchElectrodes: "<<m.GetSpacePoints()->size()<<endl;
      // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      r.Reset();

      // reco points
      r.SetTrace(true);
      r.AddSpacePoint( m.GetSpacePoints() );
      // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

      tGarf->GetEntry(i);
      PlotMCpoints(creco,garfpp_hits);

      const TClonesArray* sp = r.GetPoints();      
      PlotRecoPoints(creco,sp);

      DrawTPCxy(creco);
    }// events loop

  app.Run();

  return 0;
}
