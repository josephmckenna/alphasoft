#include "HistoStacker.h"

HistoStacker::HistoStacker():fColour()
{
   fLegend = new TLegend();
}

HistoStacker::~HistoStacker()
{
   delete fLegend;
   //I do not own the histograms in this list
   fHistos.clear();
}


