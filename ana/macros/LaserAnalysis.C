#include "../../recolib/include/TPCconstants.hh"
#include <TMath.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
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
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TCutG.h>
#include <TCut.h>
#include <TFitResult.h>
#include <TProfile.h>
#include <TPolyMarker.h>

#include "LaserProfile.hh"

using std::set;
using namespace TMath;

bool allcols = false;           // if false, difficult pad column #1 gets suppressed

const TCut awOFcut = "amp < 40000";// remove overflow
const TCut awNoisecut = "amp > 10000";// remove pickup
const TCut pOFcut = "amp < 4000";  // remove overflow
// TCut awampcut = "amp > 10000";
// TCut pampcut = "amp > 1000";

int awbin = 10;
int nawbin = 701;
const int padbin = 16;
const int npadbin = 512;

const double sigmas = 4.;             // how many sigma around peak should time cut go

const double padColTol = 2.;          // How many pad columns around expected hit line should still be expected as "real"

const double padRowTol = 5.;          // How many pad row around Al strips should be considered "real"

int awTol = 4;

double phi_shift = 0;
// 2.93;  // determined from AW fit of runs 2301-2303
//col2deg(-1.72); // determined from pad fit at end of macro

double phi_lorentz = 0.;        // expected Lorentz displacement due to nominal magnetic field

map<int,pair<char,int> > portmap =
    {
        // TRIUMF
        {2301, pair<char,int>('T',11)},
        {2302, pair<char,int>('T',11)},
        {2303, pair<char,int>('T',11)},
        {2304, pair<char,int>('T',3)},
        {2305, pair<char,int>('T',3)},
        {2306, pair<char,int>('T',3)},
        {2307, pair<char,int>('B',15)},
        {2308, pair<char,int>('B',15)},
        {2309, pair<char,int>('B',15)},
        {2310, pair<char,int>('B',7)},
        {2312, pair<char,int>('B',7)},
        {2313, pair<char,int>('B',7)},
        {2314, pair<char,int>('B',7)},
        {2315, pair<char,int>('B',7)},
        {2316, pair<char,int>('B',7)},
        {2317, pair<char,int>('B',7)},
        // CERN
        {2657, pair<char,int>('B',15)},
        {2683, pair<char,int>('B',15)},
        {2684, pair<char,int>('B',15)},
        {2685, pair<char,int>('T',3)},     // labeled T11, seems swapped to T03
        {2686, pair<char,int>('T',3)},     // labeled T11, seems swapped to T03
        {2687, pair<char,int>('T',11)},      // labeled T03, seems swapped to T11
        {2688, pair<char,int>('B',7)},
        {4382, pair<char,int>('B',7)},
        {4388, pair<char,int>('B',7)},
        {4389, pair<char,int>('T',11)},
        {4396, pair<char,int>('T',11)},
        {4399, pair<char,int>('T',3)},
        {4400, pair<char,int>('T',3)},
        {4401, pair<char,int>('B',15)},
        {4402, pair<char,int>('B',15)},
        {4403, pair<char,int>('B',15)},
        {4404, pair<char,int>('B',15)},
        {4407, pair<char,int>('B',15)},
        {4408, pair<char,int>('B',15)},
        {4409, pair<char,int>('B',15)},
    };

map<pair<char,int>, double> portTheta = // ONLY VALID FOR RUNS 2301-2317
    {
        {pair<char,int>('T',11), 4.11},
        {pair<char,int>('T',3), 4.52},
        {pair<char,int>('B',7), 1.35},
        {pair<char,int>('B',15), 2.48}
    };


vector<TString> files =
    {
        // "output02301.root",     // TRIUMF
        // "output02302.root",     // TRIUMF
        // "output02303.root",     // TRIUMF
        // "output02304.root",     // TRIUMF
        // "output02305.root",     // TRIUMF
        // "output02306.root",     // TRIUMF
        // "output02307.root",     // TRIUMF
        // "output02308.root",     // TRIUMF
        // "output02309.root",     // TRIUMF
        // "output02310.root",     // TRIUMF
        // "output02312.root",     // TRIUMF
        // "output02313.root",     // TRIUMF
        // "output02314.root",     // TRIUMF
        // "output02315.root",     // TRIUMF
        // "output02316.root",     // TRIUMF
        // "output02317.root",     // TRIUMF

        // "output02683.root",     // B15
        // "output02684.root",     // B15
        // "output02688.root",     // B07
        // "output02685.root",     // labeled T11, seems swapped to T03
        // "output02686.root",     // labeled T11, seems swapped to T03
        // "output02687.root"      // labeled T03, seems swapped to T11
        // "output04382.root",     // B07
        // "output04388.root"     // B07
        // "output04396sub000.root"     // T11
        // "lmtmp/output04382.root",     // B07
      // "output04399.root",
      // "output04400.root",
      // "output04401.root",
      "output04402.root",
      // "output04403.root",
      // "output04404.root",
      // "output04407.root",
      // "output04408.root",
      // "output04409.root"
    };

// TCut timecut1_p, timecut2_p, timecut1_a, timecut2_a;

map<int, double> tp1, tp2, sigp1, sigp2, ta1, ta2, siga1, siga2, tda, tdp, stda, stdp, pcolOff, pcolOffSig, awshift, awshiftErr, thetaFit;

map<int, TH2D*> hitpatterns_t1_p, hitpatterns_t1_noOF_p, hitpatterns_t2_p, hitpatterns_t2_noOF_p, timeVrow_p, timeVcol_p, ampVrow_p, ampVcol_p, timeVamp_p;

map<int, TH1D*> time_p, timeNoOF_p, time_a, timeNoOF_a;

map<pair<char,int>, TCutG*> hitcuts;

map<int, TCut> timecut1, timecut2;

TCut colcut("");

map<pair<char,int>, map<double, TH2D*> > pad_amp_time; // grouped by port and strip, but not by run, so runs at different intensities get added

map<int, set<double> > brightStrips_p;

TGraphErrors *gPortTime_p = nullptr;
TGraphErrors *gPortTime_a = nullptr;
TGraphErrors *gPortTime_diff = nullptr;

TH2D *hpdt, *hpdtct;

TDirectory *timedir(nullptr), *paddir(nullptr), *awdir(nullptr);

//////////////// Helper functions

int runNo(TString filename){
    filename.Remove(0,6);
    filename.Remove(filename.Length()-5);
    int run = filename.Atoi();
    if(run < 3000){
      nawbin = 701;
      awbin = 10;
    } else {
      nawbin = 511;
      awbin = 16;
    }
    return filename.Atoi();
}

TGraph *LightPointsGraph_p(pair<char, int> port){
    double phi_offset = portOffset[port.second];
    TGraph *g = new TGraph;
    vector<pair<double, double> > points = GetLightPoints(phi_offset, phi_lorentz+phi_shift, portTheta[port], port.first=='T');
    for(auto &p: points){
        double r = mm2pad(p.first);
        double c = deg2col(p.second);
        g->SetPoint(g->GetN(),r,c);
    }
    g->SetMarkerStyle(4);
    g->SetMarkerSize(2.5);
    return g;
}

vector<TLine*> HitAnodes(pair<char, int> port, double ymin, double ymax){
    double phi_offset = portOffset[port.second];
    vector<TLine*> lines;
    vector<pair<double, double> > points = GetLightPoints(phi_offset, phi_lorentz+phi_shift, portTheta[port]);
    for(auto &p: points){
        double aw = deg2aw(p.second);
        lines.push_back(new TLine (aw,ymin,aw,ymax));
        lines.back()->SetLineColor(kRed);
    }
    return lines;
}

//////////////// Graphical cut, limiting to expected hit positions. FIXME: currently only B=0

map<pair<char,int>, TCutG*> GetHitCuts(){
    if(hitcuts.size()) return hitcuts;
    for(auto &fn: files){
        int run = runNo(fn);
        if(portmap.count(run)){
            auto port = portmap[run];
            if(!hitcuts.count(port)){
                double phi_offset = portOffset[port.second];
                vector<TF1*> profiles = GetPadProfile(phi_offset, phi_lorentz+phi_shift, portTheta[port], port.first=='T');
                TString cn = TString::Format("cut_port%c%02d", port.first, port.second);
                TCutG *cg = new TCutG(cn,2*_padrow);
                cg->SetVarX("row");
                cg->SetVarY("col");
                for(int x = 0; x < _padrow; x++){
                    double y = profiles[0]->Eval(x) - padColTol;
                    if(profiles.size() > 1){
                        if(y < 0 || y > _padcol){
                            y = profiles[1]->Eval(x) - padColTol;
                        }
                    }
                    cg->SetPoint(cg->GetN(), x, y);
                }
                for(int x = _padrow - 1; x >= 0; x--){
                    double y = profiles[0]->Eval(x) + padColTol;
                    if(profiles.size() > 1){
                        if(y < 0 || y > _padcol){
                            y = profiles[1]->Eval(x) + padColTol;
                        }
                    }
                    cg->SetPoint(cg->GetN(), x, y);
                }
                hitcuts[port] = cg;
            }
        }
    }
    return hitcuts;
}

