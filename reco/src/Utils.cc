#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>

#include "SignalsType.h"

#include "TClonesArray.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TEllipse.h"
#include "TPolyMarker.h"
#include "TLine.h"
#include "TMath.h"

#include "TMChit.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"

#include "NeuralFinder.hh"

#include "Histo.hh"

void PlotNeurons(TCanvas* c, const set<NeuralFinder::Neuron*> &neurons, int col_ = kBlack)
{
   int col = col_;
   for(auto n: neurons){
      if(n->GetActive()){
         const TSpacePoint *p1 = n->GetStartPt();
         const TSpacePoint *p2 = n->GetEndPt();
         if(p1 && p2){
            if(col_ < 0){
               // cout << "Neuron V = " << n->GetV() << endl;
               if(n->GetV() > 0.9) col = kBlack;
               else if(n->GetV() > 0.8) col = kRed;
               else if(n->GetV() > 0.7) col = kBlue;
               else if(n->GetV() > 0.6) col = kGreen;
               else if(n->GetV() > 0.5) col = kOrange;
               else if(n->GetV() > 0.4) col = kMagenta;
               else col = kGray;
            }

            TLine *lxy = new TLine(p1->GetX(),p1->GetY(),p2->GetX(),p2->GetY());
            lxy->SetLineWidth(1);
            lxy->SetLineColor(col);
            c->cd(1);
            lxy->Draw("same");

            TLine *lrz = new TLine(p1->GetR(),p1->GetZ(),p2->GetR(),p2->GetZ());
            lrz->SetLineWidth(1);
            lrz->SetLineColor(col);
            c->cd(2);
            lrz->Draw("same");

            TLine *lrphi = new TLine(p1->GetR(),p1->GetPhi()*TMath::RadToDeg(),p2->GetR(),p2->GetPhi()*TMath::RadToDeg());
            lrphi->SetLineWidth(1);
            lrphi->SetLineColor(col);
            c->cd(3);
            lrphi->Draw("same");

            TLine *lzphi = new TLine(p1->GetZ(),p1->GetPhi()*TMath::RadToDeg(),p2->GetZ(),p2->GetPhi()*TMath::RadToDeg());
            lzphi->SetLineWidth(1);
            lzphi->SetLineColor(col);
            c->cd(4);
            lzphi->Draw("same");
         }
      }
   }
}

void PlotMCpoints(TCanvas* c, const TClonesArray* points) // drift electron origins "real" track
{
   int Npoints = points->GetEntries();
   std::cout<<"[utils]#  GarfHits --> "<<Npoints<<std::endl;
   TGraph* gxy = new TGraph(Npoints);
   gxy->SetMarkerStyle(6);
   gxy->SetMarkerColor(kGreen+2);
   gxy->SetTitle("G4/Garf++ Reco Hits X-Y;x [mm];y [mm]");
   TGraph* grz = new TGraph(Npoints);
   grz->SetMarkerStyle(6);
   grz->SetMarkerColor(kGreen+2);
   grz->SetTitle("G4/Garf++ Reco Hits R-Z;r [mm];z [mm]");
   TGraph* grphi = new TGraph(Npoints);
   grphi->SetMarkerStyle(6);
   grphi->SetMarkerColor(kGreen+2);
   grphi->SetTitle("G4/Garf++ Reco Hits R-#phi;r [mm];#phi [deg]");
   TGraph* gzphi = new TGraph(Npoints);
   gzphi->SetMarkerStyle(6);
   gzphi->SetMarkerColor(kGreen+2);
   gzphi->SetTitle("G4/Garf++ Reco Hits Z-#phi;z [mm];#phi [deg]");
   for( int i=0; i<Npoints; ++i )
      {
         //TMChit* h = (TMChit*) points->ConstructedAt(i);
         TMChit* h = (TMChit*) points->At(i);
         gxy->SetPoint(i,h->GetX(),h->GetY());
         grz->SetPoint(i,h->GetRadius(),h->GetZ());
         grphi->SetPoint(i,h->GetRadius(),h->GetPhi()*TMath::RadToDeg());
         gzphi->SetPoint(i,h->GetZ(),h->GetPhi()*TMath::RadToDeg());
      }
   c->cd(1);
   gxy->Draw("AP");
   gxy->GetXaxis()->SetRangeUser(109.,190.);
   gxy->GetYaxis()->SetRangeUser(0.,190.);
   c->cd(2);
   grz->Draw("AP");
   grz->GetXaxis()->SetRangeUser(109.,190.);
   grz->GetYaxis()->SetRangeUser(-10.,10.);
   c->cd(3);
   grphi->Draw("AP");
   grphi->GetXaxis()->SetRangeUser(109.,190.);
   grphi->GetYaxis()->SetRangeUser(0.,40.);
   c->cd(4);
   TH1D* hh = new TH1D("hh","G4/Garf++ Reco Hits Z-#phi;z [mm];#phi [deg]",1,-10.,10.);
   hh->SetStats(kFALSE);
   hh->Draw();
   //  gzphi->Draw("AP");
   gzphi->Draw("Psame");
   //  gzphi->GetXaxis()->SetRangeUser(-10.,10.);
   //  gzphi->GetYaxis()->SetRangeUser(20.,30.);
   hh->GetYaxis()->SetRangeUser(21.,28.);
}

