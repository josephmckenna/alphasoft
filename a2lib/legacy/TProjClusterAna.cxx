//TProjClusterAna.cxx
#include <TMath.h>
#include <TPad.h>
#include <TROOT.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TLine.h>
#include <TMarker.h>
#include <TCanvas.h>
#include <TPaveStats.h>
#include "TProjClusterAna.h"

ClassImp( TProjClusterAna )

//_____________________________________________________________________
TProjClusterAna::TProjClusterAna()
{
  //ctor
  fprojclusters = new TObjArray();
  fi = 0;
  fj = 0;
  fSeed = -1;
}

//_____________________________________________________________________
TProjClusterAna::~TProjClusterAna()
{
  //dtor
   Clear();
}
//_____________________________________________________________________
void TProjClusterAna::Clear(Option_t * /*option*/)
{

  fprojclusters->SetOwner(kTRUE);
  fprojclusters->Delete();
  if (fprojclusters) delete fprojclusters;
  fi = 0;
  fj = 0;
  fSeed = -1;
}
//_____________________________________________________________________
Double_t TProjClusterAna::FindClosestPoint()
{
  Double_t min = 999999.;
  Int_t NClusters = fprojclusters->GetEntriesFast();

  TProjCluster *pci = (TProjCluster*) fprojclusters->At(fSeed);

  for( Int_t j = 0; j<NClusters; j++ )
    {
      if(j == fSeed) continue;

      TProjCluster *pcj = (TProjCluster*) fprojclusters->At(j);
      
      // Check to make sure none of them are form the same track
      Bool_t skip = kFALSE;
      for( Int_t ii = 0; ii<pci->GetEntriesFast(); ii++ )
      {
        for( Int_t jj = 0; jj<pcj->GetEntriesFast(); jj++ )
        {
          TProjClusterBase * iib = (TProjClusterBase *) pci->At(ii);
          TProjClusterBase * jjb = (TProjClusterBase *) pcj->At(jj);
          if( iib->GetTrack() == jjb->GetTrack() ) skip = kTRUE;
        }
      }
      if( skip ) continue;
      
      TProjClusterBase *pcbi = (TProjClusterBase*) pci->GetMean();
      Double_t iz = pcbi->Z();
      Double_t irphi = pcbi->RPhi();
      Double_t izsig = pcbi->GetZSigma();
      Double_t irphisig = pcbi->GetRPhiSigma();
      delete pcbi;
      
      TProjClusterBase *pcbj = (TProjClusterBase*) pcj->GetMean();
      Double_t jz = pcbj->Z();
      Double_t jrphi = pcbj->RPhi();
      Double_t jzsig = pcbj->GetZSigma();
      Double_t jrphisig = pcbj->GetRPhiSigma();
      delete pcbj;

//      Double_t d = ProjDistance(iz, jz, irphi, jrphi);
      Double_t d = ProjDistance(iz, jz, irphi, jrphi, izsig, jzsig, irphisig, jrphisig);

      if( d < min )
	{
	  min = d;
	  fj = j;
	  fi = fSeed;
	}
    }
    
  return min;
}

//_____________________________________________________________________
Double_t TProjClusterAna::FindMinPair()
{
  Double_t min = 999999.;
  Int_t NClusters = fprojclusters->GetEntriesFast();

  for( Int_t i = 0; i<NClusters; i++ )
    for( Int_t j = i; j<NClusters; j++ )
      {
        TProjCluster *pci = (TProjCluster*) fprojclusters->At(i);
        TProjCluster *pcj = (TProjCluster*) fprojclusters->At(j);

        // Check to make sure none of them are form the same track
        Bool_t skip = kFALSE;
        for( Int_t ii = 0; ii<pci->GetEntriesFast(); ii++ )
          for( Int_t jj = 0; jj<pcj->GetEntriesFast(); jj++ )
            {
              TProjClusterBase * iib = (TProjClusterBase *) pci->At(ii);
              TProjClusterBase * jjb = (TProjClusterBase *) pcj->At(jj);
              if( iib->GetTrack() == jjb->GetTrack() ) skip = kTRUE;
            }
        if( skip ) continue;

        TProjClusterBase *pcbi = (TProjClusterBase*) pci->GetMean();
        Double_t iz = pcbi->Z();
        Double_t irphi = pcbi->RPhi();
        Double_t izsig = pcbi->GetZSigma();
        Double_t irphisig = pcbi->GetRPhiSigma();
        delete pcbi;

        TProjClusterBase *pcbj = (TProjClusterBase*) pcj->GetMean();
        Double_t jz = pcbj->Z();
        Double_t jrphi = pcbj->RPhi();
        Double_t jzsig = pcbj->GetZSigma();
        Double_t jrphisig = pcbj->GetRPhiSigma();
        delete pcbj;

        // Double_t d = ProjDistance(iz, jz, irphi, jrphi);
	
        Double_t d = ProjDistance(iz, jz, irphi, jrphi, izsig, jzsig, irphisig, jrphisig);
	

        if( d < min )
          {
            min = d;
            fi = i;
            fj = j;
          }
      }
  
  return min;
}

