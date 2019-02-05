#include "../../reco/include/TPCconstants.hh"

double strippitch = 265.;
double stripwidth = 6.;

TString timecut1 = "time>1200 & time<2000";
TString timecut2 = "time>5100 & time<5600";
TString awampcut = "amp > 10000";
TString pampcut = "amp > 1000";

int awbin = 10;
int padbin = 16;

double sigmas = 3.;             // how many sigma around peak should time cut go

map<int,pair<char,int> > portmap =
    {
        {2657, pair<char,int>('b',15)},
        {2683, pair<char,int>('b',15)},
        {2684, pair<char,int>('b',15)},
        {2685, pair<char,int>('t',11)},
        {2686, pair<char,int>('t',11)},
        {2687, pair<char,int>('t',3)},
        {2688, pair<char,int>('b',7)}
    };

vector<TString> files =
    {
        // "output02657.root",     // B15, different/empty?
        // "output02683.root",     // B15
        // "output02684.root",     // B15
        "output02688.root",     // B07
        // "output02685.root",     // T11
        // "output02686.root",     // T11
        // "output02687.root"      // T03
    };

TString timecut1_p, timecut2_p, timecut1_a, timecut2_a;

double tp1, tp2, ta1, ta2, sigp1, sigp2, siga1, siga2;

set<double> GetStrips(){
    set<double> strips;
    for(double x = 0; x < _halflength; x += strippitch){
        strips.insert(x);
        strips.insert(-x);
    }
    return strips;
}

double mm2pad(double mm){
    return (mm + _halflength)/_padpitch;
}

void timeAnalysis(TTree *fPadTree, TTree *fAnodeTree){
    TCanvas *c = new TCanvas("ctime","time",800,600);
    c->SetLogy();
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
    TSpectrum sp(2);
    int np = sp.Search(hpt,30,"",0.2);
    TF1 *fp = new TF1("fp","gaus(0)+gaus(3)",hpt->GetXaxis()->GetXmin(),hpt->GetXaxis()->GetXmax());
    fp->SetLineStyle(2);
    fp->SetLineColor(kRed);
    Double_t *x = sp.GetPositionX();
    Double_t *y = sp.GetPositionY();
    for(int i = 0; i < np; i++){
        fp->SetParameter(i*3, y[i]);
        fp->SetParameter(i*3+1, x[i]);
        fp->SetParameter(i*3+2, 50);
    }
    hpt->Fit(fp);

    TF1 *fa = new TF1("fa","gaus(0)+gaus(3)+gaus(6)",hat->GetXaxis()->GetXmin(),hat->GetXaxis()->GetXmax());
    fa->SetLineStyle(2);
    fa->SetLineColor(kBlack);
    for(int i = 0; i < np; i++){
        fa->SetParameter(i*3, y[i]);
        fa->SetParameter(i*3+1, x[i]);
        fa->SetParameter(i*3+2, 50);
    }
    fa->FixParameter(1,fp->GetParameter(1));
    fa->SetParameter(6,y[0]);
    fa->SetParameter(7,x[0]-100);
    fa->SetParameter(8,50);
    hat->Fit(fa);

    hpt->Draw();
    hat->Draw("same");
    c->Update();

    tp1 = fp->GetParameter(1);
    sigp1 = fp->GetParameter(2);
    tp2 = fp->GetParameter(4);
    sigp2 = fp->GetParameter(5);

    ta1 = fa->GetParameter(1);
    siga1 = fa->GetParameter(2);
    ta2 = fa->GetParameter(4);
    siga2 = fa->GetParameter(5);

    timecut1_p = TString::Format("time > %f & time < %f", tp1 - sigmas*sigp1, tp1 + sigmas*sigp1);
    timecut2_p = TString::Format("time > %f & time < %f", tp2 - sigmas*sigp2, tp2 + sigmas*sigp2);

    timecut1_a = TString::Format("time > %f & time < %f", ta1 - sigmas*siga1, ta1 + sigmas*siga1); // Losing some of the electronic pickup, which we don't care about
    timecut2_a = TString::Format("time > %f & time < %f", ta2 - sigmas*siga2, ta2 + sigmas*siga2);
}

