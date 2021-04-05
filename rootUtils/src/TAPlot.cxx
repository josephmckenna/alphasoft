#include "TAPlot.h"

ClassImp(TAPlot);
//Default Constructor
//TAPlot::TAPlot(Bool_t ApplyCuts)//, Int_t MVAMode)
TAPlot::TAPlot(bool zerotime) : ZeroTimeAxis(zerotime)
{
   ObjectConstructionTime = TTimeStamp();
   //Set to zero for 'unset'
   DataLoadedTime = TTimeStamp(0);
   Nbin=100; 
   DrawStyle=0;
   gLegendDetail=1; 
   MVAMode = 0;
   fClassifierCut = -99;
   fApplyCuts = -1;

   FirstTmin=1E99;
   LastTmax=1.;
   BiggestTzero = 0.;
   MaxDumpLength = 0.;

   fTotalTime = -1.;
   fTotalVert = -1.;

   fVerbose=false;
}

TAPlot::TAPlot(const TAPlot& m_TAPlot) : ZeroTimeAxis(m_TAPlot.ZeroTimeAxis)
{
   title                         = m_TAPlot.title ;
   MVAMode                       = m_TAPlot.MVAMode ;
   Nbin                          = m_TAPlot.Nbin ; 
   DrawStyle                     = m_TAPlot.DrawStyle ;
   gLegendDetail                 = m_TAPlot.gLegendDetail ; 
   fApplyCuts                    = m_TAPlot.fApplyCuts ;
   fClassifierCut                = m_TAPlot.fClassifierCut ;
   FirstTmin                     = m_TAPlot.FirstTmin ;
   LastTmax                      = m_TAPlot.LastTmax ;
   BiggestTzero                  = m_TAPlot.BiggestTzero ;
   MaxDumpLength                 = m_TAPlot.MaxDumpLength ;

   fTotalTime                    = m_TAPlot.fTotalTime ;
   fTotalVert                    = m_TAPlot.fTotalVert ;
   fVerbose                      = m_TAPlot.fVerbose ;
   tFactor                       = m_TAPlot.tFactor ;

   TimeWindows                = m_TAPlot.TimeWindows;
   NewVertexEvents               = m_TAPlot.NewVertexEvents;

   for(int i=0;i<m_TAPlot.Ejections.size();i++)
      Ejections.push_back(m_TAPlot.Ejections.at(i));
   
   for(int i=0;i<m_TAPlot.Injections.size();i++)
      Injections.push_back(m_TAPlot.Injections.at(i));
   
   for(int i=0;i<m_TAPlot.DumpStarts.size();i++)
      DumpStarts.push_back(m_TAPlot.DumpStarts.at(i));
   
   for(int i=0;i<m_TAPlot.DumpStops.size();i++)
      DumpStops.push_back(m_TAPlot.DumpStops.at(i));
   
   for(int i=0;i<m_TAPlot.Runs.size();i++)
      Runs.push_back(m_TAPlot.Runs.at(i));
   
   for(int i=0;i<m_TAPlot.feGEM.size();i++)
      feGEM.push_back(m_TAPlot.feGEM.at(i));
   
   for(int i=0;i<m_TAPlot.feLV.size();i++)
      feLV.push_back(m_TAPlot.feLV.at(i));

   ObjectConstructionTime        = m_TAPlot.ObjectConstructionTime ;
   DataLoadedTime                = m_TAPlot.DataLoadedTime ;
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
      TimeWindows.AddTimeWindow(runNumber,tmin[i],tmax[i],tzero[i]);
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
   TimeWindows.AddTimeWindow(runNumber,tmin,tmax,tzero);
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
      //for (auto& t: GetTimeWindows())
      for (size_t i=0; i<GetTimeWindows().tmax.size(); i++)
      {
         TATimeWindows t = GetTimeWindows();
         if (t.runNumber.at(i)==runNumber)
         {
            if (t.tmax.at(i)<0) 
               last_time = 1E99;
            if (last_time < t.tmax.at(i))
               last_time = t.tmax.at(i);
            if (first_time > t.tmin.at(i) )
               first_time = t.tmin.at(i);
         }
      }
      LoadfeGEMData(runNumber, first_time, last_time);
      LoadfeLVData(runNumber, first_time, last_time);
      LoadRun(runNumber, first_time, last_time);
   }
   LoadingDataLoadingDone();
   return;
}

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

