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

class TrackyMatchFlags {
public:
   bool         fRecOff      = false; // Turn reconstruction off
   AnaSettings *ana_settings = NULL;
   bool         fDiag        = false;
   bool         fTrace       = false;
   bool         fForceReco   = false;

   int TotalThreads = 0;
   TrackyMatchFlags() // ctor
   {
   }

   ~TrackyMatchFlags() // dtor
   {
   }
};

// For now use root's KDTree implementation
#include "TKDTree.h"


class TrackyMatchModule : public TARunObject {
public:
   TrackyMatchFlags *fFlags     = NULL;
   bool              fTrace     = false;
   int               fCounter   = 0;
   bool              diagnostic = false;



public:
   TrackyMatchModule(TARunInfo *runinfo, TrackyMatchFlags *f) : TARunObject(runinfo)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName = "TrackyMatch";
#endif
      if (fTrace) printf("TrackyMatchModule::ctor!\n");

      fFlags = f;

      diagnostic = fFlags->fDiag;  // dis/en-able histogramming
      fTrace     = fFlags->fTrace; // enable verbosity
   }

   ~TrackyMatchModule()
   {
      if (fTrace) printf("TrackyMatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("TrackyMatchModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
   }
   void EndRun(TARunInfo *runinfo)
   {
      if (fTrace) printf("TrackyMatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
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
      if (fTrace) printf("TrackyMatchModule::Analyze, run %d, counter %d\n", runinfo->fRunNo, fCounter++);

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

      if (!SigFlow->awSig) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }
      if (fTrace) {
         printf("TrackyMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
         printf("TrackyMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
      }

      std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>> *spacepoints = NULL;

      // std::vector<TSignalNode<2>> pad_list;
      if (SigFlow->pdSig) {
         printf("TrackyMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));

         int i = 0;
         // Roots generic kdtree wont understand cylindrical coordinates..
         std::vector<double> x;
         std::vector<double> y;
         for (const ALPHAg::signal &s : *(SigFlow->pdSig)) {
            x.emplace_back(s.t * sin(s.phi));
            y.emplace_back(s.t * cos(s.phi));
            i++;
         }
         TKDTreeID pad_tree(x.size(), 2, 1);
         pad_tree.SetData(0, x.data());
         pad_tree.SetData(1, y.data());
         pad_tree.Build();
         // std::cout << node_list.size() << " nodes\n";
         spacepoints = new std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>>();
         for (const ALPHAg::signal &s : *(SigFlow->awSig)) {
            if (s.t > 0) {
               std::array<double, 2> data = {s.t * sin(s.phi), s.t * cos(s.phi)};
               int                   index;
               double                distance;
               pad_tree.FindNearestNeighbors(data.begin(), 1, &index, &distance);
               if (distance < 100)
               {
                  spacepoints->emplace_back(
                          std::make_pair(
                              s,
                              SigFlow->pdSig->at(index)
                              )
                  );
                  //std::cout <<"Index:"<< index << "\t"<< distance <<std::endl;
               }
            }
         }
      }
      // allow events without pwbs
      if (SigFlow->combinedPads) {
         // spacepoints = match->CombPoints(spacepoints);
      } else if (fFlags->fForceReco) // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
      {
         if (fTrace) printf("TrackyMatchModule::Analyze, NO combined pads, Set Z=0\n");
         // delete match->GetCombinedPads();?
         //         spacepoints = match->FakePads( SigFlow->awSig );
      }

      if (spacepoints) {
         if (fFlags->fTrace) printf("TrackyMatchModule::Analyze, Spacepoints # %d\n", int(spacepoints->size()));
         if (spacepoints->size() > 0) SigFlow->AddMatchSignals(spacepoints);
      } else
         printf("TrackyMatchModule::Analyze Spacepoints should exists at this point\n");

      delete spacepoints;
      return flow;
   }
};

class TrackyMatchFactory : public TAFactory {
public:
   TrackyMatchFlags fFlags;

public:
   void Help()
   {
      printf("TrackyMatchFactory::Help\n");
      printf("\t--forcereco\t\tEnable reconstruction when no pads are associated with the event by setting z=0\n");
   }
   void Usage() { Help(); }

   void Init(const std::vector<std::string> &args)
   {
      TString json = "default";
      // printf("TrackyMatchFactory::Init!\n");
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

   TrackyMatchFactory() {}

   void Finish()
   {
      if (fFlags.fTrace == true) printf("TrackyMatchFactory::Finish!\n");
   }

   TARunObject *NewRunObject(TARunInfo *runinfo)
   {
      if (fFlags.fTrace == true)
         printf("TrackyMatchFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TrackyMatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new TrackyMatchFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