void padAnalysis(TTree *fPadTree){
    set<double> strips = GetStrips();
    vector<TLine*> stripBounds;
    for(auto s: strips){
        double x1 = mm2pad(s-0.5*stripwidth);
        double x2 = mm2pad(s+0.5*stripwidth);
        stripBounds.push_back(new TLine(x1,0,x1,10000));
        stripBounds.back()->SetLineColor(kGray);
        stripBounds.push_back(new TLine(x2,0,x2,10000));
        stripBounds.back()->SetLineColor(kGray);
    }

    TCanvas *c = new TCanvas("cpad","pads",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp1 = new TH2D("hp1","timecut 1",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1",timecut1_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hp2 = new TH2D("hp2","timecut 2",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2",timecut2_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    {
        TTreeReader reader(fPadTree);
        TTreeReaderValue<int> col(reader, "col");
        TTreeReaderValue<int> row(reader, "row");
        TTreeReaderValue<double> time(reader, "time");

        c = new TCanvas("cpadtime","pad time",1100,1100);
        c->Divide(1,2);
        int tb1b = floor(tp1 - sigmas*sigp1);
        int tb1t = ceil(tp1 + sigmas*sigp1);
        int nb1 = tb1t-tb1b;
        int tb2b = floor(tp2 - sigmas*sigp2);
        int tb2t = ceil(tp2 + sigmas*sigp2);
        int nb2 = tb2t-tb2b;
        Double_t t1levels[nb1];
        for(int i = 0; i < nb1; i++)
            t1levels[i] = tb1b+i;
        Double_t t2levels[nb2];
        for(int i = 0; i < nb2; i++)
            t2levels[i] = tb2b+i;

        TH2D *hptime1 = new TH2D("hptime1","timecut 1",576,0,576,32,0,32);
        // hptime1->SetContour(nb1, t1levels);
        hptime1->SetContour(100);
        hptime1->GetZaxis()->SetRangeUser(tb1b,tb1t);
        TH2D *hptime2 = new TH2D("hptime2","timecut 2",576,0,576,32,0,32);
        // hptime2->SetContour(nb2, t2levels);
        hptime2->GetZaxis()->SetRangeUser(tb2b,tb2t);
        hptime2->SetContour(100);

        while (reader.Next()) {
            int colbin = hp1->GetYaxis()->FindBin(*col);
            int rowbin = hp1->GetXaxis()->FindBin(*row);
            int bin = hp1->GetBin(rowbin,colbin);
            if(abs(*time - tp1) < sigmas*sigp1){
                // hptime1->Fill(*row,*col,*time);
                hptime1->AddBinContent(bin,*time);
            } else if(abs(*time - tp2) < sigmas*sigp2){
                // hptime2->Fill(*row,*col,*time);
                hptime2->AddBinContent(bin,*time);
            }
        }

        double occCut = 2./(hp1->GetNbinsX()*hp1->GetNbinsY());

        TH1D *hppt1 = new TH1D("hppt1","timecut 1",nb1,tb1b,tb1t);
        TH1D *hppt2 = new TH1D("hppt2","timecut 2",nb2,tb2b,tb2t);
        for(int bx = 0; bx <= hp1->GetNbinsX(); bx++){
            for(int by = 0; by <= hp1->GetNbinsY(); by++){
                double n = hp1->GetBinContent(bx,by);
                if(n > 0){
                    if(n > occCut*hp1->GetEntries()){
                        hptime1->SetBinContent(bx,by, hptime1->GetBinContent(bx,by)/n);
                        hppt1->Fill(hptime1->GetBinContent(bx,by));
                    } else
                        hptime1->SetBinContent(bx,by,0);
                }
                n = hp2->GetBinContent(bx,by);
                if(n > 0){
                    if(n > occCut*hp2->GetEntries()){
                        hptime2->SetBinContent(bx,by, hptime2->GetBinContent(bx,by)/n);
                        hppt2->Fill(hptime2->GetBinContent(bx,by));
                    } else
                        hptime2->SetBinContent(bx,by,0);
                }
            }
        }
        hptime1->Sumw2(true);
        hptime2->Sumw2(true);
        c->cd(1);
        hptime1->Draw("COLZ");
        for(auto l: stripBounds) l->Draw("same");
        c->cd(2);
        hptime2->Draw("COLZ");
        for(auto l: stripBounds) l->Draw("same");
        c->Update();

        c = new TCanvas("cppt","popular times",1100,1100);
        c->Divide(1,2);
        c->cd(1);
        hppt1->Draw();
        c->cd(2);
        hppt2->Draw();
    }

    /*
    c = new TCanvas("cpadac","pads ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp1ac = new TH2D("hp1ac","timecut 1 + ampcut",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1ac",timecut1_p + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hp2ac = new TH2D("hp2ac","timecut 2 + ampcut",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2ac",timecut2_p + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    c = new TCanvas("cpad2","pad col amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpc1 = new TH2D("hpc1","timecut 1",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc1",timecut1_p,"colz");
    c->cd(2);
    TH2D *hpc2 = new TH2D("hpc2","timecut 2",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc2",timecut2_p,"colz");
    c->Update();

    c = new TCanvas("cpad2t","pad col time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1 = new TH2D("hpct1","timecut 1",32,0,32,800,1200,2000);
    fPadTree->Draw("time:col>>hpct1",timecut1_p,"colz");
    c->cd(2);
    TH2D *hpct2 = new TH2D("hpct2","timecut 2",32,0,32,500,5100,5600);
    fPadTree->Draw("time:col>>hpct2",timecut2_p,"colz");
    c->Update();

    c = new TCanvas("cpad2tac","pad col time ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1ac = new TH2D("hpct1ac","timecut 1 + ampcut",32,0,32,800,1200,2000);
    fPadTree->Draw("time:col>>hpct1ac",timecut1_p + " & " + pampcut,"colz");
    c->cd(2);
    TH2D *hpct2ac = new TH2D("hpct2ac","timecut 2 + ampcut",32,0,32,500,5100,5600);
    fPadTree->Draw("time:col>>hpct2ac",timecut2_p + " & " + pampcut,"colz");
    c->Update();

    c = new TCanvas("cpad3","pad row amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpr1 = new TH2D("hpr1","timecut 1",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr1",timecut1_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hpr2 = new TH2D("hpr2","timecut 2",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr2",timecut2_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    c = new TCanvas("cpad3t","pad row time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1 = new TH2D("hprt1","timecut 1",576,0,576,800,1200,2000);
    fPadTree->Draw("time:row>>hprt1",timecut1_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hprt2 = new TH2D("hprt2","timecut 2",576,0,576,500,5100,5600);
    fPadTree->Draw("time:row>>hprt2",timecut2_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    TGraphErrors *g = new TGraphErrors;
    c = new TCanvas("cpadrt_proj","pad row time projection",800,600);
    int k = 1;
    for(auto s: strips){
        double x1 = mm2pad(s-0.5*stripwidth);
        double x2 = mm2pad(s+0.5*stripwidth);
        TString pn = TString::Format("p_%.0f",s);
        TH1D *p = hprt2->ProjectionY(pn,hprt2->GetXaxis()->FindBin(x1),hprt2->GetXaxis()->FindBin(x2));
        p->SetTitle(p->GetName());
        p->SetLineColor(k);
        p->Rebin(10);
        p->DrawNormalized(k++>1?"same":"");
        int n = g->GetN();
        g->SetPoint(n,mm2pad(s),p->GetMean());
        g->SetPointError(n,0,p->GetRMS());
    }
    c->BuildLegend();
    c = new TCanvas("cpadrt_prof","pad row time profile",800,600);
    g->Draw("AP");

    c = new TCanvas("cpad3tac","pad row time ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1ac = new TH2D("hprt1ac","timecut 1 + ampcut",576,0,576,800,1200,2000);
    fPadTree->Draw("time:row>>hprt1ac",timecut1_p + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hprt2ac = new TH2D("hprt2ac","timecut 2 + ampcut",576,0,576,500,5100,5600);
    fPadTree->Draw("time:row>>hprt2ac",timecut2_p + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    c = new TCanvas("cpadat","pad amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpat1 = new TH2D("hpat1","timecut 1",520,0,4160,800,1200,2000);
    fPadTree->Draw("time:amp>>hpat1",timecut1_p,"colz");
    c->cd(2);
    TH2D *hpat2 = new TH2D("hpat2","timecut 2",520,0,4160,500,5100,5600);
    fPadTree->Draw("time:amp>>hpat2",timecut2_p,"colz");
    c->Update();
    */
}

void awAnalysis(TTree *fAnodeTree){
    TCanvas *c = new TCanvas("caw","AW",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *ha1 = new TH2D("ha1","timecut 1",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha1",timecut1_a,"colz");
    c->cd(2);
    TH2D *ha2 = new TH2D("ha2","timecut 2",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha2",timecut2_a,"colz");
    c->Update();

    c = new TCanvas("cawt","AW time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1 = new TH2D("hat1","timecut 1",256,256,512,800,1200,2000);
    fAnodeTree->Draw("time:wire>>hat1",timecut1_a,"colz");
    c->cd(2);
    TH2D *hat2 = new TH2D("hat2","timecut 2",256,256,512,500,5100,5600);
    fAnodeTree->Draw("time:wire>>hat2",timecut2_a,"colz");
    c->Update();

    c = new TCanvas("cawtac","AW ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1ac = new TH2D("hat1ac","timecut 1 + ampcut",256,256,512,800,1200,2000);
    fAnodeTree->Draw("time:wire>>hat1ac",timecut1_a + " & " + awampcut,"colz");
    c->cd(2);
    TH2D *hat2ac = new TH2D("hat2ac","timecut 2 + ampcut",256,256,512,500,5100,5600);
    fAnodeTree->Draw("time:wire>>hat2ac",timecut2_a + " & " + awampcut,"colz");
    c->Update();

    c = new TCanvas("cawat","aw amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hawat1 = new TH2D("hawat1","timecut 1",700,0,70000,800,1200,2000);
    fAnodeTree->Draw("time:amp>>hawat1",timecut1_a,"colz");
    c->cd(2);
    TH2D *hawat2 = new TH2D("hawat2","timecut 2",700,0,70000,500,5100,5600);
    fAnodeTree->Draw("time:amp>>hawat2",timecut2_a,"colz");
    c->Update();
}

void LaserAnalysis(){
    TChain padChain("tpc_tree/fPadTree");
    for(TString fn: files){
        padChain.Add(fn);
    }
    vector<Long64_t> padtreeindex;
    int i(0);
    while( padChain.LoadTree(i) >= 0){
//        cout << i << '\t' << padChain.GetTree()->GetEntries() << endl;
        padtreeindex.push_back(i);
        i += padChain.GetTree()->GetEntries();
    }
    padChain.ls();
    TChain anodeChain("tpc_tree/fAnodeTree");
    anodeChain.Add("output*.root");
    vector<Long64_t> anodetreeindex;
    i=0;
    while( anodeChain.LoadTree(i) >= 0){
//        cout << i << '\t' << anodeChain.GetTree()->GetEntries() << endl;
        anodetreeindex.push_back(i);
        i += anodeChain.GetTree()->GetEntries();
    }
    anodeChain.ls();
    gStyle->SetOptStat(0);
    timeAnalysis((TTree*)&padChain, (TTree*)&anodeChain);
    padAnalysis((TTree*)&padChain);
    // awAnalysis((TTree*)&anodeChain);

}