void TAPlot::ClearHisto() //Destroy all histograms
{
   HISTOS.SetOwner(kTRUE);
   HISTOS.Delete();
}

void TAPlot::AddToTAPlot(TAPlot *ialphaplot)
{
   //LMG As far as I can tell this function is unused. Can it be deleted?
  ClearHisto();
  Ejections.insert(Ejections.end(), ialphaplot->Ejections.begin(), ialphaplot->Ejections.end());
  Injections.insert(Injections.end(), ialphaplot->Injections.begin(), ialphaplot->Injections.end());
  Runs.insert(Runs.end(), ialphaplot->Runs.begin(), ialphaplot->Runs.end());
  //Draw();
}

void TAPlot::SetGEMChannel(const std::string& name, int ArrayEntry, std::string title)
{
   for (auto& d: feGEM)
   {
      if (d.array_number!= ArrayEntry)
         continue;
      if (d.name!=name)
         continue;
      std::cout<<"GEM Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
   }
   feGEMdata new_entry;
   new_entry.name = name;
   if (title.size() == 0)
      new_entry.title = name;
   else
      new_entry.title = title;
   new_entry.array_number = ArrayEntry;
   
   feGEM.push_back(new_entry);
}

void TAPlot::SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title)
{
   //Perhaps this line should be a function used everywhere
   std::string name = feGEMdata::CombinedName(Category, Varname);
   return SetGEMChannel(name,ArrayEntry,title);
}

std::vector<std::pair<std::string,int>> TAPlot::GetGEMChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& f: feGEM)
      channels.push_back({f.GetName(),f.array_number});
   return channels;
}

std::pair<TLegend*,TMultiGraph*> TAPlot::GetGEMGraphs()
{
   TMultiGraph *feGEMmg = NULL;
   TLegend* legend = NULL;
   if (feLV.size()==0)
      return {legend,feGEMmg};
   feGEMmg = new TMultiGraph();
   legend = new TLegend(0.1,0.7,0.48,0.9);
   //For each unique variable being logged
   int ColourOffset = 0;
   for (auto& f: feGEM)
   {
      std::map<std::string,TGraph*> unique_labels;
      const std::vector<int> UniqueRuns = GetArrayOfRuns();
      for (size_t i=0; i< TimeWindows.tmax.size(); i++)
      {
         size_t ColourID=0;
         for ( ; ColourID< UniqueRuns.size(); ColourID++)
         {
            if (TimeWindows.runNumber.at(i) == UniqueRuns.at(ColourID))
               break;
         }
         TGraph* graph = f.BuildGraph(i,ZeroTimeAxis);
         graph->SetLineColor(GetColour(ColourID + ColourOffset));
         graph->SetMarkerColor(GetColour(ColourID + ColourOffset));
         unique_labels[f.GetName()] = graph;
         //if (i==0)
         //   legend->AddEntry(graph,f.GetName().c_str());
         if (graph->GetN())
            feGEMmg->Add(graph);
      }
      for (auto& a: unique_labels)
      {
         legend->AddEntry(a.second,a.first.c_str());
      }
      ColourOffset++;
   }
   return {legend,feGEMmg};
}

