#include "TAPlot.h"

ClassImp(TAPlot);

//DEFAULT CONTRUCTORS, OPERATERS, PRINTS:

//Default Constructor
TAPlot::TAPlot(bool zerotime) : kZeroTimeAxis(zerotime)
{
   fObjectConstructionTime = TTimeStamp();
   //Set to zero for 'unset'
   fDataLoadedTime = TTimeStamp(0);
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

//Copy ctor
TAPlot::TAPlot(const TAPlot& kTAPlot) : kZeroTimeAxis(kTAPlot.kZeroTimeAxis)
{
   fTitle                        = kTAPlot.fTitle ;
   fMVAMode                      = kTAPlot.fMVAMode ;
   fNumBins                      = kTAPlot.fNumBins ; 
   fDrawStyle                    = kTAPlot.fDrawStyle ;
   fLegendDetail                 = kTAPlot.fLegendDetail ; 
   fApplyCuts                    = kTAPlot.fApplyCuts ;
   fClassifierCut                = kTAPlot.fClassifierCut ;
   fFirstTMin                    = kTAPlot.fFirstTMin ;
   fLastTMax                     = kTAPlot.fLastTMax ;
   fBiggestTZero                 = kTAPlot.fBiggestTZero ;
   fMaxDumpLength                = kTAPlot.fMaxDumpLength ;

   fTotalTime                    = kTAPlot.fTotalTime ;
   fTotalVert                    = kTAPlot.fTotalVert ;
   fVerbose                      = kTAPlot.fVerbose ;
   fTimeFactor                   = kTAPlot.fTimeFactor ;

   fTimeWindows                  = kTAPlot.fTimeWindows;
   fVertexEvents                 = kTAPlot.fVertexEvents;

   for(int i=0;i<kTAPlot.fEjections.size();i++)
      fEjections.push_back(kTAPlot.fEjections[i]);
   
   for(int i=0;i<kTAPlot.fInjections.size();i++)
      fInjections.push_back(kTAPlot.fInjections[i]);
   
   for(int i=0;i<kTAPlot.fDumpStarts.size();i++)
      fDumpStarts.push_back(kTAPlot.fDumpStarts[i]);
   
   for(int i=0;i<kTAPlot.fDumpStops.size();i++)
      fDumpStops.push_back(kTAPlot.fDumpStops[i]);
   
   for(int i=0;i<kTAPlot.fRuns.size();i++)
      fRuns.push_back(kTAPlot.fRuns[i]);
   
   for(int i=0;i<kTAPlot.fFEGEM.size();i++)
      fFEGEM.push_back(kTAPlot.fFEGEM[i]);
   
   for(int i=0;i<kTAPlot.fFELV.size();i++)
      fFELV.push_back(kTAPlot.fFELV[i]);

   fObjectConstructionTime        = kTAPlot.fObjectConstructionTime ;
   fDataLoadedTime                = kTAPlot.fDataLoadedTime ;
}

//Default Destructor
TAPlot::~TAPlot()
{
  fEjections.clear();
  fInjections.clear();
  fDumpStarts.clear();
  fDumpStops.clear();
  fRuns.clear();
  ClearHisto();
}

//Assignment.
TAPlot& TAPlot::operator=(const TAPlot& kTAPlot)
{
   std::cout << "TAPlot = operator" << std::endl;
   this->fTitle = kTAPlot.fTitle ;
   this->fMVAMode = kTAPlot.fMVAMode ;
   this->fNumBins = kTAPlot.fNumBins ; 
   this->fDrawStyle = kTAPlot.fDrawStyle ;
   this->fLegendDetail = kTAPlot.fLegendDetail ; 
   this->fApplyCuts = kTAPlot.fApplyCuts ;
   this->fClassifierCut = kTAPlot.fClassifierCut ;
   this->fFirstTMin = kTAPlot.fFirstTMin ;
   this->fLastTMax = kTAPlot.fLastTMax ;
   this->fBiggestTZero = kTAPlot.fBiggestTZero ;
   this->fMaxDumpLength = kTAPlot.fMaxDumpLength ;
   
   this->fTotalTime = kTAPlot.fTotalTime ;
   this->fTotalVert = kTAPlot.fTotalVert ;
   this->fVerbose = kTAPlot.fVerbose ;
   this->fTimeFactor = kTAPlot.fTimeFactor ;

   this->fTimeWindows = kTAPlot.fTimeWindows;
   this->fVertexEvents = kTAPlot.fVertexEvents;

   for(int i=0;i<kTAPlot.fEjections.size();i++)
      this->fEjections.push_back(kTAPlot.fEjections[i]);
   
   for(int i=0;i<kTAPlot.fInjections.size();i++)
      this->fInjections.push_back(kTAPlot.fInjections[i]);
   
   for(int i=0;i<kTAPlot.fDumpStarts.size();i++)
      this->fDumpStarts.push_back(kTAPlot.fDumpStarts[i]);
   
   for(int i=0;i<kTAPlot.fDumpStops.size();i++)
      this->fDumpStops.push_back(kTAPlot.fDumpStops[i]);
   
   for(int i=0;i<kTAPlot.fRuns.size();i++)
      this->fRuns.push_back(kTAPlot.fRuns[i]);
   
   for(int i=0;i<kTAPlot.fFEGEM.size();i++)
      this->fFEGEM.push_back(kTAPlot.fFEGEM[i]);
   
   for(int i=0;i<kTAPlot.fFELV.size();i++)
      this->fFELV.push_back(kTAPlot.fFELV[i]);

   this->fObjectConstructionTime = kTAPlot.fObjectConstructionTime ;
   this->fDataLoadedTime = kTAPlot.fDataLoadedTime ;

   return *this;
}

//=+ Operator
TAPlot& TAPlot::operator+=(const TAPlot &kTAPlotA) 
{
   std::cout << "TAPlot += operator" << std::endl;

   //Vectors- need concating
   this->fEjections.insert(this->fEjections.end(), kTAPlotA.fEjections.begin(), kTAPlotA.fEjections.end() );
   this->fInjections.insert(this->fInjections.end(), kTAPlotA.fInjections.begin(), kTAPlotA.fInjections.end() );
   this->fDumpStarts.insert(this->fDumpStarts.end(), kTAPlotA.fDumpStarts.begin(), kTAPlotA.fDumpStarts.end() );
   this->fDumpStops.insert(this->fDumpStops.end(), kTAPlotA.fDumpStops.begin(), kTAPlotA.fDumpStops.end() );
   this->fRuns.insert(this->fRuns.end(), kTAPlotA.fRuns.begin(), kTAPlotA.fRuns.end() );//check dupes - ignore copies. AddRunNumber
   this->fFEGEM.insert(this->fFEGEM.end(), kTAPlotA.fFEGEM.begin(), kTAPlotA.fFEGEM.end() );
   this->fFELV.insert(this->fFELV.end(), kTAPlotA.fFELV.begin(), kTAPlotA.fFELV.end() );

   //Strings
   this->fTitle+= ", ";
   this->fTitle+=kTAPlotA.fTitle;

   //All doubles
   this->fFirstTMin              = (this->fFirstTMin < kTAPlotA.fFirstTMin)?this->fFirstTMin:kTAPlotA.fFirstTMin;
   this->fLastTMax               = (this->fLastTMax > kTAPlotA.fLastTMax)?this->fLastTMax:kTAPlotA.fLastTMax;
   this->fBiggestTZero           = (this->fBiggestTZero > kTAPlotA.fBiggestTZero)?this->fBiggestTZero:kTAPlotA.fBiggestTZero;
   this->fMaxDumpLength          = (this->fMaxDumpLength > kTAPlotA.fMaxDumpLength)?this->fMaxDumpLength:kTAPlotA.fMaxDumpLength;
   this->fTotalTime             = (this->fTotalTime > kTAPlotA.fTotalTime)?this->fTotalTime:kTAPlotA.fTotalTime;
   this->fObjectConstructionTime = (this->fObjectConstructionTime < kTAPlotA.fObjectConstructionTime)?this->fObjectConstructionTime:kTAPlotA.fObjectConstructionTime;
   this->fDataLoadedTime         = (this->fDataLoadedTime > kTAPlotA.fDataLoadedTime)?this->fDataLoadedTime:kTAPlotA.fDataLoadedTime;
   this->fTimeFactor                = (this->fTimeFactor < kTAPlotA.fTimeFactor)?this->fTimeFactor:kTAPlotA.fTimeFactor;
   this->fTotalVert             += kTAPlotA.fTotalVert;

   for(int i = 0; i < kTAPlotA.fHistos.GetSize(); i++)
   {
      this->fHistos.Add(kTAPlotA.fHistos.At(i));
   }
   this->fHistoPositions.insert( kTAPlotA.fHistoPositions.begin(), kTAPlotA.fHistoPositions.end() );

   this->fTimeWindows+=kTAPlotA.fTimeWindows;
   this->fVertexEvents+=kTAPlotA.fVertexEvents;

   return *this;
}

//Addition
TAPlot operator+(const TAPlot& kTAPlotA, const TAPlot& kTAPlotB)
{
   std::cout << "TAPlot addition operator" << std::endl;
   TAPlot outputPlot(kTAPlotA); //Create new from copy

   //Vectors- need concacting
   outputPlot.fEjections.insert(outputPlot.fEjections.end(), kTAPlotB.fEjections.begin(), kTAPlotB.fEjections.end() );
   outputPlot.fInjections.insert(outputPlot.fInjections.end(), kTAPlotB.fInjections.begin(), kTAPlotB.fInjections.end() );
   outputPlot.fDumpStarts.insert(outputPlot.fDumpStarts.end(), kTAPlotB.fDumpStarts.begin(), kTAPlotB.fDumpStarts.end() );
   outputPlot.fDumpStops.insert(outputPlot.fDumpStops.end(), kTAPlotB.fDumpStops.begin(), kTAPlotB.fDumpStops.end() );
   outputPlot.fRuns.insert(outputPlot.fRuns.end(), kTAPlotB.fRuns.begin(), kTAPlotB.fRuns.end() );//check dupes - ignore copies. AddRunNumber
   outputPlot.fFEGEM.insert(outputPlot.fFEGEM.end(), kTAPlotB.fFEGEM.begin(), kTAPlotB.fFEGEM.end() );
   outputPlot.fFELV.insert(outputPlot.fFELV.end(), kTAPlotB.fFELV.begin(), kTAPlotB.fFELV.end() );

   //Strings - This title gets overridden in the DrawCanvas function anyway, but until then they are concacted. 
   outputPlot.fTitle+= ", ";
   outputPlot.fTitle+=kTAPlotB.fTitle;

   //All doubles
   outputPlot.fFirstTMin              = (outputPlot.fFirstTMin < kTAPlotB.fFirstTMin)?outputPlot.fFirstTMin:kTAPlotB.fFirstTMin;
   outputPlot.fLastTMax               = (outputPlot.fLastTMax > kTAPlotB.fLastTMax)?outputPlot.fLastTMax:kTAPlotB.fLastTMax;
   outputPlot.fBiggestTZero           = (outputPlot.fBiggestTZero > kTAPlotB.fBiggestTZero)?outputPlot.fBiggestTZero:kTAPlotB.fBiggestTZero;
   outputPlot.fMaxDumpLength          = (outputPlot.fMaxDumpLength > kTAPlotB.fMaxDumpLength)?outputPlot.fMaxDumpLength:kTAPlotB.fMaxDumpLength;
   outputPlot.fTotalTime             = (outputPlot.fTotalTime > kTAPlotB.fTotalTime)?outputPlot.fTotalTime:kTAPlotB.fTotalTime;
   outputPlot.fObjectConstructionTime = (outputPlot.fObjectConstructionTime < kTAPlotB.fObjectConstructionTime)?outputPlot.fObjectConstructionTime:kTAPlotB.fObjectConstructionTime;
   outputPlot.fDataLoadedTime         = (outputPlot.fDataLoadedTime > kTAPlotB.fDataLoadedTime)?outputPlot.fDataLoadedTime:kTAPlotB.fDataLoadedTime;
   outputPlot.fTimeFactor                = (outputPlot.fTimeFactor < kTAPlotB.fTimeFactor)?outputPlot.fTimeFactor:kTAPlotB.fTimeFactor;
   outputPlot.fTotalVert             += kTAPlotB.fTotalVert;

   //Histograms and maps, very posssible that these get overridden in the DrawCanvas function anyway.
   for(int i = 0; i < kTAPlotB.fHistos.GetSize(); i++)
   {
      outputPlot.fHistos.Add(kTAPlotB.fHistos.At(i));
   }
   outputPlot.fHistoPositions.insert( kTAPlotB.fHistoPositions.begin(), kTAPlotB.fHistoPositions.end() );

   outputPlot.fTimeWindows+=kTAPlotB.fTimeWindows;
   outputPlot.fVertexEvents+=kTAPlotB.fVertexEvents;

   return outputPlot;
}

//Function to print time ranges
void TAPlot::PrintTimeRanges()
{
   for (auto& window: fTimeWindows.fTMax)
      printf("w = %f", window);
}

//Prints full object, can be changed as required.
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

void TAPlot::Print(Option_t *option) const
{
  std::cout<<"TAPlot Summary"<<std::endl;
  //FillHisto();
  std::cout <<""<<std::endl<<"Run(s): ";
  for (UInt_t i=0; i<fRuns.size(); i++)
  {
     if (i>1) std::cout <<", ";
     std::cout <<fRuns[i];
  }
  std::cout <<std::endl;
  
  //Loop over TObj array and print it?
  for (int i=0; i<fHistos.GetEntries(); i++)
  {
      TH1D* histogram = dynamic_cast<TH1D*>(fHistos.At(i));
      if (histogram)
      {
         std::cout <<histogram->GetTitle()<<"\t"<<histogram->Integral()<<std::endl;
      }
  }
  
}


//Setters
void TAPlot::SetGEMChannel(const std::string& name, int arrayEntry, std::string title)
{
   for (auto& obj: fFEGEM)
   {
      if (obj.fArrayNumber!= arrayEntry)
         continue;
      if (obj.fName!=name)
         continue;
      std::cout<<"GEM Channel "<< name.c_str()<< "["<<arrayEntry<<"] already registered"<<std::endl;
   }
   TFEGEMData newEntry;
   newEntry.fName = name;
   if (title.size() == 0)
      newEntry.fTitle = name;
   else
      newEntry.fTitle = title;
   newEntry.fArrayNumber = arrayEntry;
   
   fFEGEM.push_back(newEntry);
}

void TAPlot::SetGEMChannel(const std::string& category, const std::string& varName, int arrayEntry, std::string title)
{
   //Perhaps this line should be a function used everywhere
   std::string name = TFEGEMData::CombinedName(category, varName);
   return SetGEMChannel(name,arrayEntry,title);
}

void TAPlot::SetLVChannel(const std::string& name, int arrayEntry, std::string title)
{
   for (auto& obj: fFELV)
   {
      if (obj.fArrayNumber!= arrayEntry)
         continue;
      if (obj.fName!=name)
         continue;
      std::cout<<"LV Channel "<< name.c_str()<< "["<<arrayEntry<<"] already registered"<<std::endl;
   }
   TFELVData newEntry;
   newEntry.fName = name;
   if (title.size() == 0)
      newEntry.fTitle = name;
   else
      newEntry.fTitle = title;
   newEntry.fArrayNumber = arrayEntry;
   
   fFELV.push_back(newEntry);
}

//Getters
std::vector<std::pair<std::string,int>> TAPlot::GetGEMChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& obj: fFEGEM)
      channels.push_back( {obj.GetName(), obj.fArrayNumber} );
   return channels;
}

