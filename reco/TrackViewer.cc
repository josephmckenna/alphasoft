#include "TrackViewer.hh"

#include <iostream>
#include <vector>
using std::vector;

#include <TGraph.h>
#include <TGraphErrors.h>
#include <TAxis.h>
#include <TGLCamera.h>
#include <TGLPerspectiveCamera.h>
#include <TPolyMarker3D.h>

#include "ChamberGeo.hh"

#include "TSpacePoint.hh"
#include "TFitHelix.hh"
#include "TFitLine.hh"
#include "TPCBase.hh"

void TrackViewer::Environment()
{
  double gInnerRadius = 10.*TPCBase::CathodeRadius; // mm
  double gTPCHalfLength=10.*TPCBase::HalfWidthZ;
  double gProtLength=280.;

  gCanv = new TCanvas("AgTPC");
  gCanv->Iconify();

  if(gGeoManager) delete gGeoManager;
  new TGeoManager("AGTPCgeom", "AGTPCgeom");

  TGeoVolume *top = gGeoManager->MakeBox("TOP",0,250.,250.,gTPCHalfLength);
  gGeoManager->SetTopVolume(top);
  top->SetVisibility( kFALSE );

  double gTrapRadius = 10.*TPCBase::TrapR;
  TGeoVolume *trap = gGeoManager->MakeTube("TRAP",0,gTrapRadius-1.,gTrapRadius,gProtLength);
  trap->SetLineColor(kYellow);
  top->AddNode(trap,1);

  TGeoVolume *inTPC = gGeoManager->MakeTube("INTPC",0,gInnerRadius-1.,gInnerRadius,gProtLength);
  trap->SetLineColor(kGray);
  top->AddNode(inTPC,1);

  gGeoManager->CloseGeometry();
  top->Draw("ogl");

  viewer = (TGLViewer*) gCanv->GetViewer3D();
  viewer->SetCurrentCamera(TGLViewer::kCameraPerspXOZ);

  Double_t* center = new Double_t[3];
  center[0]=center[1]=0.; center[2]=0.;

  viewer->SetGuideState(2,0,0,center);
  viewer->DrawGuides();
  viewer->UpdateScene();
  viewer->SetResetCamerasOnUpdate(kFALSE);
}

void TrackViewer::DrawPoints(const TObjArray* points_array, bool MC)
{
  int n = points_array->GetEntries();
  TPolyMarker3D* spm = new TPolyMarker3D(n,2);
  if(MC)
    {
      spm->SetMarkerColor(kRed-4);
      spm->SetMarkerSize(1);
    }
  else
    {
      spm->SetMarkerColor(kBlue+1);
      spm->SetMarkerSize(4);
    }
  for(int i=0; i<n; ++i)
    {
      TSpacePoint* p = (TSpacePoint*) points_array->At(i);
#if DEBUG>0
      cout<<n<<"\t"<<p->GetMCid()<<"\t"<<p->GetPDG()<<endl;
#endif
      spm->SetPoint(i,p->GetX(),p->GetY(),p->GetZ());
    }
  gCanv->cd();
  spm->Draw("ogl");
  gCanv->Update();
}

void TrackViewer::DrawMCpoints(const TObjArray* points_array)
{
  if(viewer)
    DrawPoints(points_array,true);
}

int TrackViewer::StartViewer()
{
  if(!aTrack)
    {
      std::cerr<<"Set the TTrack to view, please."<<std::endl;
      return -1;
    }
  if(aTrack->GetNumberOfPoints()<1)
    {
      std::cerr<<"TTrack has no points."<<std::endl;
      return 1;
    }

  Environment();

  DrawPoints(aTrack->GetPointsArray());

  gCanv->cd();
  if( aTrack->GetFieldStatus() == 0. )
    aTrack->GetLine()->GetLine()->Draw("ogl");
  else
    aTrack->GetHelix()->GetHelix()->Draw("ogl");
  gCanv->Update();

  return 0;
}


