//
// calibration module v0.1
//
// STR for ALPHA-g TPC
//
// Author: A. Capra
// Based on RofT by L. Martin
// 

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>

#include "TH2D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include "Math/MinimizerOptions.h"
#include "TFitResult.h"

#include "manalyzer.h"
#include "AgFlow.h"

#include "Signals.hh"
#include "StraightTrack.hh"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;

class CalibModule: public TAModuleInterface
{
public:
  void Init(const std::vector<std::string> &args);
  void Finish();
  TARunInterface* NewRun(TARunInfo* runinfo);
};

class CalibRun: public TARunInterface
{
public:
  int fSeparation;
  int fCosmicsFull;
  // Trigger delay
  double fTdelay = 1525.;
  //  double fTdelay = 0.;
  TH2D* hRofT_straight;
  TF1* fit_func;

public:

  CalibRun(TARunInfo* runinfo)
    : TARunInterface(runinfo),fSeparation(32),fCosmicsFull(0)
  {
    printf("CalibRun::ctor!\n");
  }

  ~CalibRun()
  {
    printf("CalibRun::dtor!\n");
  }

  void BeginRun(TARunInfo* runinfo)
  {
    printf("CalibRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
    printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
    runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
    TDirectory* dir = gDirectory->mkdir("Calibration");
    dir->cd();

    hRofT_straight = new TH2D("hRofT_straight","straight track r vs t;t in ns;r in mm", 
			      550, -500., 5000., 900, 100., 190.);
    //    hRofT_straight->GetYaxis()->SetTitleOffset(1.4);
    //    fit_func = new TF1("fSTR","gaus(0) + gaus(3)", 100., 190.);
    fit_func = new TF1("fSTR","gaus(0)", 100., 190.);
  }

  void EndRun(TARunInfo* runinfo)
  {
    printf("CalibRun::EndRun, run %d\n", runinfo->fRunNo);

    printf("CalibRun::EndRun, Full Cosmics%d\n",fCosmicsFull);

    TH2D* hh = (TH2D*) hRofT_straight->Clone();
    hh->RebinY(15);

    TString ofname = TString::Format("RofTrun%d.dat", runinfo->fRunNo);
    std::ofstream ofs(ofname.Data());
    ofs << "t\tr0\tdr0" << endl;
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(10000);

    for(int b = 1; b <= hh->GetNbinsX(); ++b)
      {
	TString ht("py_");
	ht += b;
	TH1D *h = hh->ProjectionY(ht.Data(), b, b);
	int awb = h->FindBin(182.);
	h->SetBinContent(awb, 0.);

	if(h->Integral() < 100) continue;

	TSpectrum s(1, 0.001);
	if(s.Search(h))
	  {
	    s.Print();
	    double *p = s.GetPositionX();
	    double *ph = s.GetPositionY();
	    int pb = h->FindBin(p[0]);

	    double fwhm = h->GetBinWidth(pb);
	    for(int ib=pb; ib<hh->GetNbinsX(); ++ib)
	      {
		double bc = h->GetBinContent( ib );
		if( bc < 0.5*ph[0] )
		  {
		    fwhm = 2.* h->GetBinCenter( ib-1 );
		    break;
		  }
	      }
	    double sigma = fwhm/2.355;
	  
	    fit_func->SetParameter(0, ph[0]);
	    fit_func->SetParameter(1, p[0]);
	    fit_func->SetParameter(2, sigma);
	    //h->Fit(fit_func,"Q");
	    //h->Fit("fSTR","QEM");
	    h->Fit(fit_func,"QEM");

	    double time = hh->GetXaxis()->GetBinCenter(b),
	      radius = fit_func->GetParameter(1),
	      error = fit_func->GetParameter(2);

	    if( time < 0. || radius < 0. || radius > 190. || error > 7. ) 
	      continue;

	    ofs << time << '\t' << radius << '\t' << error << endl;
	  }// peak found
      }// bins loop
    ofs.close();

    time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
    printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
  }

  void PauseRun(TARunInfo* runinfo)
  {
    printf("CalibRun::PauseRun, run %d\n", runinfo->fRunNo);
  }

  void ResumeRun(TARunInfo* runinfo)
  {
    printf("CalibRun::ResumeRun, run %d\n", runinfo->fRunNo);
  }

  TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
  {
    // printf("CalibRun::Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

    AgAwSignalsFlow* AWsigFlow = flow->Find<AgAwSignalsFlow>();
    if( !AWsigFlow || AWsigFlow->fSig.size() <= 0)
      return flow;
    std::vector<Signals::signal> awsignals(AWsigFlow->fSig);
    if( awsignals.size() > 0 )
      {
	vector<double> intersect;// = sig.FindAnodeIntersect(tdelay, 32);

	std::multiset<Signals::signal, Signals::signal::heightorder> byheight1, byheight2;
	std::multiset<Signals::signal, Signals::signal::timeorder> bytime(awsignals.begin(), 
									  awsignals.end());
	auto it = bytime.begin();

	double t1 = -1;
	double t2 = -1;
	double a0 = -1;
	double a1 = -1;

	double t_tol = 20;
	while(it != bytime.end()){
	  if(!byheight1.size()){
	    if(fTdelay == unknown || abs(it->t - fTdelay) < t_tol){
	      byheight1.insert(*it);
	      t1 = it->t;
	      a0 = it->i;
	    }
	  } else {
	    if(abs(it->i - a0) < fSeparation || abs(it->i - a0) > 255-fSeparation){
	      if(it->t == t1) byheight1.insert(*it);
	    } else {
	      if(t2 < 0 && (fTdelay == unknown || abs(it->t - fTdelay) < t_tol)) {
		t2 = it->t;
		a1 = it->i;
		byheight2.insert(*it);
	      } else if(it->t == t2){
		if(abs(it->i - a1) < fSeparation || abs(it->i - a1) > 255-fSeparation)
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
	  intersect.push_back(a0);
	}
	if(byheight2.size()){
	  a1 = 0;
	  totheight = 0;
	  for(auto s: byheight2){
	    a1 += s.height*s.i;
	    totheight += s.height;
	  }
	  a1 /= totheight;
	  intersect.push_back(a1);
	}

	if(intersect.size() == 2)
	  {
	    StraightTrack strack(intersect[0], intersect[1]);
	    //		cout << "Full cosmic!" << endl;
	    ++fCosmicsFull;
	    for(auto s: awsignals)
	      {
		double r  = strack.GetR(s.i);
	     
		// move pointless wire hit peak into overflow bin
		if( hRofT_straight->GetYaxis()->FindBin(r) == 
		    hRofT_straight->GetYaxis()->FindBin( TPCBase::TPCBaseInstance()->GetAnodeWiresRadius(true)) ) r = 1e6;
		hRofT_straight->Fill(s.t-fTdelay, r);
	      }
	  }
      }
     
    return flow;
  }

  void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
  {
    printf("CalibRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
  }
};

void CalibModule::Init(const std::vector<std::string> &args)
{
  printf("CalibModule::Init!\n");

  for (unsigned i=0; i<args.size(); i++) { }
}

void CalibModule::Finish()
{
  printf("CalibModule::Finish!\n");
}

TARunInterface* CalibModule::NewRun(TARunInfo* runinfo)
{
  printf("CalibModule::NewRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
  return new CalibRun(runinfo);
}

static TARegisterModule tarm(new CalibModule);
