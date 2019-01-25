#include "TWaveform.hh"


TWaveform::TWaveform(std::string name, std::vector<double> wf, 
		     std::string model):fName(name),fModel(model),fWaveform(wf)
{}


TWaveform::TWaveform(std::string name, std::vector<double> wf):
  fName(name),fModel(""),fWaveform(wf)
{}


ClassImp(TWaveform)
