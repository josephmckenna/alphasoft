///////////////////////////////////////////////////////////////////////////
//                                                                       //
//    TAlphaDisplay                                                     //
//                                                                       //
//    Utility class fore Alpha  Event display (Hits & Tracks)           //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

#include <TDirectory.h>
#include <TButton.h>
#include <TCanvas.h>
#include <TView3D.h>
#include <TMarker3DBox.h>
#include <TPaveText.h>
#include <TList.h>
#include <TVector3.h>
#include <TLatex.h>
#include <TDiamond.h>
#include <TPolyLine3D.h>
#include <TPolyMarker3D.h>
#include <THelix.h>
#include <TEllipse.h>
#include <TRotation.h>
#include <TMarker.h>

#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <TVirtualGeoTrack.h>

#include "TAlphaDisplay.h"
#include "TGLViewer.h"

ClassImp(TAlphaDisplay)

int gNext = 1;

void TAlphaDisplay::NextEvent()
{
  printf("**************** NEXT ********************\n");
  gNext = 1;
}

//____________________________________________________________________________
TAlphaDisplay::TAlphaDisplay(TString text, Int_t autoSaveOnDisplay, Int_t runNum)
  : TNamed("AlphaDisplay","AlphaDisplay")
{
  fCurrentEvent=NULL;
  fAutoSaveOnDisplay=autoSaveOnDisplay;
  fRunNo=runNum;
  //
  // Create an event display of Alpha event
  //
	
  fViewOne=fViewTwo=fViewThree=kTRUE;
  fViewSup=fViewHOnly=kFALSE;
  //fViewMCData=kFALSE;
  fViewMCData=kTRUE;
  fViewRecData=kTRUE;
  fViewShowAllSil=kFALSE;
  fOGLColourScheme=kFALSE;
  fViewShowAllTracks=kFALSE;
  
  /////////////////////////////////////////////////////
  //  Create display canvas 
  /////////////////////////////////////////////////////
   
  fCanvas = new TCanvas("Canvas", "Alpha Event Display",100,100,900,900);
  fCanvas->ToggleEventStatus();
  fCanvas->SetBorderSize(0);
  fCanvas->SetFillColor(0);
  //  fCanvas->MoveOpaque();
     
  /////////////////////////////////////////////////////
  //  Create main display pad  
  /////////////////////////////////////////////////////
     
  fPad = new TPad("viewpad", "Alpha display",0.15,0.,1,1);
  fPad->Draw();
  fPad->Modified();
  fPad->SetFillColor(0);
  fPad->SetBorderSize(0);
  fPad->SetBorderMode(0);

  /////////////////////////////////////////////////////
  //  Create user interface buttons
  ///////////////////////////////////////////////////// 

  fButtons = new TPad("up-but", "up-but",0.,0.75 ,0.15,1.);
  fButtons->Draw();
  fButtons->SetFillColor(38);
  fButtons->SetBorderSize(0);
  fButtons->SetBorderMode(0);
  fButtons->cd();
       
  Float_t dbutton = 0.13;
  Float_t  y = 0.96;
  Float_t dy = 0.011;
  Float_t x0 = 0.05;
  Float_t x1 = 0.95;
     
  Char_t method[200];  
  sprintf(method, "{long r__ptr=0x%lx; ((TAlphaDisplay*)r__ptr)->", (Long_t)this);
  Int_t lm = strlen(method);
          
  TButton *button;
  method[lm]='\0';  strcat(method,"SetView(90,-90,90,\"Top\");}");
  button = new TButton("Top View",method ,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();
     
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"SetView(-90,0,-90,\"Side\");}");
  button = new TButton("Side View",method ,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"SetView(0,-90,0,\"Front\");}");
  button = new TButton("Front View" ,method,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"DrawAllViews();}");
  button = new TButton("All Views" ,method,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"DrawViewX3D();}");
  button = new TButton("X3D" ,method ,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();

  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"DrawViewOGL();}");
  button = new TButton("OGL" ,method ,x0,y-dbutton,x1,y);
  button->SetFillColor(33);
  button->Draw();
  
  //  AppendPad();  
  fCanvas->cd();
  
  /////////////////////////////////////////////////////
  //  
  /////////////////////////////////////////////////////
  
  fDetSel = new TPad("sel-but", "sel-but",0.,0. ,0.15, 0.75);
  fDetSel->Draw();
  fDetSel->SetFillColor(39);
  fDetSel->SetBorderSize(0);
  fDetSel->SetBorderMode(0);
  fDetSel->cd();
  
  dbutton = 0.05;
  dy = 0.011;
  
  y = 0.96;
  method[lm]='\0';  strcat(method,"ToggleOne();}");
  fButOne = new TButton("1Si",method ,x0,y-dbutton,x1,y);
  fButOne->SetFillColor(38);
  fButOne->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleTwo();}");
  fButTwo = new TButton("2Si", method,x0,y-dbutton,x1,y);
  fButTwo->SetFillColor(38);
  fButTwo->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleThree();}");
  fButThree = new TButton("3Si", method,x0,y-dbutton,x1,y);
  fButThree->SetFillColor(38);
  fButThree->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleSup();}");
  fButSup = new TButton("Supports", method,x0,y-dbutton,x1,y);
  fButSup->SetFillColor(37);
  fButSup->Draw();
  
  y -= dbutton + dy;
  //method[lm]='\0';  strcat(method,"Delete();}");
  TButton * fButNext = new TButton("Next", ".q",x0,y-dbutton,x1,y);
  fButNext->SetFillColor(0);
  fButNext->Draw();
  
  y -= dbutton + dy;
  //method[lm]='\0';  strcat(method,"Delete();}");
  method[lm]='\0';  strcat(method,"NextEvent();}");
  TButton * xxx = new TButton("XNext", method,x0,y-dbutton,x1,y);
  xxx->SetFillColor(0);
  xxx->Draw();
  
  
  y = 0.6;
  method[lm]='\0';  strcat(method,"ToggleHitOnlyDet();}");
  fButHOnly = new TButton("Hit Only", method,x0,y-dbutton,x1,y);
  fButHOnly->SetFillColor(37);
  fButHOnly->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleMCData();}");
  fButMCData = new TButton("M. Carlo", method,x0,y-dbutton,x1,y);
  fButMCData->SetFillColor(38);
  fButMCData->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleRecData();}");
  fButRecData = new TButton("Recons.", method,x0,y-dbutton,x1,y);
  fButRecData->SetFillColor(38);
  fButRecData->Draw();
  
  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleShowAllSil();}");
  fButShowAllSil = new TButton("All Sil", method,x0,y-dbutton,x1,y);
  fButShowAllSil->SetFillColor(37);
  fButShowAllSil->Draw();

  y -= dbutton + dy;
  method[lm]='\0';  strcat(method,"ToggleShowAllTracks();}");
  fButShowAllTracks = new TButton("Tracks", method,x0,y-dbutton,x1,y);
  fButShowAllTracks->SetFillColor(37);
  fButShowAllTracks->Draw();
  
  // Legend
  y -= dbutton + 2*dy;
  TLine * bline = new TLine(0.02,y,0.25,y);
  bline->SetLineColor(kBlue);
  bline->Draw();
  y -= dy;
  TLatex *btext = new TLatex(0.27,y,"Included");
  btext->SetTextSize(0.08);
  btext->Draw();

  y -= 2*dy;
  TLine * gline = new TLine(0.02,y,0.25,y);
  gline->SetLineColor(kGray);
  gline->Draw();
  y -= dy;
  TLatex *gtext = new TLatex(0.27,y,"Not near Trap");
  gtext->SetTextSize(0.08);
  gtext->Draw();

  y -= 2*dy;
  TLine * rline = new TLine(0.02,y,0.25,y);
  rline->SetLineColor(kRed);
  rline->Draw();
  y -= dy;
  TLatex *rtext = new TLatex(0.27,y,"Shared Hits");
  rtext->SetTextSize(0.08);
  rtext->Draw();

  y -= 2*dy;
  TLine * grline = new TLine(0.02,y,0.25,y);
  grline->SetLineColor(kGreen);
  grline->Draw();
  y -= dy;
  TLatex *grtext = new TLatex(0.27,y,"Bad Chi2");
  grtext->SetTextSize(0.08);
  grtext->Draw();


  // logo 
  TDiamond *a = new TDiamond(.28,.038,.72,.16);
  a->SetFillColor(2);
  a->Draw();
  TLatex lat;
  lat.SetTextSize(.4);
  lat.DrawLatex(.40,.07,"#alpha");
  
  fCanvas->cd();
  
  TPad * ftext = new TPad("text", "text",0.19,0.99,0.74, 0.96);
  ftext->Draw();
  //ftext->SetFillColor(1);
  ftext->SetBorderSize(2);
  ftext->SetBorderSize(0);
  ftext->SetBorderMode(0);
  ftext->SetFillColor(0);
  ftext->cd();
  
  TText *textDisplay = new TText(0.01,0.3,text);
  textDisplay->SetTextColor(1);
  textDisplay->SetTextSize(0.5);
  textDisplay->Draw();

  AppendPad();  
  fCanvas->cd();
  fCanvas->Update();
  gDirectory->Append(this);
  
}

