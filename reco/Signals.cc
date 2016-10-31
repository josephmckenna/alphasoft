#include "Signals.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <cassert>

#include <TFile.h>

using std::cout;
using std::cerr;
using std::flush;
using std::endl;
using std::setw;
using std::vector;
using std::set;
using std::map;
using std::string;

Signals::Signals(short run){
    MapElectrodes(run);
    isGarfield = run<0;
    dt_an = 10;
#ifdef HAVE_ROOT
    // if(showDeconv){
        // c.SetCanvasSize(1100,1100);
        // c.SetTitle("sample signals");
        if(!c) c = new TCanvas("csig","Sample signals",1100,1100);
        c->DivideSquare(display);
        c->Iconify();
        for(unsigned int i = 0; i < display; i++){
            TString hn = TString::Format("h%dxx",i);
            h[i] = new TH1D(hn.Data(), "",7000/dt_an,0,7000);
            h[i]->SetLineColor(kOrange);
            h[i]->SetFillColor(0);
            h[i]->SetFillStyle(0);
            h[i]->SetMinimum(-8192);
            h[i]->SetMaximum(8192);
            hn = TString::Format("hsub%dxx",i);
            hsub[i] = new TH1D(hn.Data(), "",7000/binsize,0,7000);
            hsub[i]->SetLineColor(kBlack);
            hsub[i]->SetFillColor(0);
            hsub[i]->SetFillStyle(0);
            hsub[i]->SetMinimum(-8192);
            hsub[i]->SetMaximum(8192);
            hn = TString::Format("hsig%dxx",i);
            hsig[i] = new TH1D(hn.Data(), "",7000/binsize,0,7000);
            hsig[i]->SetLineColor(kRed);
            hsig[i]->SetFillColor(0);
            hsig[i]->SetFillStyle(0);
            hsig[i]->SetMinimum(0);
            hsig[i]->SetMaximum(8192);
            c->cd(i+1);
            h[i]->Draw();
            hsub[i]->Draw("same");
            hsig[i]->Draw("same");
        }
        c->Update();
    //     c->Show();
    // }
#endif
}

Signals::Signals(const Alpha16Event &anodeSignals, double timeBinA, short run){
    MapElectrodes(run);
    isGarfield = run<0;
#ifdef HAVE_ROOT
    if(showDeconv){ /// FIXME: not up to date, as not used
        // c.SetCanvasSize(1100,1100);
        // c.SetTitle("sample signals");
        if(!c) c = new TCanvas("csig","Sample signals",1100,1100);
        c->DivideSquare(display);
        for(unsigned int i = 0; i < display; i++){
            h[i]->SetLineColor(kOrange);
            h[i]->SetFillColor(0);
            h[i]->SetFillStyle(0);
            hsub[i]->SetLineColor(kBlack);
            hsub[i]->SetFillColor(0);
            hsub[i]->SetFillStyle(0);
            hsig[i]->SetLineColor(kRed);
            hsig[i]->SetFillColor(0);
            hsig[i]->SetFillStyle(0);
            c->cd(i+1);
            h[i]->Draw();
            hsub[i]->Draw("same");
            hsig[i]->Draw("same");
        }
        c->Update();
        c->Draw();
    }
#endif
    Reset(anodeSignals, timeBinA);
}

void Signals::Reset(const Alpha16Event &anodeSignals, double timeBinA){
    anodewaveform = anodeSignals.waveform;
    udpPresent = anodeSignals.udpPresent;
    dt_an = timeBinA;
    mean.clear();
    for(unsigned int i = 0; i < MAX_ALPHA16 * NUM_CHAN_ALPHA16; i++){
        if(udpPresent[i]){
            auto &s = anodewaveform[i];
            mean.push_back(std::accumulate(s.begin(), s.end(), 0.0)/static_cast<double>( s.size() ));
        } else
            mean.push_back(0);
    }
#ifdef HAVE_ROOT
    if(showDeconv){
        for(unsigned int i = 0; i < display; i++){
            h[i]->Reset();
            hsub[i]->Reset();
            hsig[i]->Reset();
            c->GetPad(i+1)->Modified();
        }
        c->Update();
    }
#endif
}

