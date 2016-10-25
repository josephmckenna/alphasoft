#include "RootPlotter.hh"

#include <iostream>

#include <TAxis.h>
#include <TSystem.h>
#include <TFile.h>
#include <TVector3.h>

#include "ChamberGeo.hh"

#include "TFitLine.hh"

// extern TFile *gOutputFile;

using std::cerr;
using std::endl;

#define MAX_ADC 9000

TCanvas *RootPlotter::createCanvas(const char *cname, const char *title){
    return new TCanvas(cname, title, 800, 800);
}

void RootPlotter::Save2D(const char* name){
    if(canvas2D) canvas2D->SaveAs(name);
}

void RootPlotter::SaveGIFtracks(string filename, unsigned int n){
    if(filename.find(".gif") != string::npos){
        gifname = filename;
        gSystem->Unlink(gifname.c_str()); // delete old file
        gifname += "+";
        ngif = n;
    }
}

void RootPlotter::Write(){
    if( hAnodeHits ) hAnodeHits->Write();
    if( hFirstTime ) hFirstTime->Write();
    if( hRofT_straight ) hRofT_straight->Write();
    if( hMulti ) hMulti->Write();
    if( hMultiP ) hMultiP->Write();
    if( hPoints ) hPoints->Write();
    if( hDiscard ) hDiscard->Write();
    if( hMean ) hMean->Write();
    if( hMax ) hMax->Write();
    if( hMaxD ) hMaxD->Write();
    if( hMaxI ) hMaxI->Write();
    if( hGaps ) hGaps->Write();
    if( hNGaps ) hNGaps->Write();
    if( hGapRatio ) hGapRatio->Write();
    if( hRMS ) hRMS->Write();
    if( hHeight ) hHeight->Write();
    if( hFitRes ) hFitRes->Write();
    if( canvasHist ){
        TCanvas c("c","points",800,800);
        TPad *p = (TPad*)canvasHist->GetPad(5)->Clone();
        p->SetCanvasSize(800,800);
        p->Draw();
        c.SaveAs("points.root");
        delete p;
    }
}

void RootPlotter::Reset(){
    if( hAnodeHits ) hAnodeHits->Reset();
    if( hFirstTime ) hFirstTime->Reset();
    if( hRofT_straight ) hRofT_straight->Reset();
    if( hMulti ) hMulti->Reset();
    if( hMultiP ) hMultiP->Reset();
    if( hPoints ) hPoints->Reset();
    if( hGaps ) hGaps->Reset();
    if( hNGaps ) hNGaps->Reset();
    if( hGapRatio ) hGapRatio->Reset();
    if( hRMS ) hRMS->Reset();
    if( hHeight ) hHeight->Reset();
    if( hFitRes ) hFitRes->Reset();
}

