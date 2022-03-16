#include "AgFlow.h"
#include "RecoFlow.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.hh"
#include <set>
#include <iostream>
#include <numeric>

#include "AnaSettings.hh"
#include "Match.hh"

class KDTreeMatchFlags {
public:
   bool         fRecOff      = false; // Turn reconstruction off
   AnaSettings *ana_settings = NULL;
   bool         fDiag        = false;
   bool         fTrace       = false;
   bool         fForceReco   = false;

   int TotalThreads = 0;
   KDTreeMatchFlags() // ctor
   {
   }

   ~KDTreeMatchFlags() // dtor
   {
   }
};

// For now use root's KDTree implementation
#include "TKDTreeMatch.hh"


class KDTreeMatchModule : public TARunObject {
public:
   KDTreeMatchFlags *fFlags     = NULL;
   bool              fTrace     = false;
   int               fCounter   = 0;
   bool              diagnostic = false;

   int fWireSignalsCut = 2000;
   int fPadSignalsCut = 100000;
   // Rescale the phi dimension to weigh it against 't'
   const double fPhiFactor = 3.;

   TKDTreeMatch fKDTreeMatch;

public:
   KDTreeMatchModule(TARunInfo *runinfo, KDTreeMatchFlags *f) : TARunObject(runinfo)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName = "KDTreeMatch";
#endif
      if (fTrace) printf("KDTreeMatchModule::ctor!\n");

      fFlags = f;

      diagnostic = fFlags->fDiag;  // dis/en-able histogramming
      fTrace     = fFlags->fTrace; // enable verbosity
   }

   ~KDTreeMatchModule()
   {
      if (fTrace) printf("KDTreeMatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("KDTreeMatchModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
   }
   void EndRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("KDTreeMatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
   }

   void PauseRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent *AnalyzeFlowEvent(TARunInfo *runinfo, TAFlags *flags, TAFlowEvent *flow)
   {
      if (fTrace) printf("KDTreeMatchModule::Analyze, run %d, counter %d\n", runinfo->fRunNo, fCounter++);

      // turn off recostruction
      if (fFlags->fRecOff) {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }

      AgSignalsFlow *SigFlow = flow->Find<AgSignalsFlow>();
      if (!SigFlow) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }

      if (SigFlow->awSig.empty()) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }
      if (fTrace) {
         printf("KDTreeMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
         printf("KDTreeMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));
      }

//Only wiresFindPads works for now
#define wiresFindPads 1

      std::vector<std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal>> spacepoints;
      AgKDTreeMatchFlow* kdtree_flow = new AgKDTreeMatchFlow(flow);

#if wiresFindPads
      KDTreeIDContainer2D* pad_tree = NULL;
      if (SigFlow->pdSig.size()) {
         //printf("KDTreeMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
         if (int(SigFlow->pdSig.size()) > fPadSignalsCut)
            return flow;

         // Roots generic kdtree wont understand cylindrical coordinates..
         const int nPads = SigFlow->pdSig.size();
         pad_tree = kdtree_flow->AddKDTree(nPads,"pad_tree");
         fKDTreeMatch.BuildTree( SigFlow->pdSig,pad_tree, fPhiFactor);
      }
#else
      KDTreeIDContainer2D* wire_tree = NULL;
      if (SigFlow->awSig.size())
      {
         if (int(SigFlow->awSig.size()) > fWireSignalsCut)
            return flow;
         const int nWires = SigFlow->awSig.size();
         wire_tree = kdtree_flow->AddKDTree(nWires,"wire_tree");
         fKDTreeMatch.BuildTree( SigFlow->awSig,wire_tree, fPhiFactor);
      }
#endif
#if wiresFindPads
      if (SigFlow->awSig.size() && pad_tree) 
      {
         fKDTreeMatch.WiresFindPads(SigFlow->awSig, SigFlow->pdSig,pad_tree, fPhiFactor,spacepoints);
      }
#else
      if (SigFlow->pdSig.size())
      {
         fKDTreeMatch.PadsFindWires(SigFlow->pdSig, SigFlow->awSig,wire_tree, fPhiFactor, spacepoints);
      }
#endif
      // allow events without pwbs
      if (SigFlow->combinedPads.size()) {
         // spacepoints = match->CombPoints(spacepoints);
      } else if (fFlags->fForceReco) // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
      {
         if (fTrace) printf("KDTreeMatchModule::Analyze, NO combined pads, Set Z=0\n");

      }

      if (spacepoints.size() && SigFlow->pdSig.size() && SigFlow->awSig.size()) {
         if (fFlags->fTrace) printf("KDTreeMatchModule::Analyze, Spacepoints # %d\n", int(spacepoints.size()));
         SigFlow->AddMatchSignals(spacepoints);
      } else
         printf("KDTreeMatchModule::Analyze Spacepoints should exists at this point\n");
      return flow;
   }
};

class KDTreeMatchFactory : public TAFactory {
public:
   KDTreeMatchFlags fFlags;

public:
   void Help()
   {
      printf("KDTreeMatchFactory::Help\n");
      printf("\t--forcereco\t\tEnable reconstruction when no pads are associated with the event by setting z=0\n");
   }
   void Usage() { Help(); }

   void Init(const std::vector<std::string> &args)
   {
      TString json = "default";
      // printf("KDTreeMatchFactory::Init!\n");
      for (unsigned i = 0; i < args.size(); i++) {
         if (args[i] == "--recoff") fFlags.fRecOff = true;
         if (args[i] == "--diag") fFlags.fDiag = true;
         if (args[i] == "--trace") fFlags.fTrace = true;
         if (args[i] == "--forcereco") fFlags.fForceReco = true;
         if (args[i] == "--anasettings") {
            i++;
            json = args[i];
            i++;
         }
      }
      fFlags.ana_settings = new AnaSettings(json);
      if (fFlags.fTrace) fFlags.ana_settings->Print();
   }

   KDTreeMatchFactory() {}

   void Finish()
   {
      if (fFlags.fTrace == true) printf("KDTreeMatchFactory::Finish!\n");
   }

   TARunObject *NewRunObject(TARunInfo *runinfo)
   {
      if (fFlags.fTrace == true)
         printf("KDTreeMatchFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new KDTreeMatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new KDTreeMatchFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