void PlotAWhits(TCanvas* c, const TClonesArray* points) // ideal reconstruction, no deconvolution
{
   LookUpTable fSTR(0.3, 1.); // uniform field version (simulation)
   int Npoints = points->GetEntries();
   TGraph* gxy = new TGraph(Npoints);
   gxy->SetMarkerStyle(6);
   gxy->SetMarkerColor(kBlue);
   gxy->SetTitle("G4/Garf++ Reco Hits X-Y;x [mm];y [mm]");
   TGraph* grz = new TGraph(Npoints);
   grz->SetMarkerStyle(6);
   grz->SetMarkerColor(kBlue);
   grz->SetTitle("G4/Garf++ Reco Hits R-Z;r [mm];z [mm]");
   TGraph* grphi = new TGraph(Npoints);
   grphi->SetMarkerStyle(6);
   grphi->SetMarkerColor(kBlue);
   grphi->SetTitle("G4/Garf++ Reco Hits R-#phi;r [mm];#phi [deg]");
   TGraph* gzphi = new TGraph(Npoints);
   gzphi->SetMarkerStyle(6);
   gzphi->SetMarkerColor(kBlue);
   gzphi->SetTitle("G4/Garf++ Reco Hits Z-#phi;z [mm];#phi [deg]");
   for( int j=0; j<Npoints; ++j )
      {
         TMChit* h = (TMChit*) points->At(j);
         double time = h->GetTime(),
            zed = h->GetZ();
         double rad = fSTR.GetRadius( time , zed );
         double phi = h->GetPhi() - fSTR.GetAzimuth( time , zed );
         if( phi < 0. ) phi += TMath::TwoPi();
         if( phi >= TMath::TwoPi() )
            phi = fmod(phi,TMath::TwoPi());
         double y = rad*TMath::Sin( phi ),
            x = rad*TMath::Cos( phi );
         gxy->SetPoint(j,x,y);
         grz->SetPoint(j,rad,zed);
         grphi->SetPoint(j,rad,phi*TMath::RadToDeg());
         gzphi->SetPoint(j,zed,phi*TMath::RadToDeg());
      }
   c->cd(1);
   gxy->Draw("Psame");
   c->cd(2);
   grz->Draw("Psame");
   c->cd(3);
   grphi->Draw("Psame");
   c->cd(4);
   gzphi->Draw("Psame");
}

