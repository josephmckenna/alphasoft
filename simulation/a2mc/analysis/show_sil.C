#include "ROOT/RDataFrame.hxx"
using namespace ROOT::Experimental;

void    open_root(int);
void    get_parameters();
void    sil_geo();
void    sil_ene();
void    sil_hit_mult();

TTree *tree = 0;
ROOT::RDataFrame *d;
Int_t  nEvents = 0;
Bool_t muons = false;
Bool_t pbars = false;


///< main 
void show_sil(int runNumber) {
    std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
    gStyle->SetOptStat(stat_opt.c_str());
    open_root(runNumber);
    get_parameters();
    sil_hit_mult();
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
        if(muons) s << "Muons RUN - Layer multiplicity (per event)";
        if(pbars) s << "Pbars RUN - Layer multiplicity (per event)";
        TH1D *h_lay = new TH1D("h_lay",s.str().c_str(), 6, -0.5, 5.5);
        tree->Draw("SilHits.fLayN>>h_lay");
        if(nEvents!=0) h_lay->Scale(1./nEvents);
        h_lay->SetMinimum(0);
        h_lay->Draw("HIST,E");
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
    c_sil_geo->Modified(); c_sil_geo->Update();
}

void sil_hit_mult() {
    //##########################################################
    // Preparing the histos
    gROOT->cd();
//        auto h0 = d->Define("nh", "Sum(SilHits.fLayN==0)").Filter("nh>0").Histo1D("nh");
//        h0->Draw("");
        
//    auto h0 = d->Define("nh0", "Sum(SilHits.fLayN==0)").Filter("nh0>0").Histo1D("nh0");
//    auto h1 = d->Define("nh1", "Sum(SilHits.fLayN==1)").Filter("nh1>0").Histo1D("nh1");
//    auto h2 = d->Define("nh2", "Sum(SilHits.fLayN==2)").Filter("nh2>0").Histo1D("nh2");
//    auto h3 = d->Define("nh3", "Sum(SilHits.fLayN==3)").Filter("nh3>0").Histo1D("nh3");
//    auto h4 = d->Define("nh4", "Sum(SilHits.fLayN==4)").Filter("nh4>0").Histo1D("nh4");
//    auto h5 = d->Define("nh5", "Sum(SilHits.fLayN==5)").Filter("nh5>0").Histo1D("nh5");

    auto nh0 = d->Define("nh0", "Sum(SilHits.fLayN==0)").Filter("nh0>0");
    auto nh1 = d->Define("nh1", "Sum(SilHits.fLayN==1)").Filter("nh1>0");
    auto nh2 = d->Define("nh2", "Sum(SilHits.fLayN==2)").Filter("nh2>0");
    auto nh3 = d->Define("nh3", "Sum(SilHits.fLayN==3)").Filter("nh3>0");
    auto nh4 = d->Define("nh4", "Sum(SilHits.fLayN==4)").Filter("nh4>0");
    auto nh5 = d->Define("nh5", "Sum(SilHits.fLayN==5)").Filter("nh5>0");

    auto h0 = nh0.Histo1D({"h0", "Number of hits in Layer 0", 16, -0.5, 15.5}, "nh0");
    auto h1 = nh1.Histo1D({"h1", "Number of hits in Layer 1", 16, -0.5, 15.5}, "nh1");
    auto h2 = nh2.Histo1D({"h2", "Number of hits in Layer 2", 16, -0.5, 15.5}, "nh2");
    auto h3 = nh3.Histo1D({"h3", "Number of hits in Layer 3", 16, -0.5, 15.5}, "nh3");
    auto h4 = nh4.Histo1D({"h4", "Number of hits in Layer 4", 16, -0.5, 15.5}, "nh4");
    auto h5 = nh5.Histo1D({"h5", "Number of hits in Layer 5", 16, -0.5, 15.5}, "nh5");

    TH1D *h0_Clone = (TH1D*)h0->Clone();
    TH1D *h1_Clone = (TH1D*)h1->Clone();
    TH1D *h2_Clone = (TH1D*)h2->Clone();
    TH1D *h3_Clone = (TH1D*)h3->Clone();
    TH1D *h4_Clone = (TH1D*)h4->Clone();
    TH1D *h5_Clone = (TH1D*)h5->Clone();

    TCanvas *c_sil_hit_mult = new TCanvas("c_sil_hit_mult","c_sil_hit_mult",1500,1200);
    c_sil_hit_mult->Divide(3,2);
    c_sil_hit_mult->cd(1);
        h0_Clone->Draw("HIST, E");
    c_sil_hit_mult->cd(2);
        h1_Clone->Draw("HIST, E");
    c_sil_hit_mult->cd(3);
        h2_Clone->Draw("HIST, E");
    c_sil_hit_mult->cd(4);
        h3_Clone->Draw("HIST, E");
    c_sil_hit_mult->cd(5);
        h4_Clone->Draw("HIST, E");
    c_sil_hit_mult->cd(6);
        h5_Clone->Draw("HIST, E");
    c_sil_hit_mult->Modified(); c_sil_hit_mult->Update();
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
    c_sil_ene->Modified(); c_sil_ene->Update();
}

void get_parameters() {
    nEvents = *d->Filter("fPdgCode!=0").Count();
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


