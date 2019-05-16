#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TBrowser.h"

#include "TH1D.h"
#include "TH2D.h"

#include "argparse.hh"

#include "Deconv.hh"
#include "Match.hh"
#include "Reco.hh"

#include "Utils.hh"

#include "TFitVertex.hh"

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
   tGarf->SetBranchAddress("GarfHits",&garfpp_hits);
   TClonesArray* aw_hits = new TClonesArray("TMChit");
   tGarf->SetBranchAddress("AnodeHits",&aw_hits);

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
   
   Deconv d(settings);
   // ofstream fout("deconv_goodness.dat", ios::out | ios::app);
   // fout<<d.GetADCthres()<<"\t"<<d.GetPWBthres()<<"\t"
   // <<d.GetAWthres()<<"\t"<<d.GetPADthres()<<"\t";
   cout<<"--------------------------------------------------"<<endl;
   cout<<"[main]# Deconv Settings"<<endl;
   cout<<"        ADC delay: "<<d.GetADCdelay()<<"\tPWB delay: "<<d.GetPWBdelay()<<endl;
   cout<<"        ADC thresh: "<<d.GetADCthres()<<"\tPWB thresh: "<<d.GetPWBthres()<<endl;
   cout<<"        AW thresh: "<<d.GetAWthres()<<"\tPAD thresh: "<<d.GetPADthres()<<endl;
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
   
   Match m(settings);
   //ofstream fout("match_goodness.dat", ios::out | ios::app);
   //ofstream fout("pattrec_goodness.dat", ios::out | ios::app);

   double B=1.0;
   if( parser.count("Bfield") )
      {
         string Bfield = parser.retrieve<string>("Bfield");
         B = stod(Bfield);
      }
   cout<<"[main]# Magnetic Field: "<<B<<" T"<<endl;

   Reco r(settings,B);

   Reco rMC(settings,B);

   bool draw = false;
   if( parser.count("draw") )
      {
         draw = true;
         cout<<"[main]# Drawing Enabled"<<endl;
      }
   double tmax = 4500.;
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

   TApplication* app=0;
   if( draw )
      app = new TApplication("g4ana",&argc,argv);

   TCanvas* csig=0;
   TCanvas* creco=0;

   if( draw )
      {
         csig = new TCanvas("csig","csig",1400,1400);
         csig->Divide(2,2);

         creco = new TCanvas("creco","creco",1400,1400);
         creco->Divide(2,2);
      }

   for( int i=0; i<Nevents; ++i )
      {
         tSig->GetEntry(i);

         // anode deconv
         int nsig = d.FindAnodeTimes( AWsignals );
         cout<<"[main]# "<<i<<"\tFindAnodeTimes: "<<nsig<<endl;
         if( nsig == 0 ) return 1;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //      fout<<std::setprecision(15)<<Average( d.GetAnodeDeconvRemainder() )<<"\t";

         if( verb ) PrintSignals( d.GetAnodeSignal() );
         TH1D* haw=0;
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

         if(finder == neural)
            {
               const TracksFinder* pattrec = r.GetTracksFinder();

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

         cout<<"[main]# "<<i<<"\ttracks: "<<r.GetNumberOfTracks()<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

         //fout<<r.GetNumberOfTracks()<<"\t";

         //r.SetTrace( true );
         int nlin = r.FitLines();
         cout<<"[main]# "<<i<<"\tline: "<<nlin<<endl;
         int nhel = r.FitHelix();
         cout<<"[main]# "<<i<<"\thelix: "<<nhel<<endl;
         // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
         //r.SetTrace( false );

         //fout<<fabs(EvaluateMatch_byResZ(r.GetLines()))<<"\t";//<<endl;
         //fout<<EvaluatePattRec(r.GetLines())<<"\t";

         TFitVertex Vertex(i);
         int sv = r.RecVertex(&Vertex);
         cout<<"[main]# "<<i<<"\t";
         if( sv > 0 ) Vertex.Print();
         else cout<<"No Vertex";
         cout<<"\n";

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

               if(finder == neural)
                  {
                     const TracksFinder* pattrec = r.GetTracksFinder();
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
                  {
                     const TracksFinder* MCpattrec = rMC.GetTracksFinder();
                     TH1D *hwMC = new TH1D("hwMC","MCpattrec point weights",20,0,2.);
                     vector<double> pwMC = ((NeuralFinder*)MCpattrec)->GetPointWeights();
                     for(double w: pwMC) hwMC->Fill(w);
                     new TCanvas;
                     hwMC->Draw();
                     // MCpattrec->SetPointsDistCut(rMC.GetPointsDistCut());
                  }
               
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

               res = PointResolution(rMC.GetHelices(),mcvtx);
               cout<<"[main]# "<<i<<"\tMC Resolution: ";
               prec = cout.precision();
               cout.precision(2);
               cout<<res<<" mm"<<endl;
               cout.precision(prec);
               // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

               rMC.Reset();
            }
         
         r.Reset();

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
