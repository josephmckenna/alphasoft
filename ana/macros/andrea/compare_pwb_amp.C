#include <TROOT.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TProfile.h>
#include <TStyle.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPaletteAxis.h>
#include <TPaveStats.h>
#include <TMath.h>
#include <TTree.h>

#include <iostream>
#include <string>
//#include "padplot.C"

#define BUILD_AG
#include "TSpacePoint.hh"
#include "TStoreLine.hh"
#include "TStoreEvent.hh"
TStoreEvent* event;

TH2D* hspzp=0;
TH2D* hsppad=0;
TH1D* hocc_px=0;
TH1D* hspzp_px=0;
TH1D* hsppad_px=0;


#include "SignalsType.hh"
ALPHAg::padmap pm;

//TString lab[]={"CERN AW @3.1kV","TRIUMF AW @3.2kV","TRIUMF AW @3.1kV"};
TString lab[]={"CERN 2018","TRIUMF 2020"};
TString tag[]={"_CERN_AW_3100V","_TRIUMF_AW_3200V","_TRIUMF_AW_3100kV"};
TString hlist[]={"hsppad_px","hspzp_px","hOccPad_px"};

void plot_spacepoints(TFile* fin)
{
  hspzp = new TH2D("hspzp","Spacepoints in Tracks;z [mm];#phi [deg]",
		     600,-1200.,1200.,256,0.,360.);
  hspzp->SetStats(kFALSE);
  hsppad = new TH2D("hsppad","Spacepoints in Tracks by Pad;row;sec;N",
		      576,0.,576.,32,0.,32.);
  hsppad->SetStats(kFALSE);

  TTree* tin = (TTree*) fin->Get("StoreEventTree");
  cout<<tin->GetTitle()<<"\t"<<tin->GetEntries()<<endl;
  event = new TStoreEvent();
  tin->SetBranchAddress("StoredEvent", &event);
  for(int e=0; e<tin->GetEntries(); ++e)
    {
      if( e%100000 == 0 ) cout<<"*** "<<e<<endl;
      event->Reset();
      tin->GetEntry(e);
      const TObjArray* tracks = event->GetLineArray();
      for(int i=0; i<tracks->GetEntriesFast(); ++i)
	{
	  TStoreLine* aLine = (TStoreLine*) tracks->At(i);
	  const TObjArray* sp = aLine->GetSpacePoints();
	  for( int ip = 0; ip<sp->GetEntriesFast(); ++ip )
	    {
	      TSpacePoint* ap = (TSpacePoint*) sp->At(ip);
	      hspzp->Fill( ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
	      int spad = ap->GetPad();
	      int s,r;
	      pm.get(spad,s,r);
	      hsppad->Fill(double(r),double(s));
	    }
	}
    }
  cout<<"spacepoints END"<<endl;
}

struct hpar
{
  int TotBins;
  int Nbins;
  double xlim;
  double maxv;
  string hname;
  string htitle;

  hpar(TH1D* h)
  {
    hname=h->GetName();
    htitle=h->GetTitle();
    TotBins=h->GetNbinsX();
    Nbins=int(TotBins*0.5);
    xlim=double(TotBins*0.5);
    maxv=h->GetBinContent(h->GetMaximumBin());
    printf("hbar::ctor %s Tot. bins: %d, Half bins: %d, Max: %.2f\n",
	   hname.c_str(),TotBins,Nbins,maxv);
  }
};

void hdiff(const hpar* p1, vector<TH1D*>* h, TString label)
{
  TString hname=p1->hname;
  hname+="_diff";
  TString htitle=p1->htitle;
  htitle+=" DIFF ";
  htitle+=label;
  htitle+=";Pad Row [mod 288]";
  TH1D* hdiff=new TH1D(hname,htitle,p1->Nbins,0.,p1->xlim);
  hdiff->Sumw2();
  hdiff->SetStats(kFALSE);
  hdiff->SetLineColor(kBlack); hdiff->SetMarkerColor(kBlack);
  hdiff->SetMarkerStyle(8);
  hdiff->Add(h->at(0),h->at(1),-1.);
  hdiff->Scale(1./p1->maxv);
  h->push_back(hdiff);
}

vector<TH1D*> fold_zed(TH1D* h, TString label)
{
  const hpar p(h);
  cout<<h->GetName()<<"\tMax: "<<h->GetBinContent(h->GetMaximumBin())<<endl;

  TString hname=p.hname;
  hname+="_bot";
  TString htitle=p.htitle;
  //  htitle+=" BOT ";
  htitle+=" ";
  htitle+=label;
  htitle+=";Pad Row [mod 288]";
  TH1D* hbot=new TH1D(hname,htitle,p.Nbins,0.,p.xlim);
  hbot->SetStats(kFALSE); hbot->SetMarkerStyle(8);
  hbot->SetLineColor(kBlue); hbot->SetMarkerColor(kBlue);
  
  hname=p.hname;
  hname+="_top";
  htitle=p.htitle;
  // htitle+=" TOP ";
  htitle+=" ";
  htitle+=label;
  htitle+=";Pad Row [mod 288]";
  TH1D* htop=new TH1D(hname,htitle,p.Nbins,0.,p.xlim);
  htop->SetStats(kFALSE); htop->SetMarkerStyle(8);
  htop->SetLineColor(kRed); htop->SetMarkerColor(kRed);

  for( int b=1; b<=p.Nbins; ++b)
    {
      double bcbot=h->GetBinContent(b);
      hbot->SetBinContent(b,bcbot);
      int tb=p.TotBins-b+1;
      double bctop=h->GetBinContent(tb);
      htop->SetBinContent(b,bctop);
      //cout<<b<<"\t"<<bcbot<<"\t"<<tb<<"\t"<<bctop<<endl;
    }

  vector<TH1D*> hfolded={htop,hbot};

  hdiff(&p,&hfolded,label);

  return hfolded;
}

void plot_fold()
{
  TString cname=TString::Format("ccomparefoldedpad");
  TCanvas* c5 = new TCanvas(cname.Data(),cname.Data(),1900,1000);
  c5->Divide(2,3);

  int Ntags=sizeof(tag)/sizeof(TString);
  cout<<"Number of tags: "<<Ntags<<endl;

  double maxsp=0.,maxdiff=0.,mindiff=DBL_MAX;
  vector<vector<TH1D*>> vhisto;
  TFile* fhisto=TFile::Open("compare_pwb_amp.root","UPDATE");
  for( int n=0; n<Ntags; ++n)
    {
      TString objname("hsppad_px");
      objname+=tag[n];
      fhisto->cd();
      TH1D* hp = (TH1D*) gROOT->FindObject(objname);
      vector<TH1D*> histovector=fold_zed(hp,lab[n]);

      for(size_t i=0; i<histovector.size(); ++i)
	histovector.at(i)->Clone()->Write();

      c5->cd(n*2+1);
      histovector.at(0)->Scale(1./histovector.at(0)->Integral());
      histovector.at(0)->Draw("P");
      histovector.at(1)->Scale(1./histovector.at(1)->Integral());
      histovector.at(1)->Draw("Psame");
      TLegend* leg = new TLegend(0.7,0.2,0.85,0.35);
      leg->AddEntry(histovector.at(0),"Top x+288","pl");
      leg->AddEntry(histovector.at(1),"Bottom","pl");
      leg->Draw("same");
      maxsp=histovector.at(0)->GetBinContent(histovector.at(0)->GetMaximumBin())>maxsp?histovector.at(0)->GetBinContent(histovector.at(0)->GetMaximumBin()):maxsp;
      maxsp=histovector.at(1)->GetBinContent(histovector.at(1)->GetMaximumBin())>maxsp?histovector.at(1)->GetBinContent(histovector.at(1)->GetMaximumBin()):maxsp;

      c5->cd(n*2+2);
      histovector.at(2)->Draw("P");
      gPad->SetGridy();

      maxdiff=histovector.at(2)->GetBinContent(histovector.at(2)->GetMaximumBin())>maxdiff?histovector.at(2)->GetBinContent(histovector.at(2)->GetMaximumBin()):maxdiff;
      mindiff=histovector.at(2)->GetBinContent(histovector.at(2)->GetMinimumBin())<mindiff?histovector.at(2)->GetBinContent(histovector.at(2)->GetMinimumBin()):mindiff;
      vhisto.push_back(histovector);
    }

  for(auto v: vhisto)
    {
      v.at(0)->GetYaxis()->SetRangeUser(0.,maxsp*1.1);
      v.at(2)->GetYaxis()->SetRangeUser(mindiff*1.1,maxdiff*1.1);
    }
  
  
  c5->SaveAs(".pdf"); c5->SaveAs(".pdf");
}


void compare_pwb_amp()
{
  int col[]={kBlack,kRed,kBlue};
  // int run[]={38739,904502,904512};
  int run[]={38739,904502};

  int Nfiles=sizeof(run)/sizeof(int);
  cout<<"Number of files: "<<Nfiles<<endl;

  TProfile* pamp[Nfiles];
  TH1D* pocc[Nfiles];
  TH1D* pspp[Nfiles];
  TH1D* pspt[Nfiles];

  TString cname=TString::Format("ccompareamp");
  TCanvas* c1 = new TCanvas(cname.Data(),cname.Data(),1900,1000);

  cname=TString::Format("ccompareocc");
  TCanvas* c2 = new TCanvas(cname.Data(),cname.Data(),1900,1000);

  cname=TString::Format("ccomparepad");
  TCanvas* c3 = new TCanvas(cname.Data(),cname.Data(),1900,1000);

  cname=TString::Format("ccomparespt");
  TCanvas* c4 = new TCanvas(cname.Data(),cname.Data(),1900,1000);

  TLegend* leg = new TLegend(0.7,0.7,0.9,0.9);
  TLegend* leg2= new TLegend(0.41,0.17,0.61,0.39);
  double max4,max3,max2; max4=max3=max2=0.;

  TFile* fout = TFile::Open("compare_pwb_amp.root","RECREATE");

  for( int i=0; i<Nfiles; ++i )
    {
      TString fname=TString::Format("%s/agmini/cosmics%d.root",
				    getenv("DATADIR"),run[i]);
      cout<<fname<<endl;
      TFile* fin=TFile::Open(fname,"READ");

      plot_spacepoints(fin);

      pspp[i]=hsppad->ProjectionX();

      TString hname=pspp[i]->GetName();
      hname+=tag[i];
      pspp[i]->SetName(hname);
      //fout->cd();
      //pspp[i]->Clone()->Write();
      
      pspp[i]->Scale(1./pspp[i]->Integral());
      pspp[i]->SetStats(kFALSE);
      pspp[i]->SetMinimum(0.);
      pspp[i]->SetLineColor(col[i]);
      pspp[i]->SetMarkerColor(col[i]);
      c3->cd();
      if(i) pspp[i]->Draw("HISTCsame");
      else  pspp[i]->Draw("HISTC");
      max3=max3>pspp[i]->GetBinContent(pspp[i]->GetMaximumBin())?max3:pspp[i]->GetBinContent(pspp[i]->GetMaximumBin());
  


      pspt[i]=hspzp->ProjectionX();
      
      hname=pspt[i]->GetName();
      hname+=tag[i];
      pspt[i]->SetName(hname);
      //fout->cd();
      //pspt[i]->Clone()->Write();

      pspt[i]->Scale(1.e2/pspt[i]->Integral());
      pspt[i]->SetStats(kFALSE);
      pspt[i]->SetMinimum(0.);
      pspt[i]->SetLineColor(col[i]);
      pspt[i]->SetMarkerColor(col[i]);
      c4->cd();
      if(i) pspt[i]->Draw("HISTCsame");
      else  pspt[i]->Draw("HISTC");
      max4=max4>pspt[i]->GetBinContent(pspt[i]->GetMaximumBin())?max4:pspt[i]->GetBinContent(pspt[i]->GetMaximumBin());

     

      
      fin->cd();
      gDirectory->cd("/paddeconv");
      TH2D* hocc = (TH2D*)gROOT->FindObject("hOccPad");
      
      //pocc[i] = hocc->ProfileX();
      pocc[i] = hocc->ProjectionX();

      hname=pocc[i]->GetName();
      hname+=tag[i];
      pocc[i]->SetName(hname);
      //fout->cd();
      //pocc[i]->Clone()->Write();

      pocc[i]->Scale(1.e2/pocc[i]->Integral());
      pocc[i]->SetStats(kFALSE);
      pocc[i]->SetMinimum(0.);
      pocc[i]->SetLineColor(col[i]);
      pocc[i]->SetMarkerColor(col[i]);
      c2->cd();
      if(i) pocc[i]->Draw("HISTCsame");
      else  pocc[i]->Draw("HISTC");
      max2=max2>pocc[i]->GetBinContent(pocc[i]->GetMaximumBin())?max2:pocc[i]->GetBinContent(pocc[i]->GetMaximumBin());
      


      fin->cd();
      gDirectory->cd("/paddeconv/pwbwf");
      pamp[i] = (TProfile*) gROOT->FindObject("hPwbAmp_prox");

      hname=pamp[i]->GetName();
      hname+=tag[i];
      pamp[i]->SetName(hname);
      fout->cd();
      pamp[i]->Clone()->Write();

      pamp[i]->SetStats(kFALSE);
      pamp[i]->SetMinimum(0.);
      pamp[i]->SetMaximum(4096.);
      pamp[i]->SetLineColor(col[i]);
      pamp[i]->SetMarkerColor(col[i]);
      c1->cd();
      if(i) pamp[i]->Draw("same");
      else  pamp[i]->Draw();
     
      TString lname(lab[i]);
      leg->AddEntry(pamp[i],lname,"pl");
      leg2->AddEntry(pamp[i],lname,"pl");
      //   }

      if(i==0){
  c1->cd();
  leg->Draw("same");

  c2->cd();
  TString htitle=pocc[0]->GetTitle();
  htitle+=" Averaged over sec";
  pocc[0]->SetTitle(htitle);
  pocc[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pocc[0]->GetYaxis()->SetRangeUser(0.,max2*1.1);
  leg2->Draw("same");

  c3->cd();
  htitle=pspp[0]->GetTitle();
  htitle+=" Averaged over sec";
  pspp[0]->SetTitle(htitle);
  pspp[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pspp[0]->GetYaxis()->SetRangeUser(0.,max3*1.1);
  leg2->Draw("same");

  c4->cd();
  htitle=pspt[0]->GetTitle();
  htitle+=" Averaged over #phi";
  pspt[0]->SetTitle(htitle);
  pspt[0]->GetYaxis()->SetTitle("Fraction [%] over dataset");
  pspt[0]->GetYaxis()->SetRangeUser(0.,max4*1.1);
  leg2->Draw("same");}

  TString sname=TString::Format("%s_%d.pdf",c1->GetName(),i+Nfiles);
  c1->SaveAs(sname); c1->SaveAs(sname);
  sname=TString::Format("%s_%d.pdf",c2->GetName(),i+Nfiles);
  c2->SaveAs(sname); c2->SaveAs(sname);
  sname=TString::Format("%s_%d.pdf",c3->GetName(),i+Nfiles);
  c3->SaveAs(sname); c3->SaveAs(sname);
  sname=TString::Format("%s_%d.pdf",c4->GetName(),i+Nfiles);
  c4->SaveAs(sname); c4->SaveAs(sname);
  // c1->SaveAs(".pdf"); c1->SaveAs(".pdf");
  // c2->SaveAs(".pdf"); c2->SaveAs(".pdf");
  // c3->SaveAs(".pdf"); c3->SaveAs(".pdf");
  // c4->SaveAs(".pdf"); c4->SaveAs(".pdf");

  //  fout->Write();
    }
  c1->Modified(); c1->Update();
  c2->Modified(); c2->Update();
  c3->Modified(); c3->Update();
  c4->Modified(); c4->Update();
}