void PlotRecoPoints(TCanvas* c, const TClonesArray* points) // points from deconvolution
{
   int Npoints = points->GetEntries();
   std::cout<<"[utils]#  Reco points --> "<<Npoints<<std::endl;
   TGraph* gxy = new TGraph(Npoints);
   gxy->SetMarkerStyle(2);
   gxy->SetMarkerColor(kRed);
   gxy->SetTitle("Reco Hits X-Y;x [mm];y [mm]");
   TGraph* grz = new TGraph(Npoints);
   grz->SetMarkerStyle(2);
   grz->SetMarkerColor(kRed);
   grz->SetTitle("Reco Hits R-Z;r [mm];z [mm]");
   TGraph* grphi = new TGraph(Npoints);
   grphi->SetMarkerStyle(2);
   grphi->SetMarkerColor(kRed);
   grphi->SetTitle("Reco Hits R-#phi;r [mm];#phi [deg]");
   TGraph* gzphi = new TGraph(Npoints);
   gzphi->SetMarkerStyle(2);
   gzphi->SetMarkerColor(kRed);
   gzphi->SetTitle("Reco Hits Z-#phi;z [mm];#phi [deg]");
   for( int i=0; i<Npoints; ++i )
      {
         //TSpacePoint* p = (TSpacePoint*) points->ConstructedAt(i);
         TSpacePoint* p = (TSpacePoint*) points->At(i);
         gxy->SetPoint(i,p->GetX(),p->GetY());
         grz->SetPoint(i,p->GetR(),p->GetZ());
         grphi->SetPoint(i,p->GetR(),p->GetPhi()*TMath::RadToDeg());
         gzphi->SetPoint(i,p->GetZ(),p->GetPhi()*TMath::RadToDeg());
      }
   c->cd(1);
   gxy->Draw("Psame");
   c->cd(2);
   grz->Draw("Psame");
   c->cd(3);
   grphi->Draw("Psame");
   c->cd(4);
   gzphi->Draw("Psame");
}

void PointErrors(Histo *h, const TClonesArray* points) // points from deconvolution
{
   int Npoints = points->GetEntries();
   std::cout<<"[utils]#  Reco points --> "<<Npoints<<std::endl;
   for( int i=0; i<Npoints; ++i )
      {
         //TSpacePoint* p = (TSpacePoint*) points->ConstructedAt(i);
         TSpacePoint* p = (TSpacePoint*) points->At(i);
         h->FillHisto("herrX", p->GetErrX());
         h->FillHisto("herrY", p->GetErrY());
         h->FillHisto("herrZ", p->GetErrZ());
         h->FillHisto("herrR", p->GetErrR());
         h->FillHisto("herrPhi", p->GetErrPhi()*180./M_PI);
      }
}

void PlotTracksFound(TCanvas* c, const TClonesArray* tracks) // points from deconvolution identified to track
{
   const int Ntracks = tracks->GetEntries();
   std::cout<<"[utils]#  Reco tracks --> "<<Ntracks<<std::endl;
   // int cols[] = {kBlack,kGray,kGray+1,kGray+2,kGray+3};
   int cols[] = {kBlack,kMagenta,kCyan,kOrange,kViolet,kGray,kPink,kTeal,kSpring};
   int ncols = 9;
   // if(Ntracks > 9) Ntracks = 9;
   for(int t=0; t<Ntracks; ++t)
      {
         TTrack* aTrack = (TTrack*) tracks->At(t);
         int Npoints = aTrack->GetNumberOfPoints();
         std::cout<<"[utils]#  Reco points in track --> "<<Npoints<<std::endl;
         TGraphErrors* gxy = new TGraphErrors(Npoints);
         gxy->SetMarkerStyle(2);
         gxy->SetMarkerColor(cols[t%ncols]);
         gxy->SetLineColor(cols[t%ncols]);
         gxy->SetTitle("Reco Hits X-Y;x [mm];y [mm]");
         TGraphErrors* grz = new TGraphErrors(Npoints);
         grz->SetMarkerStyle(2);
         grz->SetMarkerColor(cols[t%ncols]);
         grz->SetLineColor(cols[t%ncols]);
         grz->SetTitle("Reco Hits R-Z;r [mm];z [mm]");
         TGraphErrors* grphi = new TGraphErrors(Npoints);
         grphi->SetMarkerStyle(2);
         grphi->SetMarkerColor(cols[t%ncols]);
         grphi->SetLineColor(cols[t%ncols]);
         grphi->SetTitle("Reco Hits R-#phi;r [mm];#phi [deg]");
         TGraphErrors* gzphi = new TGraphErrors(Npoints);
         gzphi->SetMarkerStyle(2);
         gzphi->SetMarkerColor(cols[t%ncols]);
         gzphi->SetLineColor(cols[t%ncols]);
         gzphi->SetTitle("Reco Hits Z-#phi;z [mm];#phi [deg]");
         const std::vector<TSpacePoint*>* points = aTrack->GetPointsArray();
         for( uint i=0; i<points->size(); ++i )
            {
               TSpacePoint* p = (TSpacePoint*) points->at(i);

               gxy->SetPoint(i,p->GetX(),p->GetY());
               gxy->SetPointError(i,p->GetErrX(),p->GetErrY());

               grz->SetPoint(i,p->GetR(),p->GetZ());
               grz->SetPointError(i,p->GetErrR(),p->GetErrZ());

               grphi->SetPoint(i,p->GetR(),p->GetPhi()*TMath::RadToDeg());
               grphi->SetPointError(i,p->GetErrR(),p->GetErrPhi()*TMath::RadToDeg());

               gzphi->SetPoint(i,p->GetZ(),p->GetPhi()*TMath::RadToDeg());
               gzphi->SetPointError(i,p->GetErrZ(),p->GetErrPhi()*TMath::RadToDeg());
            }
         c->cd(1);
         gxy->Draw("Psame");
         c->cd(2);
         grz->Draw("Psame");
         c->cd(3);
         grphi->Draw("Psame");
         c->cd(4);
         gzphi->Draw("Psame");
      }
}

