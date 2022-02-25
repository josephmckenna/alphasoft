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
#include "TKDTree.h"


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

      if (!SigFlow->awSig) {
         *flags |= TAFlag_SKIP_PROFILE;
         return flow;
      }
      if (fTrace) {
         printf("KDTreeMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
         printf("KDTreeMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
      }

//Only wiresFindPads works for now
#define wiresFindPads 1

      std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>> *spacepoints = NULL;
#if wiresFindPads
      TKDTreeID* pad_tree = NULL;
      // Naive phi and t matching will have wrap around problems for phi
      std::vector<double> pad_tree_x;
      std::vector<double> pad_tree_y;
#else
      TKDTreeID* wire_tree = NULL;
      // Naive phi and t matching will have wrap around problems for phi
      std::vector<double> wire_tree_x;
      std::vector<double> wire_tree_y;
#endif


      // A fix would be multiple domains... but I can't get it working
      // std::vector<double> t[2];
      // std::vector<double> phi[2];
      // std::array<TKDTreeID*,2> pad_quadrant_trees = {NULL};
#if wiresFindPads
      if (SigFlow->pdSig) {
         //printf("KDTreeMatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
         if (int(SigFlow->pdSig->size()) > fPadSignalsCut)
            return flow;

         // Roots generic kdtree wont understand cylindrical coordinates..
         const int nPads = SigFlow->pdSig->size();
         pad_tree = new TKDTreeID(nPads, 2, 1);
         int i = 0;
         for (const ALPHAg::signal &s : *(SigFlow->pdSig)) {
            if (1)
            { 
               /*if ( s.phi < M_PI + (M_2_PI / 32) || s.phi > 2.0 * M_PI - (M_2_PI / 32) )
               {
                   t[0].emplace_back(s.t);
                   phi[0].emplace_back(fPhiFactor * s.phi);
               }
               else
               {
                   t[0].emplace_back(1.0/0.0);
                   phi[0].emplace_back(1.0/0.0);
               }
               if ( s.phi > M_PI - (M_2_PI / 32) && s.phi < M_PI + (M_2_PI / 32) )
               {
                   t[1].emplace_back(s.t);
                   // Move wrap around point of phi
                   double tempphi = s.phi - M_PI;
                   if  (tempphi < 0 )
                      tempphi += M_PI;
                   phi[1].emplace_back(fPhiFactor * tempphi);
               }
               else
               {
                   t[1].emplace_back(1.0/0.0);
                   phi[1].emplace_back(1.0/0.0);
               }*/

               pad_tree_x.emplace_back(s.t);
               pad_tree_y.emplace_back(fPhiFactor * s.phi);
            }
            else
            {
               pad_tree_x.emplace_back( 0.0/0.0 );
               pad_tree_y.emplace_back( 0.0/0.0 );
            }
            //y.emplace_back(s.phi);
            i++;
         }
         pad_tree->SetData(0, pad_tree_x.data());
         pad_tree->SetData(1, pad_tree_y.data());
         pad_tree->Build();
         /*for (int i = 0; i < 2; i++)
         {
             pad_quadrant_trees[i] = new TKDTreeID(pad_tree_x.size(), 2, 1);
             pad_quadrant_trees[i]->SetData(0,t[i].data());
             pad_quadrant_trees[i]->SetData(1,phi[i].data());
             pad_quadrant_trees[i]->Build();
         }*/
      }
#else
      if (SigFlow->awSig)
      {
         printf("KDTreeMatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
         if (int(SigFlow->awSig->size()) > fWireSignalsCut)
            return flow;
         int i = 0;
         // Roots generic kdtree wont understand cylindrical coordinates..

         for (const ALPHAg::signal &s : *(SigFlow->awSig)) {
            wire_tree_x.emplace_back(s.t);
            wire_tree_y.emplace_back(s.t);
            i++;
         }
         wire_tree = new TKDTreeID(wire_tree_x.size(), 2, 1);
         wire_tree->SetData(0, wire_tree_x.data());
         wire_tree->SetData(1, wire_tree_y.data());
         wire_tree->Build();
      }
#endif

#if wiresFindPads
      if (SigFlow->awSig)
      {
         spacepoints = new std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>>();
         for (const ALPHAg::signal &s : *(SigFlow->awSig)) {
            if (s.t > 0) {
               std::array<double, 2> data = {s.t,fPhiFactor * s.phi};// {s.t * sin(s.phi), s.t * cos(s.phi)};
               //std::array<double, 2> data = { (1/ s.t) * sin(s.phi), (1/ s.t) * cos(s.phi)};// {s.t * sin(s.phi), s.t * cos(s.phi)};
               //std::array<double, 2> data = { (6000 -  s.t) * sin(s.phi), (6000 - s.t) * cos(s.phi)}
               int                   index = 0;
               double                distance = 9999999999999999999.;

               pad_tree->FindNearestNeighbors(data.begin(), 1, &index, &distance);
               //std::cout << s.phi <<std::endl;
/*
               if (  s.phi < M_PI )
               {
                  pad_quadrant_trees[0]->FindNearestNeighbors(data.begin(), 1, &index, &distance);
               }
               else if ( s.phi >= M_PI && s.phi < 2.0*M_PI )
               {
                   double tempphi = s.phi - M_PI;
                   if  (tempphi < 0 )
                   {
                      tempphi += M_PI;
                   }
                      data[1] = fPhiFactor * tempphi;
                    
                  pad_quadrant_trees[1]->FindNearestNeighbors(data.begin(), 1, &index, &distance);
               }
               else{
                  std::cout<<"LINE" << __LINE__ << "FAIL!"<<std::endl;
               }
*/
               //std::cout <<"Dist: "<< distance << "\t"<< s.errphi<<std::endl;
               if (index < 0)
                  continue;
               const ALPHAg::signal& pad = SigFlow->pdSig->at(index);
               //std::cout <<"\tIndex: "<< index << "\tDistance: " << distance  << " ( "<<sqrt( 20 * 20 + fPhiFactor * fPhiFactor * pad.errphi * pad.errphi )<<" )"<< std::endl;
               //std::cout  <<"\tPhi: "<< pad.phi << "\tDPhi: " <<pad.errphi <<std::endl;
               
               //if (distance <  3* sqrt( 20 * 20 + fPhiFactor * fPhiFactor * pad.errphi * pad.errphi  ) )
               
               if (fabs(pad.phi - s.phi) < 3* pad.errphi &&  fabs(pad.t - s.t) < 20)
               {
                  //std::cout <<"Delta phi:\t" << fabs(pad.phi - s.phi) <<"<"<< 3* pad.errphi<< "\n";
                  //std::cout <<"Delta t:\t" << fabs(pad.t - s.t) << "\t| "<< pad.t << " - " << s.t <<" |\n";
               
                  spacepoints->emplace_back(
                          std::make_pair(
                              s,
                              pad
                              )
                  );
                  //std::cout <<"Index:"<< index << "\t"<< distance <<std::endl;
               }
            }
         }
       }
#else
       if (SigFlow->pdSig)
       {
         spacepoints = new std::vector<std::pair<ALPHAg::signal, ALPHAg::signal>>();
         for (const ALPHAg::signal &s : *(SigFlow->pdSig)) {
            if (s.t > 0) {
               std::array<double, 2> data = {s.t, fPhiFactor * s.phi};// {s.t * sin(s.phi), s.t * cos(s.phi)};
               int                   index= 0;
               double                distance = 9999999999999999999.;
               wire_tree->FindNearestNeighbors(data.begin(), 1, &index, &distance);
               if (index < 0)
                  continue;
               const ALPHAg::signal& wire = SigFlow->pdSig->at(index);
               
               if (fabs(wire.phi - s.phi) < 3* wire.errphi &&  fabs(wire.t - s.t) < 20)
               {
                  spacepoints->emplace_back(
                          std::make_pair(
                              wire,
                              s
                              )
                  );
                  //std::cout <<"Index:"<< index << "\t"<< distance <<std::endl;
               }
            }
         }
      }
#endif
      // allow events without pwbs
      if (SigFlow->combinedPads) {
         // spacepoints = match->CombPoints(spacepoints);
      } else if (fFlags->fForceReco) // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
      {
         if (fTrace) printf("KDTreeMatchModule::Analyze, NO combined pads, Set Z=0\n");
         // delete match->GetCombinedPads();?
         //         spacepoints = match->FakePads( SigFlow->awSig );
      }

      if (spacepoints) {
         if (fFlags->fTrace) printf("KDTreeMatchModule::Analyze, Spacepoints # %d\n", int(spacepoints->size()));
         if (spacepoints->size() > 0) SigFlow->AddMatchSignals(spacepoints);
      } else
         printf("KDTreeMatchModule::Analyze Spacepoints should exists at this point\n");

#if wiresFindPads
      if (pad_tree)
         delete pad_tree;
#else
      if (wire_tree)
         delete wire_tree;
#endif
      delete spacepoints;
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