std::vector<std::pair<std::string,int>> TAPlot::GetLVChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& obj: fFEGEM)
      channels.push_back( {obj.GetName(), obj.fArrayNumber} );
   return channels;
}

std::pair<TLegend*,TMultiGraph*> TAPlot::GetGEMGraphs()
{
   TMultiGraph *feGEMMG = NULL; //Technically feGEMMG is not right but i think this is okay too?
   TLegend* legend = NULL;
   if (fFELV.size()==0)
      return {legend, feGEMMG};
   feGEMMG = new TMultiGraph();
   legend = new TLegend(0.1,0.7,0.48,0.9);
   //For each unique variable being logged
   int colourOffset = 0;
   for (auto& obj: fFEGEM)
   {
      std::map<std::string,TGraph*> uniqueLabels;
      const std::vector<int> kUniqueRuns = GetArrayOfRuns();
      for (size_t i=0; i< fTimeWindows.fTMax.size(); i++)
      {
         size_t colourID = 0;
         for ( ; colourID< kUniqueRuns.size(); colourID++)
         {
            if (fTimeWindows.fRunNumber[i] == kUniqueRuns.at(colourID))
               break;
         }
         TGraph* graph = obj.BuildGraph(i,kZeroTimeAxis);
         graph->SetLineColor(GetColour(colourID + colourOffset));
         graph->SetMarkerColor(GetColour(colourID + colourOffset));
         uniqueLabels[obj.GetName()] = graph;
         //if (i==0)
         //   legend->AddEntry(graph,f.GetName().c_str());
         if (graph->GetN())
            feGEMMG->Add(graph);
      }
      for (auto& label: uniqueLabels)
      {
         legend->AddEntry(label.second,label.first.c_str());
      }
      colourOffset++;
   }
   return {legend,feGEMMG};
}

