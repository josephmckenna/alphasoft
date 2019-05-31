#include <iostream>
#include <vector>
#include <string>
using namespace std;
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TClonesArray.h>

#include <TH1D.h>
#include <TH2D.h>
#include <TRegexp.h>

#include "TStoreEvent.hh"
#include "SignalsType.h"

#include "TSpacePoint.hh"
#include "TFitVertex.hh"

#include "TFitLine.hh"

#include "CosmicFinder.hh"

#include "argparse.hh"
#include "Reco.hh"

int gVerb = 0;

TString GetTag( string filename )
{
  TString fname = filename;
  TRegexp re("[0-9][0-9][0-9][0-9][0-9]");
  int pos = fname.Index(re);
  int run = TString(fname(pos,5)).Atoi();
  TString tag("_R");
  tag+=run;
  return tag;
}

int main(int argc, char** argv)
{
  // make a new ArgumentParser
  ArgumentParser parser;
  // add some arguments
  parser.addArgument("-f","--rootfile",1,false);
  parser.addArgument("-a","--anasettings",1,true);
  parser.addArgument("-b","--Bfield",1,true);
  parser.addArgument("-e","--Nevents",1,true);
  parser.addArgument("-v","--verbose",1,true);
  parser.addArgument("--finder",1,true);
  parser.addArgument("-t","--text",1,true);
  // parse the command-line arguments - throws if invalid format
  parser.parse(argc, argv);

  string fname = parser.retrieve<string>("rootfile");
  TFile* fin = TFile::Open(fname.c_str(),"READ");
  if( !fin->IsOpen() ) return 2;

  TTree* tEvents = (TTree*) fin->Get("StoreEventTree");
  if( !tEvents ) return 3;
  int Nevents = tEvents->GetEntriesFast();
  cout<<tEvents->GetTitle()<<"\t"<<Nevents<<endl;
  
  TStoreEvent* anEvent = new TStoreEvent();
  tEvents->SetBranchAddress("StoredEvent", &anEvent);

  string settings="default";
  if( parser.count("anasettings") )
    {
      string fname = parser.retrieve<string>("anasettings");
      struct stat buffer;   
      if( stat(fname.c_str(), &buffer) == 0 )
	settings = fname;
      else
	cerr<<"AnaSettings "<<fname<<" doesn't exist"<<endl;
    }
  cout<<"AnaSettings: "<<settings<<endl;

  double MagneticField=1.0;
  if( parser.count("Bfield") )
    {
      string Bfield = parser.retrieve<string>("Bfield");
      MagneticField = stod(Bfield);
    }
  cout<<"Magnetic Field: "<<MagneticField<<" T"<<endl;

  if( parser.count("verbose"))
     {
        string verbosity = parser.retrieve<string>("verbose");
        gVerb = stoi(verbosity);
     }
  cout<<"Verbose Level set to: "<<gVerb<<endl;

  finderChoice choosen_finder = adaptive;
  if( parser.count("finder") )
    {
      string cf = parser.retrieve<string>("finder");
      if( cf == "base") choosen_finder = base;
      else if( cf == "neural") choosen_finder = neural;
      else if( cf == "adaptive") choosen_finder = adaptive;
      else cerr<<"Unknown track finder mode \""<<cf<<"\", using adaptive"<<endl;
    }
  cout<<"Using track finder: "<<choosen_finder<<endl;


  if( parser.count("Nevents") )
    {
      string nev = parser.retrieve<string>("Nevents");
      Nevents = stoi(nev);
    }
  cout<<"Processing "<<Nevents<<" events"<<endl;

  string tag="";
  if( parser.count("text") )
     {
        tag = "_";
        tag += parser.retrieve<string>("text");
     }
  cout<<"Additional Text: "<<tag<<endl;

  TString foutname("histo");
  foutname+=GetTag(fname);
  foutname+=tag;
  foutname+=".root";
  TFile* fout = new TFile(foutname,"RECREATE");

  TH1D* hNpoints = new TH1D("hpoints","Reconstructed Spacepoints",1000,0.,1000.);
  TH1D* hNpointstracks = new TH1D("hpointstracks","Reconstructed Spacepoints in Found Tracks",1000,0.,1000.);
  TH1D* hNgoodpoints = new TH1D("hgoodpoints","Reconstructed Spacepoints Used in Tracking",1000,0.,1000.);

  TH1D* hOccAwpoints = new TH1D("hOccAwpoints","Aw Occupancy for Points;aw",256,-0.5,255.5);
  TH1D* hAwpointsOccIsec = new TH1D("hAwpointsOccIsec","Number of AW hits Inside Pad Sector for Points;N",8,0.,8.);
  hAwpointsOccIsec->SetMinimum(0);
  TH2D* hOccPadpoints = new TH2D("hOccPadpoints","Pad Occupancy for Points;row;sec",576,-0.5,575.5,32,-0.5,31.5);

  TH2D* hspzphipoints = new TH2D("hspzphipoints","Spacepoint Axial-Azimuth for Points;z [mm];#phi [deg]",
                           500,-1152.,1152.,100,0.,360.);
  TH2D* hspxypoints = new TH2D("hspxypoints","Spacepoint X-Y for Points;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

  TH1D* hNtracks = new TH1D("hNtracks","Reconstructed Tracks",10,0.,10.);
  TH1D* hNgoodtracks = new TH1D("hNgoodtracks","Reconstructed Good tracks",10,0.,10.);

  TH1D* hpattreceff = new TH1D("hpattreceff","Reconstructed Spacepoints/Tracks",200,0.,200.);
  TH1D* hgoodpattreceff = new TH1D("hgoodpattreceff","Reconstructed Good Spacepoints/Tracks",200,0.,200.);

  TH2D* hOccPadtracks = new TH2D("hOccPadtracks","Pad Occupancy for Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
  TH1D* hOccAwtracks = new TH1D("hOccAwtracks","Aw Occupancy for Tracks;aw",256,-0.5,255.5);

  TH1D* hspradtracks = new TH1D("hspradtracks","Spacepoint Radius for Tracks;r [mm]",100,109.,174.);
  TH1D* hspphitracks = new TH1D("hspphitracks","Spacepoint Azimuth for Tracks;#phi [deg]",180,0.,360.);
  hspphitracks->SetMinimum(0);
  TH1D* hspzedtracks = new TH1D("hspzedtracks","Spacepoint Axial for Tracks;z [mm]",125,-1152.,1152.);

  TH2D* hspzphitracks = new TH2D("hspzphitracks","Spacepoint Axial-Azimuth for Tracks;z [mm];#phi [deg]",
                           500,-1152.,1152.,100,0.,360.);
  TH2D* hspxytracks = new TH2D("hspxytracks","Spacepoint X-Y for Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

  TH1D* hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",200,0.,200.); // chi^2 of line fit

  TH1D* hhchi2R = new TH1D("hhchi2R","Hel #chi^{2}_{R}",200,0.,200.); // R chi^2 of helix
  TH1D* hhchi2Z = new TH1D("hhchi2Z","Hel #chi^{2}_{Z}",200,0.,1000.); // Z chi^2 of helix
  TH1D* hhD = new TH1D("hhD","Hel D;[mm]",500,-190.,190.);
  hhD->SetMinimum(0);

  TH2D* hOccPad = new TH2D("hOccPad","Pad Occupancy for Good Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
  TH1D* hOccAw = new TH1D("hOccAw","Aw Occupancy for Good Tracks;aw",256,-0.5,255.5);
  hOccAw->SetMinimum(0);
  TH1D* hAwOccIsec = new TH1D("hAwOccIsec","Number of AW hits Inside Pad Sector for Good Tracks;N",8,0.,8.);
  hAwOccIsec->SetMinimum(0);

  TH1D* hsprad = new TH1D("hsprad","Spacepoint Radius for Good Tracks;r [mm]",100,109.,174.);
  TH1D* hspphi = new TH1D("hspphi","Spacepoint Azimuth for Good Tracks;#phi [deg]",180,0.,360.);
  hspphi->SetMinimum(0);
  TH1D* hspzed = new TH1D("hspzed","Spacepoint Axial for Good Tracks;z [mm]",125,-1152.,1152.);

  TH2D* hspzphi = new TH2D("hspzphi","Spacepoint Axial-Azimuth for Good Tracks;z [mm];#phi [deg]",
                           500,-1152.,1152.,100,0.,360.);
  TH2D* hspxy = new TH2D("hspxy","Spacepoint X-Y for Good Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

  TH1D* hTrackXaw = new TH1D("hTrackXaw","Number of Good Tracks per AW;aw",256,-0.5,255.5);
  TH2D* hTrackXpad = new TH2D("hTrackXpad","Number of Good Tracks per Pad;row;sec",576,-0.5,575.5,32,-0.5,31.5);

  TH1D* hvtxrad = new TH1D("hvtxrad","Vertex R;r [mm]",200,0.,190.);
  TH1D* hvtxphi = new TH1D("hvtxphi","Vertex #phi;#phi [deg]",360,0.,360.);
  hvtxphi->SetMinimum(0);
  TH1D* hvtxzed = new TH1D("hvtxzed","Vertex Z;z [mm]",1000,-1152.,1152.);
  TH2D* hvtxzedphi = new TH2D("hvtxzedphi","Vertex Z-#phi;z [mm];#phi [deg]",100,-1152.,1152.,180,0.,360.);

  padmap pads;
  int row,sec;

  // =============================================
  // Reconstruction All-In-One
  Reco r( settings, MagneticField );
  // =============================================
  if( gVerb ) r.SetTrace(true);

  // =============================================
  // Cosmic Analysis
  CosmicFinder cosfind( MagneticField );
  // =============================================

  for( int n=0; n<Nevents; ++n)
    {
      tEvents->GetEntry(n);
      const TObjArray* points = anEvent->GetSpacePoints();
      if( n%1000 == 0 || gVerb > 0 )
	cout<<n<<"\tEvent Number: "<<anEvent->GetEventNumber()
	    <<"\tTime of the Event: "<<anEvent->GetTimeOfEvent()<<"s"<<endl;
      for(int p=0; p<points->GetEntriesFast(); ++p)
         {
            TSpacePoint* ap = (TSpacePoint*) points->At(p);
            hOccAwpoints->Fill(ap->GetWire());
            hAwpointsOccIsec->Fill(ap->GetWire()%8);
            pads.get(ap->GetPad(),sec,row);
            hOccPadpoints->Fill(row,sec);
            
            hspzphipoints->Fill(ap->GetZ(),ap->GetPhi()*TMath::RadToDeg());
            hspxypoints->Fill(ap->GetX(),ap->GetY());
         }
      
      r.AddSpacePoint( points );

      int nt = r.FindTracks( choosen_finder );
      if( n%1000 == 0 || gVerb > 0 )
	cout<<"\t# of Points: "<<setw(3)<<points->GetEntriesFast()<<"\t# of Tracks: "<<nt<<endl;

      std::vector<TTrack*>* found_tracks = r.GetTracks();
      int Npointstracks=0;
      for(size_t t=0; t<found_tracks->size(); ++t)
         {
            TTrack* at = (TTrack*) found_tracks->at(t);
            const std::vector<TSpacePoint*>* spacepoints = at->GetPointsArray();
            for( auto& it: *spacepoints )
               {
		  hOccAwtracks->Fill(it->GetWire());
		  pads.get(it->GetPad(),sec,row);
		  hOccPadtracks->Fill(row,sec);
		  
		  hspradtracks->Fill(it->GetR());
		  hspphitracks->Fill(it->GetPhi()*TMath::RadToDeg());
		  hspzedtracks->Fill(it->GetZ());
		  
		  hspzphitracks->Fill(it->GetZ(),it->GetPhi()*TMath::RadToDeg());
		  hspxytracks->Fill(it->GetX(),it->GetY());
                  ++Npointstracks;
               }
         }
      
      int nlin=0, nhel=0;
      if( MagneticField > 0. )
         {
            nhel = r.FitHelix();
            if( gVerb > 1 ) cout<<"\tN hel: "<<nhel<<endl;
         }
      else 
         {
            nlin = r.FitLines();
            if( gVerb > 1 ) cout<<"\tN Lin: "<<nlin<<endl;
         }

      std::vector<TTrack*>* tracks_array=0;
      if( nhel > 0 ) 
         tracks_array = reinterpret_cast<std::vector<TTrack*>*>(r.GetHelices());
      else if( nlin > 0 ) 
         tracks_array = reinterpret_cast<std::vector<TTrack*>*>(r.GetLines());
      
      int Npoints=0;
      std::set<int> trkXpad;
      std::set<int> trXaw;
      if( tracks_array ) 
	{
	  for(size_t t=0; t<tracks_array->size(); ++t)
	    {
	      TTrack* at = (TTrack*) tracks_array->at(t);
	      const std::vector<TSpacePoint*>* spacepoints = at->GetPointsArray();
	      for( auto& it: *spacepoints )
		{
		  hOccAw->Fill(it->GetWire());
                  hAwOccIsec->Fill(it->GetWire()%8);
                  trXaw.insert(it->GetWire());
		  pads.get(it->GetPad(),sec,row);
                  trkXpad.insert(it->GetPad());
		  hOccPad->Fill(row,sec);
		  
		  hsprad->Fill(it->GetR());
		  hspphi->Fill(it->GetPhi()*TMath::RadToDeg());
		  hspzed->Fill(it->GetZ());
		  
		  hspzphi->Fill(it->GetZ(),it->GetPhi()*TMath::RadToDeg());
		  hspxy->Fill(it->GetX(),it->GetY());

		  ++Npoints;
		}
              
              for(auto iaw = trXaw.begin(); iaw != trXaw.end(); ++iaw)
                 {
                    hTrackXaw->Fill(*iaw);
                 }
              for(auto ipd = trkXpad.begin(); ipd != trkXpad.end(); ++ipd)
                 {
                    pads.get(*ipd,sec,row);
                    hTrackXpad->Fill(row,sec);                
                 }

              if( MagneticField > 0. )
                 {
                    double chi2 = ((TFitHelix*)at)->GetRchi2();
                    double ndf = (double) ((TFitHelix*)at)->GetRDoF();
                    hhchi2R->Fill(chi2/ndf);

                    chi2 = ((TFitHelix*)at)->GetZchi2();
                    ndf = (double) ((TFitHelix*)at)->GetZDoF();
                    hhchi2Z->Fill(chi2/ndf);

                    hhD->Fill( ((TFitHelix*)at)->GetD() );
                 }
              else
                 {
                    double ndf= (double) ((TFitLine*)at)->GetDoF();
                    double chi2 = ((TFitLine*)at)->GetChi2();
                    hchi2->Fill(chi2/ndf);
                    if( gVerb > 1 )
                       cout<<"\t"<<t<<" chi^2: "<<chi2<<" ndf: "<<ndf<<endl;
                 }
	    }
          cosfind.Create(tracks_array);
	}

      TFitVertex Vertex(anEvent->GetEventNumber());
      int sv = r.RecVertex(&Vertex);
      if( sv > 0 && gVerb ) Vertex.Print();
      if( sv > 0 )
         {
            hvtxrad->Fill(Vertex.GetRadius());
            double phi = Vertex.GetAzimuth();
            if( phi < 0. ) phi+=TMath::TwoPi();
            phi*=TMath::RadToDeg();
            hvtxphi->Fill(phi);
            hvtxzed->Fill(Vertex.GetElevation());
            hvtxzedphi->Fill(Vertex.GetElevation(),phi);
         }
      
      hNpoints->Fill(r.GetNumberOfPoints());
      if(Npointstracks) hNpointstracks->Fill(Npointstracks);
      if(Npoints) hNgoodpoints->Fill(Npoints);

      hNtracks->Fill(r.GetNumberOfTracks());
      hNgoodtracks->Fill(nlin+nhel);

      if( r.GetNumberOfTracks() > 0 )
         hpattreceff->Fill( double(Npointstracks)/double(r.GetNumberOfTracks()));
      else
         hpattreceff->Fill(0.);
      
      if( nlin > 0 || nhel > 0 )
         hgoodpattreceff->Fill( double(Npoints)/double(nlin+nhel));
      else
         hgoodpattreceff->Fill(0.);

      int cf_status = cosfind.Process();
      if( gVerb > 1 )
         {
            cout<<"CosmicFinder Status: "<<cf_status<<endl;
            cosfind.Status();
         }
      
      cosfind.Reset();

      anEvent->Reset();
      r.Reset();
      if( gVerb ) 
         cout<<" ============================================="<<endl;
    }
  cout<<"End of run"<<endl;

  fout->Write();

  delete anEvent;
  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
