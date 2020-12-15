#include "ROOT/RDataFrame.hxx"
using namespace ROOT::Experimental;

void    open_root(int);
void    get_parameters();
void    primary_muon_origin();
void    primary_pbar_origin();
void    primary_pbar_decay();
void    primary_mom();
void    set_pdg_codes();
TH1D*   hDecayAnn();

TTree *tree = 0;
ROOT::RDataFrame *d;
TDatabasePDG* pdgDB = new TDatabasePDG();
map<string, int> ann_products;
Double_t x_origin_min, y_origin_min, z_origin_min;
Double_t x_origin_max, y_origin_max, z_origin_max;
Double_t x_decay_min, y_decay_min, z_decay_min;
Double_t x_decay_max, y_decay_max, z_decay_max;
Double_t p_tot_origin_max;
Bool_t muons = false;
Bool_t pbars = false;

///< main 
void show_gen(int runNumber) {
    std::string  stat_opt = "im";      // i = integral, m = mean, r = rms, etc. 
    gStyle->SetOptStat(stat_opt.c_str());
    open_root(runNumber);
    get_parameters();
    set_pdg_codes();
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
        tree->Draw("fVoz:fVox>>h_origin_XZ");
        h_origin_XZ->Draw("COLZ");
    corigin->cd(2);
        TH1D *h_origin_Y = new TH1D("h_origin_Y", "Primary origin Y", 100, y_origin_min, y_origin_max);
        tree->Draw("fVoy>>h_origin_Y");
        h_origin_Y->Draw("");
    corigin->cd(3);
        TH2D *h_origin_RvsY = new TH2D("h_origin_RvsY","Primary origin Y vs (X^2+Y^2)", 100, 0., x_origin_max, 100, y_origin_min, x_origin_max);
        tree->Draw("fVoy:sqrt(fVox*fVox+fVoz*fVoz)>>h_origin_RvsY");
        h_origin_RvsY->Draw("COLZ");
}

void primary_pbar_origin() {
    //##########################################################
    // Preparing the histos
    TCanvas *corigin = new TCanvas("corigin","corigin",1200,1200);
    corigin->Divide(2,2);
    corigin->cd(1);
        TH2D *h_origin_XY = new TH2D("h_origin_XY","Primary origin X vs Y", 100, x_origin_min, x_origin_max, 100, y_origin_min, y_origin_max);
        tree->Draw("fVoy:fVox>>h_origin_XY");
        h_origin_XY->SetXTitle("cm"); h_origin_XY->SetYTitle("cm"); 
        h_origin_XY->Draw("COLZ");
    corigin->cd(2);
        TH1D *h_origin_Z = new TH1D("h_origin_Z", "Primary origin Z", 100, z_origin_min, z_origin_max);
        tree->Draw("fVoz>>h_origin_Z");
        h_origin_Z->SetXTitle("cm");
        h_origin_Z->Draw("");
    corigin->cd(3);
        TH2D *h_origin_RvsZ = new TH2D("h_origin_RvsZ","Primary origin R vs Z", 100, z_origin_min, z_origin_max, 100, 0., x_origin_max*1.1);
        tree->Draw("sqrt(fVox*fVox+fVoy*fVoy):fVoz>>h_origin_RvsZ");
        h_origin_RvsZ->SetXTitle("cm"); h_origin_RvsZ->SetYTitle("cm"); 
        h_origin_RvsZ->Draw("COLZ");
}

void primary_pbar_decay() {
    //##########################################################
    // Preparing the histos
    TCanvas *cdecay = new TCanvas("cdecay","cdecay",1200,1200);
    cdecay->Divide(2,2);
    cdecay->cd(1);
        TH2D *h_decay_XY = new TH2D("h_decay_XY","Primary decay   X vs Y", 100, x_decay_min*1.1, x_decay_max*1.1, 100, y_decay_min*1.1, y_decay_max*1.1);
        tree->Draw("fVdy:fVdx>>h_decay_XY");
        h_decay_XY->SetXTitle("cm"); h_decay_XY->SetYTitle("cm"); 
        h_decay_XY->Draw("COLZ");
    cdecay->cd(2);
        TH1D *h_decay_Z = new TH1D("h_decay_Z", "Primary decay   Z", 100, z_decay_min, z_decay_max);
        tree->Draw("fVdz>>h_decay_Z");
        h_decay_Z->SetXTitle("cm");
        h_decay_Z->Draw("");
    cdecay->cd(3);
        TH2D *h_decay_RvsZ = new TH2D("h_decay_RvsZ","Primary decay   R vs Z", 100, z_decay_min, z_decay_max, 100, 0., x_decay_max*1.1);
        tree->Draw("sqrt(fVdx*fVdx+fVdy*fVdy):fVdz>>h_decay_RvsZ");
        h_decay_RvsZ->SetXTitle("cm"); h_decay_RvsZ->SetYTitle("cm"); 
        h_decay_RvsZ->Draw("COLZ");
    cdecay->cd(4);
        TH1D *h_decay_ann = hDecayAnn();
        h_decay_ann->Draw("HIST");
}