int Signals::MapElectrodes(short run){
    anodes.clear();
    pads.clear();

    if(run < 0){
        for(unsigned int i = 0; i < 2*TPCBase::NanodeWires; i++){
            int ii = i % TPCBase::NanodeWires;
            short us_ds = (i / TPCBase::NanodeWires);
            anodes.emplace_back(us_ds, ii);
        }
        phi0 = 0;
    } else {
        map<short, vector<short> > moduleMap;
        std::ifstream mMapFile("alpha16.map");  // FIXME: make map file run no. -> list of AWC sectors
        if(mMapFile.is_open()){
            while(mMapFile.good()){
                if(mMapFile.peek() == '#'){
                    char buf[1024];
                    mMapFile.getline(buf,1023);
                    continue;
                }
                short runN;
                mMapFile >> runN;
                if(mMapFile.good()){
                    if(moduleMap.find(runN) != moduleMap.end()){
                        std::cerr << "Duplicate entries for run " << runN << " in alpha16.map!" << endl;
                        return -1;
                    }
                    string line;
                    getline(mMapFile,line);
                    std::istringstream iss(line);
                    short sec;
                    iss >> phi0;
                    phi0 *= M_PI/180.;
                    while(iss >>  sec) moduleMap[runN].push_back(sec);
                }
            }
        } else {
	  printf("Signals::MapElectrodes: cannot read from alpha16.map!\n");
	  return -1;
	}

        vector<short> vec;
        for(auto it = moduleMap.rbegin(); it != moduleMap.rend(); it++){
            if(run >= it->first){
                cout << "run " << run << " >= " << it->first << endl;
                vec = it->second;
                break;
            }
        }
        for(unsigned short mod = 0; mod < MAX_ALPHA16 && mod < vec.size(); mod++){
            short sec = vec[mod];
            short tb = sec<0;
            sec %= 16;
            cout << "Module " << mod << " -> AWC " << sec << '\t' << (tb?"bottom":"top") << endl;
            for(int i = 0; i < NUM_CHAN_ALPHA16; i++){
                anodes.emplace_back(tb, abs(sec)*NUM_CHAN_ALPHA16+i);
            }
        }
    }
    cout << "Electrode map set: " << anodes.size()+pads.size() << endl;
    // for(unsigned int i = 0; i < anodes.size(); i++) cout << i << '\t' << (anodes[i].sec?'b':'t') << '\t' << anodes[i].i << endl;
    return anodes.size()+pads.size();
}


bool Signals::ReadResponseFile(sigchoice choice, int binsize, double frac){
    vector<double> *resp;
    int *theBin = nullptr;
    double scale(1.);
    std::ostringstream oss;
    assert(getenv("AGTPC_TABLES"));
    oss << getenv("AGTPC_TABLES") << "/response/";
    switch(choice){
    case sig_an: oss << "anodeResponse.dat"; resp = &anodeResponse; theBin = &anodeRbin; scale = -1.; break;
    case sig_p: oss << "padResponse.dat"; resp = &padResponse; theBin = &padRbin; break;
    }
    string filename = oss.str();
    cout << "Reading in response file " << filename << endl;
    resp->clear();
    std::ifstream respFile(filename.c_str());
    if(respFile.good()){
        respFile.peek();
        double prevBinEdge(unknown), fbinsize(0);
        while(respFile.good()){
            double binedge, val;
            respFile >> binedge >> val;
            if(respFile.good()){
                if(prevBinEdge != unknown){
                    if(!fbinsize) fbinsize = binedge - prevBinEdge;
                    else if(fbinsize != binedge - prevBinEdge){
                        cerr << "File has unequal bin sizes:" << endl;
                        cerr << binedge - prevBinEdge << " != " << binedge << endl;
                        resp->clear();
                        return false;
                    }
                }
                prevBinEdge = binedge;
                resp->push_back(scale*val);
            }
        }
        if(fbinsize != 1){
            cerr << "Response files should be in 1ns steps. Aborted." << endl;
            resp->clear();
            return false;
        }
        *resp = Rebin(*resp, binsize);

        double max = *std::max_element(resp->begin(), resp->end());
        // cout << max << " --> " << frac*max << endl;
        for(*theBin = 0; *theBin < int(resp->size()); (*theBin)++){
            // cout << *theBin << '\t' << (*resp)[*theBin];
            if((*resp)[*theBin] > frac*max){
                // cout << " <---" << endl;
                break;
            }
            // cout << endl;
        }
    } else cerr << "Couldn't find response file " << filename << endl;
    return bool(resp->size());
}