//////////////// Overall time spectra

int timeSpec(TTree *pt, TTree *at, int run){
    timedir->cd();
    auto port = portmap[run];
    int awmax = awbin*nawbin;
    TString hn = TString::Format("hat%d",run);
    TH1D *hat = new TH1D(hn.Data(),"anode times;time [ns]; counts",nawbin,0,awmax);
    TH1D *hatNoOF = new TH1D(hn+"NoOF","anode times, no amp overflow;time [ns]; counts",nawbin,0,awmax);
    TH1D *hadtNoOF = new TH1D(hn+"NoOFdiff","anode diff times, no amp overflow;time [ns]; counts",nawbin+100,-100.*awbin,awmax);
    hatNoOF->SetFillStyle(3001);
    hatNoOF->SetFillColor(hatNoOF->GetLineColor());
    hat->SetBit(TH1::kNoTitle);
    hatNoOF->SetBit(TH1::kNoTitle);

    TString drawString = "time >> ";
    at->Draw(drawString + hn,"","0");
    at->Draw(drawString + hn + "NoOF", awOFcut, "0");
    drawString = "dtime >> ";
    at->Draw(drawString + hn + "NoOFdiff", awOFcut, "0");
    drawString = "time >> ";

    int padmax = padbin*npadbin;
    hn = TString::Format("hpt%d",run);
    TH1D *hpt = new TH1D(hn,"pad times;time [ns]; counts",npadbin,0,padmax);
    TH1D *hptNoOF = new TH1D(hn+"NoOF","pad times, no amp overflow;time [ns]; counts",npadbin,0,padmax);
    TH1D *hpdtNoOF = new TH1D(hn+"NoOFdiff","pad diff times, no amp overflow;time [ns]; counts",npadbin+100,-100.*padbin,padmax);
    hpt->SetLineColor(kRed);
    hptNoOF->SetLineColor(kRed);
    hptNoOF->SetFillColor(kRed);
    hptNoOF->SetFillStyle(3003);
    hpt->SetBit(TH1::kNoTitle);
    hptNoOF->SetBit(TH1::kNoTitle);
    pt->Draw(drawString + hn,"","0");
    pt->Draw(drawString + hn + "NoOF", pOFcut, "0");
    drawString = "dtime >> ";
    pt->Draw(drawString + hn + "NoOFdiff", pOFcut, "0");

    TSpectrum sp(3);
    int np = sp.Search(hptNoOF,30,"",0.005);
    // TF1 *fp = new TF1("fp","gaus(0)+gaus(3)",hpt->GetXaxis()->GetXmin(),hpt->GetXaxis()->GetXmax());
    TF1 *fp = new TF1("fp","[0]/((exp((([1]-[2])-x)/[3])+1)*(exp((x-([1]+[2]))/[3])+1)) + [4]/((exp((([5]-[6])-x)/[7])+1)*(exp((x-([5]+[6]))/[7])+1))",hpt->GetXaxis()->GetXmin(),hpt->GetXaxis()->GetXmax());
    fp->SetLineStyle(2);
    fp->SetLineColor(kRed);
    Double_t *x = sp.GetPositionX();
    Double_t *y = sp.GetPositionY();
    cout << "************** pad time fit ***************" << endl;
    for(int i = 0; i < np; i++){
        fp->SetParameter(i*4, y[i]);
        fp->SetParameter(i*4+1, x[i]);
        fp->SetParameter(i*4+2, 50);
        fp->SetParameter(i*4+3, 10);
    }
    fp->FixParameter(3,5);
    hptNoOF->Fit(fp,"");
    cout << "*******************************************" << endl;

//////////////// Anode time spectrum has electronic pickup, so fit doesn't work very well
    // TF1 *fa = new TF1("fa","gaus(0)+gaus(3)+gaus(6)+gaus(9)",hat->GetXaxis()->GetXmin(),hat->GetXaxis()->GetXmax());
    np = sp.Search(hatNoOF,30,"",0.002);
    TF1 *fa = new TF1("fa","[0]/((exp((([1]-[2])-x)/[3])+1)*(exp((x-([1]+[2]))/[3])+1)) + [4]/((exp((([5]-[6])-x)/[7])+1)*(exp((x-([5]+[6]))/[7])+1))",hpt->GetXaxis()->GetXmin(),hpt->GetXaxis()->GetXmax());
    fa->SetLineStyle(2);
    fa->SetLineColor(kBlack);
    for(int i = 0; i < np; i++){
        fa->SetParameter(i*4, y[i]);
        fa->SetParameter(i*4+1, x[i]);
        fa->SetParameter(i*4+2, 50);
        fa->SetParameter(i*4+3, 10);
    }
    fa->FixParameter(3,5);
    // for(int i = 0; i < np; i++){
    //     fa->SetParameter(i*3, y[i]);
    //     fa->SetParameter(i*3+1, x[i]);
    //     fa->SetParameter(i*3+2, 50);
    // }
    // // fa->FixParameter(1,fp->GetParameter(1));
    // fa->SetParameter(6,y[0]);
    // fa->SetParameter(7,x[0]-100);
    // fa->SetParameter(8,50);
    // fa->SetParameter(9,y[0]);
    // fa->SetParameter(10,x[0]+100);
    // fa->SetParameter(11,50);
    cout << "************** aw time fit ***************" << endl;
    hatNoOF->Fit(fa,"");
    cout << "******************************************" << endl;

//////////////// Parameters for time cuts, focussing on only prompt or only drift peak
    tp1[run] = fp->GetParameter(1);
    sigp1[run] = fp->GetParameter(2);
    tp2[run] = fp->GetParameter(5);
    sigp2[run] = fp->GetParameter(6);

    ta1[run] = fa->GetParameter(1);
    siga1[run] = fa->GetParameter(2);
    ta2[run] = fa->GetParameter(5);
    siga2[run] = fa->GetParameter(6);

    // ta1[run] = fa->GetParameter(1);
    // siga1[run] = fa->GetParameter(2);
    // ta2[run] = fa->GetParameter(4);
    // siga2[run] = fa->GetParameter(5);

//////////////// time distributions with per-event subtraction of t0 look much cleaner, at least for the drift peak
    TSpectrum sp2(1);
    hadtNoOF->GetXaxis()->SetRangeUser(2000, 7000);
    np = sp2.Search(hadtNoOF);
    double tpeak = 4000.;
    if(np){
        tpeak = sp2.GetPositionX()[0];
    }
    TF1 fg("fg","gaus",0,awmax);
    cout << "************** aw dtime fit ***************" << endl;
    hadtNoOF->GetXaxis()->SetRangeUser(tpeak - 200, tpeak + 200);
    hadtNoOF->Fit(&fg);
    tda[run] = fg.GetParameter(1);
    stda[run] = fg.GetParameter(2);
    hadtNoOF->GetXaxis()->UnZoom();
    double tdaErr = fg.GetParError(1);
    cout << "************** pad dtime fit ***************" << endl;
    hpdtNoOF->GetXaxis()->SetRangeUser(2000, 7000);
    np = sp2.Search(hpdtNoOF);
    tpeak = 4000.;
    if(np){
        tpeak = sp2.GetPositionX()[0];
    }
    hpdtNoOF->GetXaxis()->SetRangeUser(tpeak - 200, tpeak + 200);
    hpdtNoOF->Fit(&fg);
    tdp[run] = fg.GetParameter(1);
    stdp[run] = fg.GetParameter(2);
    hpdtNoOF->GetXaxis()->UnZoom();
    double tdpErr = fg.GetParError(1);
    cout << "******************************************" << endl;

//////////////// Plot anode and pad drift times and their difference vs laser port position
    if(!gPortTime_a){
        gPortTime_a = new TGraphErrors;
        gPortTime_a->SetMarkerStyle(7);
        gPortTime_a->SetMarkerColor(kOrange);
        gPortTime_a->SetLineColor(kOrange);
        gPortTime_a->SetName("g_port_time_a");
        gPortTime_a->GetXaxis()->SetTitle("laser port");
        gPortTime_a->GetYaxis()->SetTitle("time[ns]");
    }
    int point = gPortTime_a->GetN();
    double stagger = 0.1*(double(rand())/double(RAND_MAX) - 0.5);
    // gPortTime_a->SetPoint(point,port.second+stagger,ta2[run]-ta1[run]);
    gPortTime_a->SetPoint(point,port.second+stagger,tda[run]);
    gPortTime_a->SetPointError(point,0,tdaErr);

    if(!gPortTime_p){
        gPortTime_p = new TGraphErrors;
        gPortTime_p->SetMarkerStyle(7);
        gPortTime_p->SetLineStyle(0);
        gPortTime_p->SetName("g_port_time_p");
        gPortTime_p->GetXaxis()->SetTitle("laser port");
        gPortTime_p->GetYaxis()->SetTitle("time[ns]");
    }
    point = gPortTime_p->GetN();
    gPortTime_p->SetPoint(point,port.second+stagger,tdp[run]);
    gPortTime_p->SetPointError(point,0,tdpErr);

    if(!gPortTime_diff){
        gPortTime_diff = new TGraphErrors;
        gPortTime_diff->SetMarkerStyle(7);
        gPortTime_diff->SetLineStyle(0);
        gPortTime_diff->SetName("g_port_time_diff");
        gPortTime_diff->GetXaxis()->SetTitle("laser port");
        gPortTime_diff->GetYaxis()->SetTitle("time[ns]");
    }
    point = gPortTime_diff->GetN();
    gPortTime_diff->SetPoint(point,port.second+stagger,tdp[run]-tda[run]);
    gPortTime_diff->SetPointError(point,0,sqrt(tdaErr*tdaErr+tdpErr*tdpErr));

    time_p[run] = hpt;
    timeNoOF_p[run] = hptNoOF;
    time_a[run] = hat;
    timeNoOF_a[run] = hatNoOF;

    double t1_1 = tp1[run] - sigmas*sigp1[run];
    double t1_2 = tp1[run] + sigmas*sigp1[run];
    // double t2_1 = tp2[run] - sigmas*sigp2[run];
    // double t2_2 = tp2[run] + sigmas*sigp2[run];
    // double t2_1 = ta2[run] - sigmas*siga2[run];
    // double t2_2 = ta2[run] + sigmas*siga2[run];
    double t2_1 = 5400.;
    double t2_2 = 5900.;
    TString timecut = "time > %f && time < %f";
    timecut1[run] = TString::Format(timecut, t1_1, t1_2).Data();
    timecut2[run] = TString::Format(timecut, t2_1, t2_2).Data();

    cout << "Time cut 1: " << timecut1[run] << endl;
    cout << "Time cut 2: " << timecut2[run] << endl;
    return 0;
}

