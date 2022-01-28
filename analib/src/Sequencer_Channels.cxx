#include "Sequencer_Channels.h"


std::string GetSequencerName(int seqID)
{
   if (seqID < 0)
      return "N/A";
   if (seqID >= NUMSEQ)
      return "N/A";
   return SEQ_NAMES.at(seqID);
}
