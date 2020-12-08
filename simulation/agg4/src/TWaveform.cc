#include "TWaveform.hh"


// TWaveform::TWaveform(std::string name, std::vector<double> wf, 
// 		     std::string model):fName(name),fModel(model),fWaveform(wf)
// {}


// TWaveform::TWaveform(std::string name, std::vector<double> wf):
//   fName(name),fModel(""),fWaveform(wf)
// {}

TWaveform::TWaveform(std::string name, std::vector<int>* wf, 
		     std::string model):fName(name),fModel(model),fWaveform(*wf)
{}


TWaveform::TWaveform(std::string name, std::vector<int>* wf):
  fName(name),fModel(""),fWaveform(*wf)
{}


ClassImp(TWaveform)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
