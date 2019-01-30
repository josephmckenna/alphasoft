TString timecut1 = "time>1200 & time<2000";
TString timecut2 = "time>5100 & time<5600";
int awbin = 10;
int padbin = 16;

void timeAnalysis(TTree *fPadTree, TTree *fAnodeTree){
    TCanvas *c = new TCanvas("ctime","time",800,600);
    int nawbin = 7000/awbin;
    int awmax = awbin*nawbin;
    TH1D *hat = new TH1D("hat","anode times",nawbin,0,awmax);
    int npadbin = 7000/padbin;
    int padmax = padbin*npadbin;
    TH1D *hpt = new TH1D("hpt","pad times",npadbin,0,padmax);
    hpt->SetLineColor(kRed);
    hat->SetBit(TH1::kNoTitle);
    hpt->SetBit(TH1::kNoTitle);
    fAnodeTree->Draw("time >> hat");
    fPadTree->Draw("time >> hpt","","same");
    c->BuildLegend();
    c->Update();
}

void padAnalysis(TTree *fPadTree){
    TCanvas *c = new TCanvas("cpad","pads",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp1 = new TH2D("hp1","timecut 1",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1",timecut1,"colz");
    c->cd(2);
    TH2D *hp2 = new TH2D("hp2","timecut 2",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2",timecut2,"colz");
}

void awAnalysis(TTree *fAnodeTree){
    TCanvas *c = new TCanvas("caw","AW",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *ha1 = new TH2D("ha1","timecut 1",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha1",timecut1,"colz");
    c->cd(2);
    TH2D *ha2 = new TH2D("ha2","timecut 2",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha2",timecut2,"colz");
}

void AnalyzeLaserTrees(){
    TTree *fPadTree = (TTree*)_file0->FindObjectAny("fPadTree");
    TTree *fAnodeTree = (TTree*)_file0->FindObjectAny("fAnodeTree");
    gStyle->SetOptStat(0);
    timeAnalysis(fPadTree, fAnodeTree);
    padAnalysis(fPadTree);
    awAnalysis(fAnodeTree);
}
