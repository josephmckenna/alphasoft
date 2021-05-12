#include "TAPlot.h"

ClassImp(TAPlot);
//Default Constructor
//TAPlot::TAPlot(Bool_t ApplyCuts)//, Int_t MVAMode)
TAPlot::TAPlot(bool zerotime) : ZeroTimeAxis(zerotime)
{
   ObjectConstructionTime = TTimeStamp();
   //Set to zero for 'unset'
   DataLoadedTime = TTimeStamp(0);
   fNumBins=100; 
   fDrawStyle=0;
   fLegendDetail=1; 
   fMVAMode = 0;
   fClassifierCut = -99;
   fApplyCuts = -1;

   fFirstTMin=1E99;
   fLastTMax=1.;
   fBiggestTZero = 0.;
   fMaxDumpLength = 0.;

   fTotalTime = -1.;
   fTotalVert = -1.;

   fVerbose=false;
}

TAPlot::TAPlot(const TAPlot& m_TAPlot) : ZeroTimeAxis(m_TAPlot.ZeroTimeAxis)
{
   fTitle                         = m_TAPlot.fTitle ;
   fMVAMode                       = m_TAPlot.fMVAMode ;
   fNumBins                          = m_TAPlot.fNumBins ; 
   fDrawStyle                     = m_TAPlot.fDrawStyle ;
   fLegendDetail                 = m_TAPlot.fLegendDetail ; 
   fApplyCuts                    = m_TAPlot.fApplyCuts ;
   fClassifierCut                = m_TAPlot.fClassifierCut ;
   fFirstTMin                     = m_TAPlot.fFirstTMin ;
   fLastTMax                      = m_TAPlot.fLastTMax ;
   fBiggestTZero                  = m_TAPlot.fBiggestTZero ;
   fMaxDumpLength                 = m_TAPlot.fMaxDumpLength ;

   fTotalTime                    = m_TAPlot.fTotalTime ;
   fTotalVert                    = m_TAPlot.fTotalVert ;
   fVerbose                      = m_TAPlot.fVerbose ;
   fTimeFactor                       = m_TAPlot.fTimeFactor ;

   fTimeWindows                = m_TAPlot.fTimeWindows;
   fVertexEvents               = m_TAPlot.fVertexEvents;

   for(int i=0;i<m_TAPlot.fEjections.size();i++)
      fEjections.push_back(m_TAPlot.fEjections[i]);
   
   for(int i=0;i<m_TAPlot.fInjections.size();i++)
      fInjections.push_back(m_TAPlot.fInjections[i]);
   
   for(int i=0;i<m_TAPlot.fDumpStarts.size();i++)
      fDumpStarts.push_back(m_TAPlot.fDumpStarts[i]);
   
   for(int i=0;i<m_TAPlot.fDumpStops.size();i++)
      fDumpStops.push_back(m_TAPlot.fDumpStops[i]);
   
   for(int i=0;i<m_TAPlot.Runs.size();i++)
      Runs.push_back(m_TAPlot.Runs[i]);
   
   for(int i=0;i<m_TAPlot.feGEM.size();i++)
      feGEM.push_back(m_TAPlot.feGEM[i]);
   
   for(int i=0;i<m_TAPlot.feLV.size();i++)
      feLV.push_back(m_TAPlot.feLV[i]);

   ObjectConstructionTime        = m_TAPlot.ObjectConstructionTime ;
   DataLoadedTime                = m_TAPlot.DataLoadedTime ;
}

//Default Destructor
TAPlot::~TAPlot()
{
  ClearHisto();
  fEjections.clear();
  fInjections.clear();
  fDumpStarts.clear();
  fDumpStops.clear();
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
      if (length>fMaxDumpLength)
         fMaxDumpLength=length;
      fTimeWindows.AddTimeWindow(runNumber,tmin[i],tmax[i],tzero[i]);
      fTotalTime+=tmax[i]-tmin[i];
      //Find the first start window
      if (tmin[i]<fFirstTMin)
         fFirstTMin=tmin[i];
      //Largest time before 'zero' (-ve number)
      if ( tmin[i] - tzero[i] < fBiggestTZero)
         fBiggestTZero = tmin[i] - tzero[i];
      //Find the end of the last window (note: -ve tmax means end of run)
      //Skip early if we are looking for end of run anyway
      if (fLastTMax<0)
         continue;
      //Find the highest value
      if (tmax[i]>fLastTMax)
         fLastTMax=tmax[i];
      //Set -ve of we want end of run
      if (tmax[i]<0)
         fLastTMax=-1;
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
   if (length > fMaxDumpLength)
      fMaxDumpLength = length;
   fTimeWindows.AddTimeWindow(runNumber,tmin,tmax,tzero);
   fTotalTime += tmax - tmin;
   //Find the first start window
   if (tmin < fFirstTMin)
      fFirstTMin = tmin;
   //Largest time before 'zero' (-ve number)
   if ( tmin - tzero < fBiggestTZero)
      fBiggestTZero = tmin - tzero;
   //Find the end of the last window (note: -ve tmax means end of run)
   //Skip early if we are looking for end of run anyway
   if (fLastTMax < 0)
      return;
   //Find the highest value
   if (tmax > fLastTMax)
      fLastTMax = tmax;
   //Set -ve of we want end of run
   if (tmax < 0)
      fLastTMax = -1;
   return;

}