void TAPlot::SetLVChannel(const std::string& name, int ArrayEntry, std::string title)
{
   for (auto& d: feLV)
   {
      if (d.array_number!= ArrayEntry)
         continue;
      if (d.name!=name)
         continue;
      std::cout<<"LV Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
   }
   feLVdata new_entry;
   new_entry.name = name;
   if (title.size() == 0)
      new_entry.title = name;
   else
      new_entry.title = title;
   new_entry.array_number = ArrayEntry;
   
   feLV.push_back(new_entry);
}

std::vector<std::pair<std::string,int>> TAPlot::GetLVChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& f: feGEM)
      channels.push_back({f.GetName(),f.array_number});
   return channels;
}

std::pair<TLegend*,TMultiGraph*> TAPlot::GetLVGraphs()
{
   TMultiGraph *feLVmg = NULL;
   TLegend* legend = NULL;
   if (feLV.size()==0)
      return {legend,feLVmg};
   feLVmg = new TMultiGraph();
   legend = new TLegend(0.1,0.7,0.48,0.9);
   //For each unique variable being logged
   int ColourOffset = 0;
   for (auto& f: feLV)
   {
      std::map<std::string,TGraph*> unique_labels;

      const std::vector<int> UniqueRuns = GetArrayOfRuns();
      for (size_t i=0; i< TimeWindows.tmax.size(); i++)
      {
         size_t ColourID=0;
         for ( ; ColourID< UniqueRuns.size(); ColourID++)
         {
            if (TimeWindows.runNumber.at(i) == UniqueRuns.at(ColourID))
               break;
         }
         TGraph* graph = f.BuildGraph(i,ZeroTimeAxis);
         graph->SetLineColor(GetColour(ColourID + ColourOffset));
         graph->SetMarkerColor(GetColour(ColourID + ColourOffset));
         unique_labels[f.GetName()] = graph;
         
         //if (i==0)
         //   legend->AddEntry(graph,f.GetName().c_str());
         //Add the graph only if there is data in it
         if (graph->GetN())
            feLVmg->Add(graph);
      }
      for (auto& a: unique_labels)
      {
         legend->AddEntry(a.second,a.first.c_str());
      }
      ColourOffset++;
   }
   return {legend,feLVmg};
}

double TAPlot::GetApproximateProcessingTime()
{
   if ( DataLoadedTime == TTimeStamp(0) )
      LoadingDataLoadingDone();
   return DataLoadedTime.AsDouble() - ObjectConstructionTime.AsDouble();
}

void TAPlot::AddRunNumber(int runNumber)
{
   for (auto& no: Runs)
   {
      if (no==runNumber)
      {
         return;
      }
   }
   Runs.push_back(runNumber);
}

int TAPlot::GetNPassedType(const int type)
{
   int n=0;
   //for (auto& event: VertexEvents)
   TAVertexEvents event = GetVertexEvents();
   for (int i = 0; i<=NewVertexEvents.xs.size(); i++)
   {
      if (event.CutsResults[i]&type)
         n++;
   }
   return n;
}

void TAPlot::FillHistogram(const char* keyname,double x, int counts)
{
   if (HISTO_POSITION.count(keyname))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x,counts);
}

void TAPlot::FillHistogram(const char* keyname, double x)
{
   if (HISTO_POSITION.count(keyname))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x);
}

