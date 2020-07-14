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
#include "AnaSettings.h"

#include "TSpacePoint.hh"
#include "TFitVertex.hh"

#include "TFitLine.hh"

#include "CosmicFinder.hh"

#include "argparse.hh"
#include "Reco.hh"
#include "Utils.hh"

TString GetTag( string filename )
{
  TString fname = filename;
  TRegexp re("[0-9][0-9][0-9][0-9]");
  int pos = fname.Index(re);
  int run = TString(fname(pos,7)).Atoi();
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
  AnaSettings* ana_settings = new AnaSettings(settings.c_str());
  ana_settings->Print();

  double MagneticField=1.0;
  if( parser.count("Bfield") )
    {
      string Bfield = parser.retrieve<string>("Bfield");
      MagneticField = stod(Bfield);
    }
  cout<<"Magnetic Field: "<<MagneticField<<" T"<<endl;

  int Verb = 0;
  if( parser.count("verbose"))
     {
        string verbosity = parser.retrieve<string>("verbose");
        Verb = stoi(verbosity);
     }
  cout<<"Verbose Level set to: "<<Verb<<endl;

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

  std::string foutname("histo");
  foutname+=GetTag(fname);
  foutname+=tag;
  foutname+=".root";
  //  TFile* fout = new TFile(foutname,"RECREATE");
  cout<<"Output filename: "<<foutname<<endl;
  Utils u(foutname,MagneticField);
  TObjString sett = ana_settings->GetSettingsString();
  u.WriteSettings(&sett);

  // =============================================
  // Reconstruction All-In-One
  //Reco r( settings, MagneticField );
  Reco r( ana_settings, MagneticField );
  // =============================================
  if( Verb > 0 ) r.SetTrace(true);

  // =============================================
  // Cosmic Analysis
  CosmicFinder cosfind( MagneticField );
  // =============================================

  for( int n=0; n<Nevents; ++n)
    {
      tEvents->GetEntry(n);
      const TObjArray* points = anEvent->GetSpacePoints();
      if( n%1000 == 0 || Verb > 0 )
	cout<<n<<"\tEvent Number: "<<anEvent->GetEventNumber()
	    <<"\tTime of the Event: "<<anEvent->GetTimeOfEvent()<<"s"<<endl;
      
      u.FillRecoPointsHistos(points);
      // Add Spacepoints
      r.AddSpacePoint( points );

      // Tracks Finder
      int nt = r.FindTracks( choosen_finder );
      if( n%1000 == 0 || Verb > 0 )
	cout<<"\t# of Points: "<<setw(3)<<points->GetEntriesFast()
            <<"\t# of Tracks: "<<nt<<endl;

      u.FillRecoTracksHisto(r.GetTracks());
      
      // Tracks Fitter
      int nlin=0, nhel=0;
      if( MagneticField > 0. )
         {
            nhel = r.FitHelix();
            if( Verb > 1 ) cout<<"\tN hel: "<<nhel<<endl;
         }
      else 
         {
            nlin = r.FitLines();
            if( Verb > 1 ) cout<<"\tN Lin: "<<nlin<<endl;
         }

      std::vector<TTrack*>* tracks_array=0;
      if( nhel > 0 ) 
         tracks_array = reinterpret_cast<std::vector<TTrack*>*>(r.GetHelices());
      else if( nlin > 0 ) 
         tracks_array = reinterpret_cast<std::vector<TTrack*>*>(r.GetLines());
      
      if( tracks_array ) 
	{
	  u.FillFitTracksHisto(tracks_array);
          // Setup Cosmic Analysis          
          cosfind.Create(tracks_array);
	}

      // Vertexing
      TFitVertex Vertex(anEvent->GetEventNumber());
      int sv = r.RecVertex(&Vertex);
      if( sv > 0 && Verb ) Vertex.Print();
      if( sv > 0 ) u.FillRecoVertex(&Vertex);

      u.FillFinalHistos(&r,nhel+nlin);

      // Perform Cosmic Analysis
      int cf_status = cosfind.Process();
      if( Verb > 1 )
         {
            cout<<"CosmicFinder Status: "<<cf_status<<endl;
            cosfind.Status();
         }
      
      cosfind.Reset();

      anEvent->Reset();
      r.Reset();
      if( Verb ) 
         cout<<" ============================================="<<endl;
    }
  cout<<"End of run"<<endl;

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
