#include "ROOT/RDataFrame.hxx"
using namespace ROOT::Experimental;

void    open_root(int);
void    get_parameters();
void    primary_muon_origin();
void    primary_pbar_origin();
void    primary_pbar_decay();
void    primary_mom();

TTree *tree = 0;
ROOT::RDataFrame *d;
Double_t x_origin_min, y_origin_min, z_origin_min;
Double_t x_origin_max, y_origin_max, z_origin_max;
Double_t x_decay_min, y_decay_min, z_decay_min;
Double_t x_decay_max, y_decay_max, z_decay_max;
Bool_t muons = false;
Bool_t pbars = false;

///< main 
void show_gen(int runNumber) {
    std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
    gStyle->SetOptStat(stat_opt.c_str());
    open_root(runNumber);
    get_parameters();
    primary_mom();
    if(muons) primary_muon_origin();
    if(pbars) {
        primary_pbar_origin();
        primary_pbar_decay();
    }
    return;
}

///< REMEMBER ##########
///< in TH2D, when "Drawing", the first variable is the Y and the second is the X 
///< e.g. tree->Draw("Y:X") and NOT viceversa
///< functions
void primary_muon_origin() {
    //##########################################################
    // Preparing the histos
    TCanvas *corigin = new TCanvas("corigin","corigin",1200,1200);
    corigin->Divide(2,2);
    corigin->cd(1);
        TH2D *h_origin_XZ = new TH2D("h_origin_XZ","Primary origin X vs Z", 100, x_origin_min, x_origin_max, 100, z_origin_min, z_origin_max);
        tree->Draw("Primary.fVoz:Primary.fVox>>h_origin_XZ");
        h_origin_XZ->Draw("COLZ");
    corigin->cd(2);
        TH1D *h_origin_Y = new TH1D("h_origin_Y", "Primary origin Y", 100, y_origin_min, y_origin_max);
        tree->Draw("Primary.fVoy>>h_origin_Y");
        h_origin_Y->Draw("");
    corigin->cd(3);
        TH2D *h_origin_RvsY = new TH2D("h_origin_RvsY","Primary origin Y vs (X^2+Y^2)", 100, 0., x_origin_max, 100, y_origin_min, x_origin_max);
        tree->Draw("Primary.fVoy:sqrt(Primary.fVox*Primary.fVox+Primary.fVoz*Primary.fVoz)>>h_origin_RvsY");
        h_origin_RvsY->Draw("COLZ");
}

void primary_pbar_origin() {
    //##########################################################
    // Preparing the histos
    TCanvas *corigin = new TCanvas("corigin","corigin",1200,1200);
    corigin->Divide(2,2);
    corigin->cd(1);
        TH2D *h_origin_XY = new TH2D("h_origin_XY","Primary origin X vs Y", 100, x_origin_min, x_origin_max, 100, y_origin_min, y_origin_max);
        tree->Draw("Primary.fVoy:Primary.fVox>>h_origin_XY");
        h_origin_XY->Draw("COLZ");
    corigin->cd(2);
        TH1D *h_origin_Z = new TH1D("h_origin_Z", "Primary origin Z", 100, z_origin_min, z_origin_max);
        tree->Draw("Primary.fVoz>>h_origin_Z");
        h_origin_Z->Draw("");
    corigin->cd(3);
        TH2D *h_origin_RvsZ = new TH2D("h_origin_RvsZ","Primary origin R vs Z", 100, z_origin_min, z_origin_max, 100, 0., x_origin_max*1.1);
        tree->Draw("sqrt(Primary.fVox*Primary.fVox+Primary.fVoy*Primary.fVoy):Primary.fVoz>>h_origin_RvsZ");
        h_origin_RvsZ->Draw("COLZ");
}