std::pair<TLegend*,TMultiGraph*> TAPlot::GetLVGraphs()
{
   TMultiGraph *feLVMG = NULL;
   TLegend* legend = NULL;
   if (fFELV.size()==0)
      return {legend,feLVMG};
   feLVMG = new TMultiGraph();
   legend = new TLegend(0.1,0.7,0.48,0.9);
   //For each unique variable being logged
   int colourOffset = 0;
   for (auto& obj: fFELV)
   {
      std::map<std::string,TGraph*> uniqueLabels;

      const std::vector<int> uniqueRuns = GetArrayOfRuns();
      for (size_t i=0; i< fTimeWindows.fTMax.size(); i++)
      {
         size_t colourID=0;
         for ( ; colourID< uniqueRuns.size(); colourID++)
         {
            if (fTimeWindows.fRunNumber[i] == uniqueRuns.at(colourID))
               break;
         }
         TGraph* graph = obj.BuildGraph(i,kZeroTimeAxis);
         graph->SetLineColor(GetColour(colourID + colourOffset));
         graph->SetMarkerColor(GetColour(colourID + colourOffset));
         uniqueLabels[obj.GetName()] = graph;
         
         //if (i==0)
         //   legend->AddEntry(graph,f.GetName().c_str());
         //Add the graph only if there is data in it
         if (graph->GetN())
            feLVMG->Add(graph);
      }
      for (auto& label: uniqueLabels)
      {
         legend->AddEntry(label.second,label.first.c_str());
      }
      colourOffset++;
   }
   return {legend,feLVMG};
}

