#include "TAPlot.h"

ClassImp(TAPlot);

//Default Constructor
//TAPlot::TAPlot(Bool_t ApplyCuts)//, Int_t MVAMode)
TAPlot::TAPlot()
{
   Nbin=100; 
   DrawStyle=0;
   gLegendDetail=1; 

   FirstTmin=1E99;
   LastTmax=1.;


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
      double length=tmax[i]-tmin[i];
      if (length>MaxDumpLength)
         MaxDumpLength=length;
      TimeWindows.push_back({runNumber,tmin[i],tmax[i]});
      fTotalTime+=tmax[i]-tmin[i];
      //Find the first start window
      if (tmin[i]<FirstTmin)
         FirstTmin=tmin[i];
      //Find the end of the last window (note: -ve tmax means end of run)
      //Skip early if we are looking for end of run anyway
      if (LastTmax<0)
         continue;
      //Find the highest value
      if (tmax[i]>LastTmax)
         LastTmax=tmax[i];
      //Set -ve of we want end of run
      if (tmax[i]<0)
         LastTmax=-1;
   }
   return;
}
//It is slightly faster to call AddTimeGates than this function
void TAPlot::AddTimeGate(const int runNumber, const double tmin, const double tmax)
{
   AddRunNumber(runNumber);

   double length = tmax - tmin;
   if (length > MaxDumpLength)
      MaxDumpLength = length;
   TimeWindows.push_back({runNumber,tmin,tmax});
   fTotalTime += tmax - tmin;
   //Find the first start window
   if (tmin < FirstTmin)
      FirstTmin = tmin;
   //Find the end of the last window (note: -ve tmax means end of run)
   //Skip early if we are looking for end of run anyway
   if (LastTmax < 0)
      return;
   //Find the highest value
   if (tmax > LastTmax)
      LastTmax = tmax;
   //Set -ve of we want end of run
   if (tmax < 0)
      LastTmax = -1;
   return;
}

template <typename T>
void TAPlot::LoadfeGEMData(feGEMdata& f, TTreeReader* feGEMReader, const char* name, double last_time)
{
   TTreeReaderValue<TStoreGEMData<T>> GEMEvent(*feGEMReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (feGEMReader->Next())
   {
      if (GEMEvent->GetRunTime()>last_time)
         break;
      f.AddGEMEvent(&(*GEMEvent), GetTimeWindows());
   }
   return;
}

void TAPlot::LoadfeGEMData(int runNumber, double last_time)
{
   for (auto& f: feGEM)
   {
      TTreeReader* feGEMReader=Get_feGEM_Tree(runNumber,f.name);
      TTree* tree = tree;
      if  (!tree)
      {
         std::cout<<"Warning: " << f.GetName() << " not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreGEMData<double>"))
         LoadfeGEMData<double>(f,feGEMReader,"TStoreGEMData<double>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<float>"))
         LoadfeGEMData<float>(f,feGEMReader,"TStoreGEMData<float>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<bool>"))
         LoadfeGEMData<bool>(f,feGEMReader,"TStoreGEMData<bool>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<int32_t>"))
         LoadfeGEMData<int32_t>(f,feGEMReader,"TStoreGEMData<int32_t>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<uint32_t>"))
         LoadfeGEMData<uint32_t>(f,feGEMReader,"TStoreGEMData<uint32_t>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<uint16_t>"))
         LoadfeGEMData<uint16_t>(f,feGEMReader,"TStoreGEMData<uint16_t>",last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<char>"))
         LoadfeGEMData<char>(f,feGEMReader,"TStoreGEMData<char>",last_time);
      else
         std::cout<<"Warning unable to find TStoreGEMData type"<<std::endl;   
   }
}

void TAPlot::LoadData()
{
   for (size_t i=0; i<Runs.size(); i++)
   {
      double last_time=0;
      int runNumber=Runs[i];
      //Calculate our list time... so we can stop early
      for (auto& t: GetTimeWindows())
      {
         if (t.runNumber==runNumber)
         {
            if (t.tmax<0) last_time=1E99;
            if (last_time<t.tmax)
            {
               last_time=t.tmax;
            }
         }
      }
      LoadfeGEMData(runNumber, last_time);
      LoadRun(runNumber, last_time);
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


void TAPlot::Print(Option_t *option) const
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
   HISTOS.SetOwner(kTRUE);
   HISTOS.Delete();
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
