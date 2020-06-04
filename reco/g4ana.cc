#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TApplication.h>
#include <TBrowser.h>

#include "argparse.hh"
#include "fileutility.hh"
#include "ProcessEvents.hh"

#include "AnaSettings.h"
#include "Reco.hh"

using namespace std;

int main(int argc, char** argv)
{
   // make a new ArgumentParser
   ArgumentParser parser;
   parser.appName(argv[0]);
   // add some arguments
   parser.addArgument("-f","--rootfile",1,false);
   parser.addArgument("-a","--anasettings",1);
   parser.addArgument("-b","--Bfield",1);
   parser.addArgument("-e","--Nevents",1);
   parser.addArgument("--finder",1);
   parser.addArgument("-d","--draw",1);
   parser.addArgument("-v","--verb",1);
   parser.addArgument("--enableMC",1);
   parser.addArgument("-2","--twod",1);
   parser.addArgument("-l","--led",1);

    
   // parse the command-line arguments - throws if invalid format
   parser.parse(argc, argv);

   string fname = parser.retrieve<string>("rootfile");
   TFile* fin = TFile::Open(fname.c_str(),"READ");
   if( !fin->IsOpen() )
      {
         cerr<<"[main]# ROOTfile not open... Exiting!"<<endl;
         return 1;
      }
   else
      cout<<"[main]# filename: "<<fin->GetName()<<endl;

   TTree* tMC = (TTree*) fin->Get("MCinfo");
   TClonesArray* vtx = new TClonesArray("TVector3");
   tMC->SetBranchAddress("MCvertex",&vtx);

   TTree* tGarf = (TTree*) fin->Get("Garfield");
   TClonesArray* garfpp_hits = new TClonesArray("TMChit");
   TClonesArray* aw_hits = new TClonesArray("TMChit");
   if( tGarf ) 
      {
         tGarf->SetBranchAddress("GarfHits",&garfpp_hits);   
         tGarf->SetBranchAddress("AnodeHits",&aw_hits);
      }

   TTree* tSig =  (TTree*) fin->Get("Signals");
   if( !tSig )
      {
         cerr<<"[main]# ROOTfile does not contain proper simulation data... Exiting!"<<endl;
         return 1;
      }
   int Nevents = tSig->GetEntriesFast();
   cout<<"[main]# Signals Tree: "<<tSig->GetTitle()<<"\t Entries: "<<Nevents<<endl;
   if( parser.count("Nevents") )
    {
       string nev = parser.retrieve<string>("Nevents");
       Nevents = stoi(nev);
    }
   cout<<"[main]# Processing "<<Nevents<<" events"<<endl;

   TClonesArray* AWsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("AW",&AWsignals);

   TClonesArray* PADsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("PAD",&PADsignals);

   string json_file = "sim.hjson";
   ostringstream json_filepath;
   json_filepath<<getenv("AGRELEASE")<<"/ana/"<<json_file;
   string settings=json_filepath.str();
   if( parser.count("anasettings") )
    {
      string fname = parser.retrieve<string>("anasettings");
      struct stat buffer;   
      if( stat(fname.c_str(), &buffer) == 0 )
         {
            settings = fname;
            cout<<"[main]# Loading Ana Settings from: "<<settings<<endl;
         }
      else
         cerr<<"[main]# AnaSettings "<<fname<<" doesn't exist, using default: "<<settings<<endl;
    }
   AnaSettings* ana_settings = new AnaSettings(settings.c_str());
   cout<<"--------------------------------------------------"<<endl;
   cout<<"READ settings file"<<endl;
   ana_settings->Print();
   cout<<"--------------------------------------------------"<<endl;

   finderChoice finder = adaptive;
   if( parser.count("finder") )
      {
         string cf = parser.retrieve<string>("finder");
         if( cf == "base") 
            {
               finder = base;
               cout << "[main]# Using basic TracksFinder" << endl;
            }
         else if( cf == "neural") 
            {
               finder = neural;
               cout << "[main]# Using NeuralFinder" << endl;
            }
         else if( cf == "adaptive") 
            {
               finder = adaptive;
               cout << "[main]# Using AdaptiveFinder" << endl;
            }
         else cerr<<"[main]# Unknown track finder mode \""<<cf<<"\", using adaptive"<<endl;
      }
   cout<<"[main]# Using track finder: "<<finder<<endl;
   
   double B=1.0;
   if( parser.count("Bfield") )
      {
         string Bfield = parser.retrieve<string>("Bfield");
         B = stod(Bfield);
      }
   cout<<"[main]# Magnetic Field: "<<B<<" T"<<endl;

   bool draw = false;
   if( parser.count("draw") )
      {
         draw = true;
         cout<<"[main]# Drawing Enabled"<<endl;
      }
   bool verb = false;
   if( parser.count("verb") )
      {
         verb = true;
         cout<<"[main]# Verbosity Enabled"<<endl;
      }
   bool enableMC=false;
   if( parser.count("enableMC") )
      {
         enableMC=true;
         cout<<"[main]# MC reco Enabled"<<endl;
      }
   bool twod=false;
   if( parser.count("twod") )
      {
         twod=true;
         cout<<"[main]# PADS Reco Disenabled - AW ONLY!"<<endl;
      }
   bool led=false;
   if( parser.count("led") )
      {
         led=true;
         cout<<"[main]# Leading edge reconstruction!"<<endl;
      }
 
   TApplication* app=0;
   if( draw )
      app = new TApplication("g4ana",&argc,argv);

   string outname("ana");
   outname += basename(fname); // from fileutility.hh
   cout<<"[main]# saving output to: "<<outname<<endl;

   ProcessEvents proc(ana_settings,B,outname);
   if( draw ) proc.SetDraw();
   proc.SetFinder(finder);
   if( verb )
      proc.SetVerboseLevel(2);

   for( int i=0; i<Nevents; ++i )
      {
         tSig->GetEntry(i);
         proc.SetEventNumber(i);

         if(twod)
            proc.ProcessWaveform_2D(AWsignals);
         else if(led)
            proc.ProcessWaveform_led(AWsignals,PADsignals);
         else
            proc.ProcessWaveform_deconv(AWsignals,PADsignals);

         proc.ProcessTracks();

         tMC->GetEntry(i);
         TVector3* mcvtx = (TVector3*) vtx->ConstructedAt(i);

         proc.ProcessVertex(mcvtx);
         
         if( tGarf )
            { 
               tGarf->GetEntry(i);
               proc.Finish(garfpp_hits, aw_hits);
               if( enableMC ) proc.ProcessMonteCarlo(aw_hits,mcvtx);
            }
         else
            proc.Finish();  

      }// events loop
   
   cout<<"[main]# Finished"<<endl;
   if( draw ){
      // new TBrowser;
      app->Run();
   }
   cout<<"[main]# End Run"<<endl;
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
