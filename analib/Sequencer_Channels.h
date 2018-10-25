

#ifndef _SEQNAMES_
#define _SEQNAMES_

#define NUMSEQ 9  //Decared also in Sequencer2
#define USED_SEQ 5  //Decared also in Sequencer2
extern TString SeqNames[NUMSEQ];
extern TString StartDumpName[NUMSEQ];
extern TString StopDumpName[NUMSEQ];
extern TString StartSeqName[NUMSEQ];
extern uint USED_SEQ_NUM[USED_SEQ];

enum {PBAR,RECATCH,ATOM,POS,RCT_BOTG,ATM_BOTG,ATM_TOPG,RCT_TOPG,BML};
enum {NOTADUMP,DUMP,EPDUMP};  

#endif