void TAPlot::FillHistogram(const char* keyname, double x, double y)
{
   if (HISTO_POSITION.count(keyname))
      ((TH2D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Fill(x,y);
}

TH1D* TAPlot::GetTH1D(const char* keyname)
{
   if (HISTO_POSITION.count(keyname))
      return (TH1D*)HISTOS.At(HISTO_POSITION.at(keyname));
   return NULL;
}

void TAPlot::DrawHistogram(const char* keyname, const char* settings)
{
   if (HISTO_POSITION.count(keyname))
      ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Draw(settings);
   else
      std::cout<<"Warning: Histogram"<< keyname << "not found"<<std::endl;
}

TLegend* TAPlot::DrawLines(TLegend* legend, const char* keyname)
{
   double max = ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->GetMaximum();
   if (!legend)
      legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
   for (UInt_t i = 0; i < Injections.size(); i++)
   {
      TLine *l = new TLine(Injections[i]*tFactor, 0., Injections[i]*tFactor, max );
      l->SetLineColor(6);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "AD fill", "l");
   }
   for (UInt_t i = 0; i < Ejections.size(); i++)
   {
      TLine *l = new TLine(Ejections[i]*tFactor, 0., Ejections[i]*tFactor, max );
      l->SetLineColor(7);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "Beam to ALPHA", "l");
   }
   for (UInt_t i = 0; i < DumpStarts.size(); i++)
   {
      if (DumpStarts.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(DumpStarts[i]*tFactor, 0., DumpStarts[i]*tFactor, max );
      //l->SetLineColor(7);
      l->SetLineColorAlpha(kGreen, 0.35);
      //l->SetFillColorAlpha(kGreen,0.35);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "Dump Start", "l");
   }
   for (UInt_t i = 0; i < DumpStops.size(); i++)
   {
      if (DumpStops.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(DumpStops[i]*tFactor, 0., DumpStops[i]*tFactor, max );
      //l->SetLineColor(7);
      l->SetLineColorAlpha(kRed, 0.35);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "Dump Stop", "l");
   }
   legend->Draw();
   return legend;
}

TLegend* TAPlot::AddLegendIntegral(TLegend* legend, const char* message,const char* keyname)
{
   char line[201];
   if (!legend)
      legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
   snprintf(line, 200, message, ((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)))->Integral());
   legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at(keyname)), line, "f");
   legend->SetFillColor(kWhite);
   legend->SetFillStyle(1001);
   legend->Draw();
   return legend;
}

TString TAPlot::GetListOfRuns()
{
   TString runs_string="";
   std::sort(Runs.begin(), Runs.end());
   for (size_t i = 0; i < Runs.size(); i++)
   {
      //std::cout <<"Run: "<<Runs[i] <<std::endl;
      if (i > 0)
         runs_string += ",";
      runs_string += Runs[i];
   }
   return runs_string;
}

void TAPlot::PrintTimeRanges()
{
   for (auto& w: TimeWindows.tmax)
      printf("w = %f", w);
}