void RootPlotter::ShowHistograms(){
    // if(gOutputFile) gOutputFile->cd();
    if(!canvasHist){
        canvasHist = new TCanvas("canvasHist", "Analysis histograms", 1600, 1100);
        canvasHist->Divide(5,4,0.005,0.005);
        canvasHist->Draw();
    }
    int i = 0;
    if(!hAnodeHits){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hAnodeHits = new TH1D("hAnodeHits","Anode hit pattern;anode;number of signals", TPCBase::NanodeWires, 0, TPCBase::NanodeWires);
        hAnodeHits->SetFillColor(kBlue);
        hAnodeHits->GetYaxis()->SetTitleOffset(1.4);
        hAnodeHits->Draw();
    }
    if(!hMulti){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hMulti = new TH1D("hMulti","Anode multiplicity;number of anode signals per event;number of events", TPCBase::NanodeWires, 0, TPCBase::NanodeWires);
        hMulti->SetFillColor(kBlue);
        hMulti->GetYaxis()->SetTitleOffset(1.4);
        hMulti->Draw();
    }
    if(!hMultiP){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hMultiP = new TH1D("hMultiP","Clusters per event;number of 'clusters' per event;number of events", 10000, 0, 10000);
        hMultiP->SetFillColor(kBlue);
        hMultiP->GetYaxis()->SetTitleOffset(1.4);
        hMultiP->Draw();
    }
    if(!hFirstTime){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hFirstTime = new TH1D("hFirstTime","first time in event;t in ns;number of events", 700, 0, 7000);
        hFirstTime->SetFillColor(kBlue);
        hFirstTime->GetYaxis()->SetTitleOffset(1.4);
        hFirstTime->Draw();
    }
    if(!hRofT_straight){
        canvasHist->cd(++i);
        hRofT_straight = new TH2D("hRofT_straight","straight track r vs t;t in ns;r in mm", 550, -500, 5000, 900, 100, 190);
        hRofT_straight->GetYaxis()->SetTitleOffset(1.4);
        hRofT_straight->Draw("COL");
    }
    if(!hPoints){
        canvasHist->cd(++i);
        hPoints = new TH2D("hPoints","space point distribution;x in mm;y in mm", 95, -190, 190, 95, -190, 190);
        hPoints->GetYaxis()->SetTitleOffset(1.4);
        hPoints->Draw("COL");
        if(!cath.size()){
            ChamberGeo cg;
            cg.SetWindow(-10.*TPCBase::ROradius, -10.*TPCBase::ROradius, 10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
            cath = cg.GetCathode2D();
            pads = cg.GetPadCircle2D();
            if(!fwires) fwires = cg.GetFieldWires();
            if(!awires) awires = cg.GetAnodeWires();
        }
        for(auto c: cath) c->Draw();
        for(auto p: pads) p->Draw();
        fwires->SetMarkerStyle(1);
        fwires->Draw("psame");
        awires->SetMarkerStyle(1);
        awires->Draw("psame");
    }
    if(!hMean){
        canvasHist->cd(++i);
        hMean = new TH2D("hMean","Mean value of signals;anode wire;mean", TPCBase::NanodeWires, 0, TPCBase::NanodeWires, 2*MAX_ADC,-MAX_ADC,MAX_ADC);
        hMean->GetYaxis()->SetTitleOffset(1.4);
        hMean->Draw("COL");
    }
    if(!hMax){
        canvasHist->cd(++i);
        hMax = new TH2D("hMax","Signal size;anode wire;mean", 2*TPCBase::NanodeWires, -TPCBase::NanodeWires, TPCBase::NanodeWires, MAX_ADC,0,4*MAX_ADC);
        hMax->GetYaxis()->SetTitleOffset(1.4);
        hMax->Draw("COL");
    }
    if(!hMaxD){
        canvasHist->cd(++i);
        hMaxD = new TH2D("hMaxD","Signal size drift region;anode wire;mean", 2*TPCBase::NanodeWires, -TPCBase::NanodeWires, TPCBase::NanodeWires, MAX_ADC,0,4*MAX_ADC);
        hMaxD->GetYaxis()->SetTitleOffset(1.4);
        hMaxD->Draw("COL");
    }
    if(!hMaxI){
        canvasHist->cd(++i);
        hMaxI = new TH2D("hMaxI","Signal size induction region;anode wire;mean", 2*TPCBase::NanodeWires, -TPCBase::NanodeWires, TPCBase::NanodeWires, MAX_ADC,0,4*MAX_ADC);
        hMaxI->GetYaxis()->SetTitleOffset(1.4);
        hMaxI->Draw("COL");
    }
    if(!hDiscard){
        canvasHist->cd(++i);
        hDiscard = new TH1D("hDiscard","Discarded huge means;anode", TPCBase::NanodeWires, 0, TPCBase::NanodeWires);
        hDiscard->SetFillColor(kBlue);
        hDiscard->GetYaxis()->SetTitleOffset(1.4);
        hDiscard->Draw();
    }
    if(!hGaps){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hGaps = new TH1D("hGaps","Location of single wire gaps;anode;number of event gaps", TPCBase::NanodeWires, 0, TPCBase::NanodeWires);
        hGaps->SetFillColor(kBlue);
        hGaps->GetYaxis()->SetTitleOffset(1.4);
        hGaps->Draw();
    }
    if(!hNGaps){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hNGaps = new TH1D("hNGaps","Number of single wire gaps per event;number of gaps;events", 50, 0, 50);
        hNGaps->SetFillColor(kBlue);
        hNGaps->GetYaxis()->SetTitleOffset(1.4);
        hNGaps->Draw();
    }
    if(!hGapRatio){
        canvasHist->cd(++i);
        canvasHist->GetPad(i)->SetLogy();
        hGapRatio = new TH1D("hGapRatio","Ratio of single wire gaps to number of signals;N_{gap}/(N_{sig}+N_{gap});events", 110, 0, 1.1);
        hGapRatio->SetFillColor(kBlue);
        hGapRatio->GetYaxis()->SetTitleOffset(1.4);
        hGapRatio->Draw();
    }
    if(!hRMS){
        canvasHist->cd(++i);
        hRMS = new TH2D("hRMS","RMS of deconvolution remainder;anode;RMS", TPCBase::NanodeWires, 0, TPCBase::NanodeWires, 1000, 0, 1000);
        hRMS->GetYaxis()->SetTitleOffset(1.4);
        hRMS->Draw("COL");
    }
    if(!hHeight){
      canvasHist->cd(++i);
      hHeight = new TH1D("hHeight","Number of Avanlanches;number [a.u];frequency",200,0.,100.);
      hHeight->Draw();
    }
    if(!hFitRes){
      canvasHist->cd(++i);
      canvasHist->GetPad(i)->SetLogy();
      hFitRes = new TH1D("hFitRes","root mean fit residual;d [mm];frequency",1000,0.,100.);
      hFitRes->Draw();
    }
    for(int j = 1; j <= i; j++) canvasHist->GetPad(j)->Modified();
    canvasHist->Update();
    canvasHist->Draw();
}

void RootPlotter::PlotEfficiency(){
    if(!canvasEff) canvasEff = new TCanvas("canvasEff","Efficiency");
    canvasEff->cd();
    gEff.Set(0);
    //    double missed = hMulti->GetBinContent(1);
    double total = hMulti->Integral(0,-1);
    gEff.SetFillColor(kBlue);
    gEff.SetTitle("Acceptance;min. number of anodes;percentage of scintillator coincidences");

    gEff.Draw("ab");
    for(int n = 1; n < 64; n++){
        double seen = hMulti->Integral(hMulti->FindBin(n),-1);
        gEff.SetPoint(gEff.GetN(), n, 100.*seen/total);
    }
}

void RootPlotter::PlotXY(const vector<SpacePoints::Point3D> &points, double phi0){
    std::cout << "Plotting track." << endl;
    if(!canvas2D) canvas2D = createCanvas("canvas2D", "2D points");
    canvas2D->Draw();
    graph2D.Set(0);
    for(auto p: points){
        int k = graph2D.GetN();
        double x = p.x;
        double y = p.y;
        if( x != x || y != y){
            cerr << "NaN values at " << k << ": x = " << x << " , y = " << y << endl;
        } else {
            graph2D.SetPoint(k,x,y);
            graph2D.SetPointError(k,p.ex,p.ey);
        }
    }

    canvas2D->cd();
    graph2D.Draw("ap");
    graph2D.GetXaxis()->SetLimits(-10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
    graph2D.SetMinimum(-10.*TPCBase::ROradius);
    graph2D.SetMaximum(10.*TPCBase::ROradius);

    if(!cath.size()){
        ChamberGeo cg;
        cg.SetWindow(-10.*TPCBase::ROradius, -10.*TPCBase::ROradius, 10.*TPCBase::ROradius, 10.*TPCBase::ROradius);
        cath = cg.GetCathode2D();
        pads = cg.GetPadCircle2D();
        if(!fwires) fwires = cg.GetFieldWires();
        if(!awires) awires = cg.GetAnodeWires();
    }
    for(auto c: cath) c->Draw();

    for(auto p: pads) p->Draw();

    //    fwires->SetMarkerStyle(1);
    fwires->Draw("psame");


    //    awires->SetMarkerStyle(1);
    awires->Draw("psame");
    canvas2D->Update();
    canvas2D->Draw();

    if(ngif){
        ngif--;
        if(ngif){
            static int igif = 1;
            std::cout << "GIF frame " << igif++ << std::endl;
            canvas2D->Print(gifname.c_str());
        } else {
            gifname += "+";
            canvas2D->Print(gifname.c_str());
            std::cout << "Finished writing track gif." << std::endl;
            gifname.clear();
        }
    }
    //    std::cout << points.size() << " -> " << graph2D.GetN() << endl;
}

void RootPlotter::PlotTime(const vector<SpacePoints::Point3D> &points){
    std::cout << "Plotting time." << endl;
    if(!canvasTime) canvasTime = createCanvas("canvasTime", "Anode times");
    canvasTime->Draw();
    graphTime.Set(0);
    graphTimenoSP.Set(0);
    for(auto p: points){
        int k = graphTime.GetN();
        double a = p.anode;
        double t = p.t;
        if( a != a || t != t){
            cerr << "NaN values at " << k << ": anode = " << a << " , t = " << t << endl;
        } else {
            graphTime.SetPoint(k,a,t);
            if(p.x == 0 && p.y ==0){
                k = graphTimenoSP.GetN();
                graphTimenoSP.SetPoint(k,a,t);
            }
        }
    }

    canvasTime->cd();
    graphTime.Draw("ap");
    graphTimenoSP.Draw("p same");
    canvasTime->Update();
}

void RootPlotter::DrawStraight(double a0, double a1, double phi0){
    if(l) {
        delete l;
        l = nullptr;
    }
    if(!canvas2D) return;
    double x0, y0, x1, y1;
    int a00 = a0;
    int a10 = a1;
    int a01 = a00 + 1;
    int a11 = a10 + 1;

    double ratio0 = a0 - a00;
    double ratio1 = a1 - a10;
    TPCBase::GetAnodePosition(a00, x0, y0, false, phi0);
    if(ratio0){
        double xtmp, ytmp;
        TPCBase::GetAnodePosition(a01, xtmp, ytmp, false, phi0);
        x0 = ratio0*xtmp + (1-ratio0)*x0;
        y0 = ratio0*ytmp + (1-ratio0)*y0;
    }

    TPCBase::GetAnodePosition(a10, x1, y1, false, phi0);
    if(ratio1){
        double xtmp, ytmp;
        TPCBase::GetAnodePosition(a11, xtmp, ytmp, false, phi0);
        x1 = ratio1*xtmp + (1-ratio1)*x1;
        y1 = ratio1*ytmp + (1-ratio1)*y1;
    }

    canvas2D->cd();
    l = new TLine(10.*x0, 10.*y0, 10.*x1, 10.*y1);
    l->SetLineColor(kRed);
    l->SetLineWidth(1);
    l->Draw();
    canvas2D->Update();
    canvas2D->Draw();
}
