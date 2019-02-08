#include "../../reco/include/TPCconstants.hh"

using namespace TMath;

double strippitch = 265.;
double stripwidth = 6.;

TString timecut1 = "time>1200 & time<2000";
TString timecut2 = "time>5100 & time<5600";
TString awOFcut = "amp < 40000";// remove overflow
TString pOFcut = "amp < 4000";  // remove overflow
TString awampcut = "amp > 10000";
TString pampcut = "amp > 1000";

int awbin = 10;
int padbin = 16;

double sigmas = 4.;             // how many sigma around peak should time cut go

map<int,pair<char,int> > portmap =
    {
        {2657, pair<char,int>('B',15)},
        {2683, pair<char,int>('B',15)},
        {2684, pair<char,int>('B',15)},
        {2685, pair<char,int>('T',3)},     // labeled T11, seems swapped to T03
        {2686, pair<char,int>('T',3)},     // labeled T11, seems swapped to T03
        {2687, pair<char,int>('T',11)},      // labeled T03, seems swapped to T11
        {2688, pair<char,int>('B',7)}
    };

map<int,double> portOffset =    // negative sign does NOT denote negative offset angle, but top end instead of bottom end
    {
        {15, 346.25},             // FIXME: These are placeholder values
        {11, -256.25},
        {7, 166.25},
        {3, -76.25}
    };

vector<TString> files =
    {
        // "output02657.root",     // B15, different/empty?
        "output02683.root",     // B15
        "output02684.root",     // B15
        "output02688.root",     // B07
        "output02685.root",     // labeled T11, seems swapped to T03
        "output02686.root",     // labeled T11, seems swapped to T03
        "output02687.root"      // labeled T03, seems swapped to T11
    };

TString timecut1_p, timecut2_p, timecut1_a, timecut2_a;

double tp1, tp2, ta1, ta2, sigp1, sigp2, siga1, siga2;

map<int,vector<TF1*> > profiles;

TLegend *profLegend(nullptr);

Double_t laser_profile(Double_t *x, Double_t *par)
{
    double z0 = par[0],
        Rc = par[1],
        Rr = par[2],
        theta = par[3]*DegToRad(),
        beta = par[4]*DegToRad();

    double a = Cos(beta)*Cos(theta),
        b = Sin(beta),
        c = Cos(beta)*Sin(theta)/Rc,
        d = (-Cos(beta)*Sin(theta)*z0 + Rr*Sin(beta))/Rc;

    double gamma = Sqrt(a*a+b*b),
        delta = ATan2(b,a);


    double phi = ASin((c*x[0]+d)/gamma)-delta;
    return phi*RadToDeg();
}

double mm2pad(double mm){
    return (mm + _halflength)/_padpitch;
}

double pad2mm(double pad){
    return pad*_padpitch - _halflength;
}

double deg2col(double deg){
    return deg/360.*double(_padcol);
}

double col2deg(double col){
    return col/double(_padcol)*360.;
}

Double_t laser_profile_pad(Double_t *x, Double_t *par)
{
    double phi_offset = par[5];
    bool top = phi_offset < 0.;
    phi_offset = abs(phi_offset);

    double z0 = par[0],
        Rc = par[1],
        Rr = par[2],
        theta = par[3]*DegToRad(),
        beta = par[4]*DegToRad();

    double a = Cos(beta)*Cos(theta),
        b = Sin(beta),
        c = Cos(beta)*Sin(theta)/Rc,
        d = (-Cos(beta)*Sin(theta)*z0 + Rr*Sin(beta))/Rc;

    double gamma = Sqrt(a*a+b*b),
        delta = ATan2(b,a);


    double phi;
    if(top) phi = -1.*(ASin((c*(_padrow - pad2mm(x[0])) + d)/gamma)-delta);
    else phi = ASin((c*pad2mm(x[0])+d)/gamma)-delta;
    phi = phi*RadToDeg()+phi_offset;
    // if(phi < 0.) phi += 360.;
    // else if(phi > 360.) phi -= 360.;
    return deg2col(phi);
}

