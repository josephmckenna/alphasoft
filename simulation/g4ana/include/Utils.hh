void PlotMCpoints(TCanvas* c, const TClonesArray* points);
void PlotRecoPoints(TCanvas* c, const TClonesArray* points);
void DrawTPCxy(TCanvas* c);

void PrintSignals(std::vector<signal>* sig);
TH1D* PlotSignals(std::vector<signal>* sig, std::string name);
TH1D* PlotOccupancy(std::vector<signal>* sig, std::string name);
TH2D* PlotSignals(std::vector<signal>* awsignals, 
		  std::vector<signal>* padsignals, std::string type="none");
