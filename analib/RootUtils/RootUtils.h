
#ifndef _RootUtils_
#define _RootUtils_
#include "TSeq_Event.h"
#include "TChrono_Event.h"
#include "TStoreEvent.hh"
#include "TChronoChannelName.h"
#include "../ana/include/chrono_module.h"
#include "TAGPlot.h"

#include "Rtypes.h"
#include "TTree.h"
#include "TGraph.h"

#include "RootUtils/BoolGetters.h"
#include "RootUtils/DoubleGetters.h"
#include "RootUtils/FileGetters.h"
#include "RootUtils/IntGetters.h"
#include "RootUtils/PlotGetters.h"
#include "RootUtils/PrintTools.h"
#include "RootUtils/TH1DGetters.h"
#include "RootUtils/TSeqEventGetters.h"
#include "RootUtils/TStringGetters.h"
#include "RootUtils/TreeGetters.h"
#include "RootUtils/TGraphGetters.h"
#include "RootUtils/TSplineGetters.h"

#include "Sequencer_Channels.h"
#define CLOCK_CHANNEL 59
void SetBinNumber(int i=100);
#endif
