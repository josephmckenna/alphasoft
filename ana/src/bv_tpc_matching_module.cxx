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
   //double radius = (inner_diameter+outer_diameter)/4.; // Use centre of BV
   double radius = inner_diameter/2; // Use inner edge of BV
   double length = 2604; // mm
   double speed=TMath::C();
   double cFactor=1.58;

   // Matching parameters
   double max_z_diff = 1000; // mm
   double max_phi_diff = 0.2945; // rad

   // TOF parameters
   double min_dphi = TMath::Pi()/2; //rad
   double z_cut = 800; //mm from centre

   // Cuts
   //double polar_max = TMath::Pi()/9.;

   //Histogramm declaration
   TH2D* hHits = NULL;
   TH1D* hdPhi = NULL;
   TH1D* hdZ = NULL;
   TH1D* hZ = NULL;
   TH1D* hZBV = NULL;
   TH2D* hZVZ = NULL;
   TH2D* hPhiVPhi = NULL;
   TH2D* hZVAmpTop = NULL;
   TH2D* hZVAmpBot = NULL;
   TH1D* hGoodMatch = NULL;
   TH2D* hZBVminusfit = NULL;
   TH2D* hBVtimeres = NULL;
   TH1D* hTOF = NULL;
   TH1D* hTOFcut = NULL;
   TH2D* hTOFVPath = NULL;
   TH2D* hTOFVZ = NULL;
   TH2D* hTOFVPhi = NULL;
   TH1D* hAngleCorrectedTOF = NULL;
   TH2D* hTopPlusBot = NULL;

   // Container declaration
   std::vector<TVector3> bv_points;
   std::vector<TVector3> helix_points;
   std::vector<TVector3> line_points;
   std::vector<BarHit>* BVhits;
   std::vector<TVector3> matched_points;
   std::vector<double> time_top;
   std::vector<double> time_bot;

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
      hdPhi = new TH1D("hdPhi","Delta phi between BV hit and TPC track;Delta Phi [rad]",360,-TMath::Pi(),TMath::Pi());
      hdZ = new TH1D("hdZ","Delta z between BV hit and TPC track;Delta Z [mm]",1500,-3000,3000);
      hZ = new TH1D("hZ","Z of TPC track;Z [mm]",1500,-3000,3000);
      hZBV = new TH1D("hZBV","Z of BV hit (after TPC matching);Z [mm]",1500,-3000,3000);
      hZVZ = new TH2D("hZVZ","Z of BV hit and TPC track;Z (BV Hit) [mm]; Z (TPC Track) [mm]",1500,-3000,3000,1500,-3000,3000);
      hPhiVPhi = new TH2D("hPhiVPhi","Phi of BV hit and TPC track;Phi (BV Hit) [rad]; Phi (TPC Track) [rad]",360,-TMath::Pi(),TMath::Pi(),360,-TMath::Pi(),TMath::Pi());
      hZVAmpTop = new TH2D("hZVAmpTop","Amplitude of top ADC pulse at different TPC Z;TPC Zed [mm];ADC Amplitude",1500,-3000,3000,2000,0,35000);
      hZVAmpBot = new TH2D("hZVAmpBot","Amplitude of bottom ADC pulse at different TPC Z;TPC Zed [mm];ADC Amplitude",1500,-3000,3000,2000,0,35000);
      hGoodMatch = new TH1D("hGoodMatch","Number of hits matched within 3 bars and 1.0m in z;1=Good match, 0=Bad match;Counts",2,-0.5,1.5);
      hZBVminusfit = new TH2D("hZTPCminusfit","Z(BV) with fit subtracted;Bar Number;Z(BV) minus fit [mm];Counts",64,0,64,2000,-1000,1000);
      hBVtimeres = new TH2D("hBVtimeres","BV time difference with fit from Z TPC subtracted;Bar Number;BV time difference [ps];Counts",64,0,64,2000,-5000,5000);
      hTOF = new TH1D("hTOF","Time of flight;Time of flight [s];Counts",1000,0,10e-9);
      hTOFcut = new TH1D("hTOFcut","Time of flight (dphi>pi/2);Time of flight [s];Counts",1000,0,10e-9);
      hTOFVPath = new TH2D("hTOFVPath","Distance between BV hits vs Time of flight (dphi>pi/2);Distance between BV hits [mm];Time of flight [s]",1500,0,1500,1000,0,10e-9);
      hTOFVZ = new TH2D("hTOFVZ","Delta Z vs Time of flight (dphi>pi/2);Z distance between BV hits [mm];Time of flight [s]",1500,0,1500,1000,0,10e-9);
      hTOFVPhi = new TH2D("hTOFVPhi","Delta phi vs Time of flight (dphi>pi/2);Angle [rad];Time of flight [s]",360,-TMath::Pi(),TMath::Pi(),1000,0,10e-9);
      hAngleCorrectedTOF = new TH1D("hAngleCorrectedTOF","Angle Corrected Time of flight (dphi>pi/2);Time of flight [s];Counts",1000,0,10e-9);
      hTopPlusBot = new TH2D("hTopPlusBot","Time at top of bar plus time at bottom of bar;Hit 1 [s];Hit 2 [s]",1000,0,1000e-9,1000,0,1000e-9);

   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      // Delete histos
      delete hHits;
      delete hdPhi;
      delete hdZ;
      delete hZVZ;
      delete hPhiVPhi;
      delete hZ;
      delete hZBV;
      delete hZVAmpTop;
      delete hZVAmpBot;
      delete hZBVminusfit;
      delete hBVtimeres;
      delete hTOF;
      delete hTOFcut;
      delete hTOFVPath;
      delete hTOFVZ;
      delete hTOFVPhi;
      delete hAngleCorrectedTOF;
      delete hTopPlusBot;
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
      time_top.clear();
      time_bot.clear();
      matched_points.clear();

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
      TimeOfFlight();

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
      for (auto line: *LineArray)
         {
            TStoreLine* l = (TStoreLine*)line;
            TFitLine* lin = new TFitLine(l);
            //double polar_angle = lin->GetU().Theta(); // Angle from pointed directly up (+z)
            //if (TMath::Abs(polar_angle-TMath::Pi()/2)<polar_max) line_points.push_back(lin->Evaluate(radius*radius));
            line_points.push_back(lin->Evaluate(radius*radius));
            delete lin;
         }
   }
   void GetHelices(TAFlowEvent* flow)
   {
      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();
      if (!anaflow) {std::cout<<"matchingmodule: AgAnalysisFlow not found!"<<std::endl; return; }
      else std::cout<<"FOUNDED!"<<std::endl;
      TStoreEvent* e = anaflow->fEvent;
      const TObjArray* HelixArray = e->GetHelixArray();
      for (auto helix: *HelixArray)
         {
            TStoreHelix* h = (TStoreHelix*)helix;
            TFitHelix* hel = new TFitHelix(h);
            helix_points.push_back(hel->Evaluate(radius*radius));
            delete hel;
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
            int bar_number = BVhits->at(i).GetBar();
            std::vector<double> distances;
            for (TVector3 tpc_point: tpc_points)
               {
                  // Weighted cylindrical distance
                  double dphi = std::min(TMath::Abs(bv_point.DeltaPhi(tpc_point)),TMath::Abs(tpc_point.DeltaPhi(bv_point)));
                  double dz = bv_point.z()-tpc_point.z();
                  distances.push_back(TMath::Power(dphi/TMath::Pi(),2)+0.5*TMath::Power(dz/length,2));
               }
            int min_index = std::min_element(distances.begin(), distances.end())-distances.begin(); // Minimize distance
            TVector3 tpc_point = tpc_points.at(min_index);
            double dz = bv_point.z()-tpc_point.z();
            double dphi = bv_point.DeltaPhi(tpc_point);
            hGoodMatch->Fill(TMath::Abs(dz)<max_z_diff && TMath::Abs(dphi)<max_phi_diff);
            if (TMath::Abs(dz)>max_z_diff || TMath::Abs(dphi)>max_phi_diff) continue;
            hdPhi->Fill(dphi);
            hdZ->Fill(dz);
            hZVZ->Fill(bv_point.z(),tpc_point.z());
            hPhiVPhi->Fill(bv_point.Phi(),tpc_point.Phi());
            hZ->Fill(tpc_point.z());
            hZBV->Fill(bv_point.z());
            hZVAmpTop->Fill(tpc_point.z(),BVhits->at(i).GetAmpTop());
            hZVAmpBot->Fill(tpc_point.z(),BVhits->at(i).GetAmpBot());
            double zbvminusfit = bv_point.z()-(1.5856*tpc_point.z()-77.98);
            hZBVminusfit->Fill(bar_number,zbvminusfit);
            hBVtimeres->Fill(bar_number,zbvminusfit/(speed/cFactor)*1e9);


            matched_points.push_back(TVector3(bv_point.X(),bv_point.Y(),tpc_point.Z()));
            time_top.push_back(BVhits->at(i).GetTDCTop());
            time_bot.push_back(BVhits->at(i).GetTDCBot());
         }
   }

   void TimeOfFlight()
   {
      if (matched_points.size()!=2) return;
      if (TMath::Abs(matched_points[0].Z())>z_cut) return;
      if (TMath::Abs(matched_points[1].Z())>z_cut) return;
      double TOF = TMath::Abs(time_top[1]+time_bot[1]-time_top[0]-time_bot[0])/2.;
      hTOF->Fill(TOF);
      double dphi = TMath::Abs(matched_points[0].DeltaPhi(matched_points[1]));
      //double dphi = std::min(TMath::Abs(matched_points[0].DeltaPhi(matched_points[1])),TMath::Abs(matched_points[1].DeltaPhi(matched_points[0])));
      if (dphi<min_dphi) return;
      hTOFcut->Fill(TOF);
      double dz = TMath::Abs(matched_points[0].Z()-matched_points[1].Z());
      double IdealPathLength = 2*radius;
      double TruePathLength = TMath::Sqrt( dz*dz + 2*radius*radius*(1-TMath::Cos(dphi)));
      hTOFVPath->Fill(TruePathLength,TOF);
      hTOFVZ->Fill(dz,TOF);
      hTOFVPhi->Fill(dphi,TOF);
      double AngleCorrectedTOF = TOF*(IdealPathLength/TruePathLength);
      hAngleCorrectedTOF->Fill(AngleCorrectedTOF);
      hTopPlusBot->Fill(time_top[0]+time_bot[0]+2*1784291.121733e-12,time_top[1]+time_bot[1]+2*1784291.121733e-12);
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
