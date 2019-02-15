#include "../../reco/include/TPCconstants.hh"
#include <TMath.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TLegend.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TLine.h>
#include <TSpectrum.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TChain.h>
#include <TGraphErrors.h>

#include "LaserProfile.hh"

using std::set;
using namespace TMath;

bool allcols = false;           // if false, noisy pad column #1 gets suppressed

double strippitch = 265.;
double stripwidth = 6.;

TString awOFcut = "amp < 40000";// remove overflow
TString pOFcut = "amp < 4000";  // remove overflow
// TString awampcut = "amp > 10000";
// TString pampcut = "amp > 1000";

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
        {15, 346.3},             // FIXME: These are placeholder values
        {11, -256.3},
        {7, 166.3},
        {3, -76.3}
    };

vector<TString> files =
    {
        // "output02657.root",     // B15, different/empty?
        // "output02683.root",     // B15
        // "output02684.root",     // B15
        // "output02688.root",     // B07
        // "output02685.root",     // labeled T11, seems swapped to T03
        "output02686.root",     // labeled T11, seems swapped to T03
        "output02687.root"      // labeled T03, seems swapped to T11
    };

// TString timecut1_p, timecut2_p, timecut1_a, timecut2_a;

map<int, double> tp1, tp2, sigp1, sigp2, ta1, ta2, siga1, siga2;

map<int, TH2D*> hitpatterns_t1_p, hitpatterns_t1_noOF_p, hitpatterns_t2_p, hitpatterns_t2_noOF_p, timeVrow_p, timeVcol_p, ampVrow_p, ampVcol_p, timeVamp_p;

map<int, TH1D*> time_p, timeNoOF_p, time_a, timeNoOF_a;

int runNo(TString filename){
    filename.Remove(0,6);
    filename.Remove(filename.Length()-5);
    return filename.Atoi();
}

set<double> GetStrips(){
    set<double> strips;
    for(double x = 0; x < _halflength; x += strippitch){
        strips.insert(x);
        strips.insert(-x);
    }
    return strips;
}

int DrawProfiles(pair<char, int> port){
    double phi_offset = portOffset[port.second];
    vector<TF1*> profiles = GetPadProfile(phi_offset);
    for(TF1 *p: profiles){
        p->Draw("same");
    }
    return profiles.size();
}

int timeSpec(TTree *pt, TTree *at, int run){
    int nawbin = 7000/awbin;
    int awmax = awbin*nawbin;
    TString hn = TString::Format("hat%d",run);
    TH1D *hat = new TH1D(hn.Data(),"anode times;time [ns]; counts",nawbin,0,awmax);
    TH1D *hatNoOF = new TH1D(hn+"NoOF","anode times, no amp overflow;time [ns]; counts",nawbin,0,awmax);
    hatNoOF->SetFillStyle(3001);
    hatNoOF->SetFillColor(hatNoOF->GetLineColor());
    hat->SetBit(TH1::kNoTitle);
    hatNoOF->SetBit(TH1::kNoTitle);

    TString drawString = "time >> ";
    at->Draw(drawString + hn,"","0");
    at->Draw(drawString + hn + "NoOF", awOFcut, "0");

    int npadbin = 7000/padbin;
    int padmax = padbin*npadbin;
    hn = TString::Format("hpt%d",run);
    TH1D *hpt = new TH1D(hn,"pad times;time [ns]; counts",npadbin,0,padmax);
    TH1D *hptNoOF = new TH1D(hn+"NoOF","pad times, no amp overflow;time [ns]; counts",npadbin,0,padmax);
    hpt->SetLineColor(kRed);
    hptNoOF->SetLineColor(kRed);
    hptNoOF->SetFillColor(kRed);
    hptNoOF->SetFillStyle(3003);
    hpt->SetBit(TH1::kNoTitle);
    hptNoOF->SetBit(TH1::kNoTitle);
    pt->Draw(drawString + hn,"","0");
    pt->Draw(drawString + hn + "NoOF", pOFcut, "0");

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
    hatNoOF->Fit(fa,"0");

    tp1[run] = fp->GetParameter(1);
    sigp1[run] = fp->GetParameter(2);
    tp2[run] = fp->GetParameter(4);
    sigp2[run] = fp->GetParameter(5);

    ta1[run] = fa->GetParameter(1);
    siga1[run] = fa->GetParameter(2);
    ta2[run] = fa->GetParameter(4);
    siga2[run] = fa->GetParameter(5);

    time_p[run] = hpt;
    timeNoOF_p[run] = hptNoOF;
    time_a[run] = hat;
    timeNoOF_a[run] = hatNoOF;
    return 0;
}

