#include "TAPlot.h"

ClassImp(TAPlot);

//Default Constructor
//TAPlot::TAPlot(Bool_t ApplyCuts)//, Int_t MVAMode)
TAPlot::TAPlot()
{
  Nbin=100; 
  DrawStyle=0;
  gLegendDetail=1; 

  fTotalTime = -1.;
  fTotalVert = -1.;

  fVerbose=false;

  //fApplyCuts=ApplyCuts;
}

void TAPlot::AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax)
{
   AddRunNumber(runNumber);
   
   assert(tmin.size()==tmax.size());
   
   for (size_t i=0; i<tmin.size(); i++)
   {
      TimeWindows.push_back({runNumber,tmin[i],tmax[i]});
      fTotalTime+=tmax[i]-tmin[i];
   }
   return;
}


void TAPlot::LoadData()
{
   for (size_t i=0; i<Runs.size(); i++)
   {
      LoadRun(Runs[i]);
   }
   return;
}

/*
void TAPlot::Draw(const char* title,Option_t *option)
{
  TCanvas* a=Canvas(title);
  a->Draw(option);
  return;
}
*/

/*
void TAPlot::Print()
{
  std::cout<<"TAPlot Summary"<<std::endl;
  //FillHisto();
  std::cout <<""<<std::endl<<"Run(s): ";
  for (UInt_t i=0; i<Runs.size(); i++)
  {
     if (i>1) std::cout <<", ";
     std::cout <<Runs[i];
  }
  std::cout <<std::endl;
  
  //Loop over TObj array and print it?
  for (int i=0; i<HISTOS.GetEntries(); i++)
  {
      TH1D* a=dynamic_cast<TH1D*>(HISTOS.At(i));
      if (a)
      {
         std::cout <<a->GetTitle()<<"\t"<<a->Integral()<<std::endl;
      }
  }
  
}
*/
//Default Destructor
TAPlot::~TAPlot()
{
  ClearHisto();
  Ejections.clear();
  Injections.clear();
  DumpStarts.clear();
  DumpStops.clear();
  Runs.clear();
  VertexEvents.clear();
}

void TAPlot::ClearHisto() //Destroy all histograms
{
   //HISTOS.SetOwner(kTRUE);
   //HISTOS.Delete();
}


void TAPlot::AddToTAPlot(TAPlot *ialphaplot)
{
  ClearHisto();
  VertexEvents.insert(VertexEvents.end(), ialphaplot->VertexEvents.begin(), ialphaplot->VertexEvents.end());
  Ejections.insert(Ejections.end(), ialphaplot->Ejections.begin(), ialphaplot->Ejections.end());
  Injections.insert(Injections.end(), ialphaplot->Injections.begin(), ialphaplot->Injections.end());
  Runs.insert(Runs.end(), ialphaplot->Runs.begin(), ialphaplot->Runs.end());
  //Draw();
}