void TAPlot::PrintFull()
{
   std::cout << "===========================" << std::endl;
   std::cout << "Printing TAPlot located at " << this << std::endl;
   std::cout << "===========================" << std::endl;
   std::cout << "Title is " << title << std::endl;

   std::cout << "MVAMode = " << MVAMode << std::endl;
   std::cout << "Nbin = " << Nbin << std::endl;
   std::cout << "DrawStyle = " << DrawStyle << std::endl;

   std::cout << "gLegendDetail = " << gLegendDetail << std::endl;
   std::cout << "fApplyCuts = " << fApplyCuts << std::endl;
   std::cout << "fClassifierCut = " << fClassifierCut << std::endl;

   std::cout << "FirstTmin = " << FirstTmin << std::endl;
   std::cout << "LastTmax = " << LastTmax << std::endl;
   std::cout << "BiggestTzero = " << BiggestTzero << std::endl;
   std::cout << "MaxDumpLength = " << MaxDumpLength << std::endl;

   std::cout << "Printing TimeWindows at " << &TimeWindows << std::endl;
   std::cout << "TimeWindows.size() = " << TimeWindows.runNumber.size() << std::endl;
   std::cout << "First time window info:" << std::endl;
   std::cout << "TimeWindows[0].runNumber = " << TimeWindows.runNumber.at(0) << std::endl;
   std::cout << "TimeWindows[0].tmax = " << TimeWindows.tmax.at(0) << std::endl;
   std::cout << "TimeWindows[0].tmin = " << TimeWindows.tmin.at(0) << std::endl;
   std::cout << "TimeWindows[0].tzero = " << TimeWindows.tzero.at(0) << std::endl;

   std::cout << "fTotalTime = " << fTotalTime << std::endl;
   std::cout << "fTotalVert = " << fTotalVert << std::endl;

   std::cout << "Printing HISTOS at " << &HISTOS << std::endl;
   std::cout << "HISTOS.size() = " << HISTOS.GetSize() << std::endl;
   std::cout << "First HISTOS info:" << std::endl;
   std::cout << "HISTOS[0] = " << HISTOS.At(0) << std::endl;

   std::cout << "Printing HISTO_POSITION at " << &HISTO_POSITION << std::endl;
   std::cout << "HISTO_POSITION.size() = " << HISTO_POSITION.size() << std::endl;
   std::cout << "First HISTO_POSITION info:" << std::endl;
   std::cout << "HISTO_POSITION[0] string = " << HISTO_POSITION.begin()->first << std::endl;
   std::cout << "HISTO_POSITION[0] int = " << HISTO_POSITION.begin()->second << std::endl;

   std::cout << "Printing Ejections at " << &Ejections << std::endl;
   std::cout << "Ejections.size() = " << Ejections.size() << std::endl;
   std::cout << "First Ejections info:" << std::endl;
   
   if(Ejections.size() > 0)
      std::cout << "Ejections[0] = " << Ejections.at(0) << std::endl;
   else
      std::cout << "Empty sorry, moving on to next member." << std::endl;
   
   std::cout << "Printing Injections at " << &Injections << std::endl;
   std::cout << "Injections.size() = " << Injections.size() << std::endl;
   std::cout << "First Injections info:" << std::endl;
   
   if(Injections.size() > 0)
      std::cout << "Injections[0] = " << Injections.at(0) << std::endl;
   else
      std::cout << "Empty sorry, moving on to next member." << std::endl; 

   std::cout << "Printing DumpStarts at " << &DumpStarts << std::endl;
   std::cout << "DumpStarts.size() = " << DumpStarts.size() << std::endl;
   std::cout << "First DumpStarts info:" << std::endl;
   
   if(DumpStarts.size() > 0)
      std::cout << "DumpStarts[0] = " << DumpStarts.at(0) << std::endl;
   else 
      std::cout << "Empty sorry, moving on to next member." << std::endl; 

   std::cout << "Printing DumpStops at " << &DumpStops << std::endl;
   std::cout << "DumpStops.size() = " << DumpStops.size() << std::endl;
   std::cout << "First DumpStops info:" << std::endl;
   
   if(DumpStops.size() > 0)
      std::cout << "DumpStops[0] = " << DumpStops.at(0) << std::endl;
   else
      std::cout << "Empty sorry, moving on to next member." << std::endl;

   std::cout << "Printing Runs at " << &Runs << std::endl;
   std::cout << "Runs.size() = " << Runs.size() << std::endl;
   std::cout << "First Runs info:" << std::endl;
   
   if(Runs.size() > 0)
      std::cout << "Runs[0] = " << Runs.at(0) << std::endl;
   else
      std::cout << "Empty sorry, moving on to next member." << std::endl;
   
   //Leaving out for now, might add later.
   //std::vector<feGEMdata> feGEM;
   //std::vector<feLVdata> feLV;
   //std::chrono::high_resolution_clock::time_point ObjectConstructionTime;
   //std::chrono::high_resolution_clock::time_point DataLoadedTime;

   std::cout << "===========================" << std::endl;
   std::cout << std::endl << std::endl << std::endl;
}