void DrawTPCxy(TCanvas* c)
{
   TEllipse* TPCcath = new TEllipse(0.,0.,109.,109.);
   TPCcath->SetFillStyle(0);
   c->cd(1);
   TPCcath->Draw("same");
   TEllipse* TPCpads = new TEllipse(0.,0.,190.,190.);
   TPCpads->SetFillStyle(0);
   c->cd(1);
   TPCpads->Draw("same");

   double pitch = 2.*M_PI / 256., offset = 0.5*pitch;

   TPolyMarker* AWxy = new TPolyMarker(256);
   AWxy->SetMarkerStyle(45);
   AWxy->SetMarkerColor(kBlack);
   TPolyMarker* AWrphi = new TPolyMarker(256);
   AWrphi->SetMarkerStyle(45);
   AWrphi->SetMarkerColor(kBlack);
   TPolyMarker* FWxy = new TPolyMarker(256);
   FWxy->SetMarkerStyle(43);
   FWxy->SetMarkerColor(kBlack);
   TPolyMarker* FWrphi = new TPolyMarker(256);
   FWrphi->SetMarkerStyle(43);
   FWrphi->SetMarkerColor(kBlack);
   TLine* AWzphi[256];
   for( int p = 0; p<256; ++p )
      {
         double AWphi = pitch * p + offset;
         AWxy->SetPoint(p,182.*cos(AWphi),182.*sin(AWphi));
         AWrphi->SetPoint(p,182.,AWphi*TMath::RadToDeg());

         double FWphi = pitch * p;
         FWxy->SetPoint(p,174.*cos(FWphi),174.*sin(FWphi));
         FWrphi->SetPoint(p,174.,FWphi*TMath::RadToDeg());

         AWzphi[p] = new TLine(-10.,AWphi*TMath::RadToDeg(),10.,AWphi*TMath::RadToDeg());
         AWzphi[p]->SetLineColor(kGray+1);
         AWzphi[p]->SetLineStyle(2);
         AWzphi[p]->SetLineWidth(1);
         c->cd(4);
         AWzphi[p]->Draw("same");
      }

   c->cd(1);
   AWxy->Draw("same");
   FWxy->Draw("same");

   c->cd(3);
   AWrphi->Draw("same");
   FWrphi->Draw("same");

   TLine* AWrz = new TLine(182.,-10.,182.,10.);
   AWrz->SetLineColor(kGray+1);
   AWrz->SetLineStyle(2);
   AWrz->SetLineWidth(2);
   c->cd(2);
   AWrz->Draw("same");
   TLine* FWrz = new TLine(174.,-10.,174.,10.);
   FWrz->SetLineColor(kGray+1);
   FWrz->SetLineStyle(3);
   FWrz->SetLineWidth(2);
   c->cd(2);
   FWrz->Draw("same");
}

void PrintSignals(std::vector<signal>* sig)
{
   for(auto s: *sig)
      s.print();
}

TH1D* PlotSignals(std::vector<signal>* sig, std::string name)
{
   std::ostringstream hname;
   hname<<"hsig"<<name;
   std::string htitle(";t [ns];H [a.u.]");
   TH1D* h = new TH1D(hname.str().c_str(),htitle.c_str(),411,0.,16.*411.);
   h->SetStats(kFALSE);
   for(auto s: *sig)
      {
         if( s.t < 16. ) continue;
         h->Fill(s.t,s.height);
      }
   return h;
}

TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name)
{
   std::ostringstream hname;
   hname<<"hocc"<<name;
   std::string htitle("Occupancy Azimuth;AW index / PAD sector");
   TH1D* h = new TH1D(hname.str().c_str(),htitle.c_str(),256,0.,256.);
   h->SetStats(kFALSE);
   for(auto s: *sig)
      {
         if( s.t < 16. ) continue;
         if( name == "anodes" )
            h->Fill(s.idx);
         else if( name == "pads" )
            {
               // int col = s.sec*8+4;
               for(int i=0; i<8; ++i)
                  {
                     int col = s.sec*8+i;
                     h->Fill(col);
                  }
            }
      }
   return h;
}

TH2D* PlotSignals(std::vector<signal>* awsignals,
		  std::vector<signal>* padsignals, std::string type="none")
{
   std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(),
                                                      awsignals->end());
   std::multiset<signal, signal::timeorder> pad_bytime(padsignals->begin(),
                                                       padsignals->end());
   int Nmatch=0;
   std::ostringstream hname;
   hname<<"hmatch"<<type;
   std::ostringstream htitle;
   htitle<<"AW vs PAD Time with Matching "<<type<<";AW [ns];PAD [ns]";
   TH2D* hh = new TH2D(hname.str().c_str(),htitle.str().c_str(),375,0.,6000.,375,0.,6000.);
   hh->SetStats(kFALSE);
   for( auto iaw=aw_bytime.begin(); iaw!=aw_bytime.end(); ++iaw )
      {
         for( auto ipd=pad_bytime.begin(); ipd!=pad_bytime.end(); ++ipd )
            {
               bool match=false;
               if( type == "time" )
                  {
                     double delta = fabs( iaw->t - ipd->t );
                     if( delta < 16. )
                        match=true;
                  }
               else if( type == "sector" )
                  {
                     short sector = short(iaw->idx/8);
                     if( sector == ipd->sec )
                        match=true;
                  }
               else if( type == "both" )
                  {
                     double delta = fabs( iaw->t - ipd->t );
                     short sector = short(iaw->idx/8);
                     if( delta < 16. && sector == ipd->sec )
                        match=true;
                  }
               else
                  match=true;

               if( match )
                  {
                     hh->Fill( iaw->t , ipd->t );
                     ++Nmatch;
                  }
            }
      }
   return hh;
}

double Average(std::vector<double>* v)
{
   if( v->size() == 0 ) return -1.;
   double avg=0.;
   for( auto& x: *v ) avg+=x;
   return avg/double(v->size());
}

double EvaluateMatch_byResZ(TClonesArray* lines)
{
   int Nlines = lines->GetEntriesFast();
   if( Nlines > 1 )
      std::cerr<<"WARNING EvaluateMatch # of lines = "<<Nlines<<std::endl;
   double resZ=0.;
   for( int n=0; n<Nlines; ++n )
      {
         resZ += ((TFitLine*) lines->At(n))->GetResidual().Z();
      }
   if( Nlines != 0 ) resZ/=double(Nlines);
   return resZ;
}

int EvaluatePattRec(TClonesArray* lines)
{
   int Nlines = lines->GetEntriesFast();
   if( Nlines > 1 )
      std::cerr<<"WARNING EvaluateMatch # of lines = "<<Nlines<<std::endl;
   int Npoints=0;
   for( int n=0; n<Nlines; ++n )
      {
         Npoints+=((TFitLine*) lines->At(n))->GetNumberOfPoints();
      }
   if( Nlines != 0 ) Npoints/=Nlines;
   return Npoints;
}

