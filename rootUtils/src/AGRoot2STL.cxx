#include "AGRoot2STL.h"


#ifdef BUILD_AG_SIM

std::vector<ALPHAg::wf_ref> ConvertWaveformArray(TClonesArray *wfarray)
{
   std::vector<ALPHAg::wf_ref> wfvec;
   int                         Nentries = wfarray->GetEntries();
   wfvec.reserve(Nentries);
   for (int j = 0; j < Nentries; ++j) {
      TWaveform              *w     = (TWaveform *)wfarray->ConstructedAt(j);
      std::string             wname = w->GetElectrode();
      const std::vector<int> &wf    = w->GetWaveform();
    //   std::cout << "ConvertWaveFormArray " << j << " electrode: " << wname << " size: " << wf.size() << std::endl;

      const char eltype = wname.at(0);
      int        elnum  = -1;
      short      sec    = -1;
      if (eltype == 'a') {
         //   std::cout << "Anode!" << std::endl;
         sec   = 0; // always =0 for top readout of anodes
         elnum = stoi(wname.substr(1));
      } else if (eltype == 'p') {
         //   std::cout << "Pad!" << std::endl;
         const char delim = '_';
         size_t     pos   = wname.find(delim);
         size_t     pos2  = wname.find(delim, pos + 1);
         sec              = stoi(wname.substr(pos + 1, pos2 - pos - 1));
         elnum            = stoi(wname.substr(pos2 + 1));
      } else {
         std::cerr << "Unknown electrode type " << eltype << std::endl;
      }
      wfvec.emplace_back(elnum, sec, new std::vector<double>(wf.begin(), wf.end()));
   }

   return wfvec;
}

#endif
