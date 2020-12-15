#include "ROOT/RDataFrame.hxx"
using namespace ROOT::Experimental;

void    open_root(int);
void    get_parameters();
void    sil_geo();
void    sil_ene();
TH1D*   hDecayAnn();

TTree *tree = 0;
ROOT::RDataFrame *d;
Bool_t muons = false;
Bool_t pbars = false;


///< main 
void show_sil(int runNumber) {
    std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
    gStyle->SetOptStat(stat_opt.c_str());
    open_root(runNumber);
    get_parameters();
    sil_geo();
    sil_ene();
    return;
}

///< REMEMBER ##########
///< in TH2D, when "Drawing", the first variable is the Y and the second is the X 
///< e.g. tree->Draw("Y:X") and NOT viceversa
///< functions
void sil_geo() {
    //##########################################################
    // Preparing the histos
    TCanvas *c_sil_geo = new TCanvas("c_sil_geo","c_sil_geo",1500,1200);
    c_sil_geo->Divide(3,2);
    c_sil_geo->cd(1);
        ostringstream s;
        if(muons) s << "Muons RUN - Layer multiplicity";
        if(pbars) s << "Pbars RUN - Layer multiplicity";
        TH1D *h_lay = new TH1D("h_lay",s.str().c_str(), 6, -0.5, 5.5);
        tree->Draw("SilHits.fLayN>>h_lay");
        h_lay->SetMinimum(0);
        h_lay->Draw("");
    c_sil_geo->cd(2);
        TH2D *h_lay_mod = new TH2D("h_lay_mod","Module vs Layer", 15, -0.5, 14.5, 6, -0.5, 5.5);
        tree->Draw("SilHits.fLayN:SilHits.fModN>>h_lay_mod");
        h_lay_mod->Draw("COLZ");
    c_sil_geo->cd(3);
        TH2D *h_hit_XY = new TH2D("h_hit_XY","HIT Y vs X", 500, -15.0, 15.0, 500, -15., 15.);
        tree->Draw("SilHits.fPosY:SilHits.fPosX>>h_hit_XY");
        h_hit_XY->SetXTitle("cm"); h_hit_XY->SetYTitle("cm");
        h_hit_XY->Draw("COLZ");
    c_sil_geo->cd(6);
        TH1D *h_hit_Z = new TH1D("h_hit_Z","HIT Z", 500, -25.0, 25.0);
        tree->Draw("SilHits.fPosZ>>h_hit_Z");
        h_hit_Z->SetXTitle("cm");
        h_hit_Z->Draw("");
    c_sil_geo->cd(4);
        TH1D *h_n_strip = new TH1D("h_n_strip","n-strip", 256, -0.5, 255.5);
        tree->Draw("SilHits.fnStrp>>h_n_strip");
        h_n_strip->SetXTitle("#");
        h_n_strip->Draw("");
    c_sil_geo->cd(5);
        TH1D *h_p_strip = new TH1D("h_p_strip","p-strip", 256, -0.5, 255.5);
        tree->Draw("SilHits.fpStrp>>h_p_strip");
        h_p_strip->SetXTitle("#");
        h_p_strip->Draw("");
}

void sil_ene() {
    //##########################################################
    // Preparing the histos
    TCanvas *c_sil_ene = new TCanvas("c_sil_ene","c_sil_ene",900,600);
    c_sil_ene->Divide(1,1);
    c_sil_ene->cd(1);
        ostringstream s;
        if(muons) s << "Muons RUN - Energy deposition in a single module";
        if(pbars) s << "Pbars RUN - Energy deposition in a single module";
        TH1D *h_hit_edep = new TH1D("h_hit_edep",s.str().c_str(), 501, -0.005, 1.005);
        tree->Draw("SilHits.fEdep*1000.>>h_hit_edep");
        h_hit_edep->SetXTitle("MeV");
        h_hit_edep->Draw("");
}

void get_parameters() {
    auto pdg = d->Max("fPdgCode");
    if((int)*pdg==-2212) pbars = true;
    if((int)*pdg==-13||(int)*pdg==+13) muons = true;
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

TH1D*   hDecayAnn() {
//    auto conta = d->Filter("MCTracks.fPdgCode < 10").Count();
//    cout << *conta << endl;
//    
    return nullptr;
}