//It is slightly faster to call AddTimeGates than this function
void TAPlot::AddTimeGate(const int runNumber, const double tmin, const double tmax)
{
   return AddTimeGate(runNumber,tmin,tmax,tmin);
}

template <typename T>
void TAPlot::LoadfeGEMData(TFEGEMData& f, TTreeReader* feGEMReader, const char* name, double first_time, double last_time)
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
      f.AddGEMEvent(&(*GEMEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadfeGEMData(int runNumber, double first_time, double last_time)
{
   for (auto& f: feGEM)
   {
      TTreeReader* feGEMReader=Get_feGEM_Tree(runNumber,f.fName);
      TTree* tree = feGEMReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << f.GetName() << " ("<<f.fName<<") not found for run " << runNumber << std::endl;
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

void TAPlot::LoadfeLVData(TFELVData& f, TTreeReader* feLVReader, const char* name, double first_time, double last_time)
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
      f.AddLVEvent(&(*LVEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadfeLVData(int runNumber, double first_time, double last_time)
{
   //For each unique variable being logged
   for (auto& f: feLV)
   {
      TTreeReader* feLVReader=Get_feLV_Tree(runNumber,f.fName);
      TTree* tree = feLVReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << f.GetName() << " ("<<f.fName<<") not found for run " << runNumber << std::endl;
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
      //TTimeWindows t = GetTimeWindows();
      for (size_t i=0; i<GetTimeWindows()->fTmax.size(); i++)
      {
         if (GetTimeWindows()->fRunNumber[i]==runNumber)
         {
            if (GetTimeWindows()->fTmax[i]<0) 
               last_time = 1E99;
            if (last_time < GetTimeWindows()->fTmax[i])
               last_time = GetTimeWindows()->fTmax[i];
            if (first_time > GetTimeWindows()->fTMin[i] )
               first_time = GetTimeWindows()->fTMin[i];
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
  for (int i=0; i<fHistos.GetEntries(); i++)
  {
      TH1D* a=dynamic_cast<TH1D*>(fHistos.At(i));
      if (a)
      {
         std::cout <<a->GetTitle()<<"\t"<<a->Integral()<<std::endl;
      }
  }
  
}

void TAPlot::ClearHisto() //Destroy all histograms
{
   fHistos.SetOwner(kTRUE);
   fHistos.Delete();
}

void TAPlot::SetGEMChannel(const std::string& name, int ArrayEntry, std::string title)
{
   for (auto& d: feGEM)
   {
      if (d.fArrayNumber!= ArrayEntry)
         continue;
      if (d.fName!=name)
         continue;
      std::cout<<"GEM Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
   }
   TFEGEMData new_entry;
   new_entry.fName = name;
   if (title.size() == 0)
      new_entry.fTitle = name;
   else
      new_entry.fTitle = title;
   new_entry.fArrayNumber = ArrayEntry;
   
   feGEM.push_back(new_entry);
}

void TAPlot::SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title)
{
   //Perhaps this line should be a function used everywhere
   std::string name = TFEGEMData::CombinedName(Category, Varname);
   return SetGEMChannel(name,ArrayEntry,title);
}

std::vector<std::pair<std::string,int>> TAPlot::GetGEMChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& f: feGEM)
      channels.push_back({f.GetName(),f.fArrayNumber});
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
      for (size_t i=0; i< fTimeWindows.fTmax.size(); i++)
      {
         size_t ColourID=0;
         for ( ; ColourID< UniqueRuns.size(); ColourID++)
         {
            if (fTimeWindows.fRunNumber[i] == UniqueRuns.at(ColourID))
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
      if (d.fArrayNumber!= ArrayEntry)
         continue;
      if (d.fName!=name)
         continue;
      std::cout<<"LV Channel "<< name.c_str()<< "["<<ArrayEntry<<"] already registered"<<std::endl;
   }
   TFELVData new_entry;
   new_entry.fName = name;
   if (title.size() == 0)
      new_entry.fTitle = name;
   else
      new_entry.fTitle = title;
   new_entry.fArrayNumber = ArrayEntry;
   
   feLV.push_back(new_entry);
}

std::vector<std::pair<std::string,int>> TAPlot::GetLVChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& f: feGEM)
      channels.push_back({f.GetName(),f.fArrayNumber});
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
      for (size_t i=0; i< fTimeWindows.fTmax.size(); i++)
      {
         size_t ColourID=0;
         for ( ; ColourID< UniqueRuns.size(); ColourID++)
         {
            if (fTimeWindows.fRunNumber[i] == UniqueRuns.at(ColourID))
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
   //const TVertexEvents* event = GetVertexEvents();
   for (int i = 0; i<fVertexEvents.fXVertex.size(); i++)
   {
      if (fVertexEvents.fCutsResults[i]&type)
         n++;
   }
   return n;
}

void TAPlot::FillHistogram(const char* keyname,double x, int counts)
{
   if (fHistoPositions.count(keyname))
      ((TH1D*)fHistos.At(fHistoPositions.at(keyname)))->Fill(x,counts);
}

void TAPlot::FillHistogram(const char* keyname, double x)
{
   if (fHistoPositions.count(keyname))
      ((TH1D*)fHistos.At(fHistoPositions.at(keyname)))->Fill(x);
}

void TAPlot::FillHistogram(const char* keyname, double x, double y)
{
   if (fHistoPositions.count(keyname))
      ((TH2D*)fHistos.At(fHistoPositions.at(keyname)))->Fill(x,y);
}

TH1D* TAPlot::GetTH1D(const char* keyname)
{
   if (fHistoPositions.count(keyname))
      return (TH1D*)fHistos.At(fHistoPositions.at(keyname));
   return NULL;
}

void TAPlot::DrawHistogram(const char* keyname, const char* settings)
{
   if (fHistoPositions.count(keyname))
      ((TH1D*)fHistos.At(fHistoPositions.at(keyname)))->Draw(settings);
   else
      std::cout<<"Warning: Histogram"<< keyname << "not found"<<std::endl;
}

TLegend* TAPlot::DrawLines(TLegend* legend, const char* keyname)
{
   double max = ((TH1D*)fHistos.At(fHistoPositions.at(keyname)))->GetMaximum();
   if (!legend)
      legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
   for (UInt_t i = 0; i < fInjections.size(); i++)
   {
      TLine *l = new TLine(fInjections[i]*fTimeFactor, 0., fInjections[i]*fTimeFactor, max );
      l->SetLineColor(6);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "AD fill", "l");
   }
   for (UInt_t i = 0; i < fEjections.size(); i++)
   {
      TLine *l = new TLine(fEjections[i]*fTimeFactor, 0., fEjections[i]*fTimeFactor, max );
      l->SetLineColor(7);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "Beam to ALPHA", "l");
   }
   for (UInt_t i = 0; i < fDumpStarts.size(); i++)
   {
      if (fDumpStarts.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(fDumpStarts[i]*fTimeFactor, 0., fDumpStarts[i]*fTimeFactor, max );
      //l->SetLineColor(7);
      l->SetLineColorAlpha(kGreen, 0.35);
      //l->SetFillColorAlpha(kGreen,0.35);
      l->Draw();
      if (i == 0)
         legend->AddEntry(l, "Dump Start", "l");
   }
   for (UInt_t i = 0; i < fDumpStops.size(); i++)
   {
      if (fDumpStops.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *l = new TLine(fDumpStops[i]*fTimeFactor, 0., fDumpStops[i]*fTimeFactor, max );
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
   snprintf(line, 200, message, ((TH1D*)fHistos.At(fHistoPositions.at(keyname)))->Integral());
   legend->AddEntry((TH1D*)fHistos.At(fHistoPositions.at(keyname)), line, "f");
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
   for (auto& w: fTimeWindows.fTmax)
      printf("w = %f", w);
}

void TAPlot::PrintFull()
{
   std::cout << "===========================" << std::endl;
   std::cout << "Printing TAPlot located at " << this << std::endl;
   std::cout << "===========================" << std::endl;
   std::cout << "Title is " << fTitle << std::endl;

   for(int i=0;i<fVertexEvents.fRunNumbers.size();i++)
   {
      std::cout << fVertexEvents.fRunNumbers.at(i) << std::endl;
   }
   for(int i=0;i<fVertexEvents.fEventNos.size();i++)
   {
      std::cout << fVertexEvents.fEventNos.at(i) << std::endl;
   }

   std::cout << "===========================" << std::endl;
   std::cout << std::endl << std::endl << std::endl;
}

TAPlot& TAPlot::operator=(const TAPlot& m_TAPlot)
{
   std::cout << "TAPlot = operator" << std::endl;
   this->fTitle = m_TAPlot.fTitle ;
   this->fMVAMode = m_TAPlot.fMVAMode ;
   this->fNumBins = m_TAPlot.fNumBins ; 
   this->fDrawStyle = m_TAPlot.fDrawStyle ;
   this->fLegendDetail = m_TAPlot.fLegendDetail ; 
   this->fApplyCuts = m_TAPlot.fApplyCuts ;
   this->fClassifierCut = m_TAPlot.fClassifierCut ;
   this->fFirstTMin = m_TAPlot.fFirstTMin ;
   this->fLastTMax = m_TAPlot.fLastTMax ;
   this->fBiggestTZero = m_TAPlot.fBiggestTZero ;
   this->fMaxDumpLength = m_TAPlot.fMaxDumpLength ;
   
   this->fTotalTime = m_TAPlot.fTotalTime ;
   this->fTotalVert = m_TAPlot.fTotalVert ;
   this->fVerbose = m_TAPlot.fVerbose ;
   this->fTimeFactor = m_TAPlot.fTimeFactor ;

   this->fTimeWindows = m_TAPlot.fTimeWindows;
   this->fVertexEvents = m_TAPlot.fVertexEvents;

   for(int i=0;i<m_TAPlot.fEjections.size();i++)
      this->fEjections.push_back(m_TAPlot.fEjections[i]);
   
   for(int i=0;i<m_TAPlot.fInjections.size();i++)
      this->fInjections.push_back(m_TAPlot.fInjections[i]);
   
   for(int i=0;i<m_TAPlot.fDumpStarts.size();i++)
      this->fDumpStarts.push_back(m_TAPlot.fDumpStarts[i]);
   
   for(int i=0;i<m_TAPlot.fDumpStops.size();i++)
      this->fDumpStops.push_back(m_TAPlot.fDumpStops[i]);
   
   for(int i=0;i<m_TAPlot.Runs.size();i++)
      this->Runs.push_back(m_TAPlot.Runs[i]);
   
   for(int i=0;i<m_TAPlot.feGEM.size();i++)
      this->feGEM.push_back(m_TAPlot.feGEM[i]);
   
   for(int i=0;i<m_TAPlot.feLV.size();i++)
      this->feLV.push_back(m_TAPlot.feLV[i]);

   this->ObjectConstructionTime = m_TAPlot.ObjectConstructionTime ;
   this->DataLoadedTime = m_TAPlot.DataLoadedTime ;

   return *this;
}

TAPlot& TAPlot::operator+=(const TAPlot &plotB) 
{
   std::cout << "TAPlot += operator" << std::endl;

   //Vectors- need concating
   this->fEjections.insert(this->fEjections.end(), plotB.fEjections.begin(), plotB.fEjections.end() );
   this->fInjections.insert(this->fInjections.end(), plotB.fInjections.begin(), plotB.fInjections.end() );
   this->fDumpStarts.insert(this->fDumpStarts.end(), plotB.fDumpStarts.begin(), plotB.fDumpStarts.end() );
   this->fDumpStops.insert(this->fDumpStops.end(), plotB.fDumpStops.begin(), plotB.fDumpStops.end() );
   this->Runs.insert(this->Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
   this->feGEM.insert(this->feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
   this->feLV.insert(this->feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

   //Strings
   this->fTitle+= ", ";
   this->fTitle+=plotB.fTitle;

   //All doubles
   this->fFirstTMin              = (this->fFirstTMin < plotB.fFirstTMin)?this->fFirstTMin:plotB.fFirstTMin;
   this->fLastTMax               = (this->fLastTMax > plotB.fLastTMax)?this->fLastTMax:plotB.fLastTMax;
   this->fBiggestTZero           = (this->fBiggestTZero > plotB.fBiggestTZero)?this->fBiggestTZero:plotB.fBiggestTZero;
   this->fMaxDumpLength          = (this->fMaxDumpLength > plotB.fMaxDumpLength)?this->fMaxDumpLength:plotB.fMaxDumpLength;
   this->fTotalTime             = (this->fTotalTime > plotB.fTotalTime)?this->fTotalTime:plotB.fTotalTime;
   this->ObjectConstructionTime = (this->ObjectConstructionTime < plotB.ObjectConstructionTime)?this->ObjectConstructionTime:plotB.ObjectConstructionTime;
   this->DataLoadedTime         = (this->DataLoadedTime > plotB.DataLoadedTime)?this->DataLoadedTime:plotB.DataLoadedTime;
   this->fTimeFactor                = (this->fTimeFactor < plotB.fTimeFactor)?this->fTimeFactor:plotB.fTimeFactor;
   this->fTotalVert             += plotB.fTotalVert;

   for(int i = 0; i < plotB.fHistos.GetSize(); i++)
   {
      this->fHistos.Add(plotB.fHistos.At(i));
   }
   this->fHistoPositions.insert( plotB.fHistoPositions.begin(), plotB.fHistoPositions.end() );

   this->fTimeWindows+=plotB.fTimeWindows;
   this->fVertexEvents+=plotB.fVertexEvents;

   return *this;
}

TAPlot operator+(const TAPlot& plotA, const TAPlot& plotB)
{
   std::cout << "TAPlot addition operator" << std::endl;
   TAPlot outputplot(plotA); //Create new from copy

   //Vectors- need concacting
   outputplot.fEjections.insert(outputplot.fEjections.end(), plotB.fEjections.begin(), plotB.fEjections.end() );
   outputplot.fInjections.insert(outputplot.fInjections.end(), plotB.fInjections.begin(), plotB.fInjections.end() );
   outputplot.fDumpStarts.insert(outputplot.fDumpStarts.end(), plotB.fDumpStarts.begin(), plotB.fDumpStarts.end() );
   outputplot.fDumpStops.insert(outputplot.fDumpStops.end(), plotB.fDumpStops.begin(), plotB.fDumpStops.end() );
   outputplot.Runs.insert(outputplot.Runs.end(), plotB.Runs.begin(), plotB.Runs.end() );//check dupes - ignore copies. AddRunNumber
   outputplot.feGEM.insert(outputplot.feGEM.end(), plotB.feGEM.begin(), plotB.feGEM.end() );
   outputplot.feLV.insert(outputplot.feLV.end(), plotB.feLV.begin(), plotB.feLV.end() );

   //Strings - This title gets overridden in the DrawCanvas function anyway, but until then they are concacted. 
   outputplot.fTitle+= ", ";
   outputplot.fTitle+=plotB.fTitle;

   //All doubles
   outputplot.fFirstTMin              = (outputplot.fFirstTMin < plotB.fFirstTMin)?outputplot.fFirstTMin:plotB.fFirstTMin;
   outputplot.fLastTMax               = (outputplot.fLastTMax > plotB.fLastTMax)?outputplot.fLastTMax:plotB.fLastTMax;
   outputplot.fBiggestTZero           = (outputplot.fBiggestTZero > plotB.fBiggestTZero)?outputplot.fBiggestTZero:plotB.fBiggestTZero;
   outputplot.fMaxDumpLength          = (outputplot.fMaxDumpLength > plotB.fMaxDumpLength)?outputplot.fMaxDumpLength:plotB.fMaxDumpLength;
   outputplot.fTotalTime             = (outputplot.fTotalTime > plotB.fTotalTime)?outputplot.fTotalTime:plotB.fTotalTime;
   outputplot.ObjectConstructionTime = (outputplot.ObjectConstructionTime < plotB.ObjectConstructionTime)?outputplot.ObjectConstructionTime:plotB.ObjectConstructionTime;
   outputplot.DataLoadedTime         = (outputplot.DataLoadedTime > plotB.DataLoadedTime)?outputplot.DataLoadedTime:plotB.DataLoadedTime;
   outputplot.fTimeFactor                = (outputplot.fTimeFactor < plotB.fTimeFactor)?outputplot.fTimeFactor:plotB.fTimeFactor;
   outputplot.fTotalVert             += plotB.fTotalVert;

   //Histograms and maps, very posssible that these get overridden in the DrawCanvas function anyway.
   for(int i = 0; i < plotB.fHistos.GetSize(); i++)
   {
      outputplot.fHistos.Add(plotB.fHistos.At(i));
   }
   outputplot.fHistoPositions.insert( plotB.fHistoPositions.begin(), plotB.fHistoPositions.end() );

   outputplot.fTimeWindows+=plotB.fTimeWindows;
   outputplot.fVertexEvents+=plotB.fVertexEvents;

   return outputplot;
}
