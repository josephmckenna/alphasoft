

#ifndef _SEQNAMES_
#define _SEQNAMES_
#include <vector>
#include <string>
#include "assert.h"
#define NUMSEQ 9 
#define USED_SEQ 9 

//Dont change the order of these please

//Dont change the order of these please
enum {
   PBAR,
   RECATCH,
   ATOM,
   POS,
   RCT_BOTG,
   ATM_BOTG,
   ATM_TOPG,
   RCT_TOPG,
   BML
};

static const std::vector<std::string> SEQ_NAMES = {
   "CAT",
   "RCT",
   "ATM",
   "POS",
   //"BRK",
   "RCT_BOTG",
   "ATM_BOTG",
   "ATM_TOPG",
   "RCT_TOPG",
   "BML",
};

std::string GetSequencerName(int seqID);

const std::vector<std::string> SEQ_NAMES_SHORT = {
   "CAT",
   "RCT",
   "ATM",
   "POS",
   //"BRK",
   "RCB",
   "ATB",
   "ATT",
   "RCT",
   "BML"
};


const std::vector<std::string> SeqNames{
   "cat",
   "rct",
   "atm",
   "pos",
   //"brk",
   "rct_botg",
   "atm_botg",
   "atm_topg",
   "rct_topg",
   "bml"
};

const std::vector<uint> USED_SEQ_NUM{
   PBAR,
   RECATCH,
   ATOM,
   POS,
   RCT_BOTG,
   ATM_BOTG,
   ATM_TOPG,
   RCT_TOPG,
   BML
};

const std::vector<std::string> StartDumpName{
   "CAT_START_DUMP",
   "RCT_START_DUMP",
   "ATM_START_DUMP",
   "POS_START_DUMP",
   "RCT_BOTG_START_DUMP",
   "ATM_BOTG_START_DUMP",
   "ATM_TOPG_START_DUMP",
   "RCT_TOPG_START_DUMP",
   "BML_START_DUMP"
};

const std::vector<std::string> StopDumpName{
   "CAT_STOP_DUMP",
   "RCT_STOP_DUMP",
   "ATM_STOP_DUMP",
   "POS_STOP_DUMP",
   "RCT_BOTG_STOP_DUMP",
   "ATM_BOTG_STOP_DUMP",
   "ATM_TOPG_STOP_DUMP",
   "RCT_TOPG_STOP_DUMP",
   "BML_STOP_DUMP"
};

const std::vector<std::string> StartSeqName{
   "CAT_SEQ_RUNNING",
   "RCT_SEQ_RUNNING",
   "ATM_SEQ_RUNNING",
   "POS_SEQ_RUNNING",
   "RCT_TOPG_SEQ_RUNNING",
   "ATM_BOTG_SEQ_RUNNING",
   "ATM_TOPG_SEQ_RUNNING",
   "RCT_BOTG_SEQ_RUNNING",
   "BML_SEQ_RUNNING"
};

//static_assert(SeqNames.size()==NUMSEQ,"");
//assert(StartDumpName.size()==NUMSEQ);
//assert(StopDumpName.size()==NUMSEQ);
//assert(StartSeqName.size()==NUMSEQ);


enum {NOTADUMP,DUMP,EPDUMP};  

#endif


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
