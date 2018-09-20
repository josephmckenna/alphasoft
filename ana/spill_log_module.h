#ifndef _SPILL_LOG_MODULES
#define _SPILL_LOG_MODULES


#include <list>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"
#include "TSystem.h"
#include <TEnv.h>

#include "AgFlow.h"
#include "chrono_module.h"
#include "TChronoChannelName.h"
#include "TTree.h"

#include <vector>
//MAX DET defined here:
#include "TSpill.h"

#include "TGFrame.h"
#include "TGListBox.h"
#include "TGTextEdit.h"
#include "TGNumberEntry.h"
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif


#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500





#define NUMSEQ 9
#define USED_SEQ 4
TString SeqNames[NUMSEQ]={"cat","rct","atm","pos","rct_botg","atm_botg","atm_topg","rct_topg","bml"};
enum {PBAR,RECATCH,ATOM,POS,RCT_BOTG,ATM_BOTG,ATM_TOPG,RCT_TOPG,BML};
enum {NOTADUMP,DUMP,EPDUMP};  
uint USED_SEQ_NUM[USED_SEQ]={0,4,5,6};


#endif
