#include "AgFlow.h"
#include "TH2D.h"

#include "SignalsType.h"
#include <set>

class MatchFlags
{
   // public:
   //    bool fExportWaveforms = false;

public:
   MatchFlags() // ctor
   { }

   ~MatchFlags() // dtor
   { }
};

class MatchModule: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = true;
   
private:
   TH2D* hawcol;
   TH2D* hawcol_timecut;
   TH2D* hawcol_colcut;
   TH2D* hawcol_timecolcut;
   
   TH2D* hamprow_timecolcut;
   
   double fCoincTime; // ns
   
public:

   MatchModule(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo), fCoincTime(16.)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;   
   }

   ~MatchModule()
   {
      if (fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      // anodes histograms
      gDirectory->mkdir("match_el")->cd();

      hawcol = new TH2D("hawcol","Match Electrodes;AW;PAD COL",256,0.,256.,32,0.,32.);
      hawcol_timecut = new TH2D("hawcol_timecut","Match Electrodes Time Cut;AW;PAD COL",
				256,0.,256.,32,0.,32.);
      hawcol_colcut = new TH2D("hawcol_colcut","Match Electrodes Sector Cut;AW;PAD COL",256,0.,256.,32,0.,32.);
      hawcol_timecolcut = new TH2D("hawcol_timecolcut","Match Electrodes Time && Sector Cut;AW;PAD COL",256,0.,256.,32,0.,32.);

      hamprow_timecolcut = new TH2D("hamprow_timecolcut","Pad Amplitude By Row - Matched Electrodes by Time && Sector Cut;PAD ROW",576,0.,576.,1000,0.,10000.);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("MatchModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {           
     printf("MatchModule::Analyze, run %d\n", 
	    runinfo->fRunNo);
     const AgEventFlow* ef = flow->Find<AgEventFlow>();
     
     if (!ef || !ef->fEvent)
       return flow;
     
     AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
     if( !SigFlow ) return flow;
     
     printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig.size()));
     if( ! SigFlow->awSig.size() ) return flow;

     printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig.size()));
     if( ! SigFlow->pdSig.size() ) return flow;

     PlotMatch(&SigFlow->awSig, &SigFlow->pdSig);

     return flow;
   }

  void PlotMatch(std::vector<signal>* awsignals, std::vector<signal>* padsignals)
  {
    std::multiset<signal, signal::timeorder> aw_bytime(awsignals->begin(), 
						       awsignals->end());
    std::multiset<signal, signal::timeorder> pad_bytime(padsignals->begin(), 
							padsignals->end());

    for( auto iaw : aw_bytime )
      {
	for( auto ipd : pad_bytime )
	  {
	    hawcol->Fill(iaw.idx,ipd.sec);

            bool tmatch=false;
            bool pmatch=false;

            double delta = fabs( iaw.t - ipd.t );
	    if( delta <= fCoincTime )
	      {
		hawcol_timecut->Fill(iaw.idx,ipd.sec);
                tmatch=true;
	      }

	    short sector = short(iaw.idx)/8;
            if( sector == ipd.sec )
               {
                  hawcol_colcut->Fill(iaw.idx,ipd.sec);
                  pmatch=true;
               }

            if( tmatch && pmatch )
               {
                  hawcol_timecolcut->Fill(iaw.idx,ipd.sec);
                  hamprow_timecolcut->Fill(ipd.idx,ipd.height);
               }
	  }
      }
  }
};


class MatchModuleFactory: public TAFactory
{
public:
   MatchFlags fFlags;
   
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("MatchModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         // if (args[i] == "--wfexport")
         //    fFlags.fExportWaveforms = true;
      }
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new MatchModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