int Signals::FindTimes(sigchoice choice, short mode, const vector<double> &params, vector<double> factors){
    blergh++;
    double dt = 1.;
    double thres = 5e-3;
    double frac = 0.1;
    double scale = 1;
    double maxMean = 10000;
    discarded.clear();
    maximum.clear();

    const vector<int16_t> *waveforms;
    vector<signal> *svec(0);
    vector<electrode> *electrodes;
    unsigned int ntotal(0);
    switch(choice){
    case sig_an:
        waveforms = anodewaveform;
        svec = &sanode;
        electrodes = &anodes;
        scale = -1;
        dt = dt_an;
        ntotal = MAX_ALPHA16 * NUM_CHAN_ALPHA16;
        break;
    case sig_p:
        waveforms = padwaveform;
        svec = &spad;
        electrodes = &pads;
        factors.clear();
        dt = dt_pad;
        ntotal = 0;  // FIXME: Pad stuff is screwed up
        break;
    default: return -1;
    }
    svec->reserve(ntotal);
    svec->clear();

    if(params.size() >= 1) thres = params[0];
    if(params.size() >= 2) frac = params[1];
    if(params.size() >= 3) maxMean = params[2];

    switch(mode){
    case 1:{                  // simple leading edge detection
        for(unsigned int i = 0; i < ntotal; i++){
            if(udpPresent[i]){
                if(abs(mean[i]) <= maxMean){
                    double ped(0.);
                    if(PED_LENGTH){
                        for(int b = 0; b < PED_LENGTH; b++) ped += waveforms[i][b];
                        ped /= double(PED_LENGTH);
                    }
                    double max;
                    if(scale > 0) max = double(*std::max_element(waveforms[i].begin(), waveforms[i].end())) - ped;
                    else max = scale * (double(*std::min_element(waveforms[i].begin(), waveforms[i].end())) - ped);
                    if(debug){
                        cout << "signal " << i << ": max = " << max << " , ped = " << ped << ", thres = " << thres << endl;
                    }
                    if(max > thres){
                        unsigned int b = 0;
                        for(b = 0; b < waveforms[i].size(); b++){
                            if(scale*(double(waveforms[i][b]) - ped) > frac*max) break;
                        }
                        double t = (b+0.5) * dt;
                        svec->emplace_back(electrodes->at(i),t,max);
                        maximum.push_back(max);
                    }
                } else discarded.push_back(i);
            }
        }
        break;
    }
    case 2:{                  // leading edge with neighbour induction subtraction
        cerr << "Basic neighbour subtraction mode currently untested." << endl;    // FIXME: Fix or remove
        return -3;
        // if(choice == sig_p){
        //     cerr << "Neighbour subtraction mode not currently supported for pads." << endl;
        //     return -2;
        // }
        // vector<vector<double> > subtracted;
        // vector<electrode> w;
        // for(unsigned int i = 0; i < waveforms->size(); i++){
        //     double max;

        //     if(scale > 0) max = *std::max_element(waveforms[i].w->samples, waveforms[i].w->samples+nsamples);
        //     else max = scale * *std::min_element(waveforms[i].w->samples, waveforms[i].w->samples+nsamples);
        //     if(max > thres){
        //         subtracted.emplace_back(waveforms[i].w->samples, waveforms[i].w->samples+nsamples);
        //         w.push_back(electrodes->at(i));
        //     }
        // }

        // if(subtracted.size()){
        //     for(unsigned int i = 0; i < subtracted.size(); i++){
        //         for(unsigned int k = 0; k < w.size(); k++){
        //             for(unsigned int l = 0; l < factors.size(); l++)
        //                 if(abs(w[k].i - w[i].i)%(subtracted.size()-2) == l+1){ // FIXME: wrong
        //                     for(unsigned int b = 0; b < subtracted[i].size(); b++)
        //                         subtracted[k][b] += subtracted[i][b]*factors[l];
        //                 }
        //         }
        //     }
        //     for(unsigned int i = 0; i < subtracted.size(); i++){
        //         double max;

        //         if(scale > 0) max = *std::max_element(waveforms[i].w->samples, waveforms[i].w->samples+nsamples);
        //         else max = scale * *std::min_element(waveforms[i].w->samples, waveforms[i].w->samples+nsamples);
        //         if(max > thres){
        //             unsigned int b = 0;
        //             for(b = 0; b < nsamples; b++){
        //                 if(*(waveforms[i].w->samples+b) > frac*max) break;
        //             }
        //             double t = (b+0.5) * dt;
        //             svec->push_back(signal(w[i],t,max));
        //         }
        //     }
        // }
        break;
    }
    case 3:{                  // fancier time extraction (response fitting and induction removal)
        double minIons(10);
        if(params.size() > 3) minIons = params[3];
        // double timeBin(0);

        // FIXME: Remove this! Just for testing with reduced complexity
        //        factors.clear();
        ///////////////////////////////////////////////////////////////

        vector<double> *response;
        rms.clear();

        vector<vector<double> > subtracted;
        vector<vector<double> > *result;
        vector<electrode> *w;
        std::map<electrode, int, electrode_cmp> displayMap, indexMap, resIndexMap;
        int theBin(-1);
        switch(choice){
        case sig_an:
            response = &anodeResponse;
            result = &aresult;
            w = &aresIndex;
            break;
        case sig_p:
            response = &padResponse;
            result = &presult;
            w = &presIndex;
            break;
        default: return -1;
        }
        w->clear();
        result->clear();
        w->reserve(ntotal);
        result->reserve(ntotal);
        subtracted.reserve(ntotal);
        subtracted.clear();

        if(!response->size()){
            if(!ReadResponseFile(choice, binsize, frac)) return 0;
            switch(choice){
            case sig_an:
                response = &anodeResponse;
                break;
            case sig_p:
                response = &padResponse;
                break;
            }
        }
        switch(choice){
        case sig_an:
            theBin = anodeRbin;
            break;
        case sig_p:
            theBin = padRbin;
            break;
        }

#ifdef HAVE_ROOT
        if(showDeconv == 2){
            // string bla;
            // cout << "Enter anything to proceed." << endl;
            // std::cin >> bla;
            c->Show();
        }
#endif
        for(unsigned int i = 0; i < ntotal; i++){
            if(udpPresent[i]){
                if(abs(mean[i]) > maxMean) discarded.push_back(i);
                else {
                    double ped(0.);

                    if(PED_LENGTH){
                        for(unsigned int b = 0; b < PED_LENGTH; b++) ped += waveforms[i][b];
                        ped /= double(PED_LENGTH);
                    }
                    double max;
                    if(scale > 0) max = double(*std::max_element(waveforms[i].begin(), waveforms[i].end())) - ped;
                    else max = scale * (double(*std::min_element(waveforms[i].begin(), waveforms[i].end())) - ped);
                    if(max > thres){
                        subtracted.emplace_back(Rebin(waveforms[i], binsize/dt, ped));
#ifdef HAVE_ROOT
                        if(showDeconv){
                            displayMap[electrodes->at(i)] = -1;
                            indexMap[electrodes->at(i)] = i;
                            resIndexMap[electrodes->at(i)] = result->size();
                        }
#endif
                        maximum.push_back(max);
                        w->push_back(electrodes->at(i));
                        result->emplace_back(subtracted.back().size());
                    }
                }
            }
        }
#ifdef HAVE_ROOT
        bool animate = false;
        if(showDeconv){
            int shown = 0;
            for(auto &dispEl: displayMap){
                auto el = dispEl.first;
                if(shown < display){
                    //                        cout << i << ": " << max << " > " << thres << " (" << double(*std::min_element(waveforms[i].begin(), waveforms[i].end())) << " - " << ped << ")" << endl;
                    //                        cout << shown << " --> " << i << endl;
                    TString hn = TString::Format("ha%d_%d",el.i, el.sec);
                    h[shown]->SetName(hn.Data());
                    hn = TString::Format("hsub%d_%d",el.i, el.sec);
                    hsub[shown]->SetName(hn.Data());
                    hn = TString::Format("hsig%d_%d",el.i, el.sec);
                    hsig[shown]->SetName(hn.Data());
                    int i = indexMap[el];
                    for(unsigned int b = 0; b < waveforms[i].size(); b++){
                        h[shown]->SetBinContent(b+1, waveforms[i][b]);
                    }
                    // if(hsub[shown]->GetNbinsX() < (int)subtracted.back().size())
                    //     cout << hsub[shown]->GetNbinsX() << " < " << subtracted.back().size() << endl;
                    int resIndex = resIndexMap[el];
                    for(unsigned int b = 0; b < subtracted[resIndex].size(); b++)
                        hsub[shown]->SetBinContent(b+1, subtracted[resIndex][b]);
                    c->GetPad(shown+1)->Modified();
                    dispEl.second = shown;
                    shown++;
                }

            }
            // TFile of("sample.root","RECREATE");
            // of.cd();
            // for(int i = 0; i < display; i++){
            //     h[i].Write();
            //     hsub[i].Write();
            // }
            // of.Write();
            // of.Close();
            c->Update();
            if(saveFile.length()){
                animate = (saveFile.find("gif+") != string::npos);
                c->Print(saveFile.c_str());
            }
        }
#endif

        if(subtracted.size()){
            int nsamples = subtracted.back().size();
            for(int b = theBin; b < int(nsamples); b++){    // b is the current bin of interest
                if(verbose) cout << "\b\b\b\b" << setw(2) << 100*(b+1)/nsamples << " %" << flush;
                set<myhist,comp_hist> histset;
                for(unsigned int i = 0; i < subtracted.size(); i++){
                    myhist mh;
                    mh.h = &subtracted[i];
                    mh.index = i;
                    mh.val = scale*subtracted[i][b];
                    histset.insert(mh);
                }
                double t = (b-theBin+0.5)*double(binsize);
                // cout << "Time " << t << endl;
                map<unsigned int, signal&> found;

                double neTotal = 0;
                for(auto it = histset.begin(); it != histset.end(); it++){
                    vector<double> &wf = *it->h;
                    unsigned int i = it->index;
                    auto wire = (*w)[i];
                    double ne = scale*wf[b]/(*response)[theBin]; // number of "electrons"
                    if(ne >= minIons){
                        neTotal += ne;
                        for(int bb = b-theBin; bb < int(nsamples); bb++){  // loop over all bins for subtraction
                            int respBin = bb-b+theBin; // the bin corresponding to bb in the response
                            for(unsigned int k = 0; k < w->size(); k++){  // loop over all signals looking for neighbours
                                auto wire2 = (*w)[k];
                                for(unsigned int l = 0; l < factors.size(); l++)
                                    if((unsigned int)abs(wire2.i - wire.i) == l+1 ||
                                       (unsigned int)abs(wire2.i - wire.i - TPCBase::NanodeWires) == l+1 ||
                                       (unsigned int)abs(wire2.i - wire.i + TPCBase::NanodeWires) == l+1){
                                        // TPCBase::NanodeWires is OK because this loop is never entered for pad signals
                                        vector<double> &wf2 = subtracted[k];
                                        if(respBin < int(response->size()) && respBin >= 0){
                                            if(bb < 0 || bb >= (int)wf2.size()) cerr << "EEEEEE1: " << bb << " is not between 0 and " << wf2.size() << endl;
                                            if(respBin < 0 || respBin >= (int)response->size()) cerr << "EEEEEE2: " << respBin << " is not between 0 and " << response->size() << endl;
                                            wf2.at(bb) += ne/scale*factors.at(l)*response->at(respBin);
#ifdef HAVE_ROOT
                                            if(showDeconv){
                                                if(displayMap.find(wire2) != displayMap.end()){
                                                    int index = displayMap[wire2];
                                                    assert(resIndexMap[wire2] == int(k));
                                                    if(index >= 0){
                                                        hsub[index]->SetBinContent(bb+1, wf2[bb]);
                                                        c->GetPad(index+1)->Modified();
                                                    }
                                                }
                                            }
#endif
                                        }
                                    }
                            }
                            if(respBin < int(response->size()) && respBin >= 0){
                                if(bb < 0 || bb >= (int)wf.size()) cerr << "EEEEEE1: " << bb << " is not between 0 and " << wf.size() << endl;
                                if(respBin < 0 || respBin >= (int)response->size()) cerr << "EEEEEE2: " << respBin << " is not between 0 and " << response->size() << endl;
                                wf.at(bb) -= ne/scale*response->at(respBin);
#ifdef HAVE_ROOT
                                if(showDeconv){
                                    if(displayMap.find(wire) != displayMap.end()){
                                        int index = displayMap[wire];
                                        if(index >= 0){
                                            hsub[index]->SetBinContent(bb+1, wf[bb]);
                                            c->GetPad(index+1)->Modified();
                                        }
                                    }
                                }
#endif
                            }
                        }
                        if(b-theBin >= 0){
                            (*result)[i][b-theBin] = 1/minIons*ne;
#ifdef HAVE_ROOT
                            if(showDeconv){
                                if(displayMap.find(wire) != displayMap.end()){
                                    int index = displayMap[wire];
                                    if(index >= 0){
                                        hsig[index]->SetBinContent(b-theBin+1, 1000/minIons*ne);
                                        cout << "modified " << index << endl;
                                        c->GetPad(index+1)->Modified();
                                    }
                                }
                            }
#endif
                            //                        svec->push_back(signal(w[i],b*binsize,ne));
                            svec->emplace_back(wire,t,ne);
                            auto foundit = found.find(wire.i);
                            if(foundit != found.end()){
                                fullAnodeSig.emplace_back(foundit->second, svec->back());
                            } else {
                                found.emplace(wire.i, svec->back());
                            }
                        }
                    }

                }
#ifdef HAVE_ROOT
                if(showDeconv > 1){
                    c->Update();
                    if(animate && neTotal > 0){
                        c->Print(saveFile.c_str());
                        cout << "Frame " << b << " of " << saveFile << endl;
                    }
                    //                    usleep(500);
                }
#endif
            }
            if(verbose) cout << endl;
            for(auto s: subtracted){
                static int blub = 0;
                blub++;
                rms.push_back(sqrt(std::inner_product(s.begin(), s.end(), s.begin(), 0.)/ static_cast<double>( s.size() )));
            }
        }
#ifdef HAVE_ROOT
        if(showDeconv) c->Update();
        if(saveFile.length()){
            if(animate) saveFile += "+";
            c->Print(saveFile.c_str());
            saveFile.clear();
        }
#endif
    }
    }
    return svec->size();
}