set<double> GetStrips(){
    set<double> strips;
    for(double x = 0; x < _halflength; x += strippitch){
        strips.insert(x);
        strips.insert(-x);
    }
    return strips;
}

void DrawProfiles(){
    if(!profiles.size()){
        TF1 *lasProf = new TF1("lasProf",laser_profile_pad,0,_padrow,6);
        double z0 = -1172.5,        // rod z position
            Rc = _cathradius,       // cathode radius
            Rr = Rc + 27.35,        // rod r position

            theta = 3.0, beta = -50.;

        double phi_offset = 0.;
        //double zmin=0., zmax=576.; // mm
        lasProf->SetParameters(z0,Rc,Rr,theta,beta,phi_offset);
        for(int i = 0; i < 5; i++) lasProf->FixParameter(i, lasProf->GetParameter(i));
        lasProf->SetParNames("z0","Rc","Rr","theta","beta","phi_offset");

        for(auto f: files){
            f.Remove(0,6);
            f.Remove(f.Length()-5);
            int run = f.Atoi();
            pair<char,int> port = portmap[run];
            if(profiles.find(port.second) == profiles.end()){
                double phioff = portOffset[port.second];
                TString title = TString::Format("%c%02d",port.first, port.second);
                TF1 *fclone = new TF1(*lasProf);
                fclone->SetTitle(title);
                fclone->SetParameter(5,phioff);
                int mycolor = profiles.size()+4;
                fclone->SetLineColor(mycolor);
                profiles[port.second].push_back(fclone);
                if(fclone->Eval(fclone->GetXmin()) > _padcol || fclone->Eval(fclone->GetXmax()) > _padcol){
                    fclone = new TF1(*lasProf);
                    fclone->SetTitle(title);
                    fclone->SetParameter(5,phioff - 360.);
                    fclone->SetLineColor(mycolor);
                    profiles[port.second].push_back(fclone);
                } else if(fclone->Eval(fclone->GetXmin()) < 0 || fclone->Eval(fclone->GetXmax()) < 0){
                    fclone = new TF1(*lasProf);
                    fclone->SetTitle(title);
                    fclone->SetParameter(5,phioff + 360.);
                    fclone->SetLineColor(mycolor);
                    profiles[port.second].push_back(fclone);
                }
            }
        }
    }
    for(auto &p: profiles){
        for(auto &pp: p.second)
            pp->Draw("same");
    }
    if(!profLegend){
        profLegend = new TLegend(0.79,0.7,0.86,0.89);
        profLegend->SetHeader("Laser Port");
        for(auto &p: profiles)
            profLegend->AddEntry(p.second[0],p.second[0]->GetTitle(),"l");
    }
    profLegend->Draw();
}

