#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "AnalysisTimer.h"

#include "TBarEvent.hh"

class MatchingModuleFlags
{
public:
   double fMagneticField = -1.;

   MatchingModuleFlags() // ctor
   { }
   ~MatchingModuleFlags() // dtor
   { }
};

class matchingmodule: public TARunObject
{
public:
   MatchingModuleFlags* fFlags;

private:

   // BV geometry
   double inner_diameter = 446.0; // mm
   double outer_diameter = 486.0; // mm
   double radius = (inner_diameter+outer_diameter)/4.;
   double length = 2604; // mm

   // Matching parameters
   double max_z_diff = 2000; // mm
   double max_phi_diff = 0.8; // rad


   //Histogramm declaration
   TH2D* hHits = NULL;
   TH2D* hdPhi = NULL;
   TH2D* hdZ = NULL;
   TH1D* hZ = NULL;
   TH1D* hZBV = NULL;
   TH2D* hdZVZ = NULL;
   TH2D* hZVZ = NULL;
   TH2D* hZVAmpTop = NULL;
   TH2D* hZVAmpBot = NULL;

   // Container declaration
   std::vector<TVector3> bv_points;
   std::vector<TVector3> helix_points;
   std::vector<TVector3> line_points;
   std::vector<BarHit>* BVhits;

public:

   matchingmodule(TARunInfo* runinfo, MatchingModuleFlags* f): TARunObject(runinfo), fFlags(f)
   {
      printf("matchingmodule::ctor!\n");
   }