int Signals::ZfromChargeDiv(){   // FIXME: Can't test this with Garfield without faking it again
    int found = 0;
    if(!fullAnodeSig.size()) return 0;
    map<int, int> wmap;
    for(unsigned int i = 0; i < fullAnodeSig.size(); i++) wmap[fullAnodeSig[i].first.i] = i;
    assert(wmap.size() == fullAnodeSig.size());
    // for(auto & s: sanode){
    //     TH1D *hDS = aresDS[wmap[s.i]];
    //     TH1D *htot = aresult[wmap[s.i]];
    //     if(hDS && htot){
    //         int b = hDS->FindBin(s.t);
    //         s.z = (hDS->GetBinContent(b)/htot->GetBinContent(b) - 0.5)*2.0*TPC::HalfWidthZ;
    //         found++;
    //     }
    // }
    return found;
}

vector<set<Signals::signal, Signals::signal::heightorder> > Signals::MatchPads(double anodethres, double padthres) const{
    vector<set<signal, signal::heightorder> > padmatch(sanode.size());
    if(!presult.size()){
        padmatch.clear();
        return padmatch;
    }
    int totMatches(0);
    for(unsigned int i = 0; i < sanode.size(); i++){
        if(sanode[i].height < anodethres) continue;
        auto &matches = padmatch[i];
        double anodeR, anodephi;
        TPCBase::GetAnodePosition(sanode[i].i, anodeR, anodephi);

        for(unsigned int p = 0; p < presult.size(); p++){
            electrode padnum = presIndex[p];
            double padz, padphi;
            TPCBase::GetPadPosition(padnum.i, padz, padphi);
            if(sanode[i].z != unknown && abs(sanode[i].z - padz) > ztolerance){
                //                cout << "z mismatch: " << sanode[i].z << " and " << padz << endl;
                continue;
            }
            if(padphi != unknown && abs(anodephi - padphi) > phitolerance){
                //                cout << "Phi mismatch: " << anodephi << " and " << padphi << endl;
                continue;
            }

            int b = sanode[i].t/dt_pad;
            if(presult[p][b] > padthres){
                matches.emplace(padnum, (b+0.5)*dt_pad, presult[p][b]);
                totMatches++;
            }
        }
    }
    if(!totMatches) padmatch.clear();
    return padmatch;
}

