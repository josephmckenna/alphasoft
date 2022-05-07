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
#include "TStoreEvent.hh"

#include "TStoreEvent.hh"

class Utils {
private:
   Histo          fHisto;
   ALPHAg::padmap pmap;
   double         fMagneticField;

   int Npoints       = 0;
   int Npointstracks = 0;

   double tmax;

public:
   Utils(double);
   Utils(std::string, double);

   TCanvas *csig  = 0;
   TCanvas *creco = 0;
   void     MakeCanvases();

   void BookG4Histos();
   void BookRecoHistos();
   void BookAGG4Histos();

   void FillRecoPointsHistos(const std::vector<TSpacePoint> *points);
   void FillRecoTracksHisto(const std::vector<TTrack> *found_tracks);
   void FillFitTracksHisto(const std::vector<TFitHelix> *tracks_array);
   void FillFitTracksHisto(const std::vector<TFitLine> *tracks_array);

   void FillRecoVertex(const TFitVertex *Vertex);

   void FillFinalHistos(const TStoreEvent *r, int ntracks);
   void FillFinalHistos(int total_points, int total_tracks, int good_tracks);

   void DebugNeuralNet(NeuralFinder *);
   void DebugNeuralNetMC(NeuralFinder *);
   void DisplayNeuralNet(NeuralFinder *);
   void PlotNeurons(TCanvas *c, const std::set<NeuralFinder::Neuron *> &neurons, int col = kBlack);

   void Display(const TClonesArray *mcpoints, const TClonesArray *awpoints,
                const std::vector<TSpacePoint> *recopoints, const std::vector<TTrack> *tracks,
                const std::vector<TFitHelix> *helices);
   void Display(const std::vector<TSpacePoint> *recopoints, const std::vector<TTrack> *tracks,
                const std::vector<TFitHelix> *helices);
   void PlotMCpoints(TCanvas *c, const TClonesArray *points);
   void PlotAWhits(TCanvas *c, const TClonesArray *points);
   void PlotRecoPoints(TCanvas *c, const std::vector<TSpacePoint> *points, bool autoscale = false);
   void PlotTracksFound(TCanvas *c, const std::vector<TTrack> *tracks);
   void PlotFitHelices(TCanvas *c, const std::vector<TFitHelix> *tracks);
   void DrawTPCxy(TCanvas *c);

   void Draw(std::vector<ALPHAg::TWireSignal> *awsig, std::vector<ALPHAg::TPadSignal> *padsig,
             std::vector<ALPHAg::TPadSignal> *combpads, bool norm = true);
   void Draw(std::vector<ALPHAg::TWireSignal> *awsig, std::vector<ALPHAg::TPadSignal> *padsig, bool norm = true);

   template <class T>
   void PrintSignals(std::vector<T> *sig)
{
   for(auto s: *sig)
      static_cast<ALPHAg::signal>(s).print();
}

private: // If these are to be made public, their implementation needs to move into the header
   template <class T>
   TH1D *PlotSignals(std::vector<T> *sig, std::string name);

   template <class T>
   TH1D *PlotOccupancy(std::vector<T> *sig, std::string name);

public:
   TH2D *PlotSignals(std::vector<ALPHAg::TWireSignal> *awsignals, std::vector<ALPHAg::TPadSignal> *padsignals,
                     std::string type = "none");

   double Average(std::vector<double> *v);

   double EvaluateMatch_byResZ(TClonesArray *lines);
   int    EvaluatePattRec(TClonesArray *lines);
   double PointResolution(std::vector<TFitHelix> *helices, const TVector3 *vtx);

   void   HelixPlots(std::vector<TFitHelix> *helices);
   void   UsedHelixPlots(const std::vector<TFitHelix*> *helices);
   void   UsedHelixPlots(const TObjArray *helices);
   double VertexResolution(const TVector3 *vtx, const TVector3 *mcvtx);
   void   VertexPlots(const TFitVertex *v);

   void WriteSettings(TObjString *);
   void WriteHisto();

   TStoreEvent CreateStoreEvent(const std::vector<TSpacePoint> *points, const std::vector<TFitHelix> *hels = nullptr,
                                const std::vector<TFitLine> *lines = nullptr);

   inline void SetTmax(double t) { tmax = t; }
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
