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

#include "AnaSettings.hh"
#include "Reco.hh"


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

   std::string fname = parser.retrieve<std::string>("rootfile");
   TFile* fin = TFile::Open(fname.c_str(),"READ");
   if( !fin->IsOpen() )
      {
         std::cerr<<"[main]# ROOTfile not open... Exiting!"<<std::endl;
         return 1;
      }
   else
      std::cout<<"[main]# filename: "<<fin->GetName()<<std::endl;

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
         std::cerr<<"[main]# ROOTfile does not contain proper simulation data... Exiting!"<<std::endl;
         return 1;
      }
   int Nevents = tSig->GetEntriesFast();
   std::cout<<"[main]# Signals Tree: "<<tSig->GetTitle()<<"\t Entries: "<<Nevents<<std::endl;
   if( parser.count("Nevents") )
    {
       std::string nev = parser.retrieve<std::string>("Nevents");
       Nevents = stoi(nev);
    }
   std::cout<<"[main]# Processing "<<Nevents<<" events"<<std::endl;

   TClonesArray* AWsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("AW",&AWsignals);

   TClonesArray* PADsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("PAD",&PADsignals);

   std::string json_file = "sim.hjson";
   std::ostringstream json_filepath;
   json_filepath<<getenv("AGRELEASE")<<"/ana/"<<json_file;
   std::string settings=json_filepath.str();
   if( parser.count("anasettings") )
    {
      std::string fname = parser.retrieve<std::string>("anasettings");
      struct stat buffer;   
      if( stat(fname.c_str(), &buffer) == 0 )
         {
            settings = fname;
            std::cout<<"[main]# Loading Ana Settings from: "<<settings<<std::endl;
         }
      else
         std::cerr<<"[main]# AnaSettings "<<fname<<" doesn't exist, using default: "<<settings<<std::endl;
    }
   AnaSettings* ana_settings = new AnaSettings(settings.c_str());
<<<<<<< Updated upstream
   std::cout<<"--------------------------------------------------"<<std::endl;
   cout<<"READ settings file"<<endl;
   ana_settings->Print();
   std::cout<<"--------------------------------------------------"<<std::endl;
=======
   ana_settings->Print(); //Original line PW
   //cout<<"ana_settings are: "<<ana_settings<<endl; //PW
   cout<<"settings are located here: "<<settings<<endl; //PW
   cout<<"HERE"<<endl; //PW

   Deconv d(settings);
   d.SetPWBdelay(50.);
   d.SetPedestalLength(0);
   //d.SetTrace(true);
   cout<<"--------------------------------------------------"<<endl;
   cout<<"[main]# Deconv Settings"<<endl;
   d.PrintADCsettings();
   d.PrintPWBsettings();
   cout<<"--------------------------------------------------"<<endl;
>>>>>>> Stashed changes

   finderChoice finder = adaptive;
   if( parser.count("finder") )
      {
         std::string cf = parser.retrieve<std::string>("finder");
         if( cf == "base") 
            {
               finder = base;
               std::cout << "[main]# Using basic TracksFinder" << std::endl;
            }
         else if( cf == "neural") 
            {
               finder = neural;
               std::cout << "[main]# Using NeuralFinder" << std::endl;
            }
         else if( cf == "adaptive") 
            {
               finder = adaptive;
               std::cout << "[main]# Using AdaptiveFinder" << std::endl;
            }
         else std::cerr<<"[main]# Unknown track finder mode \""<<cf<<"\", using adaptive"<<std::endl;
      }
<<<<<<< Updated upstream
   std::cout<<"[main]# Using track finder: "<<finder<<std::endl;
=======
   cout<<"[main]# Using track finder: "<<finder<<endl;
   
   //Match m(settings);
   Match m(ana_settings); //uncomment this PW
   //   m.SetDiagnostic(false);
   m.SetDiagnostic(true); //uncomment this PW
>>>>>>> Stashed changes
   
   double B=1.0;
   if( parser.count("Bfield") )
      {
         std::string Bfield = parser.retrieve<std::string>("Bfield");
         B = stod(Bfield);
      }
<<<<<<< Updated upstream
   std::cout<<"[main]# Magnetic Field: "<<B<<" T"<<std::endl;
=======
   cout<<"[main]# Magnetic Field: "<<B<<" T"<<endl;

   //Reco r(settings,B);
   Reco r(ana_settings,B);

   Reco rMC(ana_settings,B);
   //Reco rMC(settings,B);
>>>>>>> Stashed changes

   bool draw = false;
   if( parser.count("draw") )
      {
         draw = true;
         std::cout<<"[main]# Drawing Enabled"<<std::endl;
      }
   bool verb = false;
   if( parser.count("verb") )
      {
         verb = true;
         std::cout<<"[main]# Verbosity Enabled"<<std::endl;
      }
   bool enableMC=false;
   if( parser.count("enableMC") )
      {
         enableMC=true;
         std::cout<<"[main]# MC reco Enabled"<<std::endl;
      }
   bool twod=false;
   if( parser.count("twod") )
      {
         twod=true;
         std::cout<<"[main]# PADS Reco Disenabled - AW ONLY!"<<std::endl;
      }
   bool led=false;
   if( parser.count("led") )
      {
         led=true;
         std::cout<<"[main]# Leading edge reconstruction!"<<std::endl;
      }
 
   TApplication* app=0;
   if( draw )
      app = new TApplication("g4ana",&argc,argv);

   std::string outname("ana");
   outname += basename(fname); // from fileutility.hh
   std::cout<<"[main]# saving output to: "<<outname<<std::endl;

<<<<<<< Updated upstream
   ProcessEvents proc(ana_settings,B,outname);
   if( draw ) proc.SetDraw();
   proc.SetFinder(finder);
   if( verb )
      proc.SetVerboseLevel(2);
=======
   Utils u(outname,B,draw);
   TObjString sett = ana_settings->GetSettingsString();
   u.WriteSettings(&sett);
   m.Setup(0); //Uncomment this PW
>>>>>>> Stashed changes

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

         tMC->GetEntry(i);
         TVector3* mcvtx = (TVector3*) vtx->ConstructedAt(i);
<<<<<<< Updated upstream
=======
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
>>>>>>> Stashed changes

         proc.ProcessVertex(mcvtx);
         
         if( tGarf )
            { 
               tGarf->GetEntry(i);
<<<<<<< Updated upstream
               proc.Finish(garfpp_hits, aw_hits);
               if( enableMC ) proc.ProcessMonteCarlo(aw_hits,mcvtx);
=======
               
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
>>>>>>> Stashed changes
            }
         else
            proc.Finish();  

      }// events loop
   
   std::cout<<"[main]# Finished"<<std::endl;
   if( draw ){
      // new TBrowser;
      app->Run();
   }
   std::cout<<"[main]# End Run"<<std::endl;
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
