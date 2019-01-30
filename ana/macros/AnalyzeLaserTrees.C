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

    c = new TCanvas("cpad2","pad col amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpc1 = new TH2D("hpc1","timecut 1",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc1",timecut1,"colz");
    c->cd(2);
    TH2D *hpc2 = new TH2D("hpc2","timecut 2",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc2",timecut2,"colz");

    c = new TCanvas("cpad2t","pad col time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1 = new TH2D("hpct1","timecut 1",32,0,32,800,1200,2000);
    fPadTree->Draw("time:col>>hpct1",timecut1,"colz");
    c->cd(2);
    TH2D *hpct2 = new TH2D("hpct2","timecut 2",32,0,32,500,5100,5600);
    fPadTree->Draw("time:col>>hpct2",timecut2,"colz");

    c = new TCanvas("cpad3","pad row amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpr1 = new TH2D("hpr1","timecut 1",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr1",timecut1,"colz");
    c->cd(2);
    TH2D *hpr2 = new TH2D("hpr2","timecut 2",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr2",timecut2,"colz");

    c = new TCanvas("cpad3t","pad row time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1 = new TH2D("hprt1","timecut 1",576,0,576,800,1200,2000);
    fPadTree->Draw("time:row>>hprt1",timecut1,"colz");
    c->cd(2);
    TH2D *hprt2 = new TH2D("hprt2","timecut 2",576,0,576,500,5100,5600);
    fPadTree->Draw("time:row>>hprt2",timecut2,"colz");
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
