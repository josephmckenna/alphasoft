#ifndef Signals_h
#define Signals_h
#include <vector>
#include <set>
#include <map>
#include <string>

#include "TPCBase.hh"
// #include "GarfieldAlphaPacket.h"
#include "Alpha16.h"

#ifdef HAVE_ROOT
#include <TCanvas.h>
#include <TH1D.h>
#include <TSystem.h>
#endif

#define PED_LENGTH 100

using std::vector;
using std::set;
using std::pair;
using std::string;

enum sigchoice { sig_an, sig_p };

class Signals{
public:
    Signals(short run = 0);
    Signals(const Alpha16Event &anodeSignals, double timebinA = 1., short run = 0);
    int MapElectrodes(short run);   // reads ADC to electrode mapping from alpha16.map
    double GetPhi0(){    // rotational offset of TPC
        return phi0;
    }
#ifdef HAVE_ROOT
    ~Signals(){
        delete c;
        for(int i = 0; i < display; i++){
            delete h[i];
            delete hsub[i];
            delete hsig[i];
        }
    };
    void SetShowDeconv(int sd = 1){  // set to 1 to show the result of signal deconvolution on some sample signals, set to 2 to show deconv in real-time
        showDeconv = sd;
    }
    void SaveDeconv(string filename){
        if(c){
            saveFile = filename;
            gSystem->Unlink(saveFile.c_str()); // delete old file
        } else std::cerr << "No canvas to save." << std::endl;
    }
#endif
    void Reset(const Alpha16Event &anodeSignals, double timebinA = 1.);
    int FindTimes(sigchoice choice, short mode, const vector<double> &params, vector<double> factors = {
            0.1275, 0.0365, 0.012, 0.0042} ); // mode = 1 for simple leading edge, 2 for LE with neighbour subtraction (not implemented), 3 for deconvolution

    struct electrode{
        electrode(short s, int ind, double g = 1.){ sec = s; i = ind; gain = g;};
        short sec;  // for anodes sec=0 for top, sec=1 for bottom
        int i;
        double gain;
    };

    class signal{
    public:
        int i;
        short sec;  // for anodes sec=0 for top, sec=1 for bottom
        double t, height, z;
        signal(electrode el, double tt, double hh){
            i = el.i;
            sec = el.sec;
            t = tt;
            height = hh/el.gain;  // should the gain be used here?
            z = unknown;
        }

        signal(int ii, short ss, double tt, double hh){
            i = ii;
            sec = ss;
            t = tt;
            height = hh;
            z = unknown;
        }

        struct indexorder {
            bool operator() (const signal& lhs, const signal& rhs) const {
                return lhs.i<rhs.i;
            }
        };

        struct timeorder {
            bool operator() (const signal& lhs, const signal& rhs) const {
                return lhs.t<rhs.t;
            }
        };

        struct heightorder {
            bool operator() (const signal& lhs, const signal& rhs) const {
                return lhs.height>rhs.height;
            }
        };
    };

    // bool AddChargeDivZ();
    int ZfromChargeDiv();

    void PrintSignals();

    vector<set<signal, signal::heightorder> > MatchPads(double anodethres = 0, double padthres = 0) const;

    vector<electrode> aresIndex, presIndex;    // anode/pad corresponding to result waveforms
    vector<vector<double> > aresult, presult;  // waveform remainder after any deconvolution subtraction


    vector<double> FindAnodeIntersect(double t0 = unknown, int separation = 5);
    void SetDebug(bool d = true) { debug = d; };
    void SetVerbose(bool v = true) { verbose = v; };

    vector<electrode> anodes, pads;
    vector<signal> sanode, spad;
    vector<double> rms, mean, maximum;
    const vector<int16_t> *anodewaveform, *padwaveform;
    vector<int> discarded;
private:
    bool debug = false;
    bool verbose = false;
    bool isGarfield = false;
    int blergh = 0;

    double phi0 = 0;

    const bool     *udpPresent;
    const Alpha16Packet *udpPacket;
    unsigned int nsamples_a, nsamples_p;
    double dt_an, dt_pad;
    const double ztolerance = 4.;
    unsigned int binsize = 20;
    const double phitolerance = TPCBase::PadWidthPhi;

    vector<pair<signal&, signal&> > fullAnodeSig;
    vector<double> anodeResponse, padResponse;
    int anodeRbin, padRbin;

    template <typename T>
    vector<double> Rebin(const vector<T> &in, int binsize, double ped = 0.){
        //    if(binsize == 1) return in;
        vector<double> result;
        result.reserve(in.size()/binsize);
        for(unsigned int i = 0; i < in.size(); i++){
            if(i / binsize == result.size()-1) result.back() += double(in[i])-ped;
            else if(i / binsize == result.size()) result.push_back(double(in[i])-ped);
        }
        if(result.size()*binsize > in.size()){
            result.pop_back();
            //            std::cout << "Cannot rebin without rest, dropping final " << in.size() % result.size() << std::endl;
        }
        return result;
    };

    bool ReadResponseFile(sigchoice choice, int binsize = 1, double frac = 0.1);
    // ADC channels 0-15 map this way to preamp channels:
    vector<short> alpha16Channels = { 14, 12, 10, 8, 6, 4, 2, 0, 15, 13, 11, 9, 7, 5, 3, 1 };

    struct myhist {
        vector<double> *h;
        double val;
        unsigned int index;
    };

    struct comp_hist {
        bool operator() (const myhist &lhs, const myhist &rhs) const
        {return lhs.val >= rhs.val;}
    };

#ifdef HAVE_ROOT
    int showDeconv = 0;
    string saveFile;

    TCanvas *c = nullptr;
    static const unsigned short display = 16;
    TH1D *h[display], *hsub[display], *hsig[display];

    struct electrode_cmp
    {
        bool operator() (const electrode& lhs, const electrode& rhs) const
        {
            return (lhs.sec < rhs.sec) || (lhs.sec == rhs.sec && lhs.i < rhs.i);
        }
    };
#endif

};

#endif