void DrawHitResiduals(){        // Per laser port Draw hitpattern relative to expected profile line
    map<pair<char,int>,TH2D*> pResHistMap;
    if(!profiles.size()){
        cerr << "DrawProfiles() needs to be called before DrawHitResiduals()" << endl;
        return;
    }
    for(auto f: files){
        TFile fil(f);
        TTree *t = (TTree*)fil.Get("tpc_tree/fPadTree");
        f.Remove(0,6);
        f.Remove(f.Length()-5);
        int run = f.Atoi();
        pair<char,int> port = portmap[run];
        TString hname = TString::Format("hresid%c%0d",port.first,port.second);
        TH2D *h = (TH2D*)gROOT->FindObjectAny(hname.Data()); // FIXME: For some reason TH2D goes away.
        if(!h){
            cout << "Creating Histogram " << hname.Data() << " for port " << port.first << port.second << endl;
            TString title = TString::Format("Residual hit position port %c%02d; pad row; degrees", port.first, port.second);
            h = new TH2D(TString::Format("hresid%c%0d",port.first,port.second).Data(), title, _padrow, 0, _padrow, 40, -20, 20);
            cout << h << " = " << (TH2D*)gROOT->FindObjectAny(hname.Data()) << endl;
        }
        TTreeReader reader(t);
        TTreeReaderValue<int> col(reader, "col");
        TTreeReaderValue<int> row(reader, "row");
        TTreeReaderValue<double> time(reader, "time");
        TTreeReaderValue<double> amp(reader, "amp");

        cout << port.first << '\t' << port.second  << '\t' << h << endl;
        cout << port.second << '\t' <<  h->GetTitle() << endl;
        while (reader.Next()) {
            double delta = 100000.;
            for(auto &p: profiles[port.second]){
                double d = *col - p->Eval(double(*row));
                if(abs(d) < abs(delta)) delta = d;
            }
            double r = *row;
            delta *= 360./double(_padcol);
            // h->Fill(r, delta);
        }
    }
    // TCanvas *c = new TCanvas("chitres","Hit residuals",1100,1100);
    // c->Divide(1,pResHistMap.size());
    // int i = 1;
    // for(auto &h: pResHistMap){
    //     c->cd(i++);
    //     h.second->Draw();
    // }
    cout << "OK" << endl;
}

