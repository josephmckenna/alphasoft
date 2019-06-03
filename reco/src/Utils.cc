#include "Utils.hh"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TEllipse.h"
#include "TPolyMarker.h"
#include "TLine.h"
#include "TMath.h"

#include "TMChit.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TFitLine.hh"


Utils::Utils(double B):fHisto(),pmap(),
                       fMagneticField(B),tmax(4500.)
{
   BookG4Histos();

   csig = new TCanvas("csig","csig",1400,1400);
   csig->Divide(2,2);
   
   creco = new TCanvas("creco","creco",1400,1400);
   creco->Divide(2,2);
}

Utils::Utils(std::string fname, double B):fHisto(fname),pmap(),
                                          fMagneticField(B),tmax(4500.)
{
   BookRecoHistos();
}

void Utils::BookG4Histos()
{
   fHisto.Book("hNhel","Reconstructed Helices",10,0.,10.);
   fHisto.Book("hhchi2R","Hel #chi^{2}_{R}",100,0.,200.);
   fHisto.Book("hhchi2Z","Hel #chi^{2}_{Z}",100,0.,200.);
   fHisto.Book("hhD","Hel D;[mm]",200,-200.,200.);
   fHisto.Book("hhc","Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
   fHisto.Book("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
               100,-190.,190.,100,-190.,190.);
   fHisto.Book("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
               600,-1200.,1200.,61,109.,174.);
   fHisto.Book("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
               600,-1200.,1200.,100,0.,360.);
   fHisto.Book("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
               100,0.,TMath::TwoPi(),61,109.,174.);
   fHisto.Book("hNusedhel","Used Helices",10,0.,10.);
   fHisto.Book("huhchi2R","Used Hel #chi^{2}_{R}",100,0.,200.);
   fHisto.Book("huhchi2Z","Used Hel #chi^{2}_{Z}",100,0.,200.);
   fHisto.Book("huhD","Used Hel D;[mm]",200,-200.,200.);
   fHisto.Book("huhc","Used Hel c;[mm^{-1}]",200,-1.e-1,1.e-1);
   fHisto.Book("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
               100,-190.,190.,100,-190.,190.);
   fHisto.Book("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
               600,-1200.,1200.,61,109.,174.);
   fHisto.Book("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
               600,-1200.,1200.,100,0.,360.);
   fHisto.Book("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
               100,0.,TMath::TwoPi(),90,109.,174.);
   fHisto.Book("hvtxres","Vertex Resolution;[mm]",200,0.,200.);
}

void Utils::BookRecoHistos()
{
   fHisto.Book("hNpoints","Reconstructed Spacepoints",1000,0.,1000.);
   fHisto.Book("hNpointstracks","Reconstructed Spacepoints in Found Tracks",1000,0.,1000.);
   fHisto.Book("hNgoodpoints","Reconstructed Spacepoints Used in Tracking",1000,0.,1000.);

   fHisto.Book("hOccAwpoints","Aw Occupancy for Points;aw",256,-0.5,255.5);
   fHisto.Book("hAwpointsOccIsec","Number of AW hits Inside Pad Sector for Points;N",8,0.,8.);
   fHisto.GetHisto("hAwpointsOccIsec")->SetMinimum(0);
   fHisto.Book("hOccPadpoints","Pad Occupancy for Points;row;sec",576,-0.5,575.5,32,-0.5,31.5);

   fHisto.Book("hspzphipoints","Spacepoint Axial-Azimuth for Points;z [mm];#phi [deg]",
               500,-1152.,1152.,100,0.,360.);
   fHisto.Book("hspxypoints","Spacepoint X-Y for Points;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

   fHisto.Book("hNtracks","Reconstructed Tracks",10,0.,10.);
   fHisto.Book("hNgoodtracks","Reconstructed Good tracks",10,0.,10.);

   fHisto.Book("hpattreceff","Reconstructed Spacepoints/Tracks",200,0.,200.);
   fHisto.Book("hgoodpattreceff","Reconstructed Good Spacepoints/Tracks",200,0.,200.);

   fHisto.Book("hOccPadtracks","Pad Occupancy for Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
   fHisto.Book("hOccAwtracks","Aw Occupancy for Tracks;aw",256,-0.5,255.5);

   fHisto.Book("hspradtracks","Spacepoint Radius for Tracks;r [mm]",100,109.,174.);
   fHisto.Book("hspphitracks","Spacepoint Azimuth for Tracks;#phi [deg]",100,0.,360.);
   fHisto.GetHisto("hspphitracks")->SetMinimum(0);
   fHisto.Book("hspzedtracks","Spacepoint Axial for Tracks;z [mm]",125,-1152.,1152.);

   fHisto.Book("hspzphitracks","Spacepoint Axial-Azimuth for Tracks;z [mm];#phi [deg]",
               500,-1152.,1152.,100,0.,360.);
   fHisto.Book("hspxytracks","Spacepoint X-Y for Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

   fHisto.Book("hchi2","#chi^{2} of Straight Lines",200,0.,200.); // chi^2 of line fit

   fHisto.Book("hhchi2R","Hel #chi^{2}_{R}",200,0.,200.); // R chi^2 of helix
   fHisto.Book("hhchi2Z","Hel #chi^{2}_{Z}",200,0.,1000.); // Z chi^2 of helix
   fHisto.Book("hhD","Hel D;[mm]",500,-190.,190.);
   fHisto.GetHisto("hhD")->SetMinimum(0);

   fHisto.Book("hOccPad","Pad Occupancy for Good Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
   fHisto.Book("hOccAw","Aw Occupancy for Good Tracks;aw",256,-0.5,255.5);
   fHisto.GetHisto("hOccAw")->SetMinimum(0);
   fHisto.Book("hAwOccIsec","Number of AW hits Inside Pad Sector for Good Tracks;N",8,0.,8.);
   fHisto.GetHisto("hAwOccIsec")->SetMinimum(0);

   fHisto.Book("hsprad","Spacepoint Radius for Good Tracks;r [mm]",100,109.,174.);
   fHisto.Book("hspphi","Spacepoint Azimuth for Good Tracks;#phi [deg]",100,0.,360.);
   fHisto.GetHisto("hspphi")->SetMinimum(0);
   fHisto.Book("hspzed","Spacepoint Axial for Good Tracks;z [mm]",125,-1152.,1152.);

   fHisto.Book("hspzphi","Spacepoint Axial-Azimuth for Good Tracks;z [mm];#phi [deg]",
               500,-1152.,1152.,100,0.,360.);
   fHisto.Book("hspxy","Spacepoint X-Y for Good Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

   fHisto.Book("hTrackXaw","Number of Good Tracks per AW;aw",256,-0.5,255.5);
   fHisto.Book("hTrackXpad","Number of Good Tracks per Pad;row;sec",576,-0.5,575.5,32,-0.5,31.5);

   fHisto.Book("hvtxrad","Vertex R;r [mm]",200,0.,190.);
   fHisto.Book("hvtxphi","Vertex #phi;#phi [deg]",360,0.,360.);
   fHisto.GetHisto("hvtxphi")->SetMinimum(0);
   fHisto.Book("hvtxzed","Vertex Z;z [mm]",1000,-1152.,1152.);
   fHisto.Book("hvtxzedphi","Vertex Z-#phi;z [mm];#phi [deg]",100,-1152.,1152.,180,0.,360.);
}

void Utils::FillRecoPointsHistos(const TObjArray* points)
{  
   int row,sec;
   for(int p=0; p<points->GetEntriesFast(); ++p)
      {
         TSpacePoint* ap = (TSpacePoint*) points->At(p);
         fHisto.FillHisto("hOccAwpoints",ap->GetWire());
         fHisto.FillHisto("hAwpointsOccIsec",ap->GetWire()%8);
         pmap.get(ap->GetPad(),sec,row);
         fHisto.FillHisto("hOccPadpoints",row,sec);
         
         fHisto.FillHisto("hspzphipoints",ap->GetZ(),ap->GetPhi()*TMath::RadToDeg());
         fHisto.FillHisto("hspxypoints",ap->GetX(),ap->GetY());
      }
}

void Utils::FillRecoTracksHisto(std::vector<TTrack*>* found_tracks)
{  
   int row,sec;
   Npointstracks=0;
   for(size_t t=0; t<found_tracks->size(); ++t)
      {
         TTrack* at = (TTrack*) found_tracks->at(t);
         const std::vector<TSpacePoint*>* spacepoints = at->GetPointsArray();
         for( auto& it: *spacepoints )
            {
               fHisto.FillHisto("hOccAwtracks",it->GetWire());
               pmap.get(it->GetPad(),sec,row);
               fHisto.FillHisto("hOccPadtracks",row,sec);
		  
               fHisto.FillHisto("hspradtracks",it->GetR());
               fHisto.FillHisto("hspphitracks",it->GetPhi()*TMath::RadToDeg());
               fHisto.FillHisto("hspzedtracks",it->GetZ());
		  
               fHisto.FillHisto("hspzphitracks",it->GetZ(),it->GetPhi()*TMath::RadToDeg());
               fHisto.FillHisto("hspxytracks",it->GetX(),it->GetY());
               ++Npointstracks;
            }
      }
}

void Utils::FillFitTracksHisto(std::vector<TTrack*>* tracks_array)
{  
   int row,sec;
   Npoints=0;
   std::set<int> trkXpad;
   std::set<int> trXaw;
   for(size_t t=0; t<tracks_array->size(); ++t)
      {
         TTrack* at = (TTrack*) tracks_array->at(t);
         const std::vector<TSpacePoint*>* spacepoints = at->GetPointsArray();
         for( auto& it: *spacepoints )
            {
               fHisto.FillHisto("hOccAw",it->GetWire());
               fHisto.FillHisto("hAwOccIsec",it->GetWire()%8);
               trXaw.insert(it->GetWire());
               pmap.get(it->GetPad(),sec,row);
               trkXpad.insert(it->GetPad());
               fHisto.FillHisto("hOccPad",row,sec);
		  
               fHisto.FillHisto("hsprad",it->GetR());
               fHisto.FillHisto("hspphi",it->GetPhi()*TMath::RadToDeg());
               fHisto.FillHisto("hspzed",it->GetZ());
		  
               fHisto.FillHisto("hspzphi",it->GetZ(),it->GetPhi()*TMath::RadToDeg());
               fHisto.FillHisto("hspxy",it->GetX(),it->GetY());

               ++Npoints;
            }
              
         for(auto iaw = trXaw.begin(); iaw != trXaw.end(); ++iaw)
            {
               fHisto.FillHisto("hTrackXaw",*iaw);
            }
         for(auto ipd = trkXpad.begin(); ipd != trkXpad.end(); ++ipd)
            {
               pmap.get(*ipd,sec,row);
               fHisto.FillHisto("hTrackXpad",row,sec);                
            }

         if( fMagneticField > 0. )
            {
               double chi2 = ((TFitHelix*)at)->GetRchi2();
               double ndf = (double) ((TFitHelix*)at)->GetRDoF();
               fHisto.FillHisto("hhchi2R",chi2/ndf);

               chi2 = ((TFitHelix*)at)->GetZchi2();
               ndf = (double) ((TFitHelix*)at)->GetZDoF();
               fHisto.FillHisto("hhchi2Z",chi2/ndf);

               fHisto.FillHisto("hhD", ((TFitHelix*)at)->GetD() );
            }
         else
            {
               double ndf= (double) ((TFitLine*)at)->GetDoF();
               double chi2 = ((TFitLine*)at)->GetChi2();
               fHisto.FillHisto("hchi2",chi2/ndf);
               // if( gVerb > 1 )
               //    cout<<"\t"<<t<<" chi^2: "<<chi2<<" ndf: "<<ndf<<endl;
            }
      }
}

void Utils::FillRecoVertex(const TFitVertex* Vertex)
{
   fHisto.FillHisto("hvtxrad",Vertex->GetRadius());
   double phi = Vertex->GetAzimuth();
   if( phi < 0. ) phi+=TMath::TwoPi();
   phi*=TMath::RadToDeg();
   fHisto.FillHisto("hvtxphi",phi);
   fHisto.FillHisto("hvtxzed",Vertex->GetElevation());
   fHisto.FillHisto("hvtxzedphi",Vertex->GetElevation(),phi);
}

void Utils::FillFinalHistos(const Reco* r, int ntracks)
{
   fHisto.FillHisto("hNpoints",r->GetNumberOfPoints());
   if(Npointstracks) fHisto.FillHisto("hNpointstracks",Npointstracks);
   if(Npoints) fHisto.FillHisto("hNgoodpoints",Npoints);

   fHisto.FillHisto("hNtracks",r->GetNumberOfTracks());
   fHisto.FillHisto("hNgoodtracks",double(ntracks));

   if( r->GetNumberOfTracks() > 0 )
      fHisto.FillHisto("hpattreceff", double(Npointstracks)/double(r->GetNumberOfTracks()));
   else
      fHisto.FillHisto("hpattreceff",0.);
      
   if( ntracks > 0 )
      fHisto.FillHisto("hgoodpattreceff", double(Npoints)/double(ntracks));
   else
      fHisto.FillHisto("hgoodpattreceff",0.);
}

// ===============================================================================================
void Utils::DebugNeuralNet(NeuralFinder* pattrec)
{
   TH1D *hw = new TH1D("hw","pattrec point weights",20,0,2.);
   std::vector<double> pw = pattrec->GetPointWeights();
   for(double w: pw) hw->Fill(w);
   new TCanvas;
   hw->Draw();
   
   TH1D *hinw = new TH1D("hinw","pattrec in neuron weights",200,0,2.);
   std::vector<double> inw = pattrec->GetInNeuronWeights();
   assert(inw.size());
   for(double w: inw) hinw->Fill(w);
   new TCanvas;
   hinw->Draw();
   
   TH1D *honw = new TH1D("honw","pattrec out neuron weights",200,0,2.);
   std::vector<double> onw = pattrec->GetOutNeuronWeights();
   for(double w: onw) honw->Fill(w);
   new TCanvas;
   honw->Draw();
   
   TH1D *hnv = new TH1D("hnv","pattrec neuron V",200,0,2.);
   std::vector<double> nv = pattrec->GetNeuronV();
   for(double v: nv) hnv->Fill(v);
   new TCanvas;
   hnv->Draw();
}

void Utils::DebugNeuralNetMC(NeuralFinder* pattrec)
{
   TH1D *hwMC = new TH1D("hwMC","MCpattrec point weights",20,0,2.);
   vector<double> pwMC = pattrec->GetPointWeights();
   for(double w: pwMC) hwMC->Fill(w);
   new TCanvas;
   hwMC->Draw();
}

void Utils::DisplayNeuralNet(NeuralFinder* pattrec)
{
   for(int i = 0; i < pattrec->GetNumberOfTracks(); i++)
      PlotNeurons(creco, pattrec->GetTrackNeurons(i), kGray+1);

   PlotNeurons(creco, pattrec->GetMetaNeurons(), kRed);
   // PlotNeurons(creco, pattrec->GetTrackNeurons(1), kMagenta);
   // PlotNeurons(creco, pattrec->GetTrackNeurons(2), kCyan);
   // PlotNeurons(creco, pattrec->GetTrackNeurons(3), kOrange);
   // PlotNeurons(creco, pattrec->GetTrackNeurons(4), kViolet);
}

void Utils::PlotNeurons(TCanvas* c, const set<NeuralFinder::Neuron*> &neurons, int col_ )
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

// ===============================================================================================

void Utils::Display(const TClonesArray* mcpoints, const TClonesArray* awpoints,
                    const std::vector<TSpacePoint*>* recopoints,
                    const std::vector<TTrack*>* tracks)
{
   PlotMCpoints(creco,mcpoints);
   PlotAWhits(creco, awpoints );
   PlotRecoPoints(creco, recopoints);
   PlotTracksFound(creco, tracks);
   DrawTPCxy(creco);
}


void Utils::PlotMCpoints(TCanvas* c, const TClonesArray* points)
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

void Utils::PlotAWhits(TCanvas* c, const TClonesArray* points)
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

void Utils::PlotRecoPoints(TCanvas* c, const std::vector<TSpacePoint*>* points)
{
   int Npoints = points->size();
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
         TSpacePoint* p = (TSpacePoint*) points->at(i);
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

void Utils::PlotTracksFound(TCanvas* c, const std::vector<TTrack*>* tracks)
{
   const int Ntracks = tracks->size();
   std::cout<<"[utils]#  Reco tracks --> "<<Ntracks<<std::endl;
   // int cols[] = {kBlack,kGray,kGray+1,kGray+2,kGray+3};
   int cols[] = {kBlack,kMagenta,kCyan,kOrange,kViolet,kGray,kPink,kTeal,kSpring};
   int ncols = 9;
   // if(Ntracks > 9) Ntracks = 9;
   for(int t=0; t<Ntracks; ++t)
      {
         TTrack* aTrack = (TTrack*) tracks->at(t);
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

void Utils::DrawTPCxy(TCanvas* c)
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

void Utils::PrintSignals(std::vector<signal>* sig)
{
   for(auto s: *sig)
      s.print();
}

TH1D* Utils::PlotSignals(std::vector<signal>* sig, std::string name)
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

void Utils::Draw(std::vector<signal>* awsig, std::vector<signal>* padsig, std::vector<signal>* combpads)
{
   TH1D* haw=PlotSignals( awsig, "anodes" );
   haw->Scale(1./haw->Integral());
   haw->SetLineColor(kRed);
   //cout<<"[main]# "<<i<<"\tPlotAnodeTimes: "<<haw->GetEntries()<<endl;
   csig->cd(1);
   haw->Draw("hist");
   haw->SetTitle("Deconv Times");
   haw->GetXaxis()->SetRangeUser(0.,tmax);

   TH1D* hpads = PlotSignals( padsig, "pads" );
   hpads->Scale(1./hpads->Integral());
   hpads->SetLineColor(kBlue);
   csig->cd(1);
   hpads->Draw("histsame");

   TH1D* hcombpads = PlotSignals( combpads, "combinedpads" );
   hcombpads->Scale(1./hcombpads->Integral());
   hcombpads->SetLineColor(kBlue);
   csig->cd(2);
   haw->Draw("hist");
   hcombpads->Draw("histsame");

   TH2D* hmatch = PlotSignals( awsig, combpads, "sector");
   //TH2D* hmatch = PlotSignals( awsig, padsig, "sector");
   csig->cd(3);
   hmatch->Draw();
   hmatch->GetXaxis()->SetRangeUser(0.,tmax);
   hmatch->GetYaxis()->SetRangeUser(0.,tmax);
   
   TH1D* hoccaw = PlotOccupancy( awsig, "anodes" );
   hoccaw->Scale(1./hoccaw->Integral());
   hoccaw->SetLineColor(kRed);
   TH1D* hocccombpads = PlotOccupancy( combpads, "pads" );
   hocccombpads->Scale(1./hocccombpads->Integral());
   hocccombpads->SetLineColor(kBlue);
   csig->cd(4);
   hoccaw->Draw("hist");
   hocccombpads->Draw("histsame");
}


TH1D* Utils::PlotOccupancy(std::vector<signal>* sig, std::string name)
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

TH2D* Utils::PlotSignals(std::vector<signal>* awsignals,
                         std::vector<signal>* padsignals, std::string type)
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

// ===============================================================================================
double Utils::Average(std::vector<double>* v)
{
   if( v->size() == 0 ) return -1.;
   double avg=0.;
   for( auto& x: *v ) avg+=x;
   return avg/double(v->size());
}

double Utils::EvaluateMatch_byResZ(TClonesArray* lines)
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

int Utils::EvaluatePattRec(TClonesArray* lines)
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

// ===============================================================================================
double Utils::PointResolution(std::vector<TFitHelix*>* helices, const TVector3* vtx)
{
   double res=0.,N=double(helices->size());
   for(size_t i=0; i<helices->size(); ++i)
      {
         TFitHelix* hel = (TFitHelix*) helices->at(i);
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

// ===============================================================================================
void Utils::HelixPlots(std::vector<TFitHelix*>* helices)
{
   int nhel=0;
   for(size_t i=0; i<helices->size(); ++i)
      {
         TFitHelix* hel = helices->at(i);
         fHisto.FillHisto("hhD",hel->GetD());
         fHisto.FillHisto("hhc",hel->GetC());
         fHisto.FillHisto("hhchi2R",hel->GetRchi2());
         fHisto.FillHisto("hhchi2Z",hel->GetZchi2());
         ++nhel;
         const vector<TSpacePoint*> *sp = hel->GetPointsArray();
         for( uint ip = 0; ip<sp->size(); ++ip )
            {
               TSpacePoint* ap = sp->at(ip);
               fHisto.FillHisto( "hhspxy" , ap->GetX(), ap->GetY() );
               fHisto.FillHisto( "hhspzp" , ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               fHisto.FillHisto( "hhspzr" , ap->GetZ(), ap->GetR() );
               fHisto.FillHisto( "hhsprp" , ap->GetPhi(), ap->GetR() );
            }
      }
   fHisto.FillHisto("hNhel",double(nhel));
}

void Utils::UsedHelixPlots(const TObjArray* helices)
{
   int nhel = 0;
   for(int i=0; i<helices->GetEntriesFast(); ++i)
      {
         TFitHelix* hel = (TFitHelix*) helices->At(i);
         fHisto.FillHisto("huhD",hel->GetD());
         fHisto.FillHisto("huhc",hel->GetC());
         fHisto.FillHisto("huhchi2R",hel->GetRchi2());
         fHisto.FillHisto("huhchi2Z",hel->GetZchi2());
         ++nhel;
         const vector<TSpacePoint*> *sp = hel->GetPointsArray();
         for( uint ip = 0; ip<sp->size(); ++ip )
            {
               TSpacePoint* ap = sp->at(ip);
               fHisto.FillHisto( "huhspxy" , ap->GetX(), ap->GetY() );
               fHisto.FillHisto( "huhspzp" , ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               fHisto.FillHisto( "huhspzr" , ap->GetZ(), ap->GetR() );
               fHisto.FillHisto( "huhsprp" , ap->GetPhi(), ap->GetR() );
            }
      }   
   fHisto.FillHisto("hNusedhel",double(nhel));
   //   std::cout<<"Utils::UsedHelixPlots Used Helices: "<<nhel<<std::endl;
}


void Utils::UsedHelixPlots(const std::vector<TFitHelix*>* helices)
{
   int nhel = 0;
   for(size_t i=0; i<helices->size(); ++i)
      {
         TFitHelix* hel =  helices->at(i);
         fHisto.FillHisto("huhD",hel->GetD());
         fHisto.FillHisto("huhc",hel->GetC());
         fHisto.FillHisto("huhchi2R",hel->GetRchi2());
         fHisto.FillHisto("huhchi2Z",hel->GetZchi2());
         ++nhel;
         const vector<TSpacePoint*> *sp = hel->GetPointsArray();
         for( uint ip = 0; ip<sp->size(); ++ip )
            {
               TSpacePoint* ap = sp->at(ip);
               fHisto.FillHisto( "huhspxy" , ap->GetX(), ap->GetY() );
               fHisto.FillHisto( "huhspzp" , ap->GetZ(), ap->GetPhi()*TMath::RadToDeg() );
               fHisto.FillHisto( "huhspzr" , ap->GetZ(), ap->GetR() );
               fHisto.FillHisto( "huhsprp" , ap->GetPhi(), ap->GetR() );
            }
      }
   fHisto.FillHisto("hNusedhel",double(nhel));
   //   std::cout<<"Utils::UsedHelixPlots Used Helices: "<<nhel<<std::endl;
}

// ===============================================================================================
double Utils::VertexResolution(const TVector3* vtx, const TVector3* mcvtx)
{
   TVector3 P(*vtx),Q(*mcvtx);
   TVector3 R = P-Q;
   double res = R.Mag();
   fHisto.FillHisto("hvtxres",res);
   return res;
}

// ===============================================================================================
void Utils::WriteSettings(TObjString* sett)
{
   std::cout<<"Utils::WriteSettings AnaSettings to rootfile... "<<std::endl;
   int status = fHisto.WriteObject(sett,"ana_settings");
   if( status > 0 ) std::cout<<"Utils: Write AnaSettings Success!"<<std::endl;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