void primary_pbar_decay() {
    //##########################################################
    // Preparing the histos
    TCanvas *cdecay = new TCanvas("cdecay","cdecay",1200,1200);
    cdecay->Divide(2,2);
    cdecay->cd(1);
        TH2D *h_decay_XY = new TH2D("h_decay_XY","Primary decay   X vs Y", 100, x_decay_min, x_decay_max, 100, y_decay_min, y_decay_max);
        tree->Draw("Primary.fVdy:Primary.fVdx>>h_decay_XY");
        h_decay_XY->Draw("COLZ");
    cdecay->cd(2);
        TH1D *h_decay_Z = new TH1D("h_decay_Z", "Primary decay   Z", 100, z_decay_min, z_decay_max);
        tree->Draw("Primary.fVdz>>h_decay_Z");
        h_decay_Z->Draw("");
    cdecay->cd(3);
        TH2D *h_decay_RvsZ = new TH2D("h_decay_RvsZ","Primary decay   R vs Z", 100, z_decay_min, z_decay_max, 100, 0., x_decay_max*1.1);
        tree->Draw("sqrt(Primary.fVdx*Primary.fVdx+Primary.fVdy*Primary.fVdy):Primary.fVdz>>h_decay_RvsZ");
        h_decay_RvsZ->Draw("COLZ");
}

void primary_mom() {
    TCanvas *cmom = new TCanvas("cmom","cmom",1200,1600);
    cmom->Divide(2,2);
    cmom->cd(1);
        TH1D *hp = new TH1D("hp","Momentum",100, -0.5, 20.5);
        tree->Draw("sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz)>>hp");
    cmom->cd(2);
        TH1D *hthe = new TH1D("hthe","#theta",91, -0.5, 90.5); hthe->SetXTitle("deg");
        tree->Draw("acos(fabs(Primary.fPoz)/sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz))*TMath::RadToDeg()>>hthe", 
                "sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz)!=0.");
    cmom->cd(3);
        TH1D *hcos = new TH1D("hcos","cos(#theta)",101, -0.05, 1.05); hcos->SetXTitle("cos(#theta)");
        tree->Draw("fabs(Primary.fPoz)/sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz)>>hcos", 
                "sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz)!=0.");
    cmom->cd(4);
        TH1D *hcos2 = new TH1D("hcos2","cos^2(#theta)",101, -0.05, 1.05); hcos2->SetXTitle("cos^2(#theta)");
        tree->Draw("pow(fabs(Primary.fPoz)/sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz),2.)>>hcos2", 
                "sqrt(Primary.fPox*Primary.fPox+Primary.fPoy*Primary.fPoy+Primary.fPoz*Primary.fPoz)!=0.");
    cmom->Modified(); cmom->Update();    
}

void get_parameters() {
    ///< Checking which primary particle (pbar, muon, etc. ?)
    auto pdg = d->Max("Primary.fPdgCode");
    if((int)*pdg==-2212) pbars = true;
    if((int)*pdg==-13||(int)*pdg==+13) muons = true;
    ///< Calculating the limits on Vx, Vy, Vz
    auto max = d->Max("Primary.fVox");
    auto min = d->Min("Primary.fVox");
    x_origin_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    x_origin_min = -x_origin_max; ///< Symmetric distributions in x

    max = d->Max("Primary.fVoy");
    min = d->Min("Primary.fVoy");
    y_origin_max = *max;
    y_origin_min = *min;

    max = d->Max("Primary.fVoz");
    min = d->Min("Primary.fVoz");
    z_origin_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    z_origin_min = -z_origin_max; ///< Symmetric distributions in z

    max = d->Max("Primary.fVdx");
    min = d->Min("Primary.fVdx");
    x_decay_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    x_decay_min = -x_decay_max; ///< Symmetric distributions in x

    max = d->Max("Primary.fVdy");
    min = d->Min("Primary.fVdy");
    y_decay_max = *max;
    y_decay_min = *min;

    max = d->Max("Primary.fVdz");
    min = d->Min("Primary.fVdz");
    z_decay_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    z_decay_min = -z_decay_max; ///< Symmetric distributions in z
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