double TAPlot::GetApproximateProcessingTime()
{
   if ( fDataLoadedTime == TTimeStamp(0) )
      LoadingDataLoadingDone();
   return fDataLoadedTime.AsDouble() - fObjectConstructionTime.AsDouble();
}

int TAPlot::GetNPassedType(const int kType)
{
   int n = 0;
   //for (auto& event: VertexEvents)
   //const TVertexEvents* event = GetVertexEvents();
   for (int i = 0; i<fVertexEvents.fXVertex.size(); i++)
   {
      if (fVertexEvents.fCutsResults[i]&kType)
         n++;
   }
   return n;
}

TString TAPlot::GetListOfRuns()
{
   TString runsString="";
   std::sort(fRuns.begin(), fRuns.end());
   for (size_t i = 0; i < fRuns.size(); i++)
   {
      //std::cout <<"Run: "<<Runs[i] <<std::endl;
      if (i > 0)
         runsString += ",";
      runsString += fRuns[i];
   }
   return runsString;
}

//Adders
void TAPlot::AddRunNumber(int runNumber)
{
   for (auto& number: fRuns)
   {
      if (number==runNumber)
      {
         return;
      }
   }
   fRuns.push_back(runNumber);
}

void TAPlot::AddTimeGates(int runNumber, std::vector<double> tmin, std::vector<double> tmax, std::vector<double> tzero)
{
   AddRunNumber(runNumber);

   assert(tmin.size() == tmax.size());
   assert(tmin.size() == tzero.size());

   for (size_t i=0; i<tmin.size(); i++)
   {
      double length = tmax[i]-tmin[i];
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
   for (auto& times: tmin)
      tzero.push_back(times);
   return AddTimeGates(runNumber,tmin,tmax,tzero);
}

void TAPlot::AddTimeGate(const int kRunNumber, const double kTMin, const double kTMax, const double kTZero)
{
   AddRunNumber(kRunNumber);

   double length = kTMax - kTMin;
   if (length > fMaxDumpLength)
      fMaxDumpLength = length;
   fTimeWindows.AddTimeWindow(kRunNumber,kTMin,kTMax,kTZero);
   fTotalTime += kTMax - kTMin;
   //Find the first start window
   if (kTMin < fFirstTMin)
      fFirstTMin = kTMin;
   //Largest time before 'zero' (-ve number)
   if ( kTMin - kTZero < fBiggestTZero)
      fBiggestTZero = kTMin - kTZero;
   //Find the end of the last window (note: -ve tmax means end of run)
   //Skip early if we are looking for end of run anyway
   if (fLastTMax < 0)
      return;
   //Find the highest value
   if (kTMax > fLastTMax)
      fLastTMax = kTMax;
   //Set -ve of we want end of run
   if (kTMax < 0)
      fLastTMax = -1;
   return;

}

//It is slightly faster to call AddTimeGates than this function
void TAPlot::AddTimeGate(const int kRunNumber, const double kTMin, const double kTMax)
{
   return AddTimeGate(kRunNumber,kTMin,kTMax,kTMin);
}

void TAPlot::AddVertexEvent(int runNumber, int eventNo, int cutsResult, int vertexStatus, double x, double y, double z, double t, double eventTime, double runTime, int numHelices, int numTracks)                 
{
   fVertexEvents.fRunNumbers.push_back(runNumber);
   fVertexEvents.fEventNos.push_back(eventNo);
   fVertexEvents.fCutsResults.push_back(cutsResult);
   fVertexEvents.fVertexStatuses.push_back(vertexStatus);
   fVertexEvents.fXVertex.push_back(x);
   fVertexEvents.fYVertex.push_back(y);
   fVertexEvents.fZVertex.push_back(z);
   fVertexEvents.fTimes.push_back(t);
   fVertexEvents.fEventTimes.push_back(eventTime);
   fVertexEvents.fRunTimes.push_back(runTime);
   fVertexEvents.fNumHelices.push_back(numHelices);
   fVertexEvents.fNumTracks.push_back(numTracks);
}


//Load data functions
template <typename T>
void TAPlot::LoadFEGEMData(TFEGEMData& f, TTreeReader* feGEMReader, const char* name, double firstTime, double lastTime)
{
   TTreeReaderValue<TStoreGEMData<T>> GEMEvent(*feGEMReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (feGEMReader->Next())
   {
      double runTime = GEMEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (runTime < firstTime)
         continue;
      if (runTime > lastTime)
         break;
      f.AddGEMEvent(&(*GEMEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadFEGEMData(int runNumber, double firstTime, double lastTime)
{
   for (auto& obj: fFEGEM)
   {
      TTreeReader* feGEMReader=Get_feGEM_Tree(runNumber, obj.fName);
      TTree* tree = feGEMReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << obj.GetName() << " ("<<obj.fName<<") not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreGEMData<double>"))
         LoadFEGEMData<double>(obj, feGEMReader, "TStoreGEMData<double>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<float>"))
         LoadFEGEMData<float>(obj, feGEMReader, "TStoreGEMData<float>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<bool>"))
         LoadFEGEMData<bool>(obj, feGEMReader, "TStoreGEMData<bool>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<int32_t>"))
         LoadFEGEMData<int32_t>(obj, feGEMReader, "TStoreGEMData<int32_t>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<uint32_t>"))
         LoadFEGEMData<uint32_t>(obj, feGEMReader, "TStoreGEMData<uint32_t>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<uint16_t>"))
         LoadFEGEMData<uint16_t>(obj, feGEMReader, "TStoreGEMData<uint16_t>", firstTime, lastTime);
      else if (tree->GetBranchStatus("TStoreGEMData<char>"))
         LoadFEGEMData<char>(obj, feGEMReader, "TStoreGEMData<char>", firstTime, lastTime);
      else
         std::cout << "Warning unable to find TStoreGEMData type" << std::endl;   
   }
}

