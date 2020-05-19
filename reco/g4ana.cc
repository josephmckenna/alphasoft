#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TBrowser.h>

#include "argparse.hh"

#include "AnaSettings.h"

#include "Deconv.hh"
#include "Match.hh"
#include "Reco.hh"

#include "Utils.hh"
#include "Histo.hh"

#include "TFitVertex.hh"

#include "fileutility.hh"

using namespace std;

int main(int argc, char** argv)
{
   // make a new ArgumentParser
   ArgumentParser parser;
   //parser.appName("ALPHA-g Geant4 Analyzer");
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
   ana_settings->Print();

   Deconv d(settings);
   d.SetPWBdelay(50.);
   d.SetPedestalLength(0);
   //d.SetTrace(true);
   cout<<"--------------------------------------------------"<<endl;
   cout<<"[main]# Deconv Settings"<<endl;
   d.PrintADCsettings();
   d.PrintPWBsettings();
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
   
   //Match m(settings);
   Match m(ana_settings);
   //   m.SetDiagnostic(false);
   m.SetDiagnostic(true);
   
   //ofstream fout("match_goodness.dat", ios::out | ios::app);
   //ofstream fout("pattrec_goodness.dat", ios::out | ios::app);

   double B=1.0;
   if( parser.count("Bfield") )
      {
         string Bfield = parser.retrieve<string>("Bfield");
         B = stod(Bfield);
      }
   cout<<"[main]# Magnetic Field: "<<B<<" T"<<endl;

   //Reco r(settings,B);
   Reco r(ana_settings,B);

   Reco rMC(settings,B);

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

   TApplication* app=0;
   if( draw )
      app = new TApplication("g4ana",&argc,argv);

   
   string outname("ana");
   outname += basename(fname);
   cout<<"[main]# saving output to: "<<outname<<endl;

   Utils u(outname,B,draw);
   TObjString sett = ana_settings->GetSettingsString();
   u.WriteSettings(&sett);
   m.Setup(0);

   for( int i=0; i<Nevents; ++i )
      {
         tSig->GetEntry(i);

         // anode deconv
         int nsig = d.FindAnodeTimes( AWsignals );
         cout<<"[main]# "<<i<<"\tFindAnodeTimes: "<<nsig<<endl;
         if( nsig == 0 ) continue;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //      fout<<std::setprecision(15)<<Average( d.GetAnodeDeconvRemainder() )<<"\t";

         if( verb ) u.PrintSignals( d.GetAnodeSignal() );
         
         if( !twod )
            {
               // pad deconv
               nsig = d.FindPadTimes( PADsignals );
               cout<<"[main]# "<<i<<"\tFindPadTimes: "<<nsig<<endl;
               if( nsig == 0 ) continue;
               if( nsig > 70000 ) continue;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               //      fout<<std::setprecision(15)<<Average( d.GetPadDeconvRemainder() )<<endl;

               if( verb ) u.PrintSignals( d.GetPadSignal() );
         
               // combine pads
               m.Init();
               if(verb) m.SetTrace(true);
               m.CombinePads( d.GetPadSignal() );
               m.SetTrace(false);
               uint npads = m.GetCombinedPads()->size();
               cout<<"[main]# "<<i<<"\tCombinePads: "<<npads<<endl;
               //if( npads == 0 ) continue;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               
               if( verb ) u.PrintSignals( m.GetCombinedPads() );
               
               if( draw ) u.Draw(d.GetAnodeSignal(),d.GetPadSignal(),m.GetCombinedPads(),false);
               
               // match electrodes
               m.MatchElectrodes( d.GetAnodeSignal() );
            }
         else
            {
               m.Init();
               m.FakePads( d.GetAnodeSignal() );
            }
         uint nmatch = m.GetSpacePoints()->size();
         cout<<"[main]# "<<i<<"\tMatchElectrodes: "<<nmatch<<endl;
         if( nmatch == 0 ) continue;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         // combine points
         m.CombPoints();
         uint nsp = m.GetSpacePoints()->size();
         cout<<"[main]# "<<i<<"\tCombinePoints: "<<nsp<<endl;
         if( nsp == 0 ) continue;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         // reco points
         if( verb ) r.SetTrace(true);
         r.AddSpacePoint( m.GetSpacePoints() );
         cout<<"[main]# "<<i<<"\tspacepoints: "<<r.GetNumberOfPoints()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //    }
         // else
         //    {
         //       if( verb ) r.SetTrace(true);
         //       r.AddSpacePoint( d.GetAnodeSignal() );
         //       cout<<"[main]# "<<i<<"\tspacepoints 2D: "<<r.GetNumberOfPoints()<<endl;
         //    }
         //fout<<r.GetNumberOfPoints()<<"\t";

         // find tracks
         r.SetTrace(true);
         int ntracks = r.FindTracks(finder);
         cout<<"[main]# "<<i<<"\tpattrec: "<<ntracks<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         if(finder == neural) 
            u.DebugNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
         
         r.PrintPattRec();
         cout<<"[main]# "<<i<<"\ttracks: "<<r.GetNumberOfTracks()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<r.GetNumberOfTracks()<<"\t";

         //r.SetTrace( true );
         int nlin = r.FitLines();
         cout<<"[main]# "<<i<<"\tline: "<<nlin<<endl;
         //r.SetTrace(true);
         int nhel = r.FitHelix();
         r.SetTrace(false);
         cout<<"[main]# "<<i<<"\thelix: "<<nhel<<endl;
         u.HelixPlots( r.GetHelices() );
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //r.SetTrace( false );

         //fout<<fabs(EvaluateMatch_byResZ(r.GetLines()))<<"\t";//<<endl;
         //fout<<EvaluatePattRec(r.GetLines())<<"\t";

         TFitVertex Vertex(i);
         int sv = r.RecVertex(&Vertex);
         cout<<"[main]# "<<i<<"\t";
         if( sv > 0 ) Vertex.Print();
         else cout<<"No Vertex\n";

         tMC->GetEntry(i);
         TVector3* mcvtx = (TVector3*) vtx->ConstructedAt(i);
         cout<<"[main]# "<<i<<"\tMCvertex: "; 
         mcvtx->Print();

         double res = kUnknown;
         if( sv > 0 ) 
            { 
               res = u.VertexResolution(Vertex.GetVertex(),mcvtx);
               u.VertexPlots(&Vertex);
            }
         else res = u.PointResolution(r.GetHelices(),mcvtx);

         cout<<"[main]# "<<i<<"\tResolution: ";        
         auto prec = cout.precision();
         cout.precision(2);
         cout<<res<<" mm"<<endl;
         cout.precision(prec);
         // cout<<"[main]# "<<i<<"\tUsedHelixPlots: "
         //     <<Vertex.GetHelixStack()->GetEntriesFast()<<endl;
         u.UsedHelixPlots( Vertex.GetHelixStack() );
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<res<<endl;

         if( tGarf )
            {
               
               tGarf->GetEntry(i);
               
               if( draw )
                  {
                     u.Display(garfpp_hits, aw_hits, r.GetPoints(), r.GetTracks(), r.GetHelices());
                     if(finder == neural) 
                        u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
                  }
               
               if( enableMC )
                  {
                     //================================================================
                     // MC hits reco
                     cout<<"[main]# "<<i<<"\tMC reco"<<endl;
                     
                     rMC.AddMChits( aw_hits );
                     cout<<"[main]# "<<i<<"\tMC spacepoints: "<<rMC.GetNumberOfPoints()<<endl;
                     // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
                     // find tracks
                     int ntracksMC = rMC.FindTracks(finder);
                     cout<<"[main]# "<<i<<"\tMCpattrec: "<<ntracksMC<<endl;
                     // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
                     if(finder == neural) 
                        u.DebugNeuralNet( (NeuralFinder*) rMC.GetTracksFinder() );
                     
                     cout<<"[main]# "<<i<<"\tMC tracks: "<<rMC.GetNumberOfTracks()<<endl;
                     // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
                     rMC.SetTrace( true );
                     nlin = rMC.FitLines();
                     cout<<"[main]# "<<i<<"\tline: "<<nlin<<endl;
                     nhel = rMC.FitHelix();
                     cout<<"[main]# "<<i<<"\tMC helix: "<<nhel<<endl;
                     // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     rMC.SetTrace( false );
                     
                     TFitVertex MCVertex(i);
                     int svMC = r.RecVertex(&MCVertex);
                     if( svMC > 0 ) MCVertex.Print();
                     
                     res = u.PointResolution(rMC.GetHelices(),mcvtx);
                     cout<<"[main]# "<<i<<"\tMC Resolution: ";
                     prec = cout.precision();
                     cout.precision(2);
                     cout<<res<<" mm"<<endl;
                     cout.precision(prec);
                     // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
                     rMC.Reset();
                  }
            }
         else if( draw )
            {
               u.Display(r.GetPoints(), r.GetTracks(), r.GetHelices());
               if(finder == neural) 
                  u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
            }
         
         r.Reset();

      }// events loop
   //fout.close();
   
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