TAPlot& TAPlot::operator=(const TAPlot& m_TAPlot)
{
   std::cout << "TAPlot = operator" << std::endl;
   this->title = m_TAPlot.title ;
   this->MVAMode = m_TAPlot.MVAMode ;
   this->Nbin = m_TAPlot.Nbin ; 
   this->DrawStyle = m_TAPlot.DrawStyle ;
   this->gLegendDetail = m_TAPlot.gLegendDetail ; 
   this->fApplyCuts = m_TAPlot.fApplyCuts ;
   this->fClassifierCut = m_TAPlot.fClassifierCut ;
   this->FirstTmin = m_TAPlot.FirstTmin ;
   this->LastTmax = m_TAPlot.LastTmax ;
   this->BiggestTzero = m_TAPlot.BiggestTzero ;
   this->MaxDumpLength = m_TAPlot.MaxDumpLength ;
   
   this->fTotalTime = m_TAPlot.fTotalTime ;
   this->fTotalVert = m_TAPlot.fTotalVert ;
   this->fVerbose = m_TAPlot.fVerbose ;
   this->tFactor = m_TAPlot.tFactor ;

   this->TimeWindows = m_TAPlot.TimeWindows;
   this->NewVertexEvents = m_TAPlot.NewVertexEvents;

   for(int i=0;i<m_TAPlot.Ejections.size();i++)
      this->Ejections.push_back(m_TAPlot.Ejections.at(i));
   
   for(int i=0;i<m_TAPlot.Injections.size();i++)
      this->Injections.push_back(m_TAPlot.Injections.at(i));
   
   for(int i=0;i<m_TAPlot.DumpStarts.size();i++)
      this->DumpStarts.push_back(m_TAPlot.DumpStarts.at(i));
   
   for(int i=0;i<m_TAPlot.DumpStops.size();i++)
      this->DumpStops.push_back(m_TAPlot.DumpStops.at(i));
   
   for(int i=0;i<m_TAPlot.Runs.size();i++)
      this->Runs.push_back(m_TAPlot.Runs.at(i));
   
   for(int i=0;i<m_TAPlot.feGEM.size();i++)
      this->feGEM.push_back(m_TAPlot.feGEM.at(i));
   
   for(int i=0;i<m_TAPlot.feLV.size();i++)
      this->feLV.push_back(m_TAPlot.feLV.at(i));

   this->ObjectConstructionTime = m_TAPlot.ObjectConstructionTime ;
   this->DataLoadedTime = m_TAPlot.DataLoadedTime ;

   return *this;
}

