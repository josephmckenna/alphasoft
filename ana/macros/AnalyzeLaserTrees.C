TString timecut1 = "time>1200 & time<2400";
TString timecut2 = "time>5100 & time<6000";
TString awampcut = "amp > 10000";
TString pampcut = "amp > 1000";

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
    TH2D *hp1 = new TH2D("hp1","timecut 1;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1",timecut1,"colz");
    c->cd(2);
    TH2D *hp2 = new TH2D("hp2","timecut 2;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2",timecut2,"colz");

    c = new TCanvas("cpadac","pads ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp1ac = new TH2D("hp1ac","timecut 1 + ampcut;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1ac",timecut1 + " & " + pampcut,"colz");
    c->cd(2);
    TH2D *hp2ac = new TH2D("hp2ac","timecut 2 + ampcut;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2ac",timecut2 + " & " + pampcut,"colz");

    c = new TCanvas("cpad2","pad col amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpc1 = new TH2D("hpc1","timecut 1;col;amp",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc1",timecut1,"colz");
    c->cd(2);
    TH2D *hpc2 = new TH2D("hpc2","timecut 2;col;amp",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc2",timecut2,"colz");

    c = new TCanvas("cpad2t","pad col time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1 = new TH2D("hpct1","timecut 1;col;time",32,0,32,1200,1200,2400);
    fPadTree->Draw("time:col>>hpct1",timecut1,"colz");
    c->cd(2);
    TH2D *hpct2 = new TH2D("hpct2","timecut 2;col;time",32,0,32,900,5100,6000);
    fPadTree->Draw("time:col>>hpct2",timecut2,"colz");

    c = new TCanvas("cpad2tac","pad col time ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1ac = new TH2D("hpct1ac","timecut 1 + ampcut;col;time",32,0,32,1200,1200,2400);
    fPadTree->Draw("time:col>>hpct1ac",timecut1 + " & " + pampcut,"colz");
    c->cd(2);
    TH2D *hpct2ac = new TH2D("hpct2ac","timecut 2 + ampcut;col;time",32,0,32,900,5100,6000);
    fPadTree->Draw("time:col>>hpct2ac",timecut2 + " & " + pampcut,"colz");

    c = new TCanvas("cpad3","pad row amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpr1 = new TH2D("hpr1","timecut 1;row;amp",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr1",timecut1,"colz");
    c->cd(2);
    TH2D *hpr2 = new TH2D("hpr2","timecut 2;row;amp",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr2",timecut2,"colz");

    c = new TCanvas("cpad3t","pad row time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1 = new TH2D("hprt1","timecut 1;row;time",576,0,576,1200,1200,2400);
    fPadTree->Draw("time:row>>hprt1",timecut1,"colz");
    c->cd(2);
    TH2D *hprt2 = new TH2D("hprt2","timecut 2;row;time",576,0,576,900,5100,6000);
    fPadTree->Draw("time:row>>hprt2",timecut2,"colz");

    c = new TCanvas("cpad3tac","pad row time ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1ac = new TH2D("hprt1ac","timecut 1 + ampcut;row;time",576,0,576,1200,1200,2400);
    fPadTree->Draw("time:row>>hprt1ac",timecut1 + " & " + pampcut,"colz");
    c->cd(2);
    TH2D *hprt2ac = new TH2D("hprt2ac","timecut 2 + ampcut;row;time",576,0,576,900,5100,6000);
    fPadTree->Draw("time:row>>hprt2ac",timecut2 + " & " + pampcut,"colz");

    c = new TCanvas("cpadat","pad amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpat1 = new TH2D("hpat1","timecut 1;amp;time",520,0,4160,1200,1200,2400);
    fPadTree->Draw("time:amp>>hpat1",timecut1,"colz");
    c->cd(2);
    TH2D *hpat2 = new TH2D("hpat2","timecut 2;amp;time",520,0,4160,900,5100,6000);
    fPadTree->Draw("time:amp>>hpat2",timecut2,"colz");
}

void awAnalysis(TTree *fAnodeTree){
    TCanvas *c = new TCanvas("caw","AW",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *ha1 = new TH2D("ha1","timecut 1;aw;amp",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha1",timecut1,"colz");
    c->cd(2);
    TH2D *ha2 = new TH2D("ha2","timecut 2;aw;amp",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha2",timecut2,"colz");

    c = new TCanvas("cawt","AW time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1 = new TH2D("hat1","timecut 1;aw;time",256,256,512,1200,1200,2400);
    fAnodeTree->Draw("time:wire>>hat1",timecut1,"colz");
    c->cd(2);
    TH2D *hat2 = new TH2D("hat2","timecut 2;aw;time",256,256,512,900,5100,6000);
    fAnodeTree->Draw("time:wire>>hat2",timecut2,"colz");

    c = new TCanvas("cawtac","AW ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1ac = new TH2D("hat1ac","timecut 1 + ampcut;aw;time",256,256,512,1200,1200,2400);
    fAnodeTree->Draw("time:wire>>hat1ac",timecut1 + " & " + awampcut,"colz");
    c->cd(2);
    TH2D *hat2ac = new TH2D("hat2ac","timecut 2 + ampcut;aw;time",256,256,512,900,5100,6000);
    fAnodeTree->Draw("time:wire>>hat2ac",timecut2 + " & " + awampcut,"colz");

    c = new TCanvas("cawat","aw amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hawat1 = new TH2D("hawat1","timecut 1;amp;time",700,0,70000,1200,1200,2400);
    fAnodeTree->Draw("time:amp>>hawat1",timecut1,"colz");
    c->cd(2);
    TH2D *hawat2 = new TH2D("hawat2","timecut 2;amp;time",700,0,70000,900,5100,6000);
    fAnodeTree->Draw("time:amp>>hawat2",timecut2,"colz");
}

void AnalyzeLaserTrees(){
    TTree *fPadTree = (TTree*)_file0->FindObjectAny("fPadTree");
    TTree *fAnodeTree = (TTree*)_file0->FindObjectAny("fAnodeTree");
    gStyle->SetOptStat(0);
    timeAnalysis(fPadTree, fAnodeTree);
    padAnalysis(fPadTree);
    awAnalysis(fAnodeTree);
}
