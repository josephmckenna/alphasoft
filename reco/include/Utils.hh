#ifndef __UTILS__
#define __UTILS__ 1

#include <vector>
#include "SignalsType.hh"
#include "NeuralFinder.hh"

#include "Reco.hh"
#include "Histo.hh"
#include <TCanvas.h>
#include <TClonesArray.h>

#include "TTrack.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

class Utils
{
private:
   Histo fHisto;
   padmap pmap;
   double fMagneticField;

   int Npoints=0;
   int Npointstracks=0;

   double tmax;

public:
   Utils(double);
   Utils(std::string, double, bool);
   Utils(std::string,double);

   TCanvas* csig=0;
   TCanvas* creco=0;

   void BookG4Histos();
   void BookRecoHistos();
   void BookAGG4Histos();
   void FillRecoPointsHistos(const TObjArray* points);
   void FillRecoTracksHisto(std::vector<TTrack*>* found_tracks);
   void FillFitTracksHisto(std::vector<TTrack*>* tracks_array);
   void FillRecoVertex(const TFitVertex* Vertex);
   void FillFinalHistos(const Reco* r, int ntracks);

   void DebugNeuralNet(NeuralFinder*);
   void DebugNeuralNetMC(NeuralFinder*);
   void DisplayNeuralNet(NeuralFinder*);
   void PlotNeurons(TCanvas* c, const std::set<NeuralFinder::Neuron*> &neurons, int col = kBlack);

   void Display(const TClonesArray* mcpoints, const TClonesArray* awpoints,
                const std::vector<TSpacePoint*>* recopoints,
                const std::vector<TTrack*>* tracks,
                const std::vector<TFitHelix*>* helices);
   void Display(const std::vector<TSpacePoint*>* recopoints,
                const std::vector<TTrack*>* tracks,
                const std::vector<TFitHelix*>* helices);
   void PlotMCpoints(TCanvas* c, const TClonesArray* points);
   void PlotAWhits(TCanvas* c, const TClonesArray* points);
   void PlotRecoPoints(TCanvas* c, const std::vector<TSpacePoint*>* points,
                       bool autoscale=false);
   void PlotTracksFound(TCanvas* c, const std::vector<TTrack*>* tracks);
   void PlotFitHelices(TCanvas* c, const std::vector<TFitHelix*>* tracks);
   void DrawTPCxy(TCanvas* c);
   

   void Draw(std::vector<signal>* awsig,
             std::vector<signal>* padsig, std::vector<signal>* combpads,
             bool norm=true);
   void PrintSignals(std::vector<signal>* sig);
   TH1D* PlotSignals(std::vector<signal>* sig, std::string name);
   TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name);
   TH2D* PlotSignals(std::vector<signal>* awsignals,
                     std::vector<signal>* padsignals, std::string type="none");
   
   double Average(std::vector<double>* v);
   
   double EvaluateMatch_byResZ(TClonesArray* lines);
   int EvaluatePattRec(TClonesArray* lines);
   double PointResolution(std::vector<TFitHelix*>* helices, const TVector3* vtx);
   
   void HelixPlots(std::vector<TFitHelix*>* helices);
   void UsedHelixPlots(const std::vector<TFitHelix*>* helices);
   void UsedHelixPlots(const TObjArray* helices);
   double VertexResolution(const TVector3* vtx, const TVector3* mcvtx);
   void VertexPlots(const TFitVertex* v);

   void WriteSettings(TObjString*);

   inline void SetTmax(double t) {tmax=t;}
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