   ~matchingmodule()
   {
      printf("matchingmodule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      printf("matchingmodule::begin!");
      gDirectory->mkdir("bv_tpc_matching_module")->cd();

      // Histogramm setup
      hHits = new TH2D("hHits","Number of hits;BV helices;TPC hits",10,0,10,10,0,10);
      hdPhi = new TH2D("hdPhi","Delta phi between BV hit and TPC track;Number of BV hits;Delta Phi [rad]",10,0,10,360,-2*TMath::Pi(),2*TMath::Pi());
      hdZ = new TH2D("hdZ","Delta z between BV hit and TPC track;Number of BV hits;Delta Z [mm]",10,0,10,1500,-3000,3000);
      hZ = new TH1D("hZ","Z of TPC track;Z [mm]",1500,-3000,3000);
      hZBV = new TH1D("hZBV","Z of BV hit (after TPC matching);Z [mm]",1500,-3000,3000);
      hdZVZ = new TH2D("hdZVZ","Delta z between BV hit and TPC track;Z [mm];Delta Z [mm]",1500,-3000,3000,1500,-3000,3000);
      hZVZ = new TH2D("hZVZ","Z of BV hit and TPC track;Z (BV Hit) [mm]; Z (TPC Track) [mm]",1500,-3000,3000,1500,-3000,3000);
      hZVAmpTop = new TH2D("hZVAmpTop","Amplitude of top ADC pulse at different TPC Z;TPC Zed [mm];ADC Amplitude",1500,-3000,3000,2000,0,35000);
      hZVAmpBot = new TH2D("hZVAmpBot","Amplitude of bottom ADC pulse at different TPC Z;TPC Zed [mm];ADC Amplitude",1500,-3000,3000,2000,0,35000);

   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      // Delete histos
      delete hHits;
      delete hdPhi;
      delete hdZ;
      delete hdZVZ;
      delete hZVZ;
      delete hZ;
      delete hZBV;
      delete hZVAmpTop;
      delete hZVAmpBot;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   // Main function
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif

      // Reset containers
      bv_points.clear();
      helix_points.clear();
      line_points.clear();

      // Main functions
      GetBVHits(flow);
      if( fFlags->fMagneticField > 0. )
         {
            GetHelices(flow);
            MatchPoints(helix_points);
         }
      else
         {
            GetLines(flow);
            MatchPoints(line_points);
         }

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bv_tpc_matching_module",timer_start);
      #endif
      return flow;
   }

   //________________________________
   // MAIN FUNCTIONS

   void GetBVHits(TAFlowEvent* flow)
   {
      AgBarEventFlow *bef=flow->Find<AgBarEventFlow>(); // Gets list of adc hits from flow
      if (!bef) {std::cout<<"matchingmodule: AgBarEventFlow not found!"<<std::endl; return; }
      TBarEvent *barEvt=bef->BarEvent;
      BVhits =  barEvt->GetBars();
      for (BarHit hit: *BVhits)
         {
            double x,y;
            hit.GetXY(x,y);
            double z = hit.GetTDCZed();
            bv_points.push_back(TVector3(x*1000,y*1000,z*1000));
         }
   }

   void GetLines(TAFlowEvent* flow)
   {
      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow) {std::cout<<"matchingmodule: AgAnalysisFlow not found!"<<std::endl; return; }
      TStoreEvent* e = anaflow->fEvent;
      const TObjArray* LineArray = e->GetLineArray();
      std::cout<<"Getting lines poggers"<<std::endl;
      for (auto line: *LineArray)
         {
            TStoreLine* l = (TStoreLine*)line;
            TFitLine* lin = new TFitLine(l);
            line_points.push_back(lin->Evaluate(radius*radius));
            std::cout<<"gottem"<<std::endl;
         }
   }
   void GetHelices(TAFlowEvent* flow)
   {
      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow) {std::cout<<"matchingmodule: AgAnalysisFlow not found!"<<std::endl; return; }
      TStoreEvent* e = anaflow->fEvent;
      const TObjArray* HelixArray = e->GetHelixArray();
      for (auto helix: *HelixArray)
         {
            TStoreHelix* h = (TStoreHelix*)helix;
            TFitHelix* hel = new TFitHelix(h);
            helix_points.push_back(hel->Evaluate(radius*radius));
         }
   }
   void MatchPoints(std::vector<TVector3> tpc_points)
   {
      int n_bv = bv_points.size();
      int n_tpc = tpc_points.size();
      hHits->Fill(n_bv,n_tpc);
      if (tpc_points.size()==0) return;
      for (int i=0;i<n_bv;i++)
         {
            TVector3 bv_point = bv_points.at(i);
            std::vector<double> distances;
            for (TVector3 tpc_point: tpc_points)
               {
                  // Weighted cylindrical distance
                  double dphi = std::min(TMath::Abs(bv_point.DeltaPhi(tpc_point)),TMath::Abs(tpc_point.DeltaPhi(bv_point)));
                  double dz = bv_point.z()-tpc_point.z();
                  distances.push_back(TMath::Power(dphi/TMath::Pi(),2)+TMath::Power(dz/length,2));
               }
            int min_index = std::min_element(distances.begin(), distances.end())-distances.begin(); // Minimize distance
            TVector3 tpc_point = tpc_points.at(min_index);
            double dz = bv_point.z()-tpc_point.z();
            double dphi = bv_point.DeltaPhi(tpc_point);
            if (TMath::Abs(dz)>max_z_diff || TMath::Abs(dphi)>max_phi_diff) continue;
            hdPhi->Fill(n_bv,dphi);
            hdZ->Fill(n_bv,dz);
            hdZVZ->Fill(tpc_point.z(),dz);
            hZVZ->Fill(bv_point.z(),tpc_point.z());
            hZ->Fill(tpc_point.z());
            hZBV->Fill(bv_point.z());
            hZVAmpTop->Fill(tpc_point.z(),BVhits->at(i).GetAmpTop());
            hZVAmpBot->Fill(tpc_point.z(),BVhits->at(i).GetAmpBot());
         }
   }

};

class matchingModuleFactory: public TAFactory
{
public:
   MatchingModuleFlags fFlags;
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("matchingModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
         {
         if( args[i] == "--Bfield" )
            {
               fFlags.fMagneticField = atof(args[++i].c_str());
               printf("Magnetic Field (incompatible with --loadcalib)\n");
            }
         }
   }

   void Finish()
   {
      printf("matchingModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("matchingModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new matchingmodule(runinfo,&fFlags);
   }
};

static TARegister tar(new matchingModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
