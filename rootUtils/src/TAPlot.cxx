#include "TAPlot.h"

ClassImp(TAPlot);

//Default Constructor
//TAPlot::TAPlot(Bool_t ApplyCuts)//, Int_t MVAMode)
TAPlot::TAPlot(bool zerotime):
   ZeroTimeAxis(zerotime)
{
   ObjectConstructionTime = std::chrono::high_resolution_clock::now();
   DataLoadedTime = std::chrono::high_resolution_clock::from_time_t(0);
   Nbin=100; 
   DrawStyle=0;
   gLegendDetail=1; 

   FirstTmin=1E99;
   LastTmax=1.;
   BiggestTzero = 0.;
   MaxDumpLength = 0.;

   fTotalTime = -1.;
   fTotalVert = -1.;

   fVerbose=false;
}

void TAPlot::AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax, std::vector<double> tzero)
{
   AddRunNumber(runNumber);

   assert(tmin.size() == tmax.size());
   assert(tmin.size() == tzero.size());

   for (size_t i=0; i<tmin.size(); i++)
   {
      double length=tmax[i]-tmin[i];
      if (length>MaxDumpLength)
         MaxDumpLength=length;
      TimeWindows.push_back(
         TimeWindow(runNumber,tmin[i],tmax[i],tzero[i])
         );
      fTotalTime+=tmax[i]-tmin[i];
      //Find the first start window
      if (tmin[i]<FirstTmin)
         FirstTmin=tmin[i];
      //Largest time before 'zero' (-ve number)
      if ( tmin[i] - tzero[i] < BiggestTzero)
         BiggestTzero = tmin[i] - tzero[i];
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
void TAPlot::AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<double> tzero;
   tzero.reserve(tmin.size());
   for (auto& t: tmin)
      tzero.push_back(t);
   return AddTimeGates(runNumber,tmin,tmax,tzero);
}
void TAPlot::AddTimeGate(const int runNumber, const double tmin, const double tmax, const double tzero)
{
   AddRunNumber(runNumber);

   double length = tmax - tmin;
   if (length > MaxDumpLength)
      MaxDumpLength = length;
   TimeWindows.push_back(
      TimeWindow(runNumber,tmin,tmax,tzero)
      );
   fTotalTime += tmax - tmin;
   //Find the first start window
   if (tmin < FirstTmin)
      FirstTmin = tmin;
   //Largest time before 'zero' (-ve number)
   if ( tmin - tzero < BiggestTzero)
      BiggestTzero = tmin - tzero;
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

//It is slightly faster to call AddTimeGates than this function
void TAPlot::AddTimeGate(const int runNumber, const double tmin, const double tmax)
{
   return AddTimeGate(runNumber,tmin,tmax,tmin);
}

template <typename T>
void TAPlot::LoadfeGEMData(feGEMdata& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time)
{
   TTreeReaderValue<TStoreGEMData<T>> GEMEvent(*feGEMReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (feGEMReader->Next())
   {
      double t=GEMEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (t < first_time)
         continue;
      if (t > last_time)
         break;
      f.AddGEMEvent(&(*GEMEvent), GetTimeWindows());
   }
   return;
}

void TAPlot::LoadfeGEMData(int runNumber, double first_time, double last_time)
{
   for (auto& f: feGEM)
   {
      TTreeReader* feGEMReader=Get_feGEM_Tree(runNumber,f.name);
      TTree* tree = feGEMReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << f.GetName() << " ("<<f.name<<") not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreGEMData<double>"))
         LoadfeGEMData<double>(f, feGEMReader, "TStoreGEMData<double>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<float>"))
         LoadfeGEMData<float>(f, feGEMReader, "TStoreGEMData<float>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<bool>"))
         LoadfeGEMData<bool>(f, feGEMReader, "TStoreGEMData<bool>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<int32_t>"))
         LoadfeGEMData<int32_t>(f, feGEMReader, "TStoreGEMData<int32_t>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<uint32_t>"))
         LoadfeGEMData<uint32_t>(f, feGEMReader, "TStoreGEMData<uint32_t>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<uint16_t>"))
         LoadfeGEMData<uint16_t>(f, feGEMReader, "TStoreGEMData<uint16_t>", first_time, last_time);
      else if (tree->GetBranchStatus("TStoreGEMData<char>"))
         LoadfeGEMData<char>(f, feGEMReader, "TStoreGEMData<char>", first_time, last_time);
      else
         std::cout << "Warning unable to find TStoreGEMData type" << std::endl;   
   }
}

void TAPlot::LoadfeLVData(feLVdata& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time)
{
   TTreeReaderValue<TStoreLabVIEWEvent> LVEvent(*feLVReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (feLVReader->Next())
   {
      double t=LVEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (t < first_time)
         continue;
      if (t > last_time)
         break;
      f.AddLVEvent(&(*LVEvent), GetTimeWindows());
   }
   return;
}

void TAPlot::LoadfeLVData(int runNumber, double first_time, double last_time)
{
   //For each unique variable being logged
   for (auto& f: feLV)
   {
      TTreeReader* feLVReader=Get_feLV_Tree(runNumber,f.name);
      TTree* tree = feLVReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << f.GetName() << " ("<<f.name<<") not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreLabVIEWEvent"))
         LoadfeLVData(f, feLVReader, "TStoreLabVIEWEvent", first_time, last_time);
      else
         std::cout << "Warning unable to find TStoreLVData type" << std::endl;   
   }
}

void TAPlot::LoadData()
{
   for (size_t i=0; i<Runs.size(); i++)
   {
      double last_time = 0;
      double first_time = 1E99;
      int runNumber = Runs[i];
      //Calculate our list time... so we can stop early
      for (auto& t: GetTimeWindows())
      {
         if (t.runNumber==runNumber)
         {
            if (t.tmax<0) last_time = 1E99;
            if (last_time < t.tmax)
               last_time = t.tmax;
            if (first_time > t.tmin )
               first_time = t.tmin;
         }
      }
      LoadfeGEMData(runNumber, first_time, last_time);
      LoadfeLVData(runNumber, first_time, last_time);
      LoadRun(runNumber, first_time, last_time);
   }
   LoadingDataLoadingDone();
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
