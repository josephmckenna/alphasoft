#include "ROOT/RDataFrame.hxx"
using namespace ROOT::Experimental;

void    open_root(int);
void    get_parameters();
void    primary_muon_pos();
void    primary_pbar_pos();
void    primary_mom();

TTree *tree = 0;
ROOT::RDataFrame *d;
Double_t x_min, y_min, z_min;
Double_t x_max, y_max, z_max;
Bool_t muons = false;
Bool_t pbars = false;

///< main 
void show_gen(int runNumber) {
    std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
    gStyle->SetOptStat(stat_opt.c_str());
    open_root(runNumber);
    get_parameters();
    if(muons) primary_muon_pos();
    if(pbars) primary_pbar_pos();
    primary_mom();
    return;
}

///< REMEMBER ##########
///< in TH2D, when "Drawing", the first variable is the Y and the second is the X 
///< e.g. tree->Draw("Y:X") and NOT viceversa
///< functions
void primary_muon_pos() {
    //##########################################################
    // Preparing the histos
    TCanvas *cpos = new TCanvas("cpos","cpos",1200,1200);
    cpos->Divide(2,2);
    cpos->cd(1);
        TH2D *h_genXZ = new TH2D("h_genXZ","Primary X vs Z", 100, x_min, x_max, 100, z_min, z_max);
        tree->Draw("PrimaryVertex.fVz:PrimaryVertex.fVx>>h_genXZ");
        h_genXZ->Draw("COLZ");
    cpos->cd(2);
        TH1D *h_genY = new TH1D("h_genY", "Primary Y", 100, y_min, y_max);
        tree->Draw("PrimaryVertex.fVy>>h_genY");
        h_genY->Draw("");
    cpos->cd(3);
        TH2D *h_genRvsY = new TH2D("h_genRvsY","Primary Y vs (X^2+Y^2)", 100, 0., x_max, 100, y_min, x_max);
        tree->Draw("PrimaryVertex.fVy:sqrt(PrimaryVertex.fVx*PrimaryVertex.fVx+PrimaryVertex.fVz*PrimaryVertex.fVz)>>h_genRvsY");
        h_genRvsY->Draw("COLZ");
}

void primary_pbar_pos() {
    //##########################################################
    // Preparing the histos
    TCanvas *cpos = new TCanvas("cpos","cpos",1200,1200);
    cpos->Divide(2,2);
    cpos->cd(1);
        TH2D *h_genXY = new TH2D("h_genXY","Primary X vs Y", 100, x_min, x_max, 100, y_min, y_max);
        tree->Draw("PrimaryVertex.fVy:PrimaryVertex.fVx>>h_genXY");
        h_genXY->Draw("COLZ");
    cpos->cd(2);
        TH1D *h_genZ = new TH1D("h_genZ", "Primary Z", 100, z_min, z_max);
        tree->Draw("PrimaryVertex.fVz>>h_genZ");
        h_genZ->Draw("");
    cpos->cd(3);
        TH2D *h_genRvsZ = new TH2D("h_genRvsZ","Primary R vs Z", 100, z_min, z_max, 100, 0., x_max);
        tree->Draw("sqrt(PrimaryVertex.fVx*PrimaryVertex.fVx+PrimaryVertex.fVy*PrimaryVertex.fVy):PrimaryVertex.fVz>>h_genRvsZ");
        h_genRvsZ->Draw("COLZ");
}

void primary_mom() {
    TCanvas *cmom = new TCanvas("cmom","cmom",1200,1600);
    cmom->Divide(2,2);
    cmom->cd(1);
        TH1D *hp = new TH1D("hp","Momentum",100, -0.5, 20.5);
        tree->Draw("sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz)>>hp");
    cmom->cd(2);
        TH1D *hthe = new TH1D("hthe","#theta",91, -0.5, 90.5); hthe->SetXTitle("deg");
        tree->Draw("acos(fabs(PrimaryVertex.fPz)/sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz))*TMath::RadToDeg()>>hthe", 
                "sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz)!=0.");
    cmom->cd(3);
        TH1D *hcos = new TH1D("hcos","cos(#theta)",101, -0.05, 1.05); hcos->SetXTitle("cos(#theta)");
        tree->Draw("fabs(PrimaryVertex.fPz)/sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz)>>hcos", 
                "sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz)!=0.");
    cmom->cd(4);
        TH1D *hcos2 = new TH1D("hcos2","cos^2(#theta)",101, -0.05, 1.05); hcos2->SetXTitle("cos^2(#theta)");
        tree->Draw("pow(fabs(PrimaryVertex.fPz)/sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz),2.)>>hcos2", 
                "sqrt(PrimaryVertex.fPx*PrimaryVertex.fPx+PrimaryVertex.fPy*PrimaryVertex.fPy+PrimaryVertex.fPz*PrimaryVertex.fPz)!=0.");
    cmom->Modified(); cmom->Update();    
}

void get_parameters() {
    ///< Checking which primary particle (pbar, muon, etc. ?)
    auto pdg = d->Max("PrimaryVertex.fPdgCode");
    if((int)*pdg==-2212) pbars = true;
    if((int)*pdg==-13||(int)*pdg==+13) muons = true;
    ///< Calculating the limits on Vx, Vy, Vz
    auto max = d->Max("PrimaryVertex.fVx");
    auto min = d->Min("PrimaryVertex.fVx");
    x_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    x_min = -x_max; ///< Symmetric distributions in x

    max = d->Max("PrimaryVertex.fVy");
    min = d->Min("PrimaryVertex.fVy");
    y_max = *max;
    y_min = *min;

    max = d->Max("PrimaryVertex.fVz");
    min = d->Min("PrimaryVertex.fVz");
    z_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    z_min = -z_max; ///< Symmetric distributions in z
}

void open_root(int runNumber) {
    //##########################################################
    // opening the MC file and loading the a2MC TTree -> tree
    std::ostringstream sdata;
    sdata << "ls ../root/a2MC-*_" << runNumber << ".root";
    TString file_name(gSystem->GetFromPipe(sdata.str().c_str()));
    string sfile = file_name.Data();
    if(strcmp(sfile.c_str(),"")==0) {
        cout << "Please check presence of file " << sfile << endl;
        return;
    }

    TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(sfile.c_str());
    if (!f || !f->IsOpen()) {
        f = new TFile(sfile.c_str());
    }
    f->GetObject("a2MC",tree);
    ///< New RDataFrame
    d = new ROOT::RDataFrame("a2MC",sfile.c_str());
}