//____________________________________________________________________________
TAlphaDisplay::~TAlphaDisplay() {
  delete fPad;
  delete fDetSel;
  delete fButtons;
  delete fCanvas; 
}

//____________________________________________________________________________
void TAlphaDisplay::SetView(Float_t theta, Float_t phi, Float_t psi, Option_t *tit) {
  //
  //  change  viewing  angles
  //
  fPad->cd();
  fPad->Clear();
  fPad->SetFillColor(0);
  DrawView(theta,phi,psi,tit);
  fPad->Modified();
  fPad->Update();
}

//____________________________________________________________________________
void TAlphaDisplay::DrawAllViews() {

  
  //
  //  Draw  Front,  Side,  Top  views
  //
  fPad->cd();
  fPad->Clear();
  fPad->Divide(2,2,0,0);
   
  fPad->cd(1);  DrawView(30, 30,  0);
  fPad->cd(2);  DrawView(90,-90, 90,"Top");
  fPad->cd(3);  DrawView(0,-90,  0,"Front");
  fPad->cd(4);  DrawView(-90, 0,-90,"Side");

  fPad->Modified();
  fPad->Update();
}

//____________________________________________________________________________
void TAlphaDisplay::DrawViewX3D() {
  //
  // Draw View with X3D
  //
  fPad->cd();
  fPad->Clear();
  DrawView(30,30,30);
  fPad->GetViewer3D("x3d");  
}
void TAlphaDisplay::DrawViewOGL() {
  //
  // Draw View with OGL
  //
  fPad->cd();
  fPad->Clear();
  fOGLColourScheme=kTRUE;
  fViewShowAllSil=kTRUE;
  //DrawView(30,30,30);
  DrawView(30,0,90);
  fPad->GetViewer3D("ogl");  
}


