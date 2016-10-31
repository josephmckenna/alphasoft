#ifndef ROOT_PLOTTER_H
#define ROOT_PLOTTER_H

#include <TFile.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TGraph2DErrors.h>
#include <TEllipse.h>
#include <TLine.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TPolyLine.h>

#include "SpacePoints.hh"

class TFitLine;
class RootPlotter{
public:
    RootPlotter(){
        CreateHistograms();
        graphTime.SetMarkerStyle(7);
        graphTime.SetTitle("anode times;anode;time in samples");
        graphTimenoSP.SetMarkerStyle(7);
        graphTimenoSP.SetMarkerColor(kRed);
    }
    ~RootPlotter(){
        if(canvas2D) delete canvas2D;
        if(canvasTime) delete canvasTime;
        if(canvas3D) delete canvas3D;
        if(canvasHist) delete canvasHist;
        if(canvasEff) delete canvasEff;
        if(l) delete l;

        for(auto *h: histos) delete h;

        if( fwires ) delete fwires;
        if( awires ) delete awires;
        for( auto c: cath ) delete c;
        for( auto p: pads ) delete p;
    }
    void PlotXY(const vector<SpacePoints::Point3D> &points, double phi0 = 0);
    void PlotTime(const vector<SpacePoints::Point3D> &points);
    void PlotEfficiency();
    void SaveGIFtracks(string filename, unsigned int n);
    void UpdateHistograms();
    void DrawStraight(double anode0, double anode1, double phi0 = 0);
    void Save2D(const char* name);
    void Write();
    void Reset();

    TH1D *hAnodeHits = nullptr;
    TH1D *hMulti = nullptr;
    TH1D *hMultiP = nullptr;
    TH1D *hFirstTime = nullptr;
    TH2D *hRofT_straight = nullptr;
    TH2D *hPoints = nullptr;
    TH2D *hMean = nullptr;
    TH2D *hMax = nullptr;
    TH2D *hMaxD = nullptr;
    TH2D *hMaxI = nullptr;
    TH1D *hGaps = nullptr;
    TH1D *hDiscard = nullptr;
    TH1D *hNGaps = nullptr;
    TH1D *hGapRatio = nullptr;
    TH2D *hRMS = nullptr;
    TH1D *hHeight = nullptr;
    TH1D *hFitRes = nullptr;
    // TH2D *hTopSc = nullptr;
    // TH2D *hBotSc = nullptr;
    TH1D *hTopSc = nullptr;
    TH1D *hBotSc = nullptr;
private:
    void CreateHistograms();
    TH1D* AddH1(const char* hname, const char* htitle, int nx, double x0, double x1, bool log=false);
    TH2D* AddH2(const char* hname, const char* htitle, int nx, double x0, double x1, int ny, double y0, double y1);
    vector<TH1*> histos;
    vector<bool> logscale;
    TCanvas *canvas2D = nullptr, *canvasTime = nullptr, *canvas3D = nullptr, *canvasHist = nullptr, *canvasEff = nullptr;
    TGraphErrors graph2D;
    TGraph2DErrors graph3D;
    TGraph gEff, *awires = nullptr, *fwires = nullptr, graphTime, graphTimenoSP;
    TCanvas *createCanvas(const char* name, const char* title);
    vector<TEllipse*> cath, pads;
    TLine *l = nullptr;
    string gifname;
    int ngif = 0;
};

#endif