void timeAnalysis(TTree *fPadTree, TTree *fAnodeTree){
    TCanvas *c = new TCanvas("ctime","time",800,600);
    c->SetLogy();
    int nawbin = 7000/awbin;
    int awmax = awbin*nawbin;
    TH1D *hat = new TH1D("hat","anode times;time [ns]; counts",nawbin,0,awmax);
    TH1D *hatNoOF = new TH1D("hatNoOF","anode times, no amp overflow;time [ns]; counts",nawbin,0,awmax);
    hatNoOF->SetFillStyle(3001);
    hatNoOF->SetFillColor(hatNoOF->GetLineColor());
    int npadbin = 7000/padbin;
    int padmax = padbin*npadbin;
    TH1D *hpt = new TH1D("hpt","pad times;time [ns]; counts",npadbin,0,padmax);
    TH1D *hptNoOF = new TH1D("hptNoOF","pad times, no amp overflow;time [ns]; counts",npadbin,0,padmax);
    hpt->SetLineColor(kRed);
    hptNoOF->SetLineColor(kRed);
    hptNoOF->SetFillColor(kRed);
    hptNoOF->SetFillStyle(3003);
    hat->SetBit(TH1::kNoTitle);
    hpt->SetBit(TH1::kNoTitle);
    hatNoOF->SetBit(TH1::kNoTitle);
    hptNoOF->SetBit(TH1::kNoTitle);
    fAnodeTree->Draw("time >> hat");
    fAnodeTree->Draw("time >> hatNoOF", awOFcut, "same");
    fPadTree->Draw("time >> hpt","","same");
    fPadTree->Draw("time >> hptNoOF", pOFcut, "same");
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
    hptNoOF->Fit(fp);

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
    hatNoOF->Fit(fa);

    hpt->Draw();
    hat->Draw("same");
    hatNoOF->Draw("same");
    hptNoOF->Draw("same");
    c->BuildLegend();
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


    /////////////// Hitpattern
    TCanvas *c = new TCanvas("cpad","pads",1100,1100);
    c->Divide(1,2);
    TH2D *hp1 = new TH2D("hp1","timecut 1;row;col",576,0,576,32,0,32);
    TH2D *hp2 = new TH2D("hp2","timecut 2;row;col",576,0,576,32,0,32);
    TH2D *hp1NoOF = new TH2D("hp1NoOF","timecut 1;row;col",576,0,576,32,0,32);
    TH2D *hp2NoOF = new TH2D("hp2NoOF","timecut 2;row;col",576,0,576,32,0,32);
    c->cd(1);
    fPadTree->Draw("col:row>>hp1NoOF",timecut1_p + " & " + pOFcut,"colz");
    fPadTree->Draw("col:row>>hp1",timecut1_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    fPadTree->Draw("col:row>>hp2NoOF",timecut2_p + " & " + pOFcut,"contz");
    fPadTree->Draw("col:row>>hp2",timecut2_p,"contz");
    for(auto l: stripBounds) l->Draw("same");

    DrawProfiles();
    c->Update();

    ////////////////////// Hitpattern high counts only
    c = new TCanvas("cpadac","pads ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp1ac = new TH2D("hp1ac","timecut 1 + ampcut;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp1ac",timecut1_p + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hp2ac = new TH2D("hp2ac","timecut 2 + ampcut;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2ac",timecut2_p + " & " + pampcut,"contz");
    for(auto l: stripBounds) l->Draw("same");
    DrawProfiles();
    c->Update();

    /////////////////////// Residual hit positions

    DrawHitResiduals();
    /*
    /////////////// Time vs pad
    {
        TTreeReader reader(fPadTree);
        TTreeReaderValue<int> col(reader, "col");
        TTreeReaderValue<int> row(reader, "row");
        TTreeReaderValue<double> time(reader, "time");
        TTreeReaderValue<double> amp(reader, "amp");

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

        TH2D *hptime1 = new TH2D("hptime1","timecut 1;row;col",576,0,576,32,0,32);
        // hptime1->SetContour(nb1, t1levels);
        hptime1->SetContour(100);
        hptime1->GetZaxis()->SetRangeUser(tb1b,tb1t);
        TH2D *hptime2 = new TH2D("hptime2","timecut 2;row;col",576,0,576,32,0,32);
        // hptime2->SetContour(nb2, t2levels);
        hptime2->GetZaxis()->SetRangeUser(tb2b,tb2t);
        hptime2->SetContour(100);

        while (reader.Next()) {
            if(*amp < 4000){    // remove overflows
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
        }

        double occCut = 2./(hp1->GetNbinsX()*hp1->GetNbinsY());

        TH1D *hppt1 = new TH1D("hppt1","timecut 1;time [ns]; counts",nb1,tb1b,tb1t);
        TH1D *hppt2 = new TH1D("hppt2","timecut 2;time [ns]; counts",nb2,tb2b,tb2t);
        for(int bx = 0; bx <= hp1->GetNbinsX(); bx++){
            for(int by = 0; by <= hp1->GetNbinsY(); by++){
                double n = hp1NoOF->GetBinContent(bx,by);
                if(n > 0){
                    if(n > occCut*hp1->GetEntries()){
                        hptime1->SetBinContent(bx,by, hptime1->GetBinContent(bx,by)/n);
                        hppt1->Fill(hptime1->GetBinContent(bx,by));
                    } else
                        hptime1->SetBinContent(bx,by,0);
                }
                n = hp2NoOF->GetBinContent(bx,by);
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

    ////////////////////// Hitpattern for upper and lower peak of drift time peak
    c = new TCanvas("cpadsbytime","pads t < 5300 or t > 5330",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hp2early = new TH2D("hp2early","timecut 2 + t < 5300;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2early",timecut2_p + " & " + pOFcut + " & time < 5300","contz");
    for(auto l: stripBounds) l->Draw("same");
    DrawProfiles();
    c->cd(2);
    TH2D *hp2late = new TH2D("hp2late","timecut 2 + t > 5330;row;col",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2late",timecut2_p + " & " + pOFcut + " & time > 5330","contz");
    for(auto l: stripBounds) l->Draw("same");
    DrawProfiles();
    c->Update();

    ////////////////////// Amplitude for those two time peaks
    c = new TCanvas("cpadabytime","pad amp t < 5300 or t > 5330",800,600);
    TH1D *hpa2early = new TH1D("hpa2early","timecut 2 + t < 5300;amp",4160,0,4160);
    fPadTree->Draw("amp>>hpa2early",timecut2_p + " & " + pOFcut + " & time < 5300");
    TH1D *hpa2late = new TH1D("hpa2late","timecut 2 + t > 5330;amp",4160,0,4160);
    hpa2late->SetLineColor(kRed);
    fPadTree->Draw("amp>>hpa2late",timecut2_p + " & " + pOFcut + " & time > 5330","same");
    c->BuildLegend();
    c->Update();

    //////////////////////// Amplitude and time vs pad column/row
    c = new TCanvas("cpad2","pad col amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpc1 = new TH2D("hpc1","timecut 1;col;amp",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc1",timecut1_p,"colz");
    c->cd(2);
    TH2D *hpc2 = new TH2D("hpc2","timecut 2;col;amp",32,0,32,520,0,4160);
    fPadTree->Draw("amp:col>>hpc2",timecut2_p,"colz");
    c->Update();

    c = new TCanvas("cpad2t","pad col time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1 = new TH2D("hpct1","timecut 1;col;time [ns]",32,0,32,800,1200,2000);
    fPadTree->Draw("time:col>>hpct1",timecut1_p + " & " + pOFcut,"colz");
    c->cd(2);
    TH2D *hpct2 = new TH2D("hpct2","timecut 2;col;time [ns]",32,0,32,500,5100,5600);
    fPadTree->Draw("time:col>>hpct2",timecut2_p + " & " + pOFcut,"colz");
    c->Update();

    c = new TCanvas("cpad2tac","pad col time ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpct1ac = new TH2D("hpct1ac","timecut 1 + ampcut;col;time [ns]",32,0,32,800,1200,2000);
    fPadTree->Draw("time:col>>hpct1ac",timecut1_p + " & " + pOFcut + " & " + pampcut,"colz");
    c->cd(2);
    TH2D *hpct2ac = new TH2D("hpct2ac","timecut 2 + ampcut;col;time [ns]",32,0,32,500,5100,5600);
    fPadTree->Draw("time:col>>hpct2ac",timecut2_p + " & " + pOFcut + " & " + pampcut,"colz");
    c->Update();

    c = new TCanvas("cpad3","pad row amp",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpr1 = new TH2D("hpr1","timecut 1;row;amp",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr1",timecut1_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hpr2 = new TH2D("hpr2","timecut 2;row;amp",576,0,576,520,0,4160);
    fPadTree->Draw("amp:row>>hpr2",timecut2_p,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    c = new TCanvas("cpad3t","pad row time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hprt1 = new TH2D("hprt1","timecut 1;row;time [ns]",576,0,576,800,1200,2000);
    fPadTree->Draw("time:row>>hprt1",timecut1_p + " & " + pOFcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hprt2 = new TH2D("hprt2","timecut 2;row;time [ns]",576,0,576,500,5100,5600);
    fPadTree->Draw("time:row>>hprt2",timecut2_p + " & " + pOFcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    TGraphErrors *g = new TGraphErrors;
    g->SetMarkerStyle(20);
    g->SetTitle("pad row time profile;row;time [ns]");
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
    fPadTree->Draw("time:row>>hprt1ac",timecut1_p + " & " + pOFcut + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    TH2D *hprt2ac = new TH2D("hprt2ac","timecut 2 + ampcut",576,0,576,500,5100,5600);
    fPadTree->Draw("time:row>>hprt2ac",timecut2_p + " & " + pOFcut + " & " + pampcut,"colz");
    for(auto l: stripBounds) l->Draw("same");
    c->Update();

    ////////////////////////// Time vs amplitude
    c = new TCanvas("cpadat","pad amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hpat1 = new TH2D("hpat1","timecut 1",520,0,4160,800,1200,2000);
    fPadTree->Draw("time:amp>>hpat1",timecut1_p + " & " + pOFcut,"colz");
    c->cd(2);
    TH2D *hpat2 = new TH2D("hpat2","timecut 2",520,0,4160,500,5100,5600);
    fPadTree->Draw("time:amp>>hpat2",timecut2_p + " & " + pOFcut,"colz");
    c->Update();
    */
}

void awAnalysis(TTree *fAnodeTree){
    TCanvas *c = new TCanvas("caw","AW",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *ha1 = new TH2D("ha1","timecut 1",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha1",timecut1_a + " & " + awOFcut,"colz");
    c->cd(2);
    TH2D *ha2 = new TH2D("ha2","timecut 2",256,256,512,700,0,70000);
    fAnodeTree->Draw("amp:wire>>ha2",timecut2_a + " & " + awOFcut,"colz");
    c->Update();

    c = new TCanvas("cawt","AW time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1 = new TH2D("hat1","timecut 1",256,256,512,800,1200,2000);
    fAnodeTree->Draw("time:wire>>hat1",timecut1_a + " & " + awOFcut,"colz");
    c->cd(2);
    TH2D *hat2 = new TH2D("hat2","timecut 2",256,256,512,500,5100,5600);
    fAnodeTree->Draw("time:wire>>hat2",timecut2_a + " & " + awOFcut,"colz");
    c->Update();

    c = new TCanvas("cawtac","AW ampcut",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hat1ac = new TH2D("hat1ac","timecut 1 + ampcut",256,256,512,800,1200,2000);
    fAnodeTree->Draw("time:wire>>hat1ac",timecut1_a + " & " + awOFcut + " & " + awampcut,"colz");
    c->cd(2);
    TH2D *hat2ac = new TH2D("hat2ac","timecut 2 + ampcut",256,256,512,500,5100,5600);
    fAnodeTree->Draw("time:wire>>hat2ac",timecut2_a + " & " + awOFcut + " & " + awampcut,"colz");
    c->Update();

    c = new TCanvas("cawat","aw amp time",1100,1100);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hawat1 = new TH2D("hawat1","timecut 1",700,0,70000,800,1200,2000);
    fAnodeTree->Draw("time:amp>>hawat1",timecut1_a + " & " + awOFcut,"colz");
    c->cd(2);
    TH2D *hawat2 = new TH2D("hawat2","timecut 2",700,0,70000,500,5100,5600);
    fAnodeTree->Draw("time:amp>>hawat2",timecut2_a + " & " + awOFcut,"colz");
    c->Update();
}

void LaserAnalysis(){
    TChain padChain("tpc_tree/fPadTree");
    TChain anodeChain("tpc_tree/fAnodeTree");
    for(TString fn: files){
        padChain.Add(fn);
        anodeChain.Add(fn);
    }
    vector<Long64_t> padtreeindex;
    int i(0);
    while( padChain.LoadTree(i) >= 0){
//        cout << i << '\t' << padChain.GetTree()->GetEntries() << endl;
        padtreeindex.push_back(i);
        i += padChain.GetTree()->GetEntries();
    }
    padChain.ls();
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
    // // TESTPLOT
    // new TCanvas;
    // TF1 *f1 = new TF1("fff",laser_profile_pad,0,_padrow,6);
    // double z0 = -1172.5,// -5.125-_halflength,          // rod z position
    //     Rc = 109.25,              // cathode radius
    //     Rr = Rc + 27.35,         // rod r position

    //     theta = 3.0, beta = -50.;
    // double zmin=-_halflength, zmax=_halflength; // mm
    // double phi_offset = 0.;
    // //double zmin=0., zmax=576.; // mm
    // f1->SetParameters(z0,Rc,Rr,theta,beta,phi_offset);
    // f1->SetParNames("z0","Rc","Rr","theta","beta","phi_offset");
    // f1->Draw();
    // TF1 *fclone = new TF1(*f1);
    // fclone->SetLineColor(kGreen);
    // fclone->SetParameter(5,20);
    // fclone->Draw("same");
}