//____________________________________________________________________________
void TAlphaDisplay::DrawView(Float_t theta, Float_t phi, Float_t psi, Option_t *tit) {
  //
  // Draw a view of ALPHA
  //
  
  gPad->SetCursor(kWatch);
  gPad->SetFillColor(0);  //18 10
     
  TView3D * view = NULL;
  view = (TView3D*)gPad->GetView();   
  if (view) delete view;
  view = new TView3D(1,NULL,NULL);
     
  Float_t range = 20.;                                            
  view->SetRange(-range,-range,-range,range,range,range);     
 
  // ******************************************
  // View and colour Hit Detectors
  // ******************************************
  for( Int_t i = 0; i < (gGeoManager->GetListOfVolumes()->GetEntries())+1; i++)
  {
    TGeoVolume *nod = (TGeoVolume*) gGeoManager->GetVolume( i );
    //nod->SetLineColor( (Int_t) nod->GetTitle() % 7);

    //-------> mechanical structure
    const Char_t *z = nod->GetName();
    //printf("z: %s \n",nod->GetName());
    const Char_t  Z = z[0];
          
    if (Z=='M' || Z=='S') {  
      nod->SetVisibility(fViewSup);
      nod->SetLineColor(1);
      nod->SetLineWidth(2);
    }
 
    //-------> silicon
    if (Z=='0' || Z=='1' || Z == '2' || Z =='3' || Z =='4' || Z =='5') {
      if(fViewShowAllSil)
        nod->SetVisibility(!fViewHOnly); 
      else
        nod->SetVisibility(0);   
      nod->SetLineColor(11);
      if (fOGLColourScheme)
        nod->SetLineColor(kYellow-7);
      Bool_t View = kFALSE;
      if(Z=='0') View = fViewOne;
      if(Z=='1') View = fViewTwo;
      if(Z=='2') View = fViewThree;
               
      if(Z=='3') View = fViewOne;
      if(Z=='4') View = fViewTwo;
      if(Z=='5') View = fViewThree;

      for (Int_t n=0; n<fCurrentEvent->GetNSil(); n++) {
        TAlphaEventSil *sil = fCurrentEvent->GetSil(n);
        //printf("nsil: %d sil: %s, z: %s\n",sil->ReturnSilNum(sil->GetName()),sil->GetName(),z);
        //std::cout <<z<<":"<<sil->GetName() <<std::endl;
        if (fOGLColourScheme && 
           // strncmp(z,sil->GetName(),4)==0)  The Sil Module no longer has a name assigned for speed
           sil->ReturnSilNum(z)==sil->GetSilNum()) //Use the map to turn the node name into a module number
        //if (fOGLColourScheme )
        {
          //std::cout<<sil->ReturnSilNum(z) << "\t"<< n<<"\t"<<sil->GetSilNum()<<std::endl;
        
          if (sil->GetNHits()<1 )
          {
            nod->SetTransparency(70);
          }
          else {
            nod->SetLineColor(kRed);
          }
        }
         //prin
        
        //if (strcmp(z,sil->GetName())) continue;
        if (sil->ReturnSilNum(z)!=sil->GetSilNum()) continue;
        //printf("success!\n");

        if (!fOGLColourScheme) nod->SetLineColor(11);
        nod->SetLineWidth(2);
        
        nod->SetVisibility(View);
        if (fOGLColourScheme) nod->SetVisibility(1);
        if (!View) continue;
                    
        //LGCP PITCH
        Int_t NumStripPhi = 256;
        Int_t NumStripZed = 256;
                    
        //Only draw hit strips if the silicon module has hits (and in OGL mode)
        if (!fOGLColourScheme && sil->GetNHits()>0)
        {
          // Draw hit strips pside
          Double_t *pside = sil->GetADCp();
          for (Int_t k=0;k<(NumStripPhi);k++) if (pside[k]) { 
            //printf("Display -- n: %d, k: %d, phi[k]: %lf\n",n,k,pside[k]);
            TVector3 a,b;
            sil->GetStrippStartEnd(k,a,b);
            TPolyLine3D *lp = new TPolyLine3D(2);
            lp->SetPoint(0,a.X(),a.Y(),a.Z());
            lp->SetPoint(1,b.X(),b.Y(),b.Z());
            lp->SetLineColor(2);
            lp->SetLineWidth(1);
            if (fOGLColourScheme)
              lp->SetLineWidth(2);
            lp->Draw("same");
          }

          // Draw hit strips nside
          Double_t *nside = sil->GetADCn();
          for (Int_t k=0;k<(NumStripZed);k++) if (nside[k]) { 
            //printf("Display -- n: %d, k: %d, zeta[k]: %lf\n",n,k,nside[k]);
            TVector3 a,b;
            sil->GetStripnStartEnd(k,a,b);
            TPolyLine3D *lp = new TPolyLine3D(2);
            lp->SetPoint(0,a.X(),a.Y(),a.Z());
            lp->SetPoint(1,b.X(),b.Y(),b.Z());
            lp->SetLineColor(2);
            lp->SetLineWidth(1);
            if (fOGLColourScheme)
              lp->SetLineWidth(2);
            lp->Draw("same");
          }    
				}   
      }
    }
  }
  gGeoManager->GetTopVolume()->Draw("same");
  	     
  TVector3 *gV = fCurrentEvent->GetMCVertex();
  if(gV)
    {
      TMarker3DBox *hit = new TMarker3DBox(gV->X(),gV->Y(),gV->Z() , 0.1,0.1,0.1 ,0,0);
      hit->SetLineColor(9);
      hit->SetLineWidth(4);
      hit->Draw("same");
    }
     
  // ******************************************
  // Draw Monte Carlo Tracks
  // ******************************************
  if ( fViewMCData &&
       gGeoManager->GetListOfTracks() &&
       gGeoManager->GetTrack(0) &&
       ((TVirtualGeoTrack*)gGeoManager->GetTrack(0))->HasPoints() ) {
       
       gGeoManager->SetVisOption(0);	 
       gGeoManager->SetTopVisible();
       gGeoManager->DrawTracks("/D");  // this means all tracks
          // Drawing G3 tracks via TGeo is available only
	  // if geant3 is compile with -DCOLLECT_TRACK flag
	  // (to be activated in geant3/TGeant3/TGeant3gu.cxx)
  }    
     
  // ******************************************
  // Draw hits on silicon
  // ******************************************
  for (Int_t n=0; n<fCurrentEvent->GetNSil(); n++) 
    { 
      TAlphaEventSil *sil = fCurrentEvent->GetSil(n);

      for(Int_t ihit = 0; ihit < sil->GetNHits(); ihit++)
        {
          TAlphaEventHit * h = sil->GetHit(ihit);
          Double_t hitSize=0.1;
          if (fOGLColourScheme) hitSize=0.4;
          TMarker3DBox *hit = new TMarker3DBox(
					       h->XMRS(), // x center
					       h->YMRS(), // y center
					       h->ZMRS(), // z center
					       hitSize, // x halflength
					       hitSize, // y halflength
					       hitSize, // z halflength
					       0, 
					       0//h->GetCos()*180/TMath::Pi() // theta wtr to x axis
					       );
          //	  hit->SetLineColor(ihit%6+4); // make the hits different colour
          hit->SetLineColor(4);
          hit->SetLineWidth(7);
          hit->Draw("same");
	     }
    }
  
  // ******************************************
  // Draw Reconstructed Tracks
  // ******************************************
  if (fViewRecData)
    {
      for( Int_t ihelix = 0; ihelix < fCurrentEvent->GetNHelices(); ihelix++)
        {
          DrawHelix( fCurrentEvent->GetHelix( ihelix ), fCurrentEvent->GetDebug() );
        }
       
      for( Int_t ihelix = 0; ihelix < fCurrentEvent->GetNCosmicHelices(); ihelix++)
        {
          DrawHelix( fCurrentEvent->GetCosmicHelix( ihelix ), fCurrentEvent->GetDebug() );
        }
      
      // ppbar annihilation    
      TAlphaEventVertex *Ver = fCurrentEvent->GetVertex();
      if (Ver->IsGood()) 
        {
          Double_t VertexSize=0.1;
          if (fOGLColourScheme)
            VertexSize=0.3;
	        TMarker3DBox * vermark = new TMarker3DBox( Ver->X(),
						   Ver->Y(),
						   Ver->Z(),
						   VertexSize, // x halflength
						   VertexSize, // y halflength
						   VertexSize, // z halflength
						   0,
						   0);

          vermark->SetLineColor(1);

          if (fOGLColourScheme)
            vermark->SetLineWidth(16);
          else
            vermark->SetLineWidth(4);
          vermark->Draw("same");               
        }
      
      for(Int_t na=0; na<Ver->GetNDCAa(); na++)
        {
          TPolyLine3D *dca = new TPolyLine3D();
      	  TVector3 *a = Ver->GetDCAa(na);
          TVector3 *b = Ver->GetDCAb(na);
          dca->SetPoint(0,a->X(),a->Y(),a->Z());
          dca->SetPoint(1,b->X(),b->Y(),b->Z());
          dca->SetLineColor( kRed+2 );
          if (fOGLColourScheme)
            dca->SetLineWidth(16);
          else
            dca->SetLineWidth(2);
          dca->Draw("same");
	      }

      // Special markers for tracks
      // if( ev->GetNHits() < 100 )
      for( Int_t itrack = 0; itrack < fCurrentEvent->GetNTracks(); itrack++)
        {
          for( Int_t ihit = 0; ihit < fCurrentEvent->GetTrack( itrack )->GetNHits(); ihit++ )
            {
              Double_t hit_pos[3] = { 
                fCurrentEvent->GetTrack( itrack )->GetHit( ihit )->XMRS(),
                fCurrentEvent->GetTrack( itrack )->GetHit( ihit )->YMRS(),
                fCurrentEvent->GetTrack( itrack )->GetHit( ihit )->ZMRS() 
              };
              //printf("%lf %lf %lf\n",hit_pos[0],hit_pos[1],hit_pos[2]);
              TPolyMarker3D * pm = new TPolyMarker3D( 1, hit_pos, 4 );
              pm->SetMarkerColor( itrack%7+4 );
              //pm->Draw("same");
	          }
	      }
      // if it's a cosmic run, display a cosmic track!	  
      if(fCurrentEvent->IsACosmic())
      //	if(ev->GetNTracks()>0) DrawCosmicTrack(ev->GetTrack(0));
      if(fCurrentEvent->GetCosmicTrack()->Getcor()!=-999.)
        DrawCosmicTrack(fCurrentEvent->GetCosmicTrack());
    }
  
  for( Int_t i = 0; i<fCurrentEvent->GetMCNumPoint(); i++ )
    {
      TVector3 * p = fCurrentEvent->GetMCPoint( i );
      TMarker3DBox * pmark = new TMarker3DBox( p->X(),
					       p->Y(),
					       p->Z(),
					       0.1,
					       0.1,
					       0.1,
					       0,
					       0);
      pmark->SetLineColor( kGreen+2 );
      pmark->Draw("same");
    }
  
  for( Int_t iline = 0; iline < fCurrentEvent->Getnxylines(); iline++ )
    {
      TVector3 * l = fCurrentEvent->Getxyline( iline );

      Double_t m = l->X();
      Double_t b = l->Y();

      Double_t x1= -20.;
      //Double_t y1= -20.;
      Double_t y1= m*x1 + b;
      Double_t x2= +20.;
      Double_t y2= m*x2 + b;
      //Double_t y2 = 20.;

      TPolyLine3D * line = new TPolyLine3D(2);
      line->SetPoint(0,x1,y1,-200);
      line->SetPoint(1,x2,y2,-200);
      line->SetLineColor(kBlue);
      //line->Draw("same");
    }

  for( Int_t iline = 0; iline < fCurrentEvent->Getnyzlines(); iline++ )
    {
      TVector3 * l = fCurrentEvent->Getyzline( iline );

      Double_t m = l->X();
      Double_t b = l->Y();

      Double_t z1= +40.;
      Double_t y1= m*z1 + b;
      Double_t z2= -40.;
      Double_t y2= m*z2 + b;
      
      TPolyLine3D * line = new TPolyLine3D(2);
      line->SetPoint(0,-100,y1,z1);
      line->SetPoint(1,-100,y2,z2);
      line->SetLineColor(kBlue);
      //line->Draw("same");
    }

  // draw the electrode ring ( try to make in only in the
  // front view )
  if (!fOGLColourScheme)
  {
    TPolyLine3D * ring = new TPolyLine3D();
    for( Int_t iline = 0; iline <= 360;)
    {
      Double_t x = 2.1*TMath::Cos( iline*TMath::Pi()/180. );
      Double_t y = 2.1*TMath::Sin( iline*TMath::Pi()/180. );
      ring->SetNextPoint(x,y,-80);
      iline += 10;
    }
    ring->SetLineColor(kBlue);
    ring->SetLineStyle(9);
    ring->Draw("same");
  }
  AppendPad();
  Int_t iret;
  view->SetView(phi,theta,psi,iret);     
     
  // ******************************************
  //  Draw view title
  // ******************************************
  if(tit) {
    TPaveText *title = new TPaveText(0.01,0.8,0.99,0.99);
    title->SetBit(kCanDelete);
    title->SetFillColor(42);
    title->AddText(tit);
    //title->Draw();
  }   
}