//////////////// Plot pad hitpatterns with time cuts, overlay expected illumination profile
int hitPattern_p(TTree *pt, int run){
    if(!tp1.count(run)){
        cerr << "Time analysis hasn't been run for Run " << run << ". Run timeSpec() before hitpattern_p()." << endl;
        return -1;
    }

    TDirectory *d = paddir->GetDirectory("hits");
    if(!d) d = paddir->mkdir("hits");
    d->cd();
    if(strips.size() == 0) strips = GetStrips();
    vector<TLine*> stripBounds;
    for(auto s: strips){
        double x1 = mm2pad(s-0.5*stripwidth);
        double x2 = mm2pad(s+0.5*stripwidth);
        stripBounds.push_back(new TLine(x1,-10000,x1,10000));
        stripBounds.back()->SetLineColor(kGray);
        stripBounds.push_back(new TLine(x2,-10000,x2,10000));
        stripBounds.back()->SetLineColor(kGray);
    }

    /////////////// Hitpattern
    TString hn = TString::Format("hphit%d",run);
    TH2D *hp1 = new TH2D(hn+"_t1","timecut 1;row;col", _padrow, -0.5, _padrow-0.5, _padcol, -0.5, _padcol-0.5);
    TH2D *hp2 = new TH2D(hn+"_t2","timecut 2;row;col", _padrow, -0.5, _padrow-0.5, _padcol, -0.5, _padcol-0.5);
    TH2D *hp1NoOF = new TH2D(hn+"_t1"+"NoOF","timecut 1;row;col", _padrow, -0.5, _padrow-0.5, _padcol, -0.5, _padcol-0.5);
    TH2D *hp2NoOF = new TH2D(hn+"_t2"+"NoOF","timecut 2;row;col", _padrow, -0.5, _padrow-0.5, _padcol, -0.5, _padcol-0.5);

    TH2D *hp2res = new TH2D(hn+"_t2_res", "hit residual;row;col - col_{nominal}", _padrow, -0.5, _padrow-0.5, 21, -10.5,10.5);

    TString drawstring = "col:row>>";
    pt->Draw(drawstring + hn+"_t1"+"NoOF",timecut1[run] && pOFcut,"0");
    pt->Draw(drawstring + hn+"_t1",timecut1[run],"0");
    pt->Draw(drawstring + hn+"_t2"+"NoOF",timecut2[run] && colcut && pOFcut,"0");
    pt->Draw(drawstring + hn+"_t2",timecut2[run] && colcut,"0");

    TTreeReader reader(pt);
    TTreeReaderValue<int> col(reader, "col");
    TTreeReaderValue<int> row(reader, "row");
    TTreeReaderValue<double> time(reader, "time");
    TTreeReaderValue<double> amp(reader, "amp");

    double phi_offset = portOffset[portmap[run].second];
    vector<TF1*> profiles = GetPadProfile(phi_offset, phi_lorentz+phi_shift, portTheta[portmap[run]], portmap[run].first=='T');

    while (reader.Next()) {
        if(allcols || *col != 2){
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

    for(auto p: profiles){
        hp2->GetListOfFunctions()->Add(new TF1(*p));
        hp2NoOF->GetListOfFunctions()->Add(new TF1(*p));
    }

    vector<TH1D*> p, p2;
    // TF1 *fpr = new TF1("fpr","gaus(0)+gaus(3)+gaus(6)",hp2res->GetYaxis()->GetXmin(),hp2res->GetYaxis()->GetXmax());
    // for(int i = 0; i < 3; i++){
    //     fpr->SetParameter(3*i, 0.1*hp2res->GetMaximum());
    //     fpr->SetParameter(3*i+1, -4+4*i);
    //     fpr->SetParameter(3*i+2, 2);
    // }
    TDirectory *dproj = paddir->GetDirectory("strip_proj");
    if(!dproj) dproj = paddir->mkdir("strip_proj");
    dproj->cd();
    for(auto s: strips){
        int bin = hp2res->GetXaxis()->FindBin(mm2pad(s));
        p.push_back(hp2res->ProjectionY(TString::Format("%s_t2_res_proj_%d",hn.Data(),int(mm2pad(s))),bin-5,bin+5));
        // cout << hn << '\t' << s << '\t' << p.back()->GetMean() << '\t' << p.back()->GetRMS() << endl;
    }
    d->cd();
    // TDirectory *cwd = gDirectory->CurrentDirectory();
    // gROOT->cd();

    TGraphErrors *grp = new TGraphErrors;
    grp->SetName(hn+"_peakpos");
    grp->GetXaxis()->SetTitle("row");
    grp->GetYaxis()->SetTitle("col");
    grp->SetMarkerStyle(20);
    // grp->SetMarkerColor(kGreen);
    // grp->SetLineColor(kGreen);
    grp->SetLineWidth(2);
    TGraph *g = new TGraph;
    g->SetName(hn+"_res_peakpos");
    g->GetXaxis()->SetTitle("row");
    g->GetYaxis()->SetTitle("col");
    g->SetMarkerStyle(20);

    dproj->cd();
    for(auto s: strips){
        int bin = hp2->GetXaxis()->FindBin(mm2pad(s));
        p2.push_back(hp2->ProjectionY(TString::Format("%s_t2_proj_%d",hn.Data(),int(mm2pad(s))),bin-5,bin+5));
        // cout << hn << '\t' << s << '\t' << p2.back()->GetMean() << '\t' << p2.back()->GetRMS() << endl;
    }

    for(unsigned int i = 0; i < p2.size(); i++){
        // new TCanvas;
        int colour = i+1;
        if(colour == 5) colour += p2.size(); // That yellow is invisible...
        p2[i]->SetLineColor(colour);
        TH1D *ppp = (TH1D*)p2[i]->Clone();
        TSpectrum sp(5);
        int np = sp.Search(ppp,1,"nobackground");
        cout << p2[i]->GetName() << '\t' << np << " peaks" << endl;
        Double_t *x = sp.GetPositionX();
        Double_t *y = sp.GetPositionY();

        auto it = strips.begin();
        for(unsigned int j = 0; j < i; j++)
            it++;
        double padrow = mm2pad(*it);

        vector<double> minDist;
        for(auto &p: profiles){
            minDist.push_back(10000.);
            double nomCol = p->Eval(padrow);
            for(int j = 0; j < np; j++){
                double d = abs(x[j] - nomCol);
                if(d < minDist.back()) minDist.back() = d;
            }
        }

        TF1 *prof = profiles[0];
        if(minDist.size() > 1){
            if(minDist[1] < minDist[0]){
                prof = profiles[1];
            }
        }
        double nomCol = prof->Eval(padrow);
        vector<int> peakorder;

        if(false){              // Fit multiple gaussians. Maybe try with better statistics runs, fails often
            if(np){
                TString fn;
                for(int ii = 0; ii < np; ii++){
                    fn += ii?"+":"";
                    fn += TString::Format("gaus(%d)",3*ii);
                }
                // cout << fn << endl;
                TF1 *fpr = new TF1("fpr",fn,ppp->GetXaxis()->GetXmin(),ppp->GetXaxis()->GetXmax());
                for(int ii = 0; ii < np; ii++){
                    fpr->SetParameter(ii*3, y[ii]);
                    fpr->SetParameter(ii*3+1, x[ii]);
                    fpr->SetParameter(ii*3+2, 0.7);
                }
                // ppp->Draw();
                // fpr->DrawCopy("same");
                cout << "************** hit fit "<< i << " *****************" << endl;
                int result = ppp->Fit(fpr);
                cout << "***************************************************" << endl;
                if(result == 0){
                    for(int ii = 0; ii < np; ii++){
                        auto it = peakorder.begin();
                        while(it != peakorder.end()){
                            if(abs(fpr->GetParameter(ii*3+1)-nomCol) < abs(fpr->GetParameter(*it*3+1)-nomCol)) break;
                            it++;
                        }
                        peakorder.insert(it,ii);
                    }
                    if(peakorder.size()){
                        grp->SetPoint(grp->GetN(),padrow,fpr->GetParameter(0*3+1));
                        double x0 = fpr->GetParameter(0*3+1)-nomCol;
                        g->SetPoint(g->GetN(),padrow,x0);
                        if(peakorder.size() > 1){
                            double x1 = fpr->GetParameter(1*3+1)-nomCol;
                            if(abs(abs(x1) - abs(x0)) < 0.5*abs(x0)){
                                grp->SetPoint(grp->GetN(),padrow,fpr->GetParameter(1*3+1));
                                g->SetPoint(g->GetN(),padrow,x1);
                            }
                        }
                    }
                }
            }
        } else {
            for(int ii = 0; ii < np; ii++){
                auto it = peakorder.begin();
                while(it != peakorder.end()){
                    if(abs(x[ii]-nomCol) < abs(x[*it]-nomCol)) break;
                    it++;
                }
                peakorder.insert(it,ii);
            }
            // for(auto p: peakorder){
            //     cout << p << '\t' << x[p] << endl;
            // }

            if(peakorder.size()){
                int point = grp->GetN();
                grp->SetPoint(point,padrow,x[peakorder[0]]);
                grp->SetPointError(point,0,0.25*np*abs(x[peakorder[0]]-nomCol)); // Weight points by how far they are from expected position and how unambiguous the choice was, i.e. how many peaks were found
                g->SetPoint(g->GetN(),padrow,x[peakorder[0]]-nomCol);
            }
            if(peakorder.size() > 1){
                if(abs(abs(x[peakorder[1]]-nomCol) - abs(x[peakorder[0]]-nomCol)) < 0.5*abs(x[peakorder[0]]-nomCol)){
                    int point = grp->GetN();
                    grp->SetPoint(point,padrow,x[peakorder[1]]);
                    grp->SetPointError(point,0,0.25*np*abs(x[peakorder[1]]-nomCol));
                    g->SetPoint(g->GetN(),padrow,x[peakorder[1]]-nomCol);
                }
            }
        }
        // p[i]->DrawCopy(i?"same":"");
    }
    d->cd();
    TCanvas *cc = new TCanvas;
    g->Draw("ap");
    g->GetHistogram()->GetYaxis()->SetRangeUser(0.5*hp2res->GetYaxis()->GetXmin(),0.5*hp2res->GetYaxis()->GetXmax());
    g->GetHistogram()->GetXaxis()->SetTitle("row");
    g->GetHistogram()->GetYaxis()->SetTitle("col - col_{nominal} for closest peak to nominal");
    cc->SetGrid();
    cc->Update();
    d->cd();

    cout << "************** profile fit run "<< run << " *****************" << endl;
    for(auto p: profiles){
        TF1 *pp = new TF1(*p);
        pp->SetLineColor(kBlack);
        for(int i = 0; i < 3; i++)
            pp->FixParameter(i, pp->GetParameter(i));
        pp->SetParLimits(3,pp->GetParameter(3)-2.,pp->GetParameter(3)+2.);
        // pp->SetParLimits(4,pp->GetParameter(4)-2.,pp->GetParameter(4)+2.);
        for(int i = 4; i < 8; i++)
            pp->FixParameter(i, pp->GetParameter(i));
        int status = grp->Fit(pp,"R+","nodraw");
        // cout << "** status = "<< status << ", theta = " << pp->GetParameter(3) << "+-" << pp->GetParError(3) << ", beta = " << pp->GetParameter(4) << "+-" << pp->GetParError(4) << "**" << endl;
        if(status == 0 && pp->GetParError(3) > 1.e-2 && pp->GetParError(3) < 1.){ // fit converges with unrealistically small error for unusable runs
            double theta = pp->GetParameter(3);
            if(thetaFit.count(run)){
                thetaFit[run] += theta;
                thetaFit[run] *= 0.5;
            } else {
                thetaFit[run] = theta;
            }
        }
    }

    cout << "*******************************" << endl;
    g->Write();
    grp->Write();

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
    hitcuts[portmap[run]]->Draw("same");
    LightPointsGraph_p(portmap[run])->Draw("psame");
    TH2D *hhhh = GetLightStrips(phi_offset, phi_lorentz+phi_shift, 2, portTheta[portmap[run]], portmap[run].first=='T');
    hhhh->SetName(hn+"_light");
    hhhh->Draw("same");
    // grp->Draw("perr same");
    c->cd(3);
    hp2res->DrawCopy("contz");     // This one disappears if there's more than one run. No idea. Canvas gets emptied when output file is closed...
    for(auto l: stripBounds) l->Draw("same");

    // c->cd(1);

    // c->Update();
    c->SaveAs(hn+".pdf");
    c->SaveAs(hn+".png");

    pcolOff[run] = TMath::Mean(g->GetN(),g->GetY());
    pcolOffSig[run] = TMath::RMS(g->GetN(),g->GetY());

    hitpatterns_t1_p[run] = hp1;
    hitpatterns_t2_p[run] = hp2;
    hitpatterns_t1_noOF_p[run] = hp1NoOF;
    hitpatterns_t2_noOF_p[run] = hp2NoOF;
    return 0;
}

//////////////// Select only drift signals that fall in expected pad position for subsequent analysis
TTree *padHitsTree(TTree *pt, int run){
    TString cutname = TString::Format("cut_port%c%02d", portmap[run].first, portmap[run].second);
    return pt->CopyTree(timecut2[run] && colcut && TCut(cutname));
}

//////////////// Select only drift signals that fall in expected pad position for subsequent analysis
TTree *awHitsTree(TTree *at, int run){
    auto port = portmap[run];
    pair<double, double> rng = GetPhiRange(portOffset[port.second], phi_lorentz + phi_shift, portTheta[port], port.first=='T');
    int awmin = rng.first/360.*_anodes-awTol;
    int awmax = rng.second/360.*_anodes+awTol;
    TCut awcut(TString::Format("wire > %d && wire < %d", awmin, awmax));
    if(awmax < awmin) awcut = TString::Format("wire > %d || wire < %d", awmin, awmax);
    return at->CopyTree(timecut2[run] && awcut);
}

//////////////// Pad amplitud distribution, compare with expectation
int pad_amp(TTree *pt, int run){
    TDirectory *d = paddir->GetDirectory("amp");
    if(!d) d = paddir->mkdir("amp");
    d->cd();
    TString hn = TString::Format("hamp_r_p_%d",run);
    // ampVrow_p[run] = new TH2D(hn,TString::Format("Run %d",run),_padrow,-0.5,_padrow-0.5,4160,0,4160);
    TH3D *h = new TH3D(hn+"_3d","pad amplitude;row;col;amp",_padrow,-0.5,_padrow-0.5,_padcol,-0.5,_padcol-0.5,128,0,4096);
    TString drawstring = "amp:col:row >> ";
    pt->Draw(drawstring+hn+"_3d","","0");
    // TString cutname = TString::Format("cut_port%c%02d", portmap[run].first, portmap[run].second);
    // pt->Draw(drawstring+hn+"3d",timecut2[run] && colcut && TCut(cutname),"0");
    TCanvas *ccc = new TCanvas();
    TGraphErrors *growAmp = new TGraphErrors;
    growAmp->SetMarkerStyle(5);
    growAmp->SetMarkerColor(kRed);
    growAmp->SetLineColor(kRed);
    growAmp->GetXaxis()->SetLimits(h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
    growAmp->SetName(hn+"_rowGraph");
    growAmp->GetXaxis()->SetTitle("row");
    growAmp->GetYaxis()->SetTitle("amp");
    TDirectory *dd = d->GetDirectory("row_slices");
    if(!dd) dd = d->mkdir("row_slices");
    dd->cd();
    for(int i = 0; i < _padrow; i++){
        TString hn2 = hn+TString::Format("_r%03d",i);
        int bin = h->GetXaxis()->FindBin(i);
        h->GetXaxis()->SetRange(bin,bin);
        if(h->Integral() > 0.005*h->GetEntries()){
            TH2D *p = (TH2D*)h->Project3D("zy");
            p->SetName(hn2);
            p->Draw();
            TH1D *pp = p->ProjectionY();
            if(pp->Integral()/pp->GetEntries() < 0.7){ // significant number of overflows
                brightStrips_p[run].insert(FindNearestStrip(pad2mm(i)));
            }
            // For each row, fit column spectrum with gaussian
            TGraphErrors *ge = new TGraphErrors;
            ge->SetMarkerStyle(5);
            ge->SetMarkerColor(kRed);
            ge->SetLineColor(kRed);
            ge->GetXaxis()->SetTitle("col");
            ge->GetYaxis()->SetTitle("amp");
            TF1 *fg = new TF1("fg","gaus",p->GetYaxis()->GetXmin(),p->GetYaxis()->GetXmax());
            for(int b = 1; b < p->GetNbinsX(); b++){
                TH1D *pp = p->ProjectionY("py",b,b);
                if(pp->Integral() > 100){
                    fg->SetParameter(0,pp->GetMaximum());
                    fg->SetParameter(1,pp->GetBinCenter(pp->GetMaximumBin()));
                    fg->SetParameter(2,200);
                    int result = pp->Fit(fg,"0Q");
                    if(result == 0){
                        if(fg->GetParameter(1) >= 0){
                            int point = ge->GetN();
                            ge->SetPoint(point, p->GetXaxis()->GetBinCenter(b),fg->GetParameter(1));
                            // cout << point << '\t' << i << '\t' << p->GetXaxis()->GetBinCenter(b) << '\t' << fg->GetParameter(1) << ((fg->GetParameter(1)>p->GetYaxis()->GetXmax())?"\tXXXXXXXXXX":"") << endl;
                            ge->SetPointError(point, 0,fg->GetParError(1));
                        }
                    }
                }
            }
            if(ge->GetN()){
                if(TMath::MaxElement(ge->GetN(),ge->GetY()) > p->GetYaxis()->GetXmax()){
                    ge->GetXaxis()->SetLimits(p->GetXaxis()->GetXmin(),p->GetXaxis()->GetXmax());
                    ge->GetYaxis()->SetLimits(0,ge->GetHistogram()->GetYaxis()->GetXmax());
                    ge->Draw("ap");
                    p->Draw("same");
                } else {
                    p->Draw();
                }
                ge->Draw("p same");

                // Cauchy/Lorentz fits better than gaussian, no physical reason
                TF1 *fc = new TF1("fc","[0]*TMath::CauchyDist(x,[1],[2])",p->GetXaxis()->GetXmin(),p->GetXaxis()->GetXmax());
                fc->SetParameter(0,TMath::MaxElement(ge->GetN(),ge->GetY()));
                fc->SetParLimits(0,0.,100.*TMath::MaxElement(ge->GetN(),ge->GetY()));
                fc->SetParameter(1,p->GetMean());
                fc->SetParameter(2,0.2);
                fc->SetParLimits(2,0.1,1.);
                fc->SetLineColor(kRed);
                TF1 *fc2 = new TF1(*fc);
                fc2->SetLineColor(kBlue);
                cout << "###################### " << p->GetName() << " - graph ################" << endl;
                double amplitude(0.), ampErr(0.);
                cout << "**************** pad amp fit row " << i << " ***************" << endl;
                cout << "************************** graph ***************************" << endl;
                TFitResultPtr fitr = ge->Fit(fc,"RBS");
                cout << "************************************************************" << endl;

                int resultg = fitr->Status();
                if(fitr->Chi2() > 1.) resultg = 4711; // Not sure why, but sometimes get a bad fit with too small errors, only usable fits seem to have very low Chi2...
                if(resultg != 0) ge->GetListOfFunctions()->RemoveLast();
                else {
                    amplitude = fc->GetParameter(0);
                    ampErr = fc->GetParError(0);
                    cout << "Chi2, Ndf, Chi2/Ndf:\t" << fitr->Chi2() << '\t' << fitr->Ndf() << '\t' << fitr->Chi2()/fitr->Ndf() << endl;
                }
                cout << "###################### " << p->GetName() << " - histo ################" << endl;
                cout << "************************** histo ***************************" << endl;
                int resulth = p->Fit(fc2,"RB");
                cout << "************************************************************" << endl;
                if(resulth != 0) p->GetListOfFunctions()->RemoveLast();
                else {
                    amplitude = fc2->GetParameter(0); // Prefer values from histogram fit, if available, otherwise use graph fit
                    ampErr = fc2->GetParError(0);
                }
                if(resultg*resulth == 0){
                    int point = growAmp->GetN();
                    growAmp->SetPoint(point,i,amplitude);
                    growAmp->SetPointError(point,0,ampErr);
                    ccc->Update();
                    // TString nnn = p->GetName();
                    // nnn += ".png";
                    // ccc->SaveAs(nnn);
                }
            }
        }
    }
    d->cd();
    TCanvas *cccc = new TCanvas();
    h->GetXaxis()->UnZoom();
    TH2D *pr = (TH2D*)h->Project3D("zx");
    pr->SetName(hn+"_Vrow");
    if(growAmp->GetN() && TMath::MaxElement(growAmp->GetN(),growAmp->GetY()) > pr->GetYaxis()->GetXmax()){
        growAmp->GetXaxis()->SetLimits(pr->GetXaxis()->GetXmin(),pr->GetXaxis()->GetXmax());
        growAmp->GetYaxis()->SetLimits(0,growAmp->GetHistogram()->GetYaxis()->GetXmax());
        growAmp->Draw("ap");
        pr->Draw("same");
    } else {
        pr->Draw();
    }
    growAmp->Draw("p same");

    TGraphErrors *gStripAmp = new TGraphErrors;
    gStripAmp->SetMarkerStyle(5);
    gStripAmp->SetMarkerColor(kRed);
    gStripAmp->SetLineColor(kRed);
    gStripAmp->GetXaxis()->SetLimits(h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
    gStripAmp->SetName(hn+"_strip");
    gStripAmp->GetXaxis()->SetTitle("strip[row]");
    gStripAmp->GetYaxis()->SetTitle("amp");
    TF1 *fgr = new TF1("fgr","gaus");
    if(strips.size() == 0) strips = GetStrips();
    for(auto s: strips){
        double p = mm2pad(s);
        cout << "****************** pad amp fit strip " << p << " ***********************" << endl;
        int res = growAmp->Fit(fgr,"+","",p-10,p+10);
        cout << "************************************************************************" << endl;
        if(res !=0 || abs(fgr->GetParameter(1)-p) > 2. || abs(fgr->GetParameter(2)-2.5) > 1.5) {
            cout << "fit deleted strip: " << p << ", status " << res << ", peak: " << fgr->GetParameter(1) << ", difference: " << (fgr->GetParameter(1)-p) << " rows, width " << fgr->GetParameter(2) << endl;
            if(res >= 0) growAmp->GetListOfFunctions()->RemoveLast(); // status -1 doesn't add a function to list, so shouldn't delete
        } else {
            cout << "strip: " << p << ", peak: " << fgr->GetParameter(1) << ", difference: " << (fgr->GetParameter(1)-p) << " rows"<< endl;
            int point = gStripAmp->GetN();
            gStripAmp->SetPoint(point,pad2mm(fgr->GetParameter(1)),fgr->GetParameter(0));
            gStripAmp->SetPointError(point,fgr->GetParError(1),fgr->GetParError(0));
        }
    }
    pr->Write();
    growAmp->Write();
    TString on = h->GetName();
    cccc->SaveAs(on+".root");

    if(gStripAmp->GetN()){
        TF1 *fi = GetExpInt(500,(portmap[run].first=='T'));
        cccc = new TCanvas;
        gStripAmp->Draw("ap");
        cout << "******************* pad strip intensity fit *************************" << endl;
        gStripAmp->Fit(fi,"","",-0.8*_halflength,0.8*_halflength);
        cout << "*********************************************************************" << endl;
        cccc->Update();
        gStripAmp->Write();
    }
    return 0;
}

//////////////// Pad drift time vs position and amplitude
int pad_time(TTree *pt, int run){
    TDirectory *d = paddir->GetDirectory("time");
    if(!d) d = paddir->mkdir("time");
    d->cd();
    int npadbin = 2000/padbin;
    int padmin = 3000;
    int padmax = 5000;
    TString hn = TString::Format("htime_p_%d",run);
    TH3D *h = new TH3D(hn+"_3d","pad time;row;col;time",_padrow,-0.5,_padrow-0.5,_padcol,-0.5,_padcol-0.5,npadbin,padmin,padmax);
    TString drawstring = "dtime:col:row >> ";
    pt->Draw(drawstring+hn+"_3d",pOFcut,"0");
    TCanvas *c = new TCanvas("cpadtime","",1000,1000);
    c->Divide(1,2);
    c->cd(1);
    TH2D *hc = (TH2D*)h->Project3D("zy");
    hc->SetName(hn+"_vCol");
    hc->Draw("colz");
    c->cd(2);
    TH2D *hr = (TH2D*)h->Project3D("zx");
    hr->SetName(hn+"_vRow");
    hr->Draw("colz");

    auto port = portmap[run];
    TF1 *flight = new TF1("flight","[0]*x + [1]",hr->GetXaxis()->GetXmin(),hr->GetXaxis()->GetXmax());
    flight->SetLineStyle(2);
    flight->SetLineWidth(1);
    double invC = 1.e6*_padpitch/TMath::C(); // 1.e6 to convert c from m/s to mm/ns
    if(port.first=='T') invC *= -1.;
    flight->FixParameter(0, invC);
    hr->Fit(flight);

    vector<TH1D*> p;
    TGraph *g = new TGraph;
    g->SetName(TString::Format("time_v_row_p_%d",run));
    g->SetMarkerStyle(20);
    g->GetXaxis()->SetTitle("row");
    g->GetYaxis()->SetTitle("time[ns]");

    TGraphErrors *gs = new TGraphErrors;
    gs->SetName(TString::Format("time_v_strip_p_%d",run));
    gs->SetMarkerStyle(20);
    g->GetXaxis()->SetTitle("strip[row]");
    g->GetYaxis()->SetTitle("time[ns]");

    TString dn = TString::Format("t_v_a/%c%d",port.first,port.second);
    // TDirectory *dd = d->GetDirectory(dn);
    // if(!dd) dd = d->mkdir(dn);
    if(!d->cd(dn)) d->mkdir(dn);
    if(strips.size() == 0) strips = GetStrips();
    for(auto s: strips){
        if(brightStrips_p[run].count(s) == 0){
            d->cd();
            int bin = hr->GetXaxis()->FindBin(mm2pad(s));
            int bin1, bin2;
            double bincont = hr->Integral(bin,bin);
            for(bin1 = bin-5; bin1 < bin; bin1++)
                if(hr->Integral(bin1,bin1)/bincont > 0.05) break;
            for(bin2 = bin+5; bin2 > bin; bin2--)
                if(hr->Integral(bin2,bin2)/bincont > 0.05) break;

            p.push_back(hr->ProjectionY(TString::Format("%s_striprow_%d",hn.Data(),int(mm2pad(s))),bin1,bin2));
            p.back()->SetTitle(p.back()->GetName());
            int colour = p.size();
            if(colour == 5) colour += files.size(); // That yellow is invisible...
            p.back()->SetLineColor(colour);

            hr->GetXaxis()->SetRange(bin1,bin2);
            TProfile *px = hr->ProfileX();
            for(int i = 1; i <= px->GetNbinsX(); i++){
                g->SetPoint(g->GetN(), px->GetXaxis()->GetBinCenter(i), px->GetBinContent(i));
            }


            vector<double> pxdata;
            for(int i = 1; i <= px->GetNbinsX(); i++)
                pxdata.push_back(px->GetBinContent(i));
            double mean = TMath::Mean(pxdata.begin(),pxdata.end());
            double rms = TMath::RMS(pxdata.begin(),pxdata.end());

            int point = gs->GetN();
            gs->SetPoint(point,mm2pad(s),mean);
            gs->SetPointError(point,0,rms);

            // dd->cd();
            d->cd(dn);
            TH2D *h = new TH2D(TString::Format("hp_t_v_amp_r%d_%c%02d_s%.0f",run,port.first,port.second,s),
                               TString::Format("time vs amp, port %c%02d, strip %.0f;amp;time",port.first,port.second,s),
                               256,0,4096,
                               npadbin,padmin,padmax);

            double p = mm2pad(s);
            // pt->Draw(TString::Format("time:amp >> %s", h->GetName()), TString::Format("abs(row - %f) < 10",p), "0");
            pt->Draw(TString::Format("dtime:amp >> %s", h->GetName()), TString::Format("row == %d",int(p)), "0");
            if(pad_amp_time[port].count(s)==0){
                // dd->cd();
                d->cd(dn);
                pad_amp_time[port][s] = (TH2D*)h->Clone();
                pad_amp_time[port][s]->SetName(TString::Format("hp_t_v_amp_%c%02d_s%.0f",port.first,port.second,s));
            } else {
                pad_amp_time[port][s]->Add(h);
            }
        } else {
            // cout << "XXXXXXXXXXX " << s << " is in brightStrips" << endl;
        }
    }
    hr->GetXaxis()->SetRange(0,-1);
    d->cd();

    new TCanvas;
    g->Draw();
    g->GetXaxis()->SetLimits(0,_padrow);
    g->Fit(flight,"","",100,450);
    gs->SetMarkerColor(kRed);
    gs->SetLineColor(kRed);
    gs->Draw("same");
    gs->Fit(flight,"","",100,450);
    TF1 k("k","[0]",0,_padrow);
    k.SetLineStyle(3);
    k.SetLineWidth(1);
    k.SetLineColor(kGreen);
    gs->Fit(&k,"+","",100,450);
    g->Write();
    gs->Write();

    if(!hpdt){
        hpdt = new TH2D("hdrifttime","pad drift time;row;col;time",72,0,576,32,0,32);
        hpdtct = new TH2D("hdtcount","pad drift time counts;row;col;time",72,0,576,32,0,32);
    }
    vector<TF1*> profiles = GetPadProfile(portOffset[portmap[run].second], phi_lorentz+phi_shift, portTheta[portmap[run]], portmap[run].first=='T');
    for(int i = 0; i < gs->GetN(); i++){
        double row, time;
        gs->GetPoint(i,row,time);
        for(TF1 *p: profiles){
            double col = p->Eval(row);
            int b = hpdt->FindBin(row,col);
            hpdt->AddBinContent(b,time);
            hpdtct->Fill(row,col);
        }
    }

    // if(!gPortTime_p){
    //     gPortTime_p = new TGraphErrors;
    //     gPortTime_p->SetMarkerStyle(20);
    //     gPortTime_p->SetName("g_port_time_p");
    //     gPortTime_p->GetXaxis()->SetTitle("laser port");
    //     gPortTime_p->GetYaxis()->SetTitle("time[ns]");
    // }
    // int point = gPortTime_p->GetN();
    // double stagger = 0.1*(double(rand())/double(RAND_MAX) - 0.5);
    // gPortTime_p->SetPoint(point,port.second+stagger,k.GetParameter(0));
    // gPortTime_p->SetPointError(point,0,k.GetParError(0));
    for(auto h: p){
        static bool first = true;
        h->Draw(first?"":"same");
        first = false;
    }

    return 0;
}

int hitPattern_a(TTree *at, int run)
{
    if(!tp1.count(run)){
        cerr << "Time analysis hasn't been run for Run " << run << ". Run timeSpec() before hitpattern_a()." << endl;
        return -1;
    }

    TDirectory *d = awdir->GetDirectory("hits");
    if(!d) d = awdir->mkdir("hits");
    d->cd();
    /////////////// Hitpattern
    TString hn = TString::Format("hahit%d",run);
    TH1D *ha1 = new TH1D(hn+"_t1","timecut 1;anode",_anodes,-0.5,_anodes-0.5);
    TH1D *ha2 = new TH1D(hn+"_t2","timecut 2;anode",_anodes,-0.5,_anodes-0.5);
    TH1D *ha1NoOF = new TH1D(hn+"_t1"+"NoOF","timecut 1;anode",_anodes,-0.5,_anodes-0.5);
    TH1D *ha2NoOF = new TH1D(hn+"_t2"+"NoOF","timecut 2;anode",_anodes,-0.5,_anodes-0.5);

    ha2->SetLineColor(kBlue);

    TString drawstring = TString::Format("wire-%.1f >>",_anodes);
    at->Draw(drawstring + hn+"_t1"+"NoOF",timecut1[run] && awOFcut,"0");
    at->Draw(drawstring + hn+"_t1",timecut1[run],"0");
    at->Draw(drawstring + hn+"_t2"+"NoOF",timecut2[run] && awOFcut,"0");

    at->Draw(drawstring + hn+"_t2",timecut2[run],"0");

#ifdef LARSALIAS
    TCanvas *c = new TCanvas(NewCanvasName(),"",1000,800); // I'm shocked root doesn't have a function for this... Just create a new name for a canvas when this constructor is used.
#else
    TCanvas *c = new TCanvas;
    c->SetCanvasSize(1000,800); // This works, but doesn't resize the window...
#endif

    TH1D *hhhh = GetLightStrips(portOffset[portmap[run].second], phi_lorentz+phi_shift, 1, portTheta[portmap[run]], portmap[run].first=='T')->ProjectionY(hn+"_light");
    hhhh->Scale(ha2->GetMaximum()/hhhh->GetMaximum());
    hhhh->SetLineColor(kGreen);

    TH1D *hshift(nullptr);
    if(false){                  // try to shift expected hitpattern around to match data
        TSpectrum s_light(9);
        int npl = s_light.Search(hhhh,1,"nodraw");
        TPolyMarker *pm = (TPolyMarker*)hhhh->GetListOfFunctions()->FindObject("TPolyMarker");
        pm->SetMarkerColor(hhhh->GetLineColor()+2);
        Double_t *y = pm->GetY();
        for(int i = 0; i < pm->GetN(); i++){
            y[i] += 0.03*ha2->GetMaximum();
        }

        TSpectrum s_aw(9);
        int npa = s_aw.Search(ha2,1,"nodraw");
        pm = (TPolyMarker*)ha2->GetListOfFunctions()->FindObject("TPolyMarker");
        pm->SetMarkerColor(ha2->GetLineColor()+2);
        y = pm->GetY();
        for(int i = 0; i < pm->GetN(); i++){
            y[i] += 0.02*ha2->GetMaximum();
        }

        double *lp = s_light.GetPositionX();
        double *ap = s_aw.GetPositionX();
        set<double> lpeaks;
        set<double> apeaks;
        for(int i = 0; i < npl; i++) lpeaks.insert(lp[i]);
        for(int i = 0; i < npa; i++) apeaks.insert(ap[i]);

        vector<double> lpeaksv, apeaksv;
        if(portmap[run].first == 'B'){
            lpeaksv = vector<double>(lpeaks.begin(),lpeaks.end());
            apeaksv = vector<double>(apeaks.begin(),apeaks.end());
        } else {
            lpeaksv = vector<double>(lpeaks.rbegin(),lpeaks.rend());
            apeaksv = vector<double>(apeaks.rbegin(),apeaks.rend());
        }

        ////////// Deal with rollover AW 255<->0
        auto it = lpeaksv.begin();
        while(it != lpeaksv.end()){
            double val1 = *it++;
            double val2 = *it;
            if(abs(val2-val1) > 0.5*_anodes) break;
        }
        if(it !=  lpeaksv.end()){
            vector<double> buf(lpeaksv.begin(), it);
            lpeaksv.erase(lpeaksv.begin(),it);
            lpeaksv.insert(lpeaksv.end(), buf.begin(), buf.end());
        }

        it = apeaksv.begin();
        while(it != apeaksv.end()){
            double val1 = *it++;
            double val2 = *it;
            if(abs(val2-val1) > 0.5*_anodes) break;
        }
        if(it !=  apeaksv.end()){
            vector<double> buf(apeaksv.begin(), it);
            apeaksv.erase(apeaksv.begin(),it);
            apeaksv.insert(apeaksv.end(), buf.begin(), buf.end());
        }

        // improve peak positions
        for(it = apeaksv.begin(); it != apeaksv.end(); it++){
            int binc = ha2->GetXaxis()->FindBin(*it);
            int binl, binr;
            for(binl = binc - 1; binl > binc-5; binl--)
                if(ha2->GetBinContent(binl) < 0.25*ha2->GetBinContent(binc)) break;
            for(binr = binc + 1; binr < binc+5; binr++)
                if(ha2->GetBinContent(binr) < 0.25*ha2->GetBinContent(binc)) break;
            ha2->GetXaxis()->SetRange(binl,binr);
            cout << "improving peak at " << *it;
            *it = ha2->GetMean();
            cout << " to " << *it << endl;
            ha2->GetXaxis()->UnZoom();
        }

        cout << "***************** AW peak matching run " << run << " *****************" << endl;
        double minrange(10000.);
        double avgshift(0.);
        int bestj(0);
        int nmatched(0);
        vector<int> peakmatches(apeaksv.size(),-1);

        for(int j = 0; j < npl; j++){
            cout << "Match first measured peak with light peak " << j << endl;
            vector<int> peakmatches_(apeaksv.size(),-1);
            double d0 = lpeaksv[j]-apeaksv[0];
            if(d0 > 0.5*_anodes) d0 -= _anodes;
            else if(d0 < -0.5*_anodes) d0 += _anodes;
            peakmatches_[0] = j;
            double dmax(d0), dmin(d0);
            double davg(d0);
            int n(1);
            for(int i = 1; i < npa; i++){
                for(int jj = j+i; jj < npl; jj++){
                    double d = lpeaksv[jj]-apeaksv[i];
                    if(d > 0.5*_anodes) d -= _anodes;
                    else if(d < -0.5*_anodes) d += _anodes;
                    if(abs(d-d0)>5) continue;
                    davg += d;
                    n++;
                    if(d > dmax) dmax = d;
                    if(d < dmin) dmin = d;
                    if(peakmatches_[i] >= 0){
                        if(abs(d) < abs(lpeaksv[peakmatches_[i]]-apeaksv[i])-1){ // only skip a peak if the next one matches significantly better
                            cout << "XXXXXXXXX " << d << " vs " << lpeaksv[peakmatches_[i]]-apeaksv[i] << endl;
                            peakmatches_[i] = jj;
                        }
                    } else {
                        peakmatches_[i] = jj;
                    }
                }
            }
            if(n >= nmatched){
                if(dmax - dmin < minrange || (dmax - dmin == minrange && davg/double(n) < avgshift)){
                    minrange = dmax - dmin;
                    avgshift = davg/double(n);
                    bestj = j;
                    nmatched = n;
                    peakmatches = peakmatches_;
                }
            }
        }




        // TDirectory *dd = d->GetDirectory("shift");
        // if(!dd) dd = d->mkdir("shift");
        // dd->cd();
        cout << "##################################################################" << endl;
        TGraph gasp;
        gasp.SetName(hn+"_hitshift");
        double d0 = lpeaksv[bestj]-apeaksv[0];
        if(d0 > 0.5*_anodes) d0 -= _anodes;
        else if(d0 < -0.5*_anodes) d0 += _anodes;
        for(int i = 0; i < npa; i++){
            if(peakmatches[i] >= 0){
                double d = lpeaksv[peakmatches[i]]-apeaksv[i];
                if(d > 0.5*_anodes) d -= _anodes;
                else if(d < -0.5*_anodes) d += _anodes;
                if(abs(d-d0)>5) continue;
                cout << i << '\t' << peakmatches[i] << '\t' << apeaksv[i] << '\t' << d << endl;
                gasp.SetPoint(gasp.GetN(),apeaksv[i],d);
            }
        }
        cout << "##################################################################" << endl;
        TF1 fasp("fasp","[0]",0.,_anodes);
        gasp.Fit(&fasp);
        gasp.Write();
        // d->cd();

        cout << "Best match (" << minrange << ") for matching data peak " << ((bestj>=0)?0:-bestj) << " with light peak " << ((bestj>=0)?bestj:0) << ", mismatch of hit patterns is " << avgshift << " wires, or " << avgshift*360./double(_anodes) << " deg\tby fit: " << fasp.GetParameter(0) << " wires, or " << fasp.GetParameter(0)*360./double(_anodes) << "deg" << endl;

        if(minrange < 10) {
            // double aws = avgshift*360./double(_anodes);
            // awshift[run] = aws;
            double aws = fasp.GetParameter(0)*360./double(_anodes);
            hshift = GetLightStrips(portOffset[portmap[run].second], phi_lorentz+phi_shift-aws, 1, portTheta[portmap[run]], portmap[run].first=='T')->ProjectionY(hn+"_light_shifted");
            hshift->SetLineStyle(2);
            if(gasp.GetN()>2){
                awshift[run] = aws;
                awshiftErr[run] = fasp.GetParError(0)*360./double(_anodes);
                hshift->SetLineStyle(1);
            }
            hshift->Scale(ha2->GetMaximum()/hshift->GetMaximum());
            hshift->SetLineColor(kRed);
        } else {
            cout << "This match is terrible, there must be middle peaks missing." << endl;
        }
        cout << "***********************************************************************" << endl;
    }
    ha2->SetFillColor(ha2->GetLineColor());
    c->Divide(1,2);
    c->cd(1);
    ha1->Draw();
    c->cd(2);
    ha2->Draw();
    hhhh->Draw("same");
    if(hshift) hshift->Draw("same");
    // for(auto l: HitAnodes(portmap[run],ha2->GetMinimum(),ha2->GetMaximum())) l->Draw("same");
    c->SaveAs(hn+".pdf");
    c->SaveAs(hn+".png");

    return 0;
}

//////////////// Anode drift time vs position and amplitude
int aw_time(TTree *at, int run){
    TDirectory *d = awdir->GetDirectory("time");
    if(!d) d = awdir->mkdir("time");
    d->cd();
    int nawbin = 3000/awbin;
    int tawmin = 3000;
    int tawmax = 6000;
    TString hn = TString::Format("htime_a_%d",run);
    auto port = portmap[run];
    pair<double, double> rng = GetPhiRange(portOffset[port.second], phi_lorentz + phi_shift, portTheta[port], port.first=='T');
    int awmin = rng.first/360.*_anodes-awTol;
    int awmax = rng.second/360.*_anodes+awTol;

    if(awmin > awmax){
        awmin = 0;
        awmax = _anodes;
    }

    TH2D *h = new TH2D(hn+"_w","anode drift time vs wire;wire;time[ns]",awmax-awmin,awmin-0.5,awmax-0.5,nawbin,tawmin,tawmax);
    TString drawstring = TString::Format("time:(wire-%f) >> ",_anodes);
    at->Draw(drawstring+hn+"_w","","0");

    // TH2D *h2 = new TH2D(hn+"_amp","anode drift time vs amplitude;amp;time[ns]",1000,0,40000,nawbin,tawmin,tawmax);
    // drawstring = "time:amp >> ";
    // at->Draw(drawstring+hn+"_amp","","0");
    return 0;
}

int LaserAnalysis(){
    if(allcols) colcut = "";
    else colcut = "col != 2";

    // Make list of files per laser port
    map<pair<char,int>, vector<TString> > portFiles;
    for(auto fn: files){
        int run = runNo(fn);
        if(portmap.count(run)){
            portFiles[portmap[run]].push_back(fn);
        } else {
            cerr << "Run " << run << " is not listed in portmap (in macro file)" << endl;
        }
    }

    // Calculate expected hit regions
    GetHitCuts();

    TFile fout("anaOut.root","RECREATE");
    if(!fout.IsOpen()){
        cerr << "Couldn't open output file." << endl;
        return -1;
    }
    timedir = fout.mkdir("time");
    paddir = fout.mkdir("pads");
    awdir = fout.mkdir("aw");

    for(auto fn: files){
        TFile f(fn);
        TTree *pt = (TTree*)f.Get("tpc_tree/fPadTree");
        TTree *at = (TTree*)f.Get("tpc_tree/fAnodeTree");
        int run = runNo(fn);

        // overall time analysis, determine time cuts
        timeSpec(pt, at, run);

        // pad hitpattern
        hitPattern_p(pt, run);

        // aw hit pattern
        hitPattern_a(at, run);

        gROOT->cd();
        // now limit to only hits around the expected pads and time, to speed things up
        TTree *pht = padHitsTree(pt, run);
        TTree *aht = awHitsTree(at, run);

        // Pad amplitude and time
        pad_amp(pht, run);

        pad_time(pht, run);

        aw_time(aht, run);

        TCanvas c;
    }

    // Summary plots/analysis

    cout << endl << "===========================================================" << endl;
    cout << "Drift times:" << endl;
    cout << "run\tport\ttda\tstda\ttdp\tstdp"<< endl;
    for(auto ttt: tda){
        int run = ttt.first;
        auto port = portmap[run];
        cout << run << '\t' << port.first << port.second << '\t' << ttt.second << '\t' << stda[run] << '\t' << tdp[run] << '\t' << stdp[run] << endl;
    }
    cout << "===========================================================" << endl << endl;

    timedir->cd();
    double dt_a(0.), dt_p(0.);
    for(auto t: tda) dt_a += t.second;
    dt_a /= double(tda.size());
    for(auto t: tdp) dt_p += t.second;
    dt_p /= double(tdp.size());
    if(gPortTime_a){
        TF1 fcos("fcos","[0]*cos(2*pi*(x-[1])/[2])+[3]",0,16);
        fcos.SetLineColor(kOrange);
        fcos.SetLineWidth(1);
        double ymax = -999999.;
        double ymin = 999999.;
        double xmax(0), xmin(0);
        for(int i = 0; i < gPortTime_a->GetN(); i++){
            double x,y;
            gPortTime_a->GetPoint(i,x,y);
            if(y > ymax){
                ymax = y;
                xmax = x;
            }
            if(y < ymin){
                ymin = y;
                xmin = x;
            }
        }
        fcos.SetParameter(0,0.5*(ymax-ymin));
        fcos.SetParameter(1,xmax);
        fcos.FixParameter(2,16);
        fcos.SetParameter(3,0.5*(ymax+ymin));
        if(gPortTime_a->Fit(&fcos)==0){
            double shift = fcos.GetParameter(0)*(_anoderadius-_cathradius)/dt_a;
            cout << endl << "===========================================================" << endl;
            cout << "Anode time difference around phi is (somewhat) consistent with a non-concentricity of cathode and wires of ~" << shift << "mm, longest time around phi = " << fmod(fcos.GetParameter(1)*360./16.,360.) << "+-" << fcos.GetParError(1)*360./16. << endl;
            cout << "===========================================================" << endl << endl;
        }
        gPortTime_a->GetHistogram()->GetXaxis()->SetTitle("Laser port (AWC)");
        gPortTime_a->GetHistogram()->GetYaxis()->SetTitle("time [ns]");
        gPortTime_a->Write();
    }

    if(gPortTime_p){
        TF1 fcos("fcos","[0]*cos(2*pi*(x-[1])/[2])+[3]",0,16);
        fcos.SetLineColor(kBlack);
        fcos.SetLineWidth(1);
        double ymax = -999999.;
        double ymin = 999999.;
        double xmax(0), xmin(0);
        for(int i = 0; i < gPortTime_p->GetN(); i++){
            double x,y;
            gPortTime_p->GetPoint(i,x,y);
            if(y > ymax){
                ymax = y;
                xmax = x;
            }
            if(y < ymin){
                ymin = y;
                xmin = x;
            }
        }
        fcos.SetParameter(0,0.5*(ymax-ymin));
        fcos.SetParameter(1,xmax);
        fcos.FixParameter(2,16);
        fcos.SetParameter(3,0.5*(ymax+ymin));
        if(gPortTime_p->Fit(&fcos)==0){
            double shift = fcos.GetParameter(0)*(_anoderadius-_cathradius)/dt_p;
            cout << endl << "===========================================================" << endl;
            cout << "Pad time difference around phi is (somewhat) consistent with a non-concentricity of cathode and wires of ~" << shift << "mm, longest time around phi = " << fmod(fcos.GetParameter(1)*360./16.,360.) << "+-" << fcos.GetParError(1)*360./16. << endl;
            cout << "===========================================================" << endl << endl;
        }
        gPortTime_p->GetHistogram()->GetXaxis()->SetTitle("Laser port (AWC)");
        gPortTime_p->GetHistogram()->GetYaxis()->SetTitle("time [ns]");
        gPortTime_p->Write();
    }
    if(gPortTime_diff){
        gPortTime_diff->Draw("ap");
        gPortTime_diff->GetHistogram()->GetXaxis()->SetTitle("Laser port (AWC)");
        gPortTime_diff->GetHistogram()->GetYaxis()->SetTitle("t_{p}-t_{a} [ns]");
        gPortTime_diff->Write();
    }

    if(time_p.size()){
        timedir->cd();
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
        paddir->cd("hits");
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
        int k = 1;
        for(auto hp: hitpatterns_t2_p){
            if(!hhitsum_t2_p){
                hhitsum_t2_p = new TH2D(*hp.second);
                hhitsum_t2_p->SetName("hphitsum_t2");
            } else {
                hhitsum_t2_p->Add(hp.second);
            }
            for(int i = 0; i < hp.second->GetListOfFunctions()->GetEntries(); i++){
                hhitsum_t2_p->GetListOfFunctions()->Add(hp.second->GetListOfFunctions()->At(i));
            }
        }
        for(auto hp: hitpatterns_t2_noOF_p){
            if(!hhitsum_t2_NoOF_p){
                hhitsum_t2_NoOF_p = new TH2D(*hp.second);
                hhitsum_t2_NoOF_p->SetName("hphitsum_t2_NoOF");
            } else {
                hhitsum_t2_NoOF_p->Add(hp.second);
            }
            for(int i = 0; i < hp.second->GetListOfFunctions()->GetEntries(); i++){
                hhitsum_t2_NoOF_p->GetListOfFunctions()->Add(hp.second->GetListOfFunctions()->At(i));
            }
        }
    }

    if(hpdt) hpdt->Divide(hpdtct);
    // hpdt->GetZaxis()->SetRangeUser();

    cout << "Overexposed strips:" << endl;
    for(auto r: brightStrips_p){
        cout << "run " << r.first << ":";
        for(auto s: r.second){
            cout << '\t' << s;
        }
        cout << endl;
    }

    paddir->cd("hits");
    TGraphErrors ggg;
    ggg.SetName("g_phit_off_run");
    ggg.SetMarkerStyle(20);
    cout << endl << "Pad column offsets:" << endl;
    for(auto pco: pcolOff){
        int p = ggg.GetN();
        ggg.SetPoint(p, pco.first, pco.second);
        ggg.SetPointError(p, 0, pcolOffSig[pco.first]);
        cout << pco.first << '\t' << pco.second << '\t' << pcolOffSig[pco.first] << endl;
    }
    TF1 fk("fk","[0]",0,10000);
    ggg.Fit(&fk);
    ggg.Write();

    cout << endl << "Theta from pad hitpattern fit:" << endl;
    cout << "run\ttheta" << endl;
    for(auto t: thetaFit){
        cout << t.first << '\t' << t.second;
        if(abs(t.second-portTheta[portmap[t.first]]) > 0.5) cout << "\t!= " << portTheta[portmap[t.first]] << " !!!";
        cout << endl;
    }

    awdir->cd("hits");
    cout << endl << "Anode wire mismatch with expected light (in degrees):" << endl;
    TGraphErrors gga;
    gga.SetName("g_ahit_off_run");
    gga.SetMarkerStyle(20);
    cout << endl << "Anode offsets:" << endl;
    for(auto aws: awshift){
        int p = gga.GetN();
        gga.SetPoint(p, aws.first, aws.second);
        gga.SetPointError(p, 0, awshiftErr[aws.first]);
        cout << "run " << aws.first << '\t' << aws.second << '\t' << awshiftErr[aws.first] << endl;
    }
    TF1 fka("fka","[0]",0,10000);
    gga.Fit(&fka);
    gga.Write();

    fout.Write();
    fout.Close();
    return 0;
}