double PointResolution(TClonesArray* helices, const TVector3* vtx)
{
   double res=0.,N=double(helices->GetEntriesFast());
   for(int i=0; i<helices->GetEntriesFast(); ++i)
      {
         TFitHelix* hel = (TFitHelix*) helices->At(i);
         TVector3 eval = hel->Evaluate( _trapradius * _trapradius );
         eval.Print();
         res+=(eval-(*vtx)).Mag();
         std::cout<<i<<"\tPointResolution\tEval 3D: "<<(eval-(*vtx)).Mag()<<" mm"<<std::endl;
         std::cout<<i<<"\tPointResolution\tEval Z: "<<fabs(eval.Z()-vtx->Z())<<" mm"<<std::endl;

         hel->SetPoint( vtx );
         TVector3 minpoint;
         hel->MinDistPoint( minpoint );
         minpoint.Print();
         std::cout<<i<<"\tPointResolution\tMinDistPoint 3D: "<<(minpoint-(*vtx)).Mag()<<" mm"<<std::endl;
         std::cout<<i<<"\tPointResolution\tMinDistPoint R: "<<fabs(minpoint.Perp()-vtx->Perp())<<" mm"<<std::endl;
         std::cout<<i<<"\tPointResolution\tMinDistPoint Z: "<<fabs(minpoint.Z()-vtx->Z())<<" mm"<<std::endl;

         TVector3 int1,int2;
         int s = hel->TubeIntersection( int1, int2 );
         if( s )
            {
               int1.Print();
               std::cout<<i<<"\tPointResolution\tIntersection (1) 3D: "<<(int1-(*vtx)).Mag()<<" mm"<<std::endl;
               std::cout<<i<<"\tPointResolution\tIntersection (1) Z: "<<fabs(int1.Z()-vtx->Z())<<" mm"<<std::endl;
               int2.Print();
               std::cout<<i<<"\tPointResolution\tIntersection (2) 3D: "<<(int2-(*vtx)).Mag()<<" mm"<<std::endl;
               std::cout<<i<<"\tPointResolution\tIntersection (2) Z: "<<fabs(int2.Z()-vtx->Z())<<" mm"<<std::endl;
            }
         else
            {
               int1.Print();
               std::cout<<i<<"\tPointResolution\tIntersection 3D: "<<(int1-(*vtx)).Mag()<<" mm"<<std::endl;
               std::cout<<i<<"\tPointResolution\tIntersection Z: "<<fabs(int1.Z()-vtx->Z())<<" mm"<<std::endl;
            }
      }
   if( N>0 ) res/=N;
   return res;
}

void HelixPlots(Histo* h, TClonesArray* helices)
{
   for(int i=0; i<helices->GetEntriesFast(); ++i)
      {
         TFitHelix* hel = (TFitHelix*) helices->At(i);
         h->FillHisto("hhD",hel->GetD());
         h->FillHisto("hhc",hel->GetC());
         h->FillHisto("hhchi2R",hel->GetRchi2());
         h->FillHisto("hhchi2Z",hel->GetZchi2());
         const vector<TSpacePoint*> *sp = hel->GetPointsArray();
         for( uint ip = 0; ip<sp->size(); ++ip )
            {
               TSpacePoint* ap = sp->at(ip);
               h->FillHisto( "hhspxy" , ap->GetX(), ap->GetY() );
               h->FillHisto( "hhspzp" , ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               h->FillHisto( "hhspzr" , ap->GetZ(), ap->GetR() );
               h->FillHisto( "hhsprp" , ap->GetPhi(), ap->GetR() );
            }
      }
}

void UsedHelixPlots(Histo* h, const TObjArray* helices)
{
   for(int i=0; i<helices->GetEntriesFast(); ++i)
      {
         TFitHelix* hel = (TFitHelix*) helices->At(i);
         h->FillHisto("huhD",hel->GetD());
         h->FillHisto("huhc",hel->GetC());
         h->FillHisto("huhchi2R",hel->GetRchi2());
         h->FillHisto("huhchi2Z",hel->GetZchi2());
         const vector<TSpacePoint*> *sp = hel->GetPointsArray();
         for( uint ip = 0; ip<sp->size(); ++ip )
            {
               TSpacePoint* ap = sp->at(ip);
               h->FillHisto( "huhspxy" , ap->GetX(), ap->GetY() );
               h->FillHisto( "huhspzp" , ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               h->FillHisto( "huhspzr" , ap->GetZ(), ap->GetR() );
               h->FillHisto( "huhsprp" , ap->GetPhi(), ap->GetR() );
            }
      }
}

double VertexResolution(const TVector3* vtx, const TVector3* mcvtx)
{
   TVector3 P(*vtx),Q(*mcvtx);
   TVector3 R = P-Q;
   return R.Mag();
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