void TAPlot::LoadFELVData(TFELVData& f, TTreeReader* feLVReader, const char* name, double firstTime, double lastTime)
{
   TTreeReaderValue<TStoreLabVIEWEvent> LVEvent(*feLVReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (feLVReader->Next())
   {
      double runTime = LVEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (runTime < firstTime)
         continue;
      if (runTime > lastTime)
         break;
      f.AddLVEvent(&(*LVEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadFELVData(int runNumber, double firstTime, double lastTime)
{
   //For each unique variable being logged
   for (auto& obj: fFELV)
   {
      TTreeReader* feLVReader=Get_feLV_Tree(runNumber, obj.fName);
      TTree* tree = feLVReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << obj.GetName() << " ("<<obj.fName<<") not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreLabVIEWEvent"))
         LoadFELVData(obj, feLVReader, "TStoreLabVIEWEvent", firstTime, lastTime);
      else
         std::cout << "Warning unable to find TStoreLVData type" << std::endl;   
   }
}

void TAPlot::LoadData()
{
   for (size_t i=0; i<fRuns.size(); i++)
   {
      double lastTime = 0;
      double firstTime = 1E99;
      int runNumber = fRuns[i];
      //Calculate our list time... so we can stop early
      //for (auto& t: GetTimeWindows())
      //TTimeWindows t = GetTimeWindows();
      for (size_t i=0; i<GetTimeWindows()->fTMax.size(); i++)
      {
         if (GetTimeWindows()->fRunNumber[i]==runNumber)
         {
            if (GetTimeWindows()->fTMax[i]<0) 
               lastTime = 1E99;
            if (lastTime < GetTimeWindows()->fTMax[i])
               lastTime = GetTimeWindows()->fTMax[i];
            if (firstTime > GetTimeWindows()->fTMin[i] )
               firstTime = GetTimeWindows()->fTMin[i];
         }
      }
      LoadFEGEMData(runNumber, firstTime, lastTime);
      LoadFELVData(runNumber, firstTime, lastTime);
      LoadRun(runNumber, firstTime, lastTime);
   }
   LoadingDataLoadingDone();
   return;
}


//Histogram functions.
void TAPlot::FillHistogram(const char* keyname, double x, int counts)
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
      TLine *line = new TLine(fInjections[i]*fTimeFactor, 0., fInjections[i]*fTimeFactor, max );
      line->SetLineColor(6);
      line->Draw();
      if (i == 0)
         legend->AddEntry(line, "AD fill", "line");
   }
   for (UInt_t i = 0; i < fEjections.size(); i++)
   {
      TLine *line = new TLine(fEjections[i]*fTimeFactor, 0., fEjections[i]*fTimeFactor, max );
      line->SetLineColor(7);
      line->Draw();
      if (i == 0)
         legend->AddEntry(line, "Beam to ALPHA", "line");
   }
   for (UInt_t i = 0; i < fDumpStarts.size(); i++)
   {
      if (fDumpStarts.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *line = new TLine(fDumpStarts[i]*fTimeFactor, 0., fDumpStarts[i]*fTimeFactor, max );
      //line->SetLineColor(7);
      line->SetLineColorAlpha(kGreen, 0.35);
      //line->SetFillColorAlpha(kGreen,0.35);
      line->Draw();
      if (i == 0)
         legend->AddEntry(line, "Dump Start", "line");
   }
   for (UInt_t i = 0; i < fDumpStops.size(); i++)
   {
      if (fDumpStops.size() > 4) continue; //Don't draw dumps if there are lots
      TLine *line = new TLine(fDumpStops[i]*fTimeFactor, 0., fDumpStops[i]*fTimeFactor, max );
      //line->SetLineColor(7);
      line->SetLineColorAlpha(kRed, 0.35);
      line->Draw();
      if (i == 0)
         legend->AddEntry(line, "Dump Stop", "line");
   }
   legend->Draw();
   return legend;
}

TLegend* TAPlot::AddLegendIntegral(TLegend* legend, const char* message, const char* keyname)
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

void TAPlot::ClearHisto() //Destroy all histograms
{
   fHistos.SetOwner(kTRUE);
   fHistos.Delete();
}