void primary_mom() {
    TCanvas *cmom = new TCanvas("cmom","cmom",1200,1600);
    cmom->Divide(2,2);
    cmom->cd(1);
        TH1D *hp = new TH1D("hp","Momentum",100, 0., p_tot_origin_max*1000*1.2); ///< *1000 -> MeV
        tree->Draw("sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)*1000.>>hp"); ///< *1000 -> MeV
        hp->SetXTitle("MeV");
    cmom->cd(2);
        TH1D *hthe = new TH1D("hthe","#theta",91, -0.5, 90.5); hthe->SetXTitle("deg");
        tree->Draw("acos(fabs(fPoz)/sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz))*TMath::RadToDeg()>>hthe", 
                "sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)!=0.");
    cmom->cd(3);
        TH1D *hcos = new TH1D("hcos","cos(#theta)",101, -0.05, 1.05); hcos->SetXTitle("cos(#theta)");
        tree->Draw("fabs(fPoz)/sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)>>hcos", 
                "sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)!=0.");
    cmom->cd(4);
        TH1D *hcos2 = new TH1D("hcos2","cos^2(#theta)",101, -0.05, 1.05); hcos2->SetXTitle("cos^2(#theta)");
        tree->Draw("pow(fabs(fPoz)/sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz),2.)>>hcos2", 
                "sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)!=0.");
    cmom->Modified(); cmom->Update();    
}

void get_parameters() {
    ///< Checking which primary particle (pbar, muon, etc. ?)
    auto pdg = d->Max("fPdgCode");
    if((int)*pdg==-2212) pbars = true;
    if((int)*pdg==-13||(int)*pdg==+13) muons = true;
    ///< Calculating the limits on Vox, Voy, Voz - ORIGIN
    auto max = d->Filter("!isnan(fVox)").Max("fVox");
    auto min = d->Filter("!isnan(fVox)").Min("fVox");
    x_origin_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    x_origin_min = -x_origin_max; ///< Symmetric distributions in x
    max = d->Filter("!isnan(fVoy)").Max("fVoy");
    min = d->Filter("!isnan(fVoy)").Min("fVoy");
    y_origin_max = *max;
    y_origin_min = *min;
    max = d->Filter("!isnan(fVoz)").Max("fVoz");
    min = d->Filter("!isnan(fVoz)").Min("fVoz");
    z_origin_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    z_origin_min = -z_origin_max; ///< Symmetric distributions in z

    ///< Calculating the limits on Vdx, Vdy, Vdz - DECAY
    max = d->Filter("!isnan(fVdx)").Max("fVdx");
    min = d->Filter("!isnan(fVdx)").Min("fVdx");
    x_decay_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    x_decay_min = -x_decay_max; ///< Symmetric distributions
    max = d->Filter("!isnan(fVdy)").Max("fVdy");
    min = d->Filter("!isnan(fVdy)").Min("fVdy");
    y_decay_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    y_decay_min = -y_decay_max; ///< Symmetric distributions
    max = d->Filter("!isnan(fVdz)").Max("fVdz");
    min = d->Filter("!isnan(fVdz)").Min("fVdz");
    z_decay_max = (fabs(*max)>fabs(*min)) ? *max : fabs(*min);
    z_decay_min = -z_decay_max; ///< Symmetric distributions in z
    
    ///< Maximum value of p 
    p_tot_origin_max = *d->Define("ptot","sqrt(fPox*fPox+fPoy*fPoy+fPoz*fPoz)").Max("ptot");
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
    ///< ---------------------------------------------
    ///< Counters for the annihilation products
    ann_products["He4"]     = 0;
    ann_products["D"]       = 0;
    ann_products["He3"]     = 0;
    ann_products["T"]       = 0;
    ann_products["e-"]      = 0;
    ann_products["neutron"] = 0;
    ann_products["pi-"]     = 0;
    ann_products["pi0"]     = 0;
    ann_products["pi+"]     = 0;
    ann_products["proton"]  = 0;
    ///< ---------------------------------------------
    ///< Counting how many of them
    map<string, int>::iterator imap;
    for (imap = ann_products.begin(); imap != ann_products.end(); ++imap) {
        string key = imap->first;
        TParticlePDG *particle = pdgDB->GetParticle(key.c_str());
        ostringstream s;
        s   << "Sum(MCTracks.fPdgCode==" << particle->PdgCode() 
            << "&& MCTracks.fVx==fVdx && MCTracks.fVy==fVdy && MCTracks.fVz==fVdz)";
        auto n = d->Define("ntrks", s.str()).Sum("ntrks");
        imap->second = *n;
    }
    ann_products["nuclei"]   = 0;
    ostringstream s;
    s   << "Sum(MCTracks.fPdgCode>1000020040" 
        << "&& MCTracks.fVx==fVdx && MCTracks.fVy==fVdy && MCTracks.fVz==fVdz)";
    auto n = d->Define("ntrks", s.str()).Sum("ntrks");
    ann_products["nuclei"] = *n;
    ///< ---------------------------------------------
    ///< Filling an histogram
    TH1D *h_pbar_ann = new TH1D("h_pbar_ann","Annihilation multiplicity (per event)", ann_products.size(), 0., ann_products.size());
    Int_t nEvents = *d->Filter("fPdgCode!=0").Count();
    int j=0;
    for (imap = ann_products.begin(); imap != ann_products.end(); ++imap) {
        string key = imap->first;
        int    value = imap->second;
        h_pbar_ann->GetXaxis()->SetBinLabel(j+1,key.c_str());
        h_pbar_ann->Fill(j+0.5,value); 
        j++;
    }
    if(nEvents!=0) h_pbar_ann->Scale(1./nEvents);    
    return h_pbar_ann;
}

