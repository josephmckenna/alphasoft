#include "TFile.h"
#include "TNtuple.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TMath.h"

#include "ROOT/TSeq.hxx"

#include "TEllipse.h"
#include "TLine.h"

#include <iostream>
#include <vector>

void plottrack()
{
  double CathodeVoltage = -4000.,// V
    AnodeVoltage = 3200.,
     FieldVoltage = -110.;
  double MagneticField=0.0; // T
  double r0=10.9251,z0=0., // cm
    phi0=1.0,theta0=0.5; // rad

  TFile f( Form("%s/TrackAvalanche_Cathode%4.0fV_Anode%4.0fV_Field%3.0fV_B%1.2fT_initR%1.2fcm_initPhi%1.3frad_initZ%1.2fcm.root",
                            getenv("GARFIELDPP"),
			    CathodeVoltage,AnodeVoltage,FieldVoltage,
			    MagneticField,
			    r0,phi0,z0) );
  auto ntuple = f.Get<TNtuple>("ntpion");
  TGraph* gxy = new TGraph();
  gxy->SetTitle("Every #pi Interaction;x [cm];y [cm]");
  gxy->SetMarkerStyle(2);
  TGraph* grz = new TGraph();
  grz->SetTitle("Every #pi Interaction;r [cm];z [cm]");
  grz->SetMarkerStyle(2);
  TGraph* gxy_n = new TGraph();
  gxy_n->SetTitle("Selected #pi Interaction;x [cm];y [cm]");
  gxy_n->SetMarkerStyle(2);
  TGraph* grz_n = new TGraph();
  grz_n->SetTitle("Selected #pi Interaction;r [cm];z [cm]");
  grz_n->SetMarkerStyle(2);
  int n=0, nn=0;
  for(auto index : ROOT::TSeqL(ntuple->GetEntriesFast())) 
    {
      ntuple->GetEntry(index);
      //      std::cout<<index<<") \t"<<ntuple->GetArgs()[0]<<"\t"<<ntuple->GetArgs()[1]<<"\t"<<ntuple->GetArgs()[2]<<"\t"<<ntuple->GetArgs()[3]<<"\n";
      gxy->SetPoint(n,ntuple->GetArgs()[0],ntuple->GetArgs()[1]);
      double r = TMath::Sqrt(ntuple->GetArgs()[0]*ntuple->GetArgs()[0]+ntuple->GetArgs()[1]*ntuple->GetArgs()[1]);
      grz->SetPoint(n,r,ntuple->GetArgs()[2]);
      if( ntuple->GetArgs()[3] > 1.0 )
	{
	  gxy_n->SetPoint(nn,ntuple->GetArgs()[0],ntuple->GetArgs()[1]);
	  grz_n->SetPoint(nn,r,ntuple->GetArgs()[2]);
	  ++nn;
	}
      ++n;
    }
  std::cout<<"Number of selected points: "<<nn<<std::endl;

  TCanvas* c1 = new TCanvas("track","track",1600,1200);
  c1->Divide(2,2);
  c1->cd(1);
  gxy->Draw("AP");
  c1->cd(2);
  grz->Draw("AP");
  c1->cd(3);
  gxy_n->Draw("AP");
  c1->cd(4);
  grz_n->Draw("AP");

  
  TEllipse* TPCcath = new TEllipse(0.,0.,10.9,10.9);
  TPCcath->SetFillStyle(0);
  c1->cd(1);
  TPCcath->Draw("same");
  c1->cd(3);
  TPCcath->Draw("same");
  TEllipse* TPCpads = new TEllipse(0.,0.,19.0,19.0);
  TPCpads->SetFillStyle(0);
  c1->cd(1);
  TPCpads->Draw("same");
  c1->cd(3);
  TPCpads->Draw("same");
  TEllipse* TPCAW = new TEllipse(0.,0.,18.2,18.2);
  TPCAW->SetFillStyle(0);
  TPCAW->SetLineStyle(2);
  c1->cd(1);
  TPCAW->Draw("same");
  c1->cd(3);
  TPCAW->Draw("same");
  TEllipse* TPCFW = new TEllipse(0.,0.,17.4,17.4);
  TPCFW->SetFillStyle(0);
  TPCFW->SetLineStyle(3);
  c1->cd(1);
  TPCFW->Draw("same");
  c1->cd(3);
  TPCFW->Draw("same");

  TLine* lAW = new TLine(18.2,-230.4*0.5,18.2,230.4*0.5);
  lAW->SetLineColor(kGray+1);
  lAW->SetLineStyle(2);
  //  lAW->SetLineWidth(1);
  c1->cd(2);
  lAW->Draw("same");
  c1->cd(4);
  lAW->Draw("same");
  TLine* lFW = new TLine(17.4,-230.4*0.5,17.4,230.4*0.5);
  lFW->SetLineColor(kGray+1);
  lFW->SetLineStyle(3);
  //  lFW->SetLineWidth(1);
  c1->cd(2);
  lFW->Draw("same");
  c1->cd(4);
  lFW->Draw("same");
  TLine* lcath = new TLine(10.9,-230.4*0.5,10.9,230.4*0.5);
  lcath->SetLineColor(kGray+1);
  lcath->SetLineStyle(1);
  //  lcath->SetLineWidth(1);
  c1->cd(2);
  lcath->Draw("same");
  c1->cd(4);
  lcath->Draw("same");
  TLine* lpad = new TLine(19.,-230.4*0.5,19.,230.4*0.5);
  lpad->SetLineColor(kGray+1);
  lpad->SetLineStyle(1);
  //  lpad->SetLineWidth(1);
  c1->cd(2);
  lpad->Draw("same");
  c1->cd(4);
  lpad->Draw("same");
}