void TrackViewer::DrawChamber()
{
  ChamberGeo cg;
  cg.SetWindow(-10.*TPCBase::ROradius, -10.*TPCBase::ROradius, 10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
  vector<TEllipse*> cath = cg.GetCathode2D();
  vector<TEllipse*> pads = cg.GetPadCircle2D();
  TGraph* fwires = cg.GetFieldWires();
  TGraph* awires = cg.GetAnodeWires();

  gCanv->cd();
  for(auto c: cath) c->Draw();
  for(auto p: pads) p->Draw();

  fwires->Draw("psame");
  awires->Draw("psame");

  gCanv->Update();
}

TPolyLine* TrackViewer::DrawFitLine(TFitLine* aLine)
{
    TVector3 u(aLine->GetU());
  TVector3 p(aLine->Get0());

  double t0 = -p.Dot(u)/u.Mag2(),
    Npoints=100.;
  double t1=t0,t2=t0;

  //  std::cout<<t0<<std::endl;

  TVector3 pos;
  double gROradius = 10.*TPCBase::ROradius; // mm
  do
    {
      pos=aLine->GetPosition(t1);
      ++t1;
    }
  while(pos.Perp()<gROradius);
  do
    {
      pos=aLine->GetPosition(t2);
      --t2;
    }
  while(pos.Perp()<gROradius);

  double tmin=t1<t2?t1:t2;
  double tmax=t1>t2?t1:t2;

  //  std::cout<<tmin<<"\t"<<tmax<<std::endl;

  double dt=TMath::Abs(tmax-tmin)/Npoints;
  int ip=0;


  TPolyLine* atrack = new TPolyLine((int) Npoints);
  atrack->SetLineColor(kGreen);
  atrack->SetLineWidth(2);
  for(double t = tmin; t <= tmax; t += dt)
    {
      TVector3 p = aLine->GetPosition(t);
      if(p==TVector3(-9999999.,-9999999.,-9999999.))
  	continue;
      atrack->SetPoint(ip,p.X(),p.Y());
      ++ip;
    }
  return atrack;
}


// int TrackViewer::Draw2D(const char* cname)
// {
//   TString ctitle = TString::Format("%s-2Dview",cname);
//   gCanv = new TCanvas(cname,ctitle.Data(),1200,1200);

//   const TObjArray* spoints = aTrack->GetPointsArray();
//   TGraphErrors* gp = new TGraphErrors();
//   for(int ip=0; ip<spoints->GetEntries(); ++ip)
//     {
//       TSpacePoint* p = (TSpacePoint*) spoints->At(ip);
//       gp->SetPoint(ip,p->GetX(),p->GetY());
//       gp->SetPointError(ip,p->GetErrX(),p->GetErrY());
//     }
//   gCanv->cd();
//   gp->Draw("ap");
//   gCanv->Update();
//   gp->GetXaxis()->SetLimits(-10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
//   gp->SetMinimum(-10.*TPCBase::ROradius);
//   gp->SetMaximum(10.*TPCBase::ROradius);

//   DrawChamber();

//   // vector<TPolyLine*> fLines;
//   // for( auto ilin: fLines )
//   //   if(ilin) delete ilin;
//   // fLines.clear();

//   // TObjArray* tracks = aTrack->GetTracks();
//   // for(int n=0; n<tracks->GetEntries(); ++n)
//   //   {
//   //     if( ((TFitLine*) tracks->At(n))->GetNumberOfPoints() > 3
//   // 	  && ((TFitLine*) tracks->At(n))->IsGood() )
//   // 	{
//   // 	  //	  std::cout<<"Plot!"<<std::endl;
//   // 	  fLines.push_back( DrawFitLine( (TFitLine*) tracks->At(n) ) );
//   // 	}
//   //   }

//   // gCanv->cd();
//   // for( auto ilin: fLines )
//   //   {
//   //     ilin->Draw("same");
//   //     //      std::cout<<"Draw!"<<std::endl;
//   //     gCanv->Update();
//   //   }
//   return 0;
// }

int TrackViewer::Draw2D(const char* cname, int gVerb)
{
  TString ctitle = TString::Format("%s-2Dview",cname);
  gCanv = new TCanvas(cname,ctitle.Data(),1200,1200);

  const TObjArray* spoints = aTrack->GetPointsArray();
  TGraphErrors* gp = new TGraphErrors();
  gp->SetMarkerColor(kBlack);
  gp->SetLineColor(kBlack);
  for(int ip=0; ip<spoints->GetEntries(); ++ip)
    {
      TSpacePoint* p = (TSpacePoint*) spoints->At(ip);
      gp->SetPoint(ip,p->GetX(),p->GetY());
      gp->SetPointError(ip,p->GetErrX(),p->GetErrY());
      if(gVerb > 1)
	std::cout<<ip<<"\t"
		 <<p->GetX()<<"\t"
		 <<p->GetY()<<"\t"
		 <<p->GetZ()<<"\t\t"
		 <<p->GetR()<<"\t"
		 <<p->GetPhi()*TMath::RadToDeg()<<"\t\t"
		 <<p->GetHeight()<<std::endl;
    }
  gCanv->cd();
  gp->Draw("ap");
  gCanv->Update();
  gp->GetXaxis()->SetLimits(-10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
  gp->SetMinimum(-10.*TPCBase::ROradius);
  gp->SetMaximum(10.*TPCBase::ROradius);


  DrawChamber();


  vector<TPolyLine*> fLines;
  for( auto ilin: fLines )
    if(ilin) delete ilin;
  fLines.clear();

  vector<int> colors;

  TObjArray* tracks = aTrack->GetTracks();
  const int Ntracks = tracks->GetEntries();
  TGraphErrors* gpt[Ntracks];
  for(int n=0; n<Ntracks; ++n)
    {
      if( ((TFitLine*) tracks->At(n))->GetStatus() < 1 ) continue;

      if( ((TFitLine*) tracks->At(n))->GetNumberOfPoints() > 3
	  && ((TFitLine*) tracks->At(n))->IsGood() )
	{
	  //	  std::cout<<"Plot!"<<std::endl;
	  fLines.push_back( DrawFitLine( (TFitLine*) tracks->At(n) ) );

	  if(gVerb >= 1)
	    ((TFitLine*) tracks->At(n))->Print();


	  gpt[n] = new TGraphErrors();
	  gpt[n]->SetMarkerColor(n+2);
	  gpt[n]->SetLineColor(n+2);
	  colors.push_back(n+2);

	  const TObjArray* spoints = ((TFitLine*) tracks->At(n))->GetPointsArray();
	  for(int ip=0; ip<spoints->GetEntries(); ++ip)
	    {
	      TSpacePoint* p = (TSpacePoint*) spoints->At(ip);
	      gpt[n]->SetPoint(ip,p->GetX(),p->GetY());
	      gpt[n]->SetPointError(ip,p->GetErrX(),p->GetErrY());
	      if(gVerb > 1)
		std::cout<<ip<<"\t"
			 <<p->GetX()<<"\t"
			 <<p->GetY()<<"\t"
			 <<p->GetZ()<<"\t\t"
			 <<p->GetR()<<"\t"
			 <<p->GetPhi()*TMath::RadToDeg()<<"\t\t"
			 <<p->GetHeight()<<std::endl;
	    }
	  if(gVerb > 1)
	    std::cout<<"--------------------------------------------------------------------------"<<std::endl;
	  gCanv->cd();
	  gpt[n]->Draw("p");
	  gCanv->Update();
	}
    }

  gCanv->cd();
  int il=0;
  for( auto ilin: fLines )
    {
      ilin->SetLineColor(colors.at(il));
      ilin->Draw("same");
      //      std::cout<<"Draw!"<<std::endl;
      gCanv->Update();
      ++il;
    }

  //  gCanv->Print("fitline.gif+");
  return 0;
}