/**
 * An attempted rewrite of the CombineMinPair() function below.
 *
 * Rather than copying and recreating the array, this version should
 * achieve the same effect by adding all of the cluster elements from the
 * fj'th cluster to the fi'th cluster and then deleting the fj'th cluster. 
 */
//_____________________________________________________________________
void TProjClusterAna::CombineMinPair()
{
  TProjCluster *fjth = (TProjCluster*) fprojclusters->At(fj);
  TProjCluster *fith = (TProjCluster*) fprojclusters->At(fi);
  for( Int_t i = 0; i < fjth->GetEntriesFast(); i++ )
    {
      TProjClusterBase* c=(TProjClusterBase *)fjth->At(i);
      fith->Add(new TProjClusterBase(c));
    }
  //fjth contains c above...
  delete fjth;
  fprojclusters->RemoveAt(fj);

  // Shrink the empty slot out of the TObjectArray.  This might be slow, but
  // it can't be slower than copying the entire array.
  fprojclusters->Compress();
  fSeed = fprojclusters->GetEntriesFast()-1; 
}

//_____________________________________________________________________
Double_t TProjClusterAna::ProjDistance(Double_t Z1,Double_t Z2,Double_t RPhi1,Double_t RPhi2)
{
// 	Double_t d = TMath::Sqrt( ( iz - jz )*( iz - jz ) + 
// 			   ( irphi - jrphi )*( irphi - jrphi ) );
			   
	return TMath::Sqrt(TMath::Power(Z1-Z2,2.)+TMath::Power(RPhi1-RPhi2,2.));
}

//_____________________________________________________________________
Double_t TProjClusterAna::ProjDistance(Double_t Z1,Double_t Z2,Double_t RPhi1,Double_t RPhi2,
  			Double_t ResZ1, Double_t ResZ2, Double_t ResRPhi1, Double_t ResRPhi2)
{
//	Double_t zsig = izsig*izsig + jzsig*jzsig;
//	Double_t rphisig = irphisig*irphisig + jrphisig*jrphisig;
	
	Double_t ResZedQuad=TMath::Power(ResZ1,2.)+TMath::Power(ResZ2,2.);
	Double_t ResRPhiQuad=TMath::Power(ResRPhi1,2.)+TMath::Power(ResRPhi2,2.);

//	Double_t d = TMath::Sqrt( ( iz - jz )*( iz - jz ) / zsig + 
//			  ( irphi - jrphi )*( irphi - jrphi ) / rphisig );
	
	return TMath::Sqrt(TMath::Power(Z1-Z2,2.)/ResZedQuad + TMath::Power(RPhi1-RPhi2,2.)/ResRPhiQuad);
}

//_____________________________________________________________________
void TProjClusterAna::Draw(const Option_t*)
{
  if( fprojclusters->GetEntriesFast() == 0 ) return;

  static TCanvas * c = NULL;

  if(!c)
    {
      c = new TCanvas("c","c",500,500);
      c->SetBorderMode(0);
      c->SetFillColor(0);
      c->Draw();
      
      TPad* pad1 = new TPad("1","1",.85,0,1,.85);
      pad1->SetFillColor(0);
      pad1->SetNumber(1);
      pad1->SetBorderMode(0);
      pad1->SetFrameBorderMode(0);
      pad1->SetFrameLineColor(0);
      pad1->SetLeftMargin(0);
      pad1->SetRightMargin(0);
      pad1->SetTopMargin(0);
      pad1->SetBottomMargin(0.1);
      pad1->Draw();
      
      TPad* pad2 = new TPad("2","2",0,0,.85,.85);
      pad2->SetFillColor(0);
      pad2->SetNumber(2);
      pad2->SetBorderMode(0);
      pad2->SetFrameBorderMode(0);
      pad2->SetFrameLineColor(1);
      pad2->SetLeftMargin(0.1);
      pad2->SetRightMargin(0);
      pad2->SetTopMargin(0);
      pad2->SetBottomMargin(0.1);
      pad2->Draw();

      TPad* pad3 = new TPad("3","3",0,.85,.85,1);
      pad3->SetFillColor(0);
      pad3->SetNumber(3);
      pad3->SetBorderMode(0);
      pad3->SetFrameBorderMode(0);
      pad3->SetFrameLineColor(0);
      pad3->SetLeftMargin(0.1);
      pad3->SetRightMargin(0);
      pad3->SetTopMargin(0);
      pad3->SetBottomMargin(0);
      pad3->SetBottomMargin(0);
      pad3->Draw();
    }
  
  // cluster display
  static TMultiGraph * mgraph = NULL;
  static TGraphErrors * g = NULL;
  static TGraphErrors * vg = NULL;
  static TH1D * z = NULL;
  static TH1D * hphi = NULL;
  if( g )
    {
      delete mgraph;
      delete z;
      delete hphi;
      g = NULL;          
      z = NULL;
      hphi = NULL;
      mgraph = NULL;
    }
  
  c->cd(2);
  
  g = new TGraphErrors();
  vg = new TGraphErrors();
  mgraph = new TMultiGraph();
  z = new TH1D("","",50,-25,25);
  hphi = new TH1D("","",28,-9,9);
  
  Int_t in=0;
  for( Int_t i = 0; i < GetEntriesFast(); i++ )
    {
      TProjCluster * p = GetProjCluster( i );
      if(!p) continue;
      for( Int_t j = 0; j < p->GetEntriesFast(); j++ )
        {
          TProjClusterBase * pc = p->At(j);
          g->SetPoint(in,pc->Z(),pc->RPhi() );
	  g->SetPointError(in,pc->GetZSigma(),pc->GetRPhiSigma());
          z->Fill(pc->Z());
          hphi->Fill(pc->RPhi() );
          in++;
        }
    }

  TProjClusterBase * pcbv = gEvent->GetProjClusterVertex();
  if(pcbv)
    {
      vg->SetPoint(0,pcbv->Z(),pcbv->RPhi());
      vg->SetPointError(0,pcbv->GetZSigma(),pcbv->GetRPhiSigma());
    }

  g->GetXaxis()->SetLimits(-25.,25.);
  vg->GetXaxis()->SetLimits(-25.,25.);
  g->GetXaxis()->SetTitle("Z (cm)");
  g->GetYaxis()->SetLimits(-9,9);
  g->GetYaxis()->SetRangeUser(-9,9);
  vg->GetYaxis()->SetLimits(-9,9);
  vg->GetYaxis()->SetRangeUser(-9,9);
  g->GetYaxis()->SetTitle("Phi at 2.2275 cm");
  g->GetXaxis()->CenterTitle();
  g->GetYaxis()->CenterTitle();
  g->SetFillStyle(3001);
  g->SetFillColor(kBlue - 10);
  vg->SetFillStyle(3001);
  vg->SetFillColor(kRed);
  mgraph->Add(g,"ap2");
  mgraph->Add(vg,"p2");
  mgraph->Draw();
  
  for( Int_t i = 0; i < GetEntriesFast(); i++ )
    {
      TProjCluster * p = GetProjCluster( i );
      for( Int_t j = 0; j < p->GetEntriesFast(); j++ )
        {
          TProjClusterBase *pc = p->At(j);
          TMarker * m = new TMarker(pc->Z(),pc->RPhi(), 20+pc->GetTrack());
          m->SetMarkerColor(i+5);
	  m->SetMarkerSize(1.);
          m->Draw();
        }

      if(p->IsVertex())
	{
	  TProjClusterBase *pc = p->GetMean();
	  TMarker * m = new TMarker(pc->Z(),pc->RPhi(), 29);
          m->SetMarkerColor(kRed);
	  m->SetMarkerSize(1.);
          //m->Draw();

	  // TLine * vl = new TLine(pc->Z(),-9,pc->Z(),9);
	  // vl->SetLineColor(kBlue);
	  // vl->Draw("same");
	  // TLine * v2 = new TLine(-25,pc->RPhi(),25,pc->RPhi());
	  // v2->SetLineColor(kBlue);
	  // v2->Draw("same");
	}
    }

  if(pcbv)
    {
      TMarker * m = new TMarker(pcbv->Z(),pcbv->RPhi(), 29);
      m->SetMarkerColor(kRed);
      m->SetMarkerSize(1.);
      m->Draw();
    }
  
  TVector3 * mc = gEvent->GetMCVertex();

  TLine * v3 = new TLine(-25,TrapRadius*TMath::Pi(),25,TrapRadius*TMath::Pi());
  v3->SetLineColor(kGray);
  v3->SetLineStyle(2);
  v3->Draw("same");
  TLine * v4 = new TLine(-25,-TrapRadius*TMath::Pi(),25,-TrapRadius*TMath::Pi());
  v4->SetLineColor(kGray);
  v4->SetLineStyle(2);
  v4->Draw("same");

  if(mc)
    {
      // TLine * vl = new TLine(mc->Z(),-9,mc->Z(),9);
      // vl->SetLineColor(kRed);
      // vl->Draw("same");
      // TLine * v2 = new TLine(-25,TrapRadius*mc->Phi(),25,TrapRadius*mc->Phi());
      // v2->SetLineColor(kRed);
      // v2->Draw("same");

      TMarker * m2 = new TMarker(mc->Z(),TrapRadius*mc->Phi(), 28);
      m2->SetMarkerColor(kBlue);
      m2->Draw("same");
    }
  
  
  //gStyle->SetOptStat(0);
  
  c->cd(1);
  hphi->SetLineColor(kBlack);
  hphi->SetFillStyle(0);
  hphi->GetXaxis()->SetTitleSize(0.1);
  hphi->GetXaxis()->CenterTitle();
  hphi->GetYaxis()->SetLabelSize(0);
  hphi->GetXaxis()->SetTickLength(0.1);
  hphi->GetYaxis()->SetTickLength(0);
  hphi->GetYaxis()->SetLabelColor(0);
  hphi->GetYaxis()->SetAxisColor(0);
  hphi->Draw("hbar");
  c->Update();
  TPaveStats *st = (TPaveStats*)hphi->FindObject("stats");
  st->SetOptStat(0);
  if( mc->X() != 0. && mc->Y() != 0. && mc->Z() != 0. )
    {
      TLine * l3 = new TLine(0,TrapRadius*mc->Phi(),hphi->GetMaximum()+1,TrapRadius*mc->Phi());
      l3->SetLineColor(kRed);
      l3->Draw("same");
    }
  
  c->cd(3);
  z->GetXaxis()->SetTickLength(0.1);
  z->GetYaxis()->SetTickLength(0);
  z->GetXaxis()->SetTitleSize(0.1);
  z->GetXaxis()->CenterTitle();
  z->GetYaxis()->SetLabelSize(0);
  z->GetYaxis()->SetTickLength(0);
  z->GetYaxis()->SetLabelColor(0);
  z->GetYaxis()->SetAxisColor(0);
  z->Draw();
  c->Update();
  TPaveStats *st2 = (TPaveStats*)z->FindObject("stats");
  st2->SetOptStat(0);
  c->Update();
  
  if( mc->X() != 0. && mc->Y() != 0. && mc->Z() != 0. )
    {
      TLine * l = new TLine(mc->Z(),0,mc->Z(),z->GetMaximum()+1);
      l->SetLineColor(kRed);
      l->Draw("same");
    }
  
  c->Modified();
  c->Update();  
}

//_____________________________________________________________________
void TProjClusterAna::Print(Option_t *) const
{
  for( Int_t i = 0; i<fprojclusters->GetEntriesFast(); i++ )
    {
      printf("Cluster %d\n", i);
      ((TProjCluster*)fprojclusters->At(i))->Print();
    }
}