//____________________________________________________________________________
void TAlphaDisplay::DrawHelix( TAlphaEventCosmicHelix * Helix, Bool_t debug )
{
  TPolyLine3D *lt = new TPolyLine3D(200);  
  Int_t p = 0;
  for( Int_t t = 0; t < 100; t++ )
   {
     TVector3 pnt = Helix->GetPoint3D( double(t) - 50. );
     if ( TMath::Sqrt(pnt.X()*pnt.X()+pnt.Y()*pnt.Y()) > 20 ) continue;
      lt->SetPoint(p++,pnt.X(),pnt.Y(),pnt.Z());
  }
  if (debug)
    Helix->Print(); 
   /*if(debug)
    {
      switch( Helix->GetHelixStatus() )
	{
	case -7:
	  lt->SetLineColor(kOrange);
	  lt->SetLineWidth(1);
	  break;
	case -6:
	  lt->SetLineColor(kRed);
	  lt->SetLineWidth(1);
	  break;
	case -5:*/
	  lt->SetLineColor(kCyan);
          lt->SetLineWidth(1);
          lt->Draw("same");    
/*          break;
	case -4:
	  lt->SetLineColor(kGreen);
	  lt->SetLineWidth(1);
	  break;
	case -3:
	  lt->SetLineColor(kCyan);
	  lt->SetLineWidth(1);
	  break;
	case 1:
	  lt->SetLineColor(kBlue);
	  lt->SetLineWidth(1);
	  break;
	case 2:
	  lt->SetLineColor(kBlack);
	  lt->SetLineWidth(1);
	  break;
	default:
	  lt->SetLineColor(kYellow);
	  lt->SetLineWidth(1);
	  break;
	}
      lt->Draw("same");    
    }
  if( Helix->GetHelixStatus() == 1 )
    {
      lt->SetLineColor(kBlue);
      lt->SetLineWidth(1);
      lt->Draw("same");    
    }
 else if( Helix->GetHelixStatus() == 2 )
    {
      lt->SetLineColor(kBlack);
      lt->SetLineWidth(2);
      lt->Draw("same");    
    }
*/
}
//____________________________________________________________________________
void TAlphaDisplay::DrawHelix( TAlphaEventHelix * Helix, Bool_t debug )
{
  if (!Helix) return;
  TPolyLine3D *lt = new TPolyLine3D(20);  
  Int_t p = 0;

  // draw the first part of the helix
  for(Int_t r=5;r>=0;r--) 
    {
      Int_t iflag = 0;
      Double_t s = Helix->GetsFromR( double(r), iflag );
      if( iflag < 0 ) continue;

      TVector3 pnt = Helix->GetPoint3D_C( -1.*s );
      lt->SetPoint(p++,pnt.X(),pnt.Y(),pnt.Z());
    }

  // draw the rest of the helix
  for(Int_t r=0;r<=15;r++) 
    {
      Int_t iflag = 0;
      Double_t s = Helix->GetsFromR( double(r), iflag );
      if( iflag < 0 ) continue;

      TVector3 pnt = Helix->GetPoint3D_C( s );
      lt->SetPoint(p++,pnt.X(),pnt.Y(),pnt.Z());
    }

  if(debug)
    {
      switch( Helix->GetHelixStatus() )
	{
	case -7:
	  lt->SetLineColor(kOrange);
	  lt->SetLineWidth(1);
	  break;
	case -6:
	  lt->SetLineColor(kRed);
	  lt->SetLineWidth(1);
	  break;
	case -5:
	  lt->SetLineColor(kGray);
          lt->SetLineWidth(1);
          break;
	case -4:
	  lt->SetLineColor(kGreen);
	  lt->SetLineWidth(1);
	  break;
	case -3:
	  lt->SetLineColor(kCyan);
	  lt->SetLineWidth(1);
	  break;
	case 1:
	  lt->SetLineColor(kBlue);
	  lt->SetLineWidth(1);
	  
	  break;
	case 2:
	  lt->SetLineColor(kBlack);
	  lt->SetLineWidth(1);
	  break;
	default:
	  lt->SetLineColor(kYellow);
	  lt->SetLineWidth(1);
	  break;
	}
      lt->Draw("same");    
    }
  if( Helix->GetHelixStatus() == 1 )
    {
      
      lt->SetLineColor(kBlue);
      lt->SetLineWidth(1);
      if (fOGLColourScheme)
      {
        lt->SetLineColor(kViolet-2);
        lt->SetLineWidth(8);
      }
      lt->Draw("same");    
    }
 else if( Helix->GetHelixStatus() == 2 )
    {
      lt->SetLineColor(kBlack);
      lt->SetLineWidth(2);
      lt->Draw("same");    
    }

}
//____________________________________________________________________________
void TAlphaDisplay::DrawCosmicTrack(TAlphaEventTrack *cosmic)
{
	Int_t npoints=3;
	Double_t x,y,z,t=-15.;
	TVector3 p1 = cosmic->Getunitvector();
	TVector3 p2 = cosmic->Getr0();
	TPolyLine3D *ct = new TPolyLine3D(npoints);
	for(Int_t n=0; n<npoints; ++n)
	{
		x = p1.X()*t + p2.X();
		y = p1.Y()*t + p2.Y();
		z = p1.Z()*t + p2.Z();
		ct->SetPoint(n,x,y,z);
		t+=15.;
	}
	ct->SetLineColor(kBlue);
	ct->SetLineWidth(1);
	ct->Draw("same");
}
//____________________________________________________________________________
void TAlphaDisplay::ToggleOne() {
  fViewOne = fViewOne? kFALSE:kTRUE;  
  fButOne->SetFillColor(fViewOne? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleTwo() {
  fViewTwo = fViewTwo? kFALSE:kTRUE;
  fButTwo->SetFillColor(fViewTwo? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleThree() {
  fViewThree = fViewThree? kFALSE:kTRUE;
  fButThree->SetFillColor(fViewThree? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleSup() {
  fViewSup = fViewSup? kFALSE:kTRUE;
  fButSup->SetFillColor(fViewSup? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleHitOnlyDet() {
  fViewHOnly = fViewHOnly? kFALSE:kTRUE;
  fButHOnly->SetFillColor(fViewHOnly? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleMCData() {
  fViewMCData = fViewMCData? kFALSE:kTRUE;
  fButMCData->SetFillColor(fViewMCData? 38:37);
  Update();
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleRecData() {
  fViewRecData = fViewRecData? kFALSE:kTRUE;
  fButRecData->SetFillColor(fViewRecData? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleShowAllSil() {
  fViewShowAllSil = fViewShowAllSil ? kFALSE:kTRUE;
  fButShowAllSil->SetFillColor(fViewShowAllSil ? 38:37);
}

//____________________________________________________________________________
void TAlphaDisplay::ToggleShowAllTracks() {
  fViewShowAllTracks = fViewShowAllTracks ? kFALSE:kTRUE;
  //fDebug = fDebug ? kFALSE:kTRUE;
  fButShowAllTracks->SetFillColor(fViewShowAllTracks ? 38:37);
}