void set_pdg_codes() {
    double mp = 0.938272; // mass of the proton in GeV/c2
//TParticlePDG * TDatabasePDG::AddParticle 	(name,title,mass,stable,width,charge,class,PDGcode)
    pdgDB->AddParticle("D","Nuclei",    mp*2.,1.,0.,1.,"",      1000010020);
    pdgDB->AddParticle("T","Nuclei",    mp*3.,1.,0.,1.,"",      1000010030);
    pdgDB->AddParticle("He3","Nuclei",  mp*3.,1.,0.,2.,"",      1000020030);
    pdgDB->AddParticle("He4","Nuclei",  mp*4.,1.,0.,2.,"",      1000020040);
    pdgDB->AddParticle("Li","Nuclei",   mp*6.,1.,0.,3.,"",      1000030060);
    pdgDB->AddParticle("C","Nuclei",    mp*20.,1.,0.,6.,"",     1000060120);
    pdgDB->AddParticle("N","Nuclei",    mp*15.,1.,0.,7.,"",     1000070150);
    pdgDB->AddParticle("O","Nuclei",    mp*16.,1.,0.,8.,"",     1000080160);
    pdgDB->AddParticle("O15","Nuclei",  mp*15.,1.,0.,8.,"",     1000080150);
    pdgDB->AddParticle("F","Nuclei",    mp*18.,1.,0.,9.,"",     1000090180);
    pdgDB->AddParticle("Ne","Nuclei",   mp*20.,1.,0.,10.,"",    1000100200);
    pdgDB->AddParticle("Na","Nuclei",   mp*22.,1.,0.,11.,"",    1000110220);
    pdgDB->AddParticle("Na23","Nuclei", mp*23.,1.,0.,11.,"",    1000110230);
    pdgDB->AddParticle("Mg","Nuclei",   mp*24.,1.,0.,12.,"",    1000120240);
    pdgDB->AddParticle("Mg25","Nuclei", mp*25.,1.,0.,12.,"",    1000120250);
    pdgDB->AddParticle("Mg26","Nuclei", mp*26.,1.,0.,12.,"",    1000120260);
    pdgDB->AddParticle("Al","Nuclei",   mp*25.,1.,0.,13.,"",    1000130250);
    pdgDB->AddParticle("Al25","Nuclei", mp*27.,1.,0.,13.,"",    1000130270);
    pdgDB->AddParticle("Si","Nuclei",   mp*28.,1.,0.,14.,"",    1000140280);
    pdgDB->AddParticle("Si30","Nuclei", mp*30.,1.,0.,14.,"",    1000140300);
}