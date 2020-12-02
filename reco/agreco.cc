#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TClonesArray.h>

#include <TH1D.h>
#include <TH2D.h>
#include <TRegexp.h>

#include "TStoreEvent.hh"
#include "SignalsType.hh"
#include "AnaSettings.hh"

#include "TSpacePoint.hh"
#include "TFitVertex.hh"

#include "TFitLine.hh"

#include "CosmicFinder.hh"

#include "argparse.hh"
#include "Reco.hh"
#include "Utils.hh"

TString GetTag( std::string filename )
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
  parser.addArgument("-l","--location",1,true);
  parser.addArgument("-e","--Nevents",1,true);
  parser.addArgument("-v","--verbose",1,true);
  parser.addArgument("--finder",1,true);
  parser.addArgument("-t","--text",1,true);
  // parse the command-line arguments - throws if invalid format
  parser.parse(argc, argv);

  std::string fname = parser.retrieve<std::string>("rootfile");
  TFile* fin = TFile::Open(fname.c_str(),"READ");
  if( !fin->IsOpen() ) return 2;

  TTree* tEvents = (TTree*) fin->Get("StoreEventTree");
  if( !tEvents ) return 3;
  int Nevents = tEvents->GetEntriesFast();
  std::cout<<tEvents->GetTitle()<<"\t"<<Nevents<<std::endl;
  
  TStoreEvent* anEvent = new TStoreEvent();
  tEvents->SetBranchAddress("StoredEvent", &anEvent);

  std::string settings="default";
  if( parser.count("anasettings") )
    {
      std::string fname = parser.retrieve<std::string>("anasettings");
      struct stat buffer;   
      if( stat(fname.c_str(), &buffer) == 0 )
	settings = fname;
      else
	std::cerr<<"AnaSettings "<<fname<<" doesn't exist"<<std::endl;
    }
  std::cout<<"AnaSettings: "<<settings<<endl;
  AnaSettings* ana_settings = new AnaSettings(settings.c_str());
  ana_settings->Print();

  double MagneticField=1.0;
  if( parser.count("Bfield") )
    {
      std::string Bfield = parser.retrieve<std::string>("Bfield");
      MagneticField = stod(Bfield);
    }
  std::cout<<"Magnetic Field: "<<MagneticField<<" T"<<std::endl;

  std::string location="CERN";
  if( parser.count("location") )
     {
        std::string loc = parser.retrieve<std::string>("location");
        struct stat buffer;   
        if( stat(loc.c_str(), &buffer) == 0 )
           location = loc;
      }
  std::cout<<"Data taken at "<<location<<std::endl;

  int Verb = 0;
  if( parser.count("verbose"))
     {
        std::string verbosity = parser.retrieve<std::string>("verbose");
        Verb = stoi(verbosity);
     }
  std::cout<<"Verbose Level set to: "<<Verb<<std::endl;

  finderChoice choosen_finder = adaptive;
  if( parser.count("finder") )
    {
      std::string cf = parser.retrieve<std::string>("finder");
      if( cf == "base") choosen_finder = base;
      else if( cf == "neural") choosen_finder = neural;
      else if( cf == "adaptive") choosen_finder = adaptive;
      else std::cerr<<"Unknown track finder mode \""<<cf<<"\", using adaptive"<<std::endl;
    }
  std::cout<<"Using track finder: "<<choosen_finder<<std::endl;


  if( parser.count("Nevents") )
    {
      std::string nev = parser.retrieve<std::string>("Nevents");
      Nevents = stoi(nev);
    }
  std::cout<<"Processing "<<Nevents<<" events"<<std::endl;

  std::string tag="";
  if( parser.count("text") )
     {
        tag = "_";
        tag += parser.retrieve<std::string>("text");
     }
  std::cout<<"Additional Text: "<<tag<<std::endl;

  std::string foutname("histo");
  foutname+=GetTag(fname);
  foutname+=tag;
  foutname+=".root";
  std::cout<<"Output filename: "<<foutname<<std::endl;

  Utils u(foutname,MagneticField);
  u.BookRecoHistos();
  TObjString sett = ana_settings->GetSettingsString();
  u.WriteSettings(&sett);

  // =============================================
  // Reconstruction All-In-One
  //Reco r( settings, MagneticField );
  Reco r( ana_settings, MagneticField, location );
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
	std::cout<<n<<"\tEvent Number: "<<anEvent->GetEventNumber()
	    <<"\tTime of the Event: "<<anEvent->GetTimeOfEvent()<<"s"<<std::endl;
      
      u.FillRecoPointsHistos(points);
      // Add Spacepoints
      r.AddSpacePoint( points );

      // Tracks Finder
      int nt = r.FindTracks( choosen_finder );
      if( n%1000 == 0 || Verb > 0 )
	std::cout<<"\t# of Points: "<<std::setw(3)<<points->GetEntriesFast()
            <<"\t# of Tracks: "<<nt<<std::endl;

      u.FillRecoTracksHisto(r.GetTracks());
      
      // Tracks Fitter
      int nlin=0, nhel=0;
      if( MagneticField > 0. )
         {
            nhel = r.FitHelix();
            if( Verb > 1 ) std::cout<<"\tN hel: "<<nhel<<std::endl;
         }
      else 
         {
            nlin = r.FitLines();
            if( Verb > 1 ) std::cout<<"\tN Lin: "<<nlin<<std::endl;
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
            std::cout<<"CosmicFinder Status: "<<cf_status<<std::endl;
            cosfind.Status();
         }
      
      cosfind.Reset();

      anEvent->Reset();
      r.Reset();
      if( Verb ) 
         std::cout<<" ============================================="<<std::endl;
    }
  std::cout<<"End of run"<<std::endl;

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
