#include <iostream>
#include <string>
#include <sstream>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TBrowser.h"

#include "TH1D.h"
#include "TH2D.h"

#include "Deconv.hh"
#include "Match.hh"
#include "Reco.hh"

#include "Utils.hh"

using namespace std;

//enum finderChoice { base, adaptive, neural };

int main(int argc, char** argv)
{
   if( argc == 1 )
      {
         cerr<<"Please provide rootfile from command line"<<endl;
         return 1;
      }

   TFile* fin = TFile::Open(argv[1],"READ");
   if( !fin->IsOpen() )
      {
         cerr<<"rootfile not open"<<endl;
         return 1;
      }

   TTree* tMC = (TTree*) fin->Get("MCinfo");
   TClonesArray* vtx = new TClonesArray("TVector3");
   tMC->SetBranchAddress("MCvertex",&vtx);

   TTree* tGarf = (TTree*) fin->Get("Garfield");
   TClonesArray* garfpp_hits = new TClonesArray("TMChit");
   tGarf->SetBranchAddress("GarfHits",&garfpp_hits);
   TClonesArray* aw_hits = new TClonesArray("TMChit");
   tGarf->SetBranchAddress("AnodeHits",&aw_hits);

   TTree* tSig =  (TTree*) fin->Get("Signals");
   if( !tSig )
      {
         cerr<<"rootfile does not contain proper simulation data"<<endl;
         return 1;
      }

   TClonesArray* AWsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("AW",&AWsignals);

   TClonesArray* PADsignals = new TClonesArray("TWaveform");
   tSig->SetBranchAddress("PAD",&PADsignals);

   string json_file = "sim.json";
   ostringstream json_filepath;
   json_filepath<<getenv("AGRELEASE")<<"/ana/"<<json_file;
   cout<<"[main]# Loading Ana settings from: "<<json_filepath.str()<<endl;
   
   Deconv d(json_filepath.str());
   // ofstream fout("deconv_goodness.dat", ios::out | ios::app);
   // fout<<d.GetADCthres()<<"\t"<<d.GetPWBthres()<<"\t"
   // <<d.GetAWthres()<<"\t"<<d.GetPADthres()<<"\t";
   cout<<"-------------------------"<<endl;
   cout<<"Deconv Settings"<<endl;
   cout<<" ADC delay: "<<d.GetADCdelay()<<"\tPWB delay: "<<d.GetPWBdelay()<<endl;
   cout<<" ADC thresh: "<<d.GetADCthres()<<"\tPWB thresh: "<<d.GetPWBthres()<<endl;
   cout<<" AW thresh: "<<d.GetAWthres()<<"\tPAD thresh: "<<d.GetPADthres()<<endl;
   cout<<"-------------------------"<<endl;

   finderChoice finder = adaptive;
   if(argc > 2){
      switch(*argv[2]){
      case 'b':
      case 'B': finder = base; cout << "Using basic TracksFinder (untested)" << endl; break;
      case 'a':
      case 'A': finder = adaptive; cout << "Using AdaptiveFinder" << endl; break;
      case 'n':
      case 'N': finder = neural; cout << "Using NeuralFinder" << endl; break;
      default: cerr << "Finder selection " << *argv[2] << " unknown, using basic finder";
      }
   }

   Match m(json_filepath.str());
   //ofstream fout("match_goodness.dat", ios::out | ios::app);
   //ofstream fout("pattrec_goodness.dat", ios::out | ios::app);

   double B=1.;
   Reco r(json_filepath.str(),B);

   Reco rMC(json_filepath.str(),B);

   //bool draw = false;
   bool draw = true;
   bool verb = false;

   double tmax = 4500.;

   bool enableMC=false;

   TApplication* app;
   if( draw )
      app = new TApplication("g4ana",&argc,argv);

   TCanvas* csig;
   TCanvas* creco;

   if( draw )
      {
         csig = new TCanvas("csig","csig",1400,1400);
         csig->Divide(2,2);

         creco = new TCanvas("creco","creco",1400,1400);
         creco->Divide(2,2);
      }

   for( int i=0; i<tSig->GetEntries(); ++i )
      {
         tSig->GetEntry(i);

         // anode deconv
         int nsig = d.FindAnodeTimes( AWsignals );
         cout<<"[main]# "<<i<<"\tFindAnodeTimes: "<<nsig<<endl;
         if( nsig == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //      fout<<std::setprecision(15)<<Average( d.GetAnodeDeconvRemainder() )<<"\t";

         if( verb ) PrintSignals( d.GetAnodeSignal() );
         TH1D* haw;
         if( draw )
            {
               haw = PlotSignals( d.GetAnodeSignal(), "anodes" );
               haw->Scale(1./haw->Integral());
               haw->SetLineColor(kRed);
               cout<<"[main]# "<<i<<"\tPlotAnodeTimes: "<<haw->GetEntries()<<endl;
               csig->cd(1);
               haw->Draw("hist");
               haw->SetTitle("Deconv Times");
               haw->GetXaxis()->SetRangeUser(0.,tmax);
            }

         // pad deconv
         nsig = d.FindPadTimes( PADsignals );
         cout<<"[main]# "<<i<<"\tFindPadTimes: "<<nsig<<endl;
         if( nsig == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //      fout<<std::setprecision(15)<<Average( d.GetPadDeconvRemainder() )<<endl;

         if( verb ) PrintSignals( d.GetPadSignal() );
         if( draw )
            {
               TH1D* hpads = PlotSignals( d.GetPadSignal(), "pads" );
               hpads->Scale(1./hpads->Integral());
               hpads->SetLineColor(kBlue);
               csig->cd(1);
               hpads->Draw("histsame");
            }

         m.Init();

         // combine pads
         m.CombinePads( d.GetPadSignal() );
         uint npads = m.GetCombinedPads()->size();
         cout<<"[main]# "<<i<<"\tCombinePads: "<<npads<<endl;
         if( npads == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         if( verb ) PrintSignals( m.GetCombinedPads() );
         if( draw )
            {
               TH1D* hcombpads = PlotSignals( m.GetCombinedPads(), "combinedpads" );
               hcombpads->Scale(1./hcombpads->Integral());
               hcombpads->SetLineColor(kBlue);
               csig->cd(2);
               haw->Draw("hist");
               hcombpads->Draw("histsame");

               TH2D* hmatch = PlotSignals( d.GetAnodeSignal(), m.GetCombinedPads(), "sector");
               //TH2D* hmatch = PlotSignals( d.GetAnodeSignal(), d.GetPadSignal(), "sector");
               csig->cd(3);
               hmatch->Draw();
               hmatch->GetXaxis()->SetRangeUser(0.,tmax);
               hmatch->GetYaxis()->SetRangeUser(0.,tmax);

               TH1D* hoccaw = PlotOccupancy( d.GetAnodeSignal(), "anodes" );
               hoccaw->Scale(1./hoccaw->Integral());
               hoccaw->SetLineColor(kRed);
               TH1D* hocccombpads = PlotOccupancy( m.GetCombinedPads(), "pads" );
               hocccombpads->Scale(1./hocccombpads->Integral());
               hocccombpads->SetLineColor(kBlue);
               csig->cd(4);
               hoccaw->Draw("hist");
               hocccombpads->Draw("histsame");
            }

         // match electrodes
         m.MatchElectrodes( d.GetAnodeSignal() );
         uint nmatch = m.GetSpacePoints()->size();
         cout<<"[main]# "<<i<<"\tMatchElectrodes: "<<nmatch<<endl;
         if( nmatch == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         // combine points
         m.CombPoints();
         uint nsp = m.GetSpacePoints()->size();
         cout<<"[main]# "<<i<<"\tCombinePoints: "<<nsp<<endl;
         if( nsp == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         r.Reset();

         // reco points
         if( verb ) r.SetTrace(true);
         r.AddSpacePoint( m.GetSpacePoints() );
         cout<<"[main]# "<<i<<"\tspacepoints: "<<r.GetNumberOfPoints()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<r.GetNumberOfPoints()<<"\t";

         // find tracks
         int ntracks = r.FindTracks(finder);
         cout<<"[main]# "<<i<<"\tpattrec: "<<ntracks<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         // if(finder == neural){
         //    TH1D *hw = new TH1D("hw","pattrec point weights",20,0,2.);
         //    vector<double> pw = ((NeuralFinder*)pattrec)->GetPointWeights();
         //    for(double w: pw) hw->Fill(w);
         //    new TCanvas;
         //    hw->Draw();

         //    TH1D *hinw = new TH1D("hinw","pattrec in neuron weights",200,0,2.);
         //    vector<double> inw = ((NeuralFinder*)pattrec)->GetInNeuronWeights();
         //    assert(inw.size());
         //    for(double w: inw) hinw->Fill(w);
         //    new TCanvas;
         //    hinw->Draw();

         //    TH1D *honw = new TH1D("honw","pattrec out neuron weights",200,0,2.);
         //    vector<double> onw = ((NeuralFinder*)pattrec)->GetOutNeuronWeights();
         //    for(double w: onw) honw->Fill(w);
         //    new TCanvas;
         //    honw->Draw();

         //    TH1D *hnv = new TH1D("hnv","pattrec neuron V",200,0,2.);
         //    vector<double> nv = ((NeuralFinder*)pattrec)->GetNeuronV();
         //    for(double v: nv) hnv->Fill(v);
         //    new TCanvas;
         //    hnv->Draw();
         // }

         cout<<"[main]# "<<i<<"\ttracks: "<<r.GetNumberOfTracks()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<r.GetNumberOfTracks()<<"\t";

         r.SetTrace( true );
         int nlin = r.FitLines();
         cout<<"[main]# "<<i<<"\tline: "<<nlin<<endl;
         int nhel = r.FitHelix();
         cout<<"[main]# "<<i<<"\thelix: "<<nhel<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         r.SetTrace( false );

         //fout<<fabs(EvaluateMatch_byResZ(r.GetLines()))<<"\t";//<<endl;
         //fout<<EvaluatePattRec(r.GetLines())<<"\t";

         tMC->GetEntry(i);
         TVector3* mcvtx = (TVector3*) vtx->ConstructedAt(0);
         mcvtx->Print();
         double res = PointResolution(r.GetHelices(),mcvtx);
         cout<<"[main]# "<<i<<"\tResolution: ";
         auto prec = cout.precision();
         cout.precision(2);
         cout<<res<<" mm"<<endl;
         cout.precision(prec);
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<res<<endl;

         tGarf->GetEntry(i);

         if( draw )
            {
               PlotMCpoints(creco,garfpp_hits);

               PlotAWhits( creco, aw_hits );

               PlotRecoPoints(creco,r.GetPoints());

               // if(finder == neural){
               //    for(int i = 0; i < ((NeuralFinder*)pattrec)->GetNumberOfTracks(); i++)
               //       PlotNeurons(creco, ((NeuralFinder*)pattrec)->GetTrackNeurons(i), kGray+1);

               //    PlotNeurons(creco, ((NeuralFinder*)pattrec)->GetMetaNeurons(), kRed);
               //    // PlotNeurons(creco, pattrec->GetTrackNeurons(1), kMagenta);
               //    // PlotNeurons(creco, pattrec->GetTrackNeurons(2), kCyan);
               //    // PlotNeurons(creco, pattrec->GetTrackNeurons(3), kOrange);
               //    // PlotNeurons(creco, pattrec->GetTrackNeurons(4), kViolet);
               // }

               PlotTracksFound(creco,r.GetTracks());

               DrawTPCxy(creco);
            }

         if( enableMC )
            {
               //================================================================
               // MC hits reco
               cout<<"[main]# "<<i<<"\tMC reco"<<endl;
               rMC.Reset();
               rMC.AddMChits( aw_hits );
               cout<<"[main]# "<<i<<"\tMC spacepoints: "<<rMC.GetNumberOfPoints()<<endl;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

               // find tracks
               int ntracksMC = rMC.FindTracks(finder);
               cout<<"[main]# "<<i<<"\tMCpattrec: "<<ntracksMC<<endl;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

               // if(finder == neural){
               //    TH1D *hwMC = new TH1D("hwMC","MCpattrec point weights",20,0,2.);
               //    vector<double> pwMC = ((NeuralFinder*)MCpattrec)->GetPointWeights();
               //    for(double w: pwMC) hwMC->Fill(w);
               //    new TCanvas;
               //    hwMC->Draw();
               //    // MCpattrec->SetPointsDistCut(rMC.GetPointsDistCut());
               // }
               
               cout<<"[main]# "<<i<<"\tMC tracks: "<<rMC.GetNumberOfTracks()<<endl;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

               rMC.SetTrace( true );
               nlin = rMC.FitLines();
               cout<<"[main]# "<<i<<"\tline: "<<nlin<<endl;
               nhel = rMC.FitHelix();
               cout<<"[main]# "<<i<<"\tMC helix: "<<nhel<<endl;
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               rMC.SetTrace( false );

               res = PointResolution(rMC.GetHelices(),mcvtx);
               cout<<"[main]# "<<i<<"\tMC Resolution: ";
               prec = cout.precision();
               cout.precision(2);
               cout<<res<<" mm"<<endl;
               cout.precision(prec);
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            }
         
      }// events loop
   //fout.close();

   if( draw ){
      // new TBrowser;
      app->Run();
   }

   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
