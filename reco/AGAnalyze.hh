#ifndef AGANALYZE_H
#define AGANALYZE_H

#include <vector>
#include <set>
#include <string>

#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>

#include "Signals.hh"
#include "SpacePoints.hh"

#include "RootPlotter.hh"

#include "TrackViewer.hh"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;

extern TrackViewer *gView;

class AGAnalyze{
public:
    AGAnalyze(): spoints(&sig) {
        spoints.SetPhi0(phi_0);
        spoints.SetT0(tdelay);
    };

    void Analyze(Alpha16Event *e, int gVerb=0);
    void Reset();
    bool SetPlotTracks(bool pt = true){ plotTracks = pt; return plotTracks; };
    void SetDelay(double td){
        tdelay = td;
        spoints.SetT0(tdelay);
    };
    void SetPhi0(double p0){
        phi_0 = p0;
        spoints.SetPhi0(phi_0);
    };
    bool SetGas(double CO2frac = 0.1, double p = 760, double T = 293.15){
        std::cout << "AGAnalyze::SetGas(" << CO2frac << ')' << std::endl;
        gasOK = spoints.SetGas(CO2frac, p, T);
        return gasOK;
    };
    bool SetB(double b){
        if(gasOK) gasOK = spoints.SetB(b);
        return gasOK;
    };
    void NewRun(short run){
        sig.MapElectrodes(run);
        phi_0 = sig.GetPhi0();
        spoints.SetPhi0(phi_0);
    }
    void UpdatePlots(){
        rplot.UpdateHistograms();
    }
    void SaveDeconv(string filename){
        sig.SaveDeconv(filename);
    }

    void SaveGIFtracks(string filename, unsigned int n){
        rplot.SaveGIFtracks(filename, n);
    }

    void PlotEfficiency(){ rplot.PlotEfficiency(); };
    void Write(){ rplot.Write();};
    void SetShowDeconv(int sd = 1){ sig.SetShowDeconv(sd); };

private:
    // Trigger delay
    double tdelay = 1525;
    // Prototype rotation
    double phi_0 = 79.453*M_PI/180.;

    bool plotTracks = false;

    Signals sig;
    SpacePoints spoints;
    RootPlotter rplot;

    bool gasOK = false;

    unsigned int fullcosmics = 0, analyzedevents = 0, saved = 0;
    const unsigned int saveLimit = 16;

    // FIXME: Somehow this doesn't work.
    // const vector<double> params = {
    // 500,           // thres
    // 0.5,           // CFD frac
    // 0.1275,        // neighbour factor
    // 0.0365,        // 2nd neighbour factor
    // 0.012,         // 3rd neighbour factor
    // 0.0042,        // 4th neighbour factor
    // 10            // minimum height for signal deconv.
    // };
};

#endif
