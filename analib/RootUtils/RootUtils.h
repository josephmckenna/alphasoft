
#ifndef _RootUtils_
#define _RootUtils_
#include "TSeq_Event.h"
#include "TChrono_Event.h"
#include "TStoreEvent.hh"
#include "TChronoChannelName.h"
#include "chrono_module.h"
#include "TAGPlot.h"

#include "TSISEvent.h"

#include "Rtypes.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TGraph.h"

#include "TSpill.h"

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
#include "RootUtils/BinaryRunners.h"

#include "Sequencer_Channels.h"
#define CLOCK_CHANNEL 59
void SetBinNumber(int i=100);
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
