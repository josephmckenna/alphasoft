#ifndef __EVENT_TREE__
#define __EVENT_TREE__ 1

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"

#include "TLegend.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TROOT.h"

#include "TObjArray.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TObjString.h"
#include "TVector3.h"

#include "TF1.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "SignalsType.hh"
#include "TStoreEvent.hh"
#include "TSpacePoint.hh"
#include "CosmicFinder.hh"
#include "TCosmic.hh"

#include "TStringGetters.h"

#include "Histo.hh"

static const TVector3 zaxis(0.,0.,1.);

class ReadEventTree
{
public:
   ReadEventTree(TString fname, bool s);
   ~ReadEventTree();

   void MakeHistos();
   void ProcessTree();
   void ProcessData();
   void GetSignalHistos( );
   void DisplayHisto( );
   void WriteRunStats();
   void copy_file(const char* srce_file, const char* dest_file);

   void ProcessLine(const TStoreLine& aLine);
   void ProcessHelix(const TStoreHelix& hel);
   void ProcessUsed(const TFitHelix& hel);
   void ProcessVertex(const TVector3& v);

   double LineDistance(const TStoreLine& l0, const TStoreLine& l1);

   void FillCosmicsHisto(const std::vector<TCosmic>& cosmics);

   inline void SetSavePlots(bool s) {_save_plots=s;}

private:
   TString tag;
   int RunNumber;
   TString savFolder;
   std::ofstream fout;
   bool _save_plots;

   // track length plot parameters
   int blen; // number of bins
   double maxlen; // max track length
 
   TFile* fin;
   TTree* tin;

   Histo* fHisto;
   CosmicFinder* fCosmicFinder;

   // aw deconv histos
   TH1D* hht;
   TH1D* hot;
   TH1D* htt;

   // pad deconv histos
   TH1D* hhpad;
   TH1D* hocol;
   TH1D* htpad;
   TH2D* hopad;

   // aw*pad
   TH1D* hmatch;
   TH2D* hawpadsector;
   TH1D* hawamppc_px;

   // sigpoints
   TH1D* hsptawamp_px;

   ALPHAg::padmap* pmap;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
