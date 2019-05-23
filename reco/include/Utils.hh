#include "NeuralFinder.hh"

#include "Histo.hh"

void PlotNeurons(TCanvas* c, const std::set<NeuralFinder::Neuron*> &neurons, int col = kBlack);
void PlotMCpoints(TCanvas* c, const TClonesArray* points);
void PlotAWhits(TCanvas* c, const TClonesArray* points);
void PlotRecoPoints(TCanvas* c, const std::vector<TSpacePoint*>* points);
void PlotTracksFound(TCanvas* c, const std::vector<TTrack*>* tracks);
void DrawTPCxy(TCanvas* c);

void PrintSignals(std::vector<signal>* sig);
TH1D* PlotSignals(std::vector<signal>* sig, std::string name);
TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name);
TH2D* PlotSignals(std::vector<signal>* awsignals,
		  std::vector<signal>* padsignals, std::string type="none");

double Average(std::vector<double>* v);

double EvaluateMatch_byResZ(TClonesArray* lines);
int EvaluatePattRec(TClonesArray* lines);
double PointResolution(std::vector<TFitHelix*>* helices, const TVector3* vtx);

void HelixPlots(Histo* h, TClonesArray* helices);
void UsedHelixPlots(Histo* h, const TObjArray* helices);
double VertexResolution(const TVector3* vtx, const TVector3* mcvtx);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
