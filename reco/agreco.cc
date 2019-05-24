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
  parser.addArgument("--finder",1,true);
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

  TString foutname("histo");
  foutname+=GetTag(fname);
  foutname+=".root";
  TFile* fout = new TFile(foutname,"RECREATE");

  TH1D* hNpoints = new TH1D("hpoints","Reconstructed Spacepoints",1000,0.,1000.);
  TH1D* hNpointstracks = new TH1D("hpointstracks","Reconstructed Spacepoints in Found Tracks",1000,0.,1000.);
  TH1D* hNgoodpoints = new TH1D("hgoodpoints","Reconstructed Spacepoints Used in Tracking",1000,0.,1000.);

  TH1D* hNtracks = new TH1D("hNtracks","Reconstructed Tracks",10,0.,10.);
  TH1D* hNgoodtracks = new TH1D("hNgoodtracks","Reconstructed Good tracks",10,0.,10.);

  TH1D* hpattreceff = new TH1D("hpattreceff","Reconstructed Spacepoints/Tracks",100,0.,100.);
  TH1D* hgoodpattreceff = new TH1D("hgoodpattreceff","Reconstructed Good Spacepoints/Tracks",100,0.,100.);

  TH2D* hOccPadtracks = new TH2D("hOccPadtracks","Pad Occupancy for Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
  TH1D* hOccAwtracks = new TH1D("hOccAwtracks","Aw Occupancy for Tracks;aw",256,-0.5,255.5);

  TH1D* hspradtracks = new TH1D("hspradtracks","Spacepoint Radius for Tracks;r [mm]",100,109.,174.);
  TH1D* hspphitracks = new TH1D("hspphitracks","Spacepoint Azimuth for Tracks;#phi [deg]",180,0.,360.);
  TH1D* hspzedtracks = new TH1D("hspzedtracks","Spacepoint Axial for Tracks;z [mm]",125,-1152.,1152.);

  TH2D* hspzphitracks = new TH2D("hspzphitracks","Spacepoint Axial-Azimuth for Tracks;z [mm];#phi [deg]",
                           500,-1152.,1152.,100,0.,360.);
  TH2D* hspxytracks = new TH2D("hspxytracks","Spacepoint X-Y for Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

  TH1D* hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,100.); // chi^2 of line fit

  TH2D* hOccPad = new TH2D("hOccPad","Pad Occupancy for Good Tracks;row;sec",576,-0.5,575.5,32,-0.5,31.5);
  TH1D* hOccAw = new TH1D("hOccAw","Aw Occupancy for Good Tracks;aw",256,-0.5,255.5);
  TH1D* hAwOccIsec = new TH1D("hAwOccIsec","Number of AW hits Inside Pad Sector;N",8,0.,8.);

  TH1D* hsprad = new TH1D("hsprad","Spacepoint Radius for Good Tracks;r [mm]",100,109.,174.);
  TH1D* hspphi = new TH1D("hspphi","Spacepoint Azimuth for Good Tracks;#phi [deg]",180,0.,360.);
  TH1D* hspzed = new TH1D("hspzed","Spacepoint Axial for Good Tracks;z [mm]",125,-1152.,1152.);

  TH2D* hspzphi = new TH2D("hspzphi","Spacepoint Axial-Azimuth for Good Tracks;z [mm];#phi [deg]",
                           500,-1152.,1152.,100,0.,360.);
  TH2D* hspxy = new TH2D("hspxy","Spacepoint X-Y for Good Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);

  padmap pads;
  int row,sec;

  // =============================================
  // Reconstruction All-In-One
  Reco r( settings, MagneticField );
  // =============================================

  for( int n=0; n<Nevents; ++n)
    {
      tEvents->GetEntry(n);
      const TObjArray* points = anEvent->GetSpacePoints();
      if( n%1000 == 0 || gVerb > 0 )
	cout<<n<<"\tEvent Number: "<<anEvent->GetEventNumber()
	    <<"\tTime of the Event: "<<anEvent->GetTimeOfEvent()<<"s"<<endl;
      
      r.AddSpacePoint( points );

      int nt = r.FindTracks( choosen_finder );
      if( n%1000 == 0 || gVerb > 0 )
	cout<<"\t# of Points: "<<setw(3)<<points->GetEntriesFast()<<"\t# of Tracks: "<<nt<<endl;

      TClonesArray* found_tracks = r.GetTracks();
      int Npointstracks=0;
      for(int t=0; t<found_tracks->GetEntries(); ++t)
         {
            TTrack* at = (TTrack*) found_tracks->At(t);
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
      if( MagneticField > 0. ) nhel = r.FitHelix();
      else nlin = r.FitLines();

      TClonesArray* tracks_array=0;
      if( nhel > 0 ) tracks_array = r.GetHelices();
      else if( nlin > 0 ) tracks_array = r.GetLines();
      
      int Npoints=0;
      if( tracks_array ) 
	{
	  for(int t=0; t<tracks_array->GetEntries(); ++t)
	    {
	      TTrack* at = (TTrack*) tracks_array->At(t);
	      const std::vector<TSpacePoint*>* spacepoints = at->GetPointsArray();
	      for( auto& it: *spacepoints )
		{
		  hOccAw->Fill(it->GetWire());
                  hAwOccIsec->Fill(it->GetWire()%8);
		  pads.get(it->GetPad(),sec,row);
		  hOccPad->Fill(row,sec);
		  
		  hsprad->Fill(it->GetR());
		  hspphi->Fill(it->GetPhi()*TMath::RadToDeg());
		  hspzed->Fill(it->GetZ());
		  
		  hspzphi->Fill(it->GetZ(),it->GetPhi()*TMath::RadToDeg());
		  hspxy->Fill(it->GetX(),it->GetY());

		  ++Npoints;
		}
              if( MagneticField > 0. )
                 {}
              else
                 {
                    double ndf= (double) ((TFitLine*)at)->GetDoF();
                    double chi2 = ((TFitLine*)at)->GetChi2();
                    hchi2->Fill(chi2/ndf);
                 }
	    }
	}

      TFitVertex Vertex(anEvent->GetEventNumber());
      int sv = r.RecVertex(&Vertex);
      if( sv > 0 && gVerb ) Vertex.Print();
      
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
      
      anEvent->Reset();
      r.Reset();
    }
  cout<<"End of run"<<endl;

  fout->Write();

  return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
