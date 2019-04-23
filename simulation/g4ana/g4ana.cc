#include <iostream>
#include <string>
#include <sstream>

#include <chrono>  // for high_resolution_clock

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TBrowser.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TBrowser.h"

#include "Deconv.hh"
#include "Match.hh"
#include "Reco.hh"

#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"

#include "Utils.hh"

using namespace std;

enum finderChoice { base, adaptive, neural };

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

   TFile* outfile = TFile::Open("output.root","RECREATE");
   if( !outfile->IsOpen() )
      {
         cerr<<"output file not open"<<endl;
         return 1;
      }
   outfile->cd();

   std::chrono::duration<double, std::milli> t_d_aw(0.), t_d_p(0.), t_comb_p(0.), t_match(0.);

   //double ADCThres=1000., PWBThres=1000., ADCpeak=5000., PWBpeak=5000.;
   double ADCThres=atof(argv[2]), PWBThres=atof(argv[3]),
      ADCpeak=atof(argv[4]), PWBpeak=atof(argv[5]);
   //  double ADCThres=10., PWBThres=10., ADCpeak=5., PWBpeak=10.;
   //double ADCThres=1000., PWBThres=100., ADCpeak=10., PWBpeak=10.;
   //double ADCThres=1., PWBThres=1000., ADCpeak=1., PWBpeak=1000.;
   Deconv d(ADCThres, PWBThres, ADCpeak, PWBpeak);

   finderChoice finder = adaptive;
   if(argc > 6){
      switch(*argv[6]){
      case 'b':
      case 'B': finder = base; cout << "Using basic TracksFinder (untested)" << endl; break;
      case 'a':
      case 'A': finder = adaptive; cout << "Using AdaptiveFinder" << endl; break;
      case 'n':
      case 'N': finder = neural; cout << "Using NeuralFinder" << endl; break;
      default: cerr << "Finder selection " << *argv[6] << " unknown, using basic finder";
      }
   }

   // ofstream fout("deconv_goodness.dat", ios::out | ios::app);
   // fout<<ADCThres<<"\t"<<PWBThres<<"\t"<<ADCpeak<<"\t"<<PWBpeak<<"\t";

   string json_file = "sim.json";
   ostringstream json_filepath;
   json_filepath<<getenv("AGRELEASE")<<"/ana/"<<json_file;
   cout<<"[main]# Loading Ana settings from: "<<json_filepath.str()<<endl;


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
         auto start = std::chrono::high_resolution_clock::now();
         int nsig = d.FindAnodeTimes( AWsignals );
         auto end = std::chrono::high_resolution_clock::now();
         t_d_aw += end-start;
         cout<<"[main]# "<<i<<"\tFindAnodeTimes: "<<nsig<<endl;
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
         start = std::chrono::high_resolution_clock::now();
         nsig = d.FindPadTimes( PADsignals );
         end = std::chrono::high_resolution_clock::now();
         t_d_p += end-start;

         cout<<"[main]# "<<i<<"\tFindPadTimes: "<<nsig<<endl;
         if( verb ) PrintSignals( d.GetPadSignal() );
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //      fout<<std::setprecision(15)<<Average( d.GetPadDeconvRemainder() )<<endl;

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
         start = std::chrono::high_resolution_clock::now();
         m.CombinePads( d.GetPadSignal() );
         end = std::chrono::high_resolution_clock::now();
         t_comb_p += end-start;
         cout<<"[main]# "<<i<<"\tCombinePads: "<<m.GetCombinedPads()->size()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         if( verb ) PrintSignals( m.GetCombinedPads() );
         if( draw )
            {
               TH1D* hcombpads = PlotSignals( m.GetCombinedPads(), "combinedpads" );
               hcombpads->Scale(haw->GetMaximum()/hcombpads->GetMaximum());
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
         start = std::chrono::high_resolution_clock::now();
         m.MatchElectrodes( d.GetAnodeSignal() );
         end = std::chrono::high_resolution_clock::now();
         t_match += end-start;
         cout<<"[main]# "<<i<<"\tMatchElectrodes: "<<m.GetSpacePoints()->size()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         r.Reset();

         // reco points
         if( verb ) r.SetTrace(true);
         r.AddSpacePoint( m.GetSpacePoints() );
         cout<<"[main]# "<<i<<"\tspacepoints: "<<r.GetNumberOfPoints()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<r.GetNumberOfPoints()<<"\t";

         // find tracks
         TClonesArray* sp = r.GetPoints();

         TracksFinder *pattrec;

         switch(finder){
         case base:
            pattrec = new TracksFinder(sp);
            break;
         case adaptive:
            pattrec = new AdaptiveFinder(sp);
            ((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(r.GetMaxIncreseAdapt());
            break;
         case neural:
            pattrec = new NeuralFinder(sp);
            ((NeuralFinder*)pattrec)->SetLambda(r.GetLambda());
            ((NeuralFinder*)pattrec)->SetAlpha(r.GetAlpha());
            ((NeuralFinder*)pattrec)->SetB(r.GetB());
            ((NeuralFinder*)pattrec)->SetTemp(r.GetTemp());
            ((NeuralFinder*)pattrec)->SetC(r.GetC());
            ((NeuralFinder*)pattrec)->SetMu(r.GetMu());
            ((NeuralFinder*)pattrec)->SetCosCut(r.GetCosCut());
            ((NeuralFinder*)pattrec)->SetVThres(r.GetVThres());
            ((NeuralFinder*)pattrec)->SetDNormXY(r.GetDNormXY());
            ((NeuralFinder*)pattrec)->SetDNormZ(r.GetDNormZ());
            ((NeuralFinder*)pattrec)->SetTscale(r.GetTscale());
            ((NeuralFinder*)pattrec)->SetMaxIt(r.GetMaxIt());
            ((NeuralFinder*)pattrec)->SetItThres(r.GetItThres());
            break;
         }

         pattrec->SetPointsDistCut(r.GetPointsDistCut());
         pattrec->SetNpointsCut(r.GetNspacepointsCut());
         pattrec->SetSeedRadCut(r.GetSeedRadCut());

         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         pattrec->RecTracks();
         cout<<"[main]# "<<i<<"\tpattrec: "<<pattrec->GetNumberOfTracks()<<endl;

         if(finder == neural){
            TH1D *hw = new TH1D("hw","pattrec point weights",20,0,2.);
            vector<double> pw = ((NeuralFinder*)pattrec)->GetPointWeights();
            for(double w: pw) hw->Fill(w);
            new TCanvas;
            hw->Draw();

            TH1D *hinw = new TH1D("hinw","pattrec in neuron weights",200,0,2.);
            vector<double> inw = ((NeuralFinder*)pattrec)->GetInNeuronWeights();
            assert(inw.size());
            for(double w: inw) hinw->Fill(w);
            new TCanvas;
            hinw->Draw();

            TH1D *honw = new TH1D("honw","pattrec out neuron weights",200,0,2.);
            vector<double> onw = ((NeuralFinder*)pattrec)->GetOutNeuronWeights();
            for(double w: onw) honw->Fill(w);
            new TCanvas;
            honw->Draw();

            TH1D *hnv = new TH1D("hnv","pattrec neuron V",200,0,2.);
            vector<double> nv = ((NeuralFinder*)pattrec)->GetNeuronV();
            for(double v: nv) hnv->Fill(v);
            new TCanvas;
            hnv->Draw();
         }

         r.AddTracks( pattrec->GetTrackVector() );
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

               PlotRecoPoints(creco,sp);

               if(finder == neural){
                  for(int i = 0; i < ((NeuralFinder*)pattrec)->GetNumberOfTracks(); i++)
                     PlotNeurons(creco, ((NeuralFinder*)pattrec)->GetTrackNeurons(i), kGray+1);

                  PlotNeurons(creco, ((NeuralFinder*)pattrec)->GetMetaNeurons(), kRed);
                  // PlotNeurons(creco, pattrec->GetTrackNeurons(1), kMagenta);
                  // PlotNeurons(creco, pattrec->GetTrackNeurons(2), kCyan);
                  // PlotNeurons(creco, pattrec->GetTrackNeurons(3), kOrange);
                  // PlotNeurons(creco, pattrec->GetTrackNeurons(4), kViolet);
               }

               PlotTracksFound(creco,r.GetTracks());

               DrawTPCxy(creco);

               TCanvas *cawt = new TCanvas("cawt","AW times",1400,1400);
               PlotAWtimes( cawt, d.GetAnodeSignal(), aw_hits );

               TCanvas *caw = new TCanvas("caw","caw",1400,1400);
               caw->Divide(2,2);
               PlotMCpoints(caw,garfpp_hits);
               PlotAWhits( caw, aw_hits );
               Match mTmp(json_filepath.str());
               mTmp.Init();
               mTmp.FakePads(d.GetAnodeSignal());
               Reco rTmp(json_filepath.str(),B);
               rTmp.Reset();
               rTmp.AddSpacePoint( mTmp.GetSpacePoints() );
               PlotRecoPoints(caw,rTmp.GetPoints());
            }

         //================================================================
         // MC hits reco
         cout<<"[main]# "<<i<<"\tMC reco"<<endl;
         rMC.Reset();
         rMC.AddMChits( aw_hits );
         cout<<"[main]# "<<i<<"\tMC spacepoints: "<<rMC.GetNumberOfPoints()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         // find tracks
         TClonesArray* mcsp = rMC.GetPoints();

         TracksFinder *MCpattrec;

         switch(finder){
         case base:
            MCpattrec = new TracksFinder(mcsp);
            break;
         case adaptive:
            MCpattrec = new AdaptiveFinder(mcsp);
            ((AdaptiveFinder*)MCpattrec)->SetMaxIncreseAdapt(r.GetMaxIncreseAdapt());
            break;
         case neural:
            MCpattrec = new NeuralFinder(mcsp);
            ((NeuralFinder*)MCpattrec)->SetLambda(r.GetLambda());
            ((NeuralFinder*)MCpattrec)->SetAlpha(r.GetAlpha());
            ((NeuralFinder*)MCpattrec)->SetB(r.GetB());
            ((NeuralFinder*)MCpattrec)->SetTemp(r.GetTemp());
            ((NeuralFinder*)MCpattrec)->SetC(r.GetC());
            ((NeuralFinder*)MCpattrec)->SetMu(r.GetMu());
            ((NeuralFinder*)MCpattrec)->SetCosCut(r.GetCosCut());
            ((NeuralFinder*)MCpattrec)->SetVThres(r.GetVThres());
            ((NeuralFinder*)MCpattrec)->SetDNormXY(r.GetDNormXY());
            ((NeuralFinder*)MCpattrec)->SetDNormZ(r.GetDNormZ());
            ((NeuralFinder*)MCpattrec)->SetTscale(r.GetTscale());
            ((NeuralFinder*)MCpattrec)->SetMaxIt(r.GetMaxIt());
            ((NeuralFinder*)MCpattrec)->SetItThres(r.GetItThres());
            break;
         }

         MCpattrec->SetPointsDistCut(0.1*r.GetPointsDistCut());
         MCpattrec->SetNpointsCut(r.GetNspacepointsCut());
         MCpattrec->SetSeedRadCut(r.GetSeedRadCut());

         // AdaptiveFinder MCpattrec( mcsp );
         // MCpattrec.SetPointsDistCut(rMC.GetPointsDistCut());
         // MCpattrec.SetMaxIncreseAdapt(rMC.GetMaxIncreseAdapt());
         // MCpattrec.SetNpointsCut(rMC.GetNspacepointsCut());
         // MCpattrec.SetSeedRadCut(rMC.GetSeedRadCut());

         if(finder == neural){
            TH1D *hwMC = new TH1D("hwMC","MCpattrec point weights",20,0,2.);
            vector<double> pwMC = ((NeuralFinder*)MCpattrec)->GetPointWeights();
            for(double w: pwMC) hwMC->Fill(w);
            new TCanvas;
            hwMC->Draw();
            // MCpattrec->SetPointsDistCut(rMC.GetPointsDistCut());
         }

         MCpattrec->RecTracks();
         cout<<"[main]# "<<i<<"\tMC pattrec: "<<MCpattrec->GetNumberOfTracks()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         rMC.AddTracks( MCpattrec->GetTrackVector() );
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

         delete pattrec;
         delete MCpattrec;
      }// events loop
   //fout.close();

   std::cout << "Milliseconds spent on:" << std::endl;
   std::cout << "AW deconv:\t" << t_d_aw.count() << std::endl;
   std::cout << "Pad deconv:\t" << t_d_p.count() << std::endl;
   std::cout << "Pad comb:\t" << t_comb_p.count() << std::endl;
   std::cout << "Matching:\t" << t_match.count() << std::endl;

   outfile->Write();
   if( draw ){
      new TBrowser;
      app->Run();
   }
   outfile->Close();
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
