void PlotNeurons(TCanvas* c, const set<NeuralFinder::Neuron*> &neurons, int col = kBlack);
void PlotMCpoints(TCanvas* c, const TClonesArray* points);
void PlotAWhits(TCanvas* c, const TClonesArray* points);
void PlotRecoPoints(TCanvas* c, const TClonesArray* points);
void PlotTracksFound(TCanvas* c, const TClonesArray* tracks);
void DrawTPCxy(TCanvas* c);

void PrintSignals(std::vector<signal>* sig);
TH1D* PlotSignals(std::vector<signal>* sig, std::string name);
TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name);
TH2D* PlotSignals(std::vector<signal>* awsignals,
		  std::vector<signal>* padsignals, std::string type="none");

double Average(std::vector<double>* v);

double EvaluateMatch_byResZ(TClonesArray* lines);
int EvaluatePattRec(TClonesArray* lines);
double PointResolution(TClonesArray* helices, const TVector3* vtx);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