vector<double> Signals::FindAnodeIntersect(double t0, int separation){
    vector<double> result;
    if(!sanode.size()) return result;
    std::multiset<Signals::signal, Signals::signal::heightorder> byheight1, byheight2;
    std::multiset<Signals::signal, Signals::signal::timeorder> bytime(sanode.begin(), sanode.end());
    auto it = bytime.begin();

    double t1 = -1;
    double t2 = -1;
    double a0 = -1;
    double a1 = -1;

    double t_tol = 20;
    while(it != bytime.end()){
        if(!byheight1.size()){
            if(t0 == unknown || abs(it->t - t0) < t_tol){
                byheight1.insert(*it);
                t1 = it->t;
                a0 = it->i;
            }
        } else {
            if(abs(it->i - a0) < separation || abs(it->i - a0) > 255-separation){
                if(it->t == t1) byheight1.insert(*it);
            } else {
                if(t2 < 0 && (t0 == unknown || abs(it->t - t0) < t_tol)) {
                    t2 = it->t;
                    a1 = it->i;
                    byheight2.insert(*it);
                } else if(it->t == t2){
                    if(abs(it->i - a1) < separation || abs(it->i - a1) > 255-separation)
                        byheight2.insert(*it);
                } else break;
            }
        }
        it++;
    }

    double totheight = 0;
    if(byheight1.size()){
        a0 = 0;
        for(auto s: byheight1){
            a0 += s.height*s.i;
            totheight += s.height;
        }
        a0 /= totheight;
        result.push_back(a0);
    }
    if(byheight2.size()){
        a1 = 0;
        totheight = 0;
        for(auto s: byheight2){
            a1 += s.height*s.i;
            totheight += s.height;
        }
        a1 /= totheight;
        result.push_back(a1);
    }

    if(verbose && result.size()){
        cout << "Track passes by anode(s) " << result[0];
        if(result.size() == 2)
            cout << " and " << result[1];
        cout << endl;
    }
    return result;
}

void Signals::PrintSignals(){
    if(sanode.size()){
        cout << "Anode signals:" << endl;
        for(auto s: sanode){
            cout << s.i << '\t' << s.t << '\t' << s.height;
            if(s.z != unknown) cout  << '\t' << s.z;
            cout << endl;
        }
    }
    if(spad.size()){
        cout << "Pad signals:" << endl;
        for(auto s: spad){
            cout << s.i << '\t' << s.t << '\t' << s.height;
            if(s.z != unknown) cout  << '\t' << s.z;
            cout << endl;
        }
    }
}