int hitPattern_p(TTree *pt, int run){
    if(!tp1.count(run)){
        cerr << "Time analysis hasn't been run for Run " << run << ". Run timeSpec() before hitpattern_p()." << endl;
        return -1;
    }

    set<double> strips = GetStrips();
    vector<TLine*> stripBounds;
    for(auto s: strips){
        double x1 = mm2pad(s-0.5*stripwidth);
        double x2 = mm2pad(s+0.5*stripwidth);
        stripBounds.push_back(new TLine(x1,-10000,x1,10000));
        stripBounds.back()->SetLineColor(kGray);
        stripBounds.push_back(new TLine(x2,-10000,x2,10000));
        stripBounds.back()->SetLineColor(kGray);
    }

    double t1_1 = tp1[run] - sigmas*sigp1[run];
    double t1_2 = tp1[run] + sigmas*sigp1[run];
    double t2_1 = tp2[run] - sigmas*sigp2[run];
    double t2_2 = tp2[run] + sigmas*sigp2[run];
    TString timecut = "time > %f && time < %f && col != 1";
    if(allcols) timecut = "time > %f && time < %f";
    TString cut_t1 = TString::Format(timecut, t1_1, t1_2);
    TString cut_t2 = TString::Format(timecut, t2_1, t2_2);

    /////////////// Hitpattern
    TString hn = TString::Format("hphit%d",run);
    TH2D *hp1 = new TH2D(hn+"_t1","timecut 1;row;col",576,0,576,32,0,32);
    TH2D *hp2 = new TH2D(hn+"_t2","timecut 2;row;col",576,0,576,32,0,32);
    TH2D *hp1NoOF = new TH2D(hn+"_t1"+"NoOF","timecut 1;row;col",576,0,576,32,0,32);
    TH2D *hp2NoOF = new TH2D(hn+"_t2"+"NoOF","timecut 2;row;col",576,0,576,32,0,32);

    TH2D *hp2res = new TH2D(hn+"_t2_res", "hit residual;row;col - col_{nominal}", _padrow, 0, _padrow, 21, -10.5,10.5);

    TString drawstring = "col:row>>";
    pt->Draw(drawstring + hn+"_t1"+"NoOF",cut_t1 + " & " + pOFcut,"0");
    pt->Draw(drawstring + hn+"_t1",cut_t1,"0");
    pt->Draw(drawstring + hn+"_t2"+"NoOF",cut_t2 + " & " + pOFcut,"0");
    pt->Draw(drawstring + hn+"_t2",cut_t2,"0");

    TTreeReader reader(pt);
    TTreeReaderValue<int> col(reader, "col");
    TTreeReaderValue<int> row(reader, "row");
    TTreeReaderValue<double> time(reader, "time");
    TTreeReaderValue<double> amp(reader, "amp");

    double phi_offset = portOffset[portmap[run].second];
    vector<TF1*> profiles = GetPadProfile(phi_offset);
    while (reader.Next()) {
        if(allcols || *col != 1){
            if(abs(*time - tp2[run]) < sigmas*sigp2[run]){
                double delta = 100000.;
                for(auto &p: profiles){
                    double d = *col - p->Eval(double(*row));
                    if(abs(d) < abs(delta)) delta = d;
                }
                double r = *row;
                // h->Fill(r, col2deg(delta));
                hp2res->Fill(r, delta);
            }
        }
    }

    vector<TH1D*> p;
    // TF1 *fpr = new TF1("fpr","gaus(0)+gaus(3)+gaus(6)",hp2res->GetYaxis()->GetXmin(),hp2res->GetYaxis()->GetXmax());
    // for(int i = 0; i < 3; i++){
    //     fpr->SetParameter(3*i, 0.1*hp2res->GetMaximum());
    //     fpr->SetParameter(3*i+1, -4+4*i);
    //     fpr->SetParameter(3*i+2, 2);
    // }
    for(auto s: strips){
        int bin = hp2res->GetXaxis()->FindBin(mm2pad(s));
        p.push_back(hp2res->ProjectionY(TString::Format("%s_t2_res_proj_%d",hn.Data(),int(mm2pad(s))),bin-5,bin+5));
        cout << hn << '\t' << s << '\t' << p.back()->GetMean() << '\t' << p.back()->GetRMS() << endl;
    }

    TDirectory *cwd = gROOT->CurrentDirectory();
    gROOT->cd();
#ifdef LARSALIAS
    TCanvas *c = new TCanvas(NewCanvasName(),"",1000,1000); // I'm shocked root doesn't have a function for this... Just create a new name for a canvas when this constructor is used.
#else
    TCanvas *c = new TCanvas;
    c->SetCanvasSize(1000,1000); // This works, but doesn't resize the window...
#endif

    c->Divide(1,3);
    c->cd(1);
    hp1->DrawCopy("colz");
    for(auto l: stripBounds) l->Draw("same");
    c->cd(2);
    hp2->DrawCopy("contz");
    for(auto l: stripBounds) l->Draw("same");
    DrawProfiles(portmap[run]);
    c->cd(3);
    hp2res->DrawCopy("contz");     // This one disappears if there's more than one run. No idea. Canvas gets emptied when output file is closed...
    for(auto l: stripBounds) l->Draw("same");
    // c->cd(1);

    // c->Update();
    c->SaveAs(hn+".pdf");

    TGraph *g = new TGraph;
    g->SetName(hn+"_res_peakpos");
    g->SetMarkerStyle(5);
    for(unsigned int i = 0; i < p.size(); i++){
        new TCanvas;
        int colour = i+1;
        if(colour == 5) colour += p.size(); // That yellow is invisible...
        p[i]->SetLineColor(colour);
        TH1D *ppp = (TH1D*)p[i]->Clone();
        TSpectrum sp(3);
        int np = sp.Search(ppp,1);//,"nobackground new");
        cout << p[i]->GetName() << '\t' << np << " peaks" << endl;
        Double_t *x = sp.GetPositionX();
        Double_t *y = sp.GetPositionY();
        if(false){              // Fit multiple gaussians. Maybe try with better statistics runs, fails often
            TString fn;
            for(int ii = 0; ii < np; ii++){
                fn += ii?"+":"";
                fn += TString::Format("gaus(%d)",3*ii);
            }
            cout << fn << endl;
            TF1 *fpr = new TF1("fpr",fn,ppp->GetXaxis()->GetXmin(),ppp->GetXaxis()->GetXmax());
            for(int ii = 0; ii < np; ii++){
                fpr->SetParameter(ii*3, y[ii]);
                fpr->SetParameter(ii*3+1, x[ii]);
                fpr->SetParameter(ii*3+2, 1);
            }
            // ppp->Draw();
            // fpr->DrawCopy("same");
            ppp->Fit(fpr);
        } else {
            vector<int> peakorder;
            for(int i = 0; i < np; i++){
                auto it = peakorder.begin();
                while(it != peakorder.end()){
                    if(abs(x[i]) < abs(x[*it])) break;
                    it++;
                }
                peakorder.insert(it,i);
            }
            // for(auto p: peakorder){
            //     cout << p << '\t' << x[p] << endl;
            // }
            auto it = strips.begin();
            for(int j = 0; j < i; j++)
                it++;
            double padrow = mm2pad(*it);

            if(peakorder.size())
                g->SetPoint(g->GetN(),padrow,peakorder[0]);
            if(peakorder.size() > 1){
                if(abs(abs(x[peakorder[1]]) - abs(x[peakorder[0]])) < 0.5*abs(x[peakorder[0]]))
                    g->SetPoint(g->GetN(),padrow,peakorder[1]);
            }
        }
        // p[i]->DrawCopy(i?"same":"");
    }
    TCanvas *cc = new TCanvas;
    g->Draw("ap");
    g->GetHistogram()->GetYaxis()->SetRangeUser(0.5*hp2res->GetYaxis()->GetXmin(),0.5*hp2res->GetYaxis()->GetXmax());
    g->GetHistogram()->SetXTitle("row");
    g->GetHistogram()->SetYTitle("col - col_{nominal} for closest peak to nominal");
    cc->SetGrid();
    cc->Update();
    cwd->cd();
    g->Write();

    hitpatterns_t1_p[run] = hp1;
    hitpatterns_t2_p[run] = hp2;
    hitpatterns_t1_noOF_p[run] = hp1NoOF;
    hitpatterns_t2_noOF_p[run] = hp2NoOF;
    return 0;
}

