#include "AGAnalyze.hh"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <numeric>

#include <TApplication.h>
#include "TTrack.hh"
#include "TFitHelix.hh"
#include "TFitLine.hh"
#include "StraightTrack.hh"
#include "TPCBase.hh"

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::set;

void AGAnalyze::Reset(){
    fullcosmics = 0;
    analyzedevents = 0;
    saved = 0;
    rplot.Reset();
}

void AGAnalyze::Analyze(Alpha16Event *e, int gVerb){
    if(!e->numChan){
        //        cout << "EMPTY" << endl;
        return;
    }
    //    std::cout<<"Event #"<<e->eventNo<<std::endl;
    static vector<double> params = {
    500,           // thres
    0.5,           // CFD frac
    3400,             // maximum signal mean (discards huge noise signals)
    5            // minimum height for signal deconv.
    };
    params[0] = 1000;

    static vector<double> factors = {
        0.1275,        // neighbour factor
        0.0365,        // 2nd neighbour factor
        0.012,         // 3rd neighbour factor
        0.0042,        // 4th neighbour factor
    };
    // Prepare for new event
    sig.Reset(*e, 10.); /// FIXME: crashes here
    spoints.Reset(&sig);

    //assert(sig.anodes.size() >= sig.mean.size());

    if(rplot.hMean){
        for(unsigned int i = 0; i < sig.mean.size(); i++)
            rplot.hMean->Fill(sig.anodes[i].i, sig.mean[i]);
    }
    //    sig.SetDebug();

    //    sig.SetDebug();
    int times = sig.FindTimes(sig_an, 1, params, factors);
    if(rplot.hMaxD && rplot.hMaxI){
        double t0_ind = tdelay-20;
        double t1_ind = tdelay+320;
        double t2_drift = tdelay+4500;
        for(auto s: sig.sanode){
            int a = s.i+0.01; // to take care of wire +-0
            if(s.sec) a *= -1.;
            if(s.t >= t0_ind && s.t < t1_ind)
                rplot.hMaxI->Fill(a, s.height);
            else if(s.t >= t1_ind && s.t < t2_drift)
                rplot.hMaxD->Fill(a, s.height);
        }
    }
    if(rplot.hMulti) rplot.hMulti->Fill(times);

    times = sig.FindTimes(sig_an, 3, params, factors);
    if(rplot.hMax){
        assert(sig.maximum.size() == sig.aresIndex.size());
        for(unsigned int i = 0; i < sig.maximum.size(); i++){
            int a = sig.aresIndex[i].i+0.01; // to take care of wire +-0
            if(sig.aresIndex[i].sec) a *= -1.;
            rplot.hMax->Fill(a, sig.maximum[i]);
        }
    }
    if(rplot.hDiscard){
        for(unsigned int i = 0; i < sig.discarded.size(); i++)
            rplot.hDiscard->Fill(sig.anodes[sig.discarded[i]].i);
    }
    if(rplot.hMultiP) rplot.hMultiP->Fill(times);

    //    cout << times << " LE times" << endl;

    for( auto aw:sig.sanode )
	rplot.hHeight->Fill(aw.height);

    // Order signals by time and height
    set<Signals::signal, Signals::signal::heightorder> signals(sig.sanode.begin(), sig.sanode.end());
    set<Signals::signal, Signals::signal::timeorder> tsignals(sig.sanode.begin(), sig.sanode.end());

    set<int> anodes1, anodes2;

    if(signals.size() >= 8){
        for(auto s: signals){
            if(s.i < TPCBase::NanodeWires/2) anodes1.insert(s.i);   // split up to deal with the 255<->0 jump
            else anodes2.insert(s.i);
        }

        set<int> gaps;
        int last = -1;
        for(auto a: anodes1){
            if(last != -1){
                if(a - last > 1 && a - last < 3){  // currently single wire gaps, but can be changed
                    for(auto i = last+1; i < a; i++){
                        gaps.insert(i);
                    }
                }
            }
            last = a;
        }
        last = -1;
        for(auto a: anodes2){
            if(a - last > 1 && a - last < 3){
                if(a - last != 1){
                    for(auto i = last+1; i < a; i++){
                        gaps.insert(i);
                    }
                }
            }
            last = a;
        }

        if(anodes2.size() && anodes1.size()){
            int d1 = *anodes2.begin() - *anodes1.rbegin();
            int d2 = *anodes1.begin()+TPCBase::NanodeWires - *anodes2.rbegin();
            cout << "d1 = " << d1 << ", d2 = " << d2 << endl;
            if( d1 > d2 ){  // track crosses 255<->0 boundary
                if(d2 < 3){
                    for(int i = *anodes2.rbegin()+1; i < TPCBase::NanodeWires; i++) gaps.insert(i);
                    for(int i = 0; i < *anodes1.begin(); i++) gaps.insert(i);
                }
            } else {  // track only crosses 127<->128 "boundary"
                if(d1 < 3) for(int i = *anodes1.rbegin()+1; i < *anodes2.begin(); i++) gaps.insert(i);
            }
        }

        if(rplot.hNGaps) rplot.hNGaps->Fill(gaps.size());
        if(rplot.hGapRatio) rplot.hGapRatio->Fill(double(gaps.size())/double(signals.size()+gaps.size()));

        gVerb = 0;
        if(gVerb && gaps.size()){
            cout << "anodes:\t";
            for(auto a: anodes1) cout << a << '\t';
            cout << "/\t";
            for(auto a: anodes2) cout << a << '\t';

            cout << endl << "gaps:\t";
            for(auto g: gaps) cout << g << '\t';
            cout << endl;
        }
        for(auto g: gaps) rplot.hGaps->Fill(g);
    }
    if(signals.size()){
        if(rplot.hAnodeHits) rplot.hAnodeHits->Fill(signals.begin()->i);
        if(rplot.hFirstTime) rplot.hFirstTime->Fill(tsignals.begin()->t);
    }

    // params[0] = 1000;
    // times = sig.FindTimes(sig_an, 3,params);
    // times = sig.FindTimes(sig_an, 3);
    //    cout << times << " DECONV times" << endl;

    if(times){
        if(rplot.hRMS){
            for(unsigned int i = 0; i < sig.rms.size(); i++){
                int w = sig.aresIndex[i].i;
                rplot.hRMS->Fill(w, sig.rms[i]);
            }
        }
        //        sig.SetVerbose();
        // Look for tracks passing two well-separated anode wires at a time close to the scintillator trigger
        vector<double> intersect = sig.FindAnodeIntersect(tdelay, 32);
        //        sig.SetVerbose(false);

        // Produce 2D space points from signal times
        auto points = spoints.GetRPhiPoints();
        //        cout << points.size() << " points found." << endl;

	gVerb=0;
	bool Bfield=false;
	//Fit spacepoints to the appropriate track model
	if(gVerb>1) cout << "new TTrack"<<endl;;
	TTrack track(spoints.GetSpacePoints(),Bfield);
	if(gVerb>1) cout<<"# of Reconstructed Points: "<<track.GetNumberOfPoints()<<endl;
	//	track.SetDistCut(8.);
	//	track.SetDistCut(8.,0.15,4.);
	//	track.SetDistCut(8.,0.05,4.);
	//	track.SetDistCut(10.,0.05,4.);
	track.SetDistCut(9.,0.05,4.);
	int ntracks = track.TrackFinding();
	//	int result = track.Fit();
	if(ntracks > 0)
	  track.Fit();

        if(rplot.hFitRes){
            auto *fits = track.GetTracks();
            for(int i = 0; i < fits->GetEntries(); i++){
                TFitLine *fit = (TFitLine*)fits->At(i);
                double res = fit->CalculateResiduals();
                rplot.hFitRes->Fill(sqrt(res/double(fit->GetNumberOfPoints())));
            }
        }
        if(rplot.hPoints)
            for(auto p: points)
                if(!(p.x==0 && p.y==0))
                    rplot.hPoints->Fill(p.x, p.y);
        TrackViewer *view = nullptr;
	if( ntracks > 0 && plotTracks )
	   {
	     view = new TrackViewer(&track);
	     TString cname = TString::Format("AgTPC_E%04d",e->eventNo);
	     view->Draw2D(cname.Data());
	   }
        // Consider only tracks of enough points that pass by a read out anode
        if(intersect.size() && points.size() > 24){
            if(intersect.size() == 2){

                if(plotTracks){
                    rplot.PlotTime(points);
                    rplot.PlotXY(points, phi_0);
		    rplot.DrawStraight(intersect[0], intersect[1], phi_0);
		    // sleep(1);
                }
                fullcosmics++;


                // Assume straight track passing through both anode wires to interpolate r
                StraightTrack strack(intersect[0], intersect[1]);
		//		cout << "Full cosmic!" << endl;
                for(auto s: sig.sanode){
                    double r  = strack.GetR(s.i);
                    if(rplot.hRofT_straight->GetYaxis()->FindBin(r) == rplot.hRofT_straight->GetYaxis()->FindBin(TPCBase::AnodeWiresR*10.))  // move pointless wire hit peak into overflow bin
                        r = 1e6;
                    rplot.hRofT_straight->Fill(s.t-tdelay, r);
                }
            }
            //            UpdatePlots();
        } else if(plotTracks && points.size() > 10){
            rplot.PlotTime(points);
            rplot.PlotXY(points, phi_0);
	    // sleep(1);
        }
        if( plotTracks )
            {
                static int steps = 0;
                extern TApplication* xapp;
		if (xapp && steps == 0) {
		  printf("Waiting for app->Run(), use \"File->Quit ROOT\" to continue\n");
                  UpdatePlots();
		  xapp->Run(kTRUE);
                  cout << "How many events until I should pause (-1 to go until end of file, ^C to stop)?" << endl;
                  std::cin >> steps;
		}
                steps--;
            }
        if(view) delete view;
	// delete track;
    }
    analyzedevents++;
}
