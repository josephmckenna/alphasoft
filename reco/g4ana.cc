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

#include "AGRoot2STL.h"

#include "AnaSettings.hh"
#include "Reco.hh"

#include "Aged.h"

typedef int TAFlags;

#define TAFlag_OK          0
#define TAFlag_SKIP    (1<<0)
#define TAFlag_QUIT    (1<<1)
#define TAFlag_WRITE   (1<<2)
#define TAFlag_DISPLAY (1<<3)
#define TAFlag_SKIP_PROFILE (1<<4)

int main(int argc, char **argv)
{
   // make a new ArgumentParser
   ArgumentParser parser;
   parser.appName(argv[0]);
   // add some arguments
   parser.addArgument("-f", "--rootfile", 1, false);
   parser.addArgument("-a", "--anasettings", 1);
   parser.addArgument("-b", "--Bfield", 1);
   parser.addArgument("-e", "--Nevents", 1);
   parser.addArgument("--finder", 1);
   parser.addArgument("-d", "--draw", 1);
   parser.addArgument("-v", "--verb", 1);
   parser.addArgument("--enableMC", 1);
   parser.addArgument("-2", "--twod", 1);
   parser.addArgument("-l", "--led", 1);
   parser.addArgument("-D", "--aged", 1);

   // parse the command-line arguments - throws if invalid format
   parser.parse(argc, argv);

   std::string fname = parser.retrieve<std::string>("rootfile");
   TFile *     fin   = TFile::Open(fname.c_str(), "READ");
   if (!fin->IsOpen()) {
      std::cerr << "[main]# ROOTfile not open... Exiting!" << std::endl;
      return 1;
   } else
      std::cout << "[main]# filename: " << fin->GetName() << std::endl;

   TTree *       tMC = (TTree *)fin->Get("MCinfo");
   TClonesArray *vtx = new TClonesArray("TVector3");
   tMC->SetBranchAddress("MCvertex", &vtx);

   TTree *       tGarf       = (TTree *)fin->Get("Garfield");
   TClonesArray *garfpp_hits = new TClonesArray("TMChit");
   TClonesArray *aw_hits     = new TClonesArray("TMChit");
   if (tGarf) {
      std::cout << tGarf->GetTitle() << " entries: " << tGarf->GetEntriesFast() << std::endl;
      tGarf->SetBranchAddress("GarfHits", &garfpp_hits);
      tGarf->SetBranchAddress("AnodeHits", &aw_hits);
   } else {
      tGarf = (TTree *)fin->Get("TPCMCdata");
      std::cout << tGarf->GetTitle() << " entries: " << tGarf->GetEntriesFast() << std::endl;
      tGarf->SetBranchAddress("TPCMCHits", &garfpp_hits);
   }

   TTree *tSig = (TTree *)fin->Get("Signals");
   if (!tSig) {
      std::cerr << "[main]# ROOTfile does not contain proper simulation data... Exiting!" << std::endl;
      return 1;
   }
   int Nevents = tSig->GetEntriesFast();
   std::cout << "[main]# Signals Tree: " << tSig->GetTitle() << "\t Entries: " << Nevents << std::endl;
   if (parser.count("Nevents")) {
      std::string nev = parser.retrieve<std::string>("Nevents");
      Nevents         = stoi(nev);
   }
   std::cout << "[main]# Processing " << Nevents << " events" << std::endl;

   TClonesArray *AWsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("AW", &AWsignals);

   TClonesArray *PADsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("PAD", &PADsignals);

   std::string        json_file = "sim.hjson";
   std::ostringstream json_filepath;
   json_filepath << getenv("AGRELEASE") << "/ana/" << json_file;
   std::string settings = json_filepath.str();
   if (parser.count("anasettings")) {
      std::string fname = parser.retrieve<std::string>("anasettings");
      struct stat buffer;
      if (stat(fname.c_str(), &buffer) == 0) {
         settings = fname;
         std::cout << "[main]# Loading Ana Settings from: " << settings << std::endl;
      } else
         std::cerr << "[main]# AnaSettings " << fname << " doesn't exist, using default: " << settings << std::endl;
   }
   AnaSettings *ana_settings = new AnaSettings(settings.c_str());
   std::cout << "--------------------------------------------------" << std::endl;
   cout << "READ settings file" << endl;
   ana_settings->Print();
   std::cout << "--------------------------------------------------" << std::endl;

   finderChoice finder = adaptive;
   if (parser.count("finder")) {
      std::string cf = parser.retrieve<std::string>("finder");
      if (cf == "base") {
         finder = base;
         std::cout << "[main]# Using basic TracksFinder" << std::endl;
      } else if (cf == "neural") {
         finder = neural;
         std::cout << "[main]# Using NeuralFinder" << std::endl;
      } else if (cf == "adaptive") {
         finder = adaptive;
         std::cout << "[main]# Using AdaptiveFinder" << std::endl;
      } else
         std::cerr << "[main]# Unknown track finder mode \"" << cf << "\", using adaptive" << std::endl;
   }
   std::cout << "[main]# Using track finder: " << finder << std::endl;

   double B = 1.0;
   if (parser.count("Bfield")) {
      std::string Bfield = parser.retrieve<std::string>("Bfield");
      B                  = stod(Bfield);
   }
   std::cout << "[main]# Magnetic Field: " << B << " T" << std::endl;

   bool draw = false;
   if (parser.count("draw")) {
      draw = true;
      std::cout << "[main]# Drawing Enabled" << std::endl;
   }
   bool verb = false;
   if (parser.count("verb")) {
      verb = true;
      std::cout << "[main]# Verbosity Enabled" << std::endl;
   }
   bool enableMC = false;
   if (parser.count("enableMC")) {
      enableMC = true;
      std::cout << "[main]# MC reco Enabled" << std::endl;
   }
   bool twod = false;
   if (parser.count("twod")) {
      twod = true;
      std::cout << "[main]# PADS Reco Disenabled - AW ONLY!" << std::endl;
   }
   bool led = false;
   if (parser.count("led")) {
      led = true;
      std::cout << "[main]# Leading edge reconstruction!" << std::endl;
   }

   Aged *aged = nullptr;
   if (parser.count("aged")) {
      aged = new Aged;
      std::cout << "[main]# Aged visualization" << std::endl;
   }

   TApplication *app = 0;
   if (draw) app = new TApplication("g4ana", &argc, argv);

   std::string outname("ana");
   outname += basename(fname); // from fileutility.hh
   std::cout << "[main]# saving output to: " << outname << std::endl;

   ProcessEvents proc(ana_settings, B, outname);
   if (draw) proc.SetDraw();
   proc.SetFinder(finder);
   if (verb) proc.SetVerboseLevel(2);

   for (int i = 0; i < Nevents; ++i) {
      tSig->GetEntry(i);
      proc.SetEventNumber(i);

      if (twod)
         proc.ProcessWaveform_2D(AWsignals);
      else if (led)
         proc.ProcessWaveform_led(AWsignals, PADsignals);
      else
         proc.ProcessWaveform_deconv(AWsignals, PADsignals);

      tMC->GetEntry(i);
      TVector3 *mcvtx = (TVector3 *)vtx->ConstructedAt(i);
      std::cout << "[main]# " << i << "\tMCvertex: ";
      mcvtx->Print();

      proc.ProcessVertex(mcvtx);

      bool theEnd = false;
      if (aged) {
         TStoreEvent                 ev = proc.GetStoreEvent();
         TAFlags flags;
         if(ev.GetNumberOfPoints()){
         std::vector<TBarHit *>       bars;
         std::vector<ALPHAg::wf_ref> AWwf = ConvertWaveformArray(AWsignals);
         std::vector<ALPHAg::wf_ref> PADwf = ConvertWaveformArray(PADsignals);
         aged->ShowEvent(ev, bars, AWwf, PADwf, 0, &flags);
         theEnd = (flags == TAFlag_QUIT);
         }
      }

      if (tGarf) {
         tGarf->GetEntry(i);
         proc.Finish(garfpp_hits, aw_hits);
         if (enableMC) proc.ProcessMonteCarlo(aw_hits, mcvtx);
      } else
         proc.Finish();
      if(theEnd) break;
   } // events loop

   proc.End();
   std::cout << "[main]# Finished" << std::endl;

   if (draw) {
      // new TBrowser;
      app->Run();
   }
   std::cout << "[main]# End Run" << std::endl;
   if (aged) delete aged;
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