TAPlot TAPlot::operator+=(const TAPlot &plotB) 
{
   std::cout << "TAPlot += operator" << std::endl;

   //Vectors- need concating
   this->Ejections.insert(this->Ejections.end(), plotB.Ejections.begin(), plotB.Ejections.end() );
   this->Injections.insert(this->Injections.end(), plotB.Injections.begin(), plotB.Injections.end() );
   this->DumpStarts.insert(this->DumpStarts.end(), plotB.DumpStarts.begin(), plotB.DumpStarts.end() );
   this->DumpStops.insert(this->DumpStops.end(), plotB.DumpStops.begin(), plotB.DumpStops.end() );
   this->Runs.insert(this->Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
   this->feGEM.insert(this->feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
   this->feLV.insert(this->feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

   //Strings
   this->title+= ", ";
   this->title+=plotB.title;

   //All doubles
   this->FirstTmin              = (this->FirstTmin < plotB.FirstTmin)?this->FirstTmin:plotB.FirstTmin;
   this->LastTmax               = (this->LastTmax > plotB.LastTmax)?this->LastTmax:plotB.LastTmax;
   this->BiggestTzero           = (this->BiggestTzero > plotB.BiggestTzero)?this->BiggestTzero:plotB.BiggestTzero;
   this->MaxDumpLength          = (this->MaxDumpLength > plotB.MaxDumpLength)?this->MaxDumpLength:plotB.MaxDumpLength;
   this->fTotalTime             = (this->fTotalTime > plotB.fTotalTime)?this->fTotalTime:plotB.fTotalTime;
   this->ObjectConstructionTime = (this->ObjectConstructionTime < plotB.ObjectConstructionTime)?this->ObjectConstructionTime:plotB.ObjectConstructionTime;
   this->DataLoadedTime         = (this->DataLoadedTime > plotB.DataLoadedTime)?this->DataLoadedTime:plotB.DataLoadedTime;
   this->tFactor                = (this->tFactor < plotB.tFactor)?this->tFactor:plotB.tFactor;
   this->fTotalVert             += plotB.fTotalVert;

   for(int i = 0; i < plotB.HISTOS.GetSize(); i++)
   {
      this->HISTOS.Add(plotB.HISTOS.At(i));
   }
   this->HISTO_POSITION.insert( plotB.HISTO_POSITION.begin(), plotB.HISTO_POSITION.end() );

   this->TimeWindows+=plotB.TimeWindows;
   this->NewVertexEvents+=plotB.NewVertexEvents;

   return *this;
}

TAPlot operator+(const TAPlot& plotA, const TAPlot& plotB)
{
   std::cout << "TAPlot addition operator" << std::endl;
   TAPlot outputplot(plotA); //Create new from copy

   //Vectors- need concacting
   outputplot.Ejections.insert(outputplot.Ejections.end(), plotB.Ejections.begin(), plotB.Ejections.end() );
   outputplot.Injections.insert(outputplot.Injections.end(), plotB.Injections.begin(), plotB.Injections.end() );
   outputplot.DumpStarts.insert(outputplot.DumpStarts.end(), plotB.DumpStarts.begin(), plotB.DumpStarts.end() );
   outputplot.DumpStops.insert(outputplot.DumpStops.end(), plotB.DumpStops.begin(), plotB.DumpStops.end() );
   outputplot.Runs.insert(outputplot.Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
   outputplot.feGEM.insert(outputplot.feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
   outputplot.feLV.insert(outputplot.feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

   //Strings - This title gets overridden in the DrawCanvas function anyway, but until then they are concacted. 
   outputplot.title+= ", ";
   outputplot.title+=plotB.title;

   //All doubles
   outputplot.FirstTmin              = (outputplot.FirstTmin < plotB.FirstTmin)?outputplot.FirstTmin:plotB.FirstTmin;
   outputplot.LastTmax               = (outputplot.LastTmax > plotB.LastTmax)?outputplot.LastTmax:plotB.LastTmax;
   outputplot.BiggestTzero           = (outputplot.BiggestTzero > plotB.BiggestTzero)?outputplot.BiggestTzero:plotB.BiggestTzero;
   outputplot.MaxDumpLength          = (outputplot.MaxDumpLength > plotB.MaxDumpLength)?outputplot.MaxDumpLength:plotB.MaxDumpLength;
   outputplot.fTotalTime             = (outputplot.fTotalTime > plotB.fTotalTime)?outputplot.fTotalTime:plotB.fTotalTime;
   outputplot.ObjectConstructionTime = (outputplot.ObjectConstructionTime < plotB.ObjectConstructionTime)?outputplot.ObjectConstructionTime:plotB.ObjectConstructionTime;
   outputplot.DataLoadedTime         = (outputplot.DataLoadedTime > plotB.DataLoadedTime)?outputplot.DataLoadedTime:plotB.DataLoadedTime;
   outputplot.tFactor                = (outputplot.tFactor < plotB.tFactor)?outputplot.tFactor:plotB.tFactor;
   outputplot.fTotalVert             += plotB.fTotalVert;

   //Histograms and maps, very posssible that these get overridden in the DrawCanvas function anyway.
   for(int i = 0; i < plotB.HISTOS.GetSize(); i++)
   {
      outputplot.HISTOS.Add(plotB.HISTOS.At(i));
   }
   outputplot.HISTO_POSITION.insert( plotB.HISTO_POSITION.begin(), plotB.HISTO_POSITION.end() );

   outputplot.TimeWindows+=plotB.TimeWindows;
   outputplot.NewVertexEvents+=plotB.NewVertexEvents;

   return outputplot;
}