int LaserAnalysis_new(){
    map<pair<char,int>, vector<TString> > portFiles;
    for(auto fn: files){
        int run = runNo(fn);
        if(portmap.count(run)){
            portFiles[portmap[run]].push_back(fn);
        } else {
            cerr << "Run " << run << " is not listed in portmap (in macro file)" << endl;
        }
    }
    TFile fout("anaOut.root","RECREATE");
    if(!fout.IsOpen()){
        cerr << "Couldn't open output file." << endl;
        return -1;
    }
    for(auto fn: files){
        TFile f(fn);
        TTree *pt = (TTree*)f.Get("tpc_tree/fPadTree");
        TTree *at = (TTree*)f.Get("tpc_tree/fAnodeTree");
        int run = runNo(fn);

        fout.cd();
        timeSpec(pt, at, run);

        fout.cd();
        hitPattern_p(pt, run);
        TCanvas c;
    }

    if(time_p.size()){
        fout.cd();
        TH1D *htimesum_p = nullptr;
        TH1D *htimesum_NoOF_p = nullptr;
        for(auto h: time_p){
            if(!htimesum_p){
                htimesum_p = new TH1D(*h.second);
                htimesum_p->SetName("htimesum_p");
            } else {
                htimesum_p->Add(h.second);
            }
        }
        for(auto h: timeNoOF_p){
            if(!htimesum_NoOF_p){
                htimesum_NoOF_p = new TH1D(*h.second);
                htimesum_NoOF_p->SetName("htimesum_NoOF_p");
            } else {
                htimesum_NoOF_p->Add(h.second);
            }
        }
    }
    if(hitpatterns_t1_p.size()){
        fout.cd();
        TH2D *hhitsum_t1_p = nullptr;
        TH2D *hhitsum_t1_NoOF_p = nullptr;
        for(auto hp: hitpatterns_t1_p){
            if(!hhitsum_t1_p){
                hhitsum_t1_p = new TH2D(*hp.second);
                hhitsum_t1_p->SetName("hphitsum_t1");
            } else {
                hhitsum_t1_p->Add(hp.second);
            }
        }
        for(auto hp: hitpatterns_t1_noOF_p){
            if(!hhitsum_t1_NoOF_p){
                hhitsum_t1_NoOF_p = new TH2D(*hp.second);
                hhitsum_t1_NoOF_p->SetName("hphitsum_t1_NoOF");
            } else {
                hhitsum_t1_NoOF_p->Add(hp.second);
            }
        }

        TH2D *hhitsum_t2_p = nullptr;
        TH2D *hhitsum_t2_NoOF_p = nullptr;
        for(auto hp: hitpatterns_t2_p){
            if(!hhitsum_t2_p){
                hhitsum_t2_p = new TH2D(*hp.second);
                hhitsum_t2_p->SetName("hphitsum_t2");
            } else {
                hhitsum_t2_p->Add(hp.second);
            }
        }
        for(auto hp: hitpatterns_t2_noOF_p){
            if(!hhitsum_t2_NoOF_p){
                hhitsum_t2_NoOF_p = new TH2D(*hp.second);
                hhitsum_t2_NoOF_p->SetName("hphitsum_t2_NoOF");
            } else {
                hhitsum_t2_NoOF_p->Add(hp.second);
            }
        }
    }
    fout.Write();
    fout.Close();
    return 0;
}
