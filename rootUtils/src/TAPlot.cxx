#include "TAPlot.h"

ClassImp(TAPlot)

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

   fVerbose=false;
}

//Copy ctor
TAPlot::TAPlot(const TAPlot& object) : TObject(object), kZeroTimeAxis(object.kZeroTimeAxis)
{
   fCanvasTitle                  = object.fCanvasTitle ;
   fMVAMode                      = object.fMVAMode ;
   fNumBins                      = object.fNumBins ; 
   fDrawStyle                    = object.fDrawStyle ;
   fLegendDetail                 = object.fLegendDetail ; 
   fApplyCuts                    = object.fApplyCuts ;
   fClassifierCut                = object.fClassifierCut ;
   fFirstTMin                    = object.fFirstTMin ;
   fLastTMax                     = object.fLastTMax ;
   fBiggestTZero                 = object.fBiggestTZero ;
   fMaxDumpLength                = object.fMaxDumpLength ;

   fTotalTime                    = object.fTotalTime ;
   fVerbose                      = object.fVerbose ;
   fTimeFactor                   = object.fTimeFactor ;

   fTimeWindows                  = object.fTimeWindows;
   fVertexEvents                 = object.fVertexEvents;

   for(size_t i=0;i<object.fEjections.size();i++)
      fEjections.push_back(object.fEjections[i]);
   
   for(size_t i=0;i<object.fInjections.size();i++)
      fInjections.push_back(object.fInjections[i]);
   
   for(size_t i=0;i<object.fDumpStarts.size();i++)
      fDumpStarts.push_back(object.fDumpStarts[i]);
   
   for(size_t i=0;i<object.fDumpStops.size();i++)
      fDumpStops.push_back(object.fDumpStops[i]);
   
   for(size_t i=0;i<object.fRuns.size();i++)
      fRuns.push_back(object.fRuns[i]);
   
   for(size_t i=0;i<object.fFEGEM.size();i++)
      fFEGEM.push_back(object.fFEGEM[i]);
   
   for(size_t i=0;i<object.fFELV.size();i++)
      fFELV.push_back(object.fFELV[i]);

   fObjectConstructionTime        = object.fObjectConstructionTime ;
   fDataLoadedTime                = object.fDataLoadedTime ;
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
TAPlot& TAPlot::operator=(const TAPlot& rhs)
{
   //std::cout << "TAPlot = operator" << std::endl;
   this->fCanvasTitle = rhs.fCanvasTitle ;
   this->fMVAMode = rhs.fMVAMode ;
   this->fNumBins = rhs.fNumBins ; 
   this->fDrawStyle = rhs.fDrawStyle ;
   this->fLegendDetail = rhs.fLegendDetail ; 
   this->fApplyCuts = rhs.fApplyCuts ;
   this->fClassifierCut = rhs.fClassifierCut ;
   this->fFirstTMin = rhs.fFirstTMin ;
   this->fLastTMax = rhs.fLastTMax ;
   this->fBiggestTZero = rhs.fBiggestTZero ;
   this->fMaxDumpLength = rhs.fMaxDumpLength ;
   
   this->fTotalTime = rhs.fTotalTime ;
   this->fVerbose = rhs.fVerbose ;
   this->fTimeFactor = rhs.fTimeFactor ;

   this->fTimeWindows = rhs.fTimeWindows;
   this->fVertexEvents = rhs.fVertexEvents;

   for(size_t i=0;i<rhs.fEjections.size();i++)
      this->fEjections.push_back(rhs.fEjections[i]);
   
   for(size_t i=0;i<rhs.fInjections.size();i++)
      this->fInjections.push_back(rhs.fInjections[i]);
   
   for(size_t i=0;i<rhs.fDumpStarts.size();i++)
      this->fDumpStarts.push_back(rhs.fDumpStarts[i]);
   
   for(size_t i=0;i<rhs.fDumpStops.size();i++)
      this->fDumpStops.push_back(rhs.fDumpStops[i]);
   
   for(size_t i=0;i<rhs.fRuns.size();i++)
      this->fRuns.push_back(rhs.fRuns[i]);
   
   for(size_t i=0;i<rhs.fFEGEM.size();i++)
      this->fFEGEM.push_back(rhs.fFEGEM[i]);
   
   for(size_t i=0;i<rhs.fFELV.size();i++)
      this->fFELV.push_back(rhs.fFELV[i]);

   this->fObjectConstructionTime = rhs.fObjectConstructionTime ;
   this->fDataLoadedTime = rhs.fDataLoadedTime ;

   return *this;
}

//=+ Operator
TAPlot& TAPlot::operator+=(const TAPlot &rhs) 
{
   //std::cout << "TAPlot += operator" << std::endl;

   //Vectors- need concating
   this->fEjections.insert(this->fEjections.end(), rhs.fEjections.begin(), rhs.fEjections.end() );
   this->fInjections.insert(this->fInjections.end(), rhs.fInjections.begin(), rhs.fInjections.end() );
   this->fDumpStarts.insert(this->fDumpStarts.end(), rhs.fDumpStarts.begin(), rhs.fDumpStarts.end() );
   this->fDumpStops.insert(this->fDumpStops.end(), rhs.fDumpStops.begin(), rhs.fDumpStops.end() );
   this->fRuns.insert(this->fRuns.end(), rhs.fRuns.begin(), rhs.fRuns.end() );//check dupes - ignore copies. AddRunNumber
   this->fFEGEM.insert(this->fFEGEM.end(), rhs.fFEGEM.begin(), rhs.fFEGEM.end() );
   this->fFELV.insert(this->fFELV.end(), rhs.fFELV.begin(), rhs.fFELV.end() );

   //Strings
   this->fCanvasTitle+= ", ";
   this->fCanvasTitle+=rhs.fCanvasTitle;

   //All doubles
   this->fFirstTMin              = (this->fFirstTMin < rhs.fFirstTMin)?this->fFirstTMin:rhs.fFirstTMin;
   this->fLastTMax               = (this->fLastTMax > rhs.fLastTMax)?this->fLastTMax:rhs.fLastTMax;
   this->fBiggestTZero           = (this->fBiggestTZero > rhs.fBiggestTZero)?this->fBiggestTZero:rhs.fBiggestTZero;
   this->fMaxDumpLength          = (this->fMaxDumpLength > rhs.fMaxDumpLength)?this->fMaxDumpLength:rhs.fMaxDumpLength;
   this->fTotalTime             += rhs.fTotalTime;
   this->fObjectConstructionTime = (this->fObjectConstructionTime < rhs.fObjectConstructionTime)?this->fObjectConstructionTime:rhs.fObjectConstructionTime;
   this->fDataLoadedTime         = (this->fDataLoadedTime > rhs.fDataLoadedTime)?this->fDataLoadedTime:rhs.fDataLoadedTime;
   this->fTimeFactor                = (this->fTimeFactor < rhs.fTimeFactor)?this->fTimeFactor:rhs.fTimeFactor;
   
   for(int i = 0; i < rhs.fHistos.GetSize(); i++)
   {
      this->fHistos.Add(rhs.fHistos.At(i));
   }
   this->fHistoPositions.insert( rhs.fHistoPositions.begin(), rhs.fHistoPositions.end() );

   this->fTimeWindows+=rhs.fTimeWindows;
   this->fVertexEvents+=rhs.fVertexEvents;

   return *this;
}

//Addition
TAPlot operator+(const TAPlot& lhs, const TAPlot& rhs)
{
   //std::cout << "TAPlot addition operator" << std::endl;
   TAPlot outputPlot(lhs); //Create new from copy

   //Vectors- need concacting
   outputPlot.fEjections.insert(outputPlot.fEjections.end(), rhs.fEjections.begin(), rhs.fEjections.end() );
   outputPlot.fInjections.insert(outputPlot.fInjections.end(), rhs.fInjections.begin(), rhs.fInjections.end() );
   outputPlot.fDumpStarts.insert(outputPlot.fDumpStarts.end(), rhs.fDumpStarts.begin(), rhs.fDumpStarts.end() );
   outputPlot.fDumpStops.insert(outputPlot.fDumpStops.end(), rhs.fDumpStops.begin(), rhs.fDumpStops.end() );
   outputPlot.fRuns.insert(outputPlot.fRuns.end(), rhs.fRuns.begin(), rhs.fRuns.end() );//check dupes - ignore copies. AddRunNumber
   outputPlot.fFEGEM.insert(outputPlot.fFEGEM.end(), rhs.fFEGEM.begin(), rhs.fFEGEM.end() );
   outputPlot.fFELV.insert(outputPlot.fFELV.end(), rhs.fFELV.begin(), rhs.fFELV.end() );

   //Strings - This title gets overridden in the DrawCanvas function anyway, but until then they are concacted. 
   outputPlot.fCanvasTitle+= ", ";
   outputPlot.fCanvasTitle+=rhs.fCanvasTitle;

   //All doubles
   outputPlot.fFirstTMin              = (outputPlot.fFirstTMin < rhs.fFirstTMin)?outputPlot.fFirstTMin:rhs.fFirstTMin;
   outputPlot.fLastTMax               = (outputPlot.fLastTMax > rhs.fLastTMax)?outputPlot.fLastTMax:rhs.fLastTMax;
   outputPlot.fBiggestTZero           = (outputPlot.fBiggestTZero > rhs.fBiggestTZero)?outputPlot.fBiggestTZero:rhs.fBiggestTZero;
   outputPlot.fMaxDumpLength          = (outputPlot.fMaxDumpLength > rhs.fMaxDumpLength)?outputPlot.fMaxDumpLength:rhs.fMaxDumpLength;
   outputPlot.fTotalTime             += rhs.fTotalTime;
   outputPlot.fObjectConstructionTime = (outputPlot.fObjectConstructionTime < rhs.fObjectConstructionTime)?outputPlot.fObjectConstructionTime:rhs.fObjectConstructionTime;
   outputPlot.fDataLoadedTime         = (outputPlot.fDataLoadedTime > rhs.fDataLoadedTime)?outputPlot.fDataLoadedTime:rhs.fDataLoadedTime;
   outputPlot.fTimeFactor                = (outputPlot.fTimeFactor < rhs.fTimeFactor)?outputPlot.fTimeFactor:rhs.fTimeFactor;

   //Histograms and maps, very posssible that these get overridden in the DrawCanvas function anyway.
   for(int i = 0; i < rhs.fHistos.GetSize(); i++)
   {
      outputPlot.fHistos.Add(rhs.fHistos.At(i));
   }
   outputPlot.fHistoPositions.insert( rhs.fHistoPositions.begin(), rhs.fHistoPositions.end() );

   outputPlot.fTimeWindows+=rhs.fTimeWindows;
   outputPlot.fVertexEvents+=rhs.fVertexEvents;

   return outputPlot;
}

//Function to print time ranges
void TAPlot::PrintTimeRanges()
{
   for (auto& window: fTimeWindows.fMaxTime)
      printf("w = %f", window);
}

//Prints full object, can be changed as required.
void TAPlot::PrintFull()
{
   std::cout << "===========================" << std::endl;
   std::cout << "Printing TAPlot located at " << this << std::endl;
   std::cout << "===========================" << std::endl;
   std::cout << "Title is " << fCanvasTitle << std::endl;

   for(size_t i=0; i<fVertexEvents.fRunNumbers.size(); i++)
   {
      std::cout << fVertexEvents.fRunNumbers.at(i) << std::endl;
   }
   for(size_t i=0; i<fVertexEvents.fEventNos.size(); i++)
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
  for (size_t i=0; i<fRuns.size(); i++)
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
void TAPlot::SetGEMChannel(const std::string& name, int arrayEntry, std::string label)
{
   for (auto& obj: fFEGEM)
   {
      if (obj.fArrayNumber!= arrayEntry)
         continue;
      if (obj.fVariableName!=name)
         continue;
      std::cout<<"GEM Channel "<< name.c_str()<< "["<<arrayEntry<<"] already registered"<<std::endl;
   }
   TFEGEMData newEntry;
   newEntry.fVariableName = name;
   if (label.size() == 0)
      newEntry.fLabel = name;
   else
      newEntry.fLabel = label;
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
      if (obj.fVariableName!=name)
         continue;
      std::cout<<"LV Channel "<< name.c_str()<< "["<<arrayEntry<<"] already registered"<<std::endl;
   }
   TFELabVIEWData newEntry;
   newEntry.fVariableName = name;
   if (title.size() == 0)
      newEntry.fLabel = name;
   else
      newEntry.fLabel = title;
   newEntry.fArrayNumber = arrayEntry;
   
   fFELV.push_back(newEntry);
}

//Getters
std::vector<std::pair<std::string,int>> TAPlot::GetGEMChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& obj: fFEGEM)
      channels.push_back( {obj.GetVariable(), obj.fArrayNumber} );
   return channels;
}

std::vector<std::pair<std::string,int>> TAPlot::GetLVChannels()
{
   std::vector<std::pair<std::string,int>> channels;
   for (auto& obj: fFEGEM)
      channels.push_back( {obj.GetVariable(), obj.fArrayNumber} );
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
      for (size_t i=0; i< fTimeWindows.fMaxTime.size(); i++)
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
         uniqueLabels[obj.GetVariable()] = graph;
         //if (i==0)
         //   legend->AddEntry(graph,f.GetVariable().c_str());
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
      for (size_t i=0; i< fTimeWindows.fMaxTime.size(); i++)
      {
         size_t colourID=0;
         for ( ; colourID< uniqueRuns.size(); colourID++)
         {
            if (fTimeWindows.fRunNumber[i] == uniqueRuns.at(colourID))
               break;
         }
         TGraph* graph = obj.BuildGraph(i, kZeroTimeAxis);
         graph->SetLineColor(GetColour(colourID + colourOffset));
         graph->SetMarkerColor(GetColour(colourID + colourOffset));
         uniqueLabels[obj.GetVariable()] = graph;
         
         //if (i==0)
         //   legend->AddEntry(graph,f.GetVariable().c_str());
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
   for (size_t i = 0; i<fVertexEvents.fXVertex.size(); i++)
   {
      if (fVertexEvents.fCutsResults[i]&kType)
         n++;
   }
   return n;
}

int TAPlot::GetNVertexType(const int kType)
{
   int n = 0;
   //for (auto& event: VertexEvents)
   //const TVertexEvents* event = GetVertexEvents();
   for (size_t i = 0; i<fVertexEvents.fXVertex.size(); i++)
   {
      if (fVertexEvents.fVertexStatuses[i]&kType)
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

void TAPlot::AddTimeGates(int runNumber, std::vector<double> minTime, std::vector<double> maxTime, std::vector<double> zeroTime)
{
   AddRunNumber(runNumber);

   assert(minTime.size() == maxTime.size());
   assert(minTime.size() == zeroTime.size());

   for (size_t i=0; i<minTime.size(); i++)
   {
      double length = maxTime[i]-minTime[i];
      if (length>fMaxDumpLength)
         fMaxDumpLength=length;
      fTimeWindows.AddTimeWindow(runNumber,minTime[i],maxTime[i],zeroTime[i]);
      fTotalTime+=maxTime[i]-minTime[i];
      //Find the first start window
      if (minTime[i]<fFirstTMin)
         fFirstTMin=minTime[i];
      //Largest time before 'zero' (-ve number)
      if ( minTime[i] - zeroTime[i] < fBiggestTZero)
         fBiggestTZero = minTime[i] - zeroTime[i];
      //Find the end of the last window (note: -ve tmax means end of run)
      //Skip early if we are looking for end of run anyway
      if (fLastTMax<0)
         continue;
      //Find the highest value
      if (maxTime[i]>fLastTMax)
         fLastTMax=maxTime[i];
      //Set -ve of we want end of run
      if (maxTime[i]<0)
         fLastTMax=-1;
   }
   return;
}

void TAPlot::AddTimeGates(int runNumber, std::vector<double> minTime, std::vector<double> maxTime)
{
   std::vector<double> zeroTime;
   zeroTime.reserve(minTime.size());
   for (auto& times: minTime)
      zeroTime.push_back(times);
   return AddTimeGates(runNumber,minTime,maxTime,zeroTime);
}

void TAPlot::AddTimeGate(const int runNumber, const double minTime, const double maxTime, const double zeroTime)
{
   AddRunNumber(runNumber);

   double length = maxTime - minTime;
   if (length > fMaxDumpLength)
      fMaxDumpLength = length;
   fTimeWindows.AddTimeWindow(runNumber,minTime,maxTime,zeroTime);
   fTotalTime += maxTime - minTime;
   //Find the first start window
   if (minTime < fFirstTMin)
      fFirstTMin = minTime;
   //Largest time before 'zero' (-ve number)
   if ( minTime - zeroTime < fBiggestTZero)
      fBiggestTZero = minTime - zeroTime;
   //Find the end of the last window (note: -ve tmax means end of run)
   //Skip early if we are looking for end of run anyway
   if (fLastTMax < 0)
      return;
   //Find the highest value
   if (maxTime > fLastTMax)
      fLastTMax = maxTime;
   //Set -ve of we want end of run
   if (maxTime < 0)
      fLastTMax = -1;
   return;

}

//It is slightly faster to call AddTimeGates than this function
void TAPlot::AddTimeGate(const int runNumber, const double minTime, const double maxTime)
{
   return AddTimeGate(runNumber,minTime,maxTime,minTime);
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
void TAPlot::LoadFEGEMData(TFEGEMData& gemData, TTreeReader* gemReader, const char* name, double firstTime, double lastTime)
{
   TTreeReaderValue<TStoreGEMData<T>> gemEvent(*gemReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (gemReader->Next())
   {
      double runTime = gemEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (runTime < firstTime)
         continue;
      if (runTime > lastTime)
         break;
      gemData.AddGEMEvent(&(*gemEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadFEGEMData(int runNumber, double firstTime, double lastTime)
{
   for (auto& obj: fFEGEM)
   {
      TTreeReader* feGEMReader=Get_feGEM_Tree(runNumber, obj.fVariableName);
      TTree* tree = feGEMReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << obj.GetVariable() << " ("<<obj.fVariableName<<") not found for run " << runNumber << std::endl;
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

void TAPlot::LoadFELVData(int runNumber, TFELabVIEWData& labviewData, TTreeReader* labviewReader, const char* name, double firstTime, double lastTime)
{
   TTreeReaderValue<TStoreLabVIEWEvent> labviewEvent(*labviewReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (labviewReader->Next())
   {
      double runTime = labviewEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (runTime < firstTime)
         continue;
      if (runTime > lastTime)
         break;
      labviewData.AddLVEvent(runNumber, &(*labviewEvent), *GetTimeWindows());
   }
   return;
}

void TAPlot::LoadFELVData(int runNumber, double firstTime, double lastTime)
{
   //For each unique variable being logged
   for (auto& obj: fFELV)
   {
      TTreeReader* labviewReader=Get_feLV_Tree(runNumber, obj.fVariableName);
      TTree* tree = labviewReader->GetTree();
      if  (!tree)
      {
         std::cout<<"Warning: " << obj.GetVariable() << " ("<<obj.fVariableName<<") not found for run " << runNumber << std::endl;
         continue;
      }
      if (tree->GetBranchStatus("TStoreLabVIEWEvent"))
         LoadFELVData(runNumber, obj, labviewReader, "TStoreLabVIEWEvent", firstTime, lastTime);
      else
         std::cout << "Warning unable to find TStoreLVData type" << std::endl;   
   }
}

void TAPlot::LoadData(bool verbose)
{
   for (size_t i=0; i<fRuns.size(); i++)
   {
      double lastTime = 0;
      double firstTime = 1E99;
      int runNumber = fRuns[i];
      //Calculate our list time... so we can stop early
      //for (auto& t: GetTimeWindows())
      //TTimeWindows t = GetTimeWindows();
      for (size_t i=0; i<GetTimeWindows()->fMaxTime.size(); i++)
      {
         if (GetTimeWindows()->fRunNumber[i]==runNumber)
         {
            if (GetTimeWindows()->fMaxTime[i]<0) 
               lastTime = 1E99;
            if (lastTime < GetTimeWindows()->fMaxTime[i])
               lastTime = GetTimeWindows()->fMaxTime[i];
            if (firstTime > GetTimeWindows()->fMinTime[i] )
               firstTime = GetTimeWindows()->fMinTime[i];
         }
      }
      if (verbose)
      {
         std::cout<<"Loading data from run " << runNumber << " from t="<< firstTime <<" to t="<<lastTime<<"\n";
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

void TAPlot::ExportCSV(std::string filename, bool PassedCutOnly)
{
   std::string vertexFilename = filename + ".vertex.csv";
   std::string timeWindowFilename = filename + ".timewindows.csv";
   std::ofstream verts;
   verts.open(vertexFilename);
   verts << fVertexEvents.CSVTitleLine();
   for (size_t i=0; i< fVertexEvents.size(); i++)
      verts << fVertexEvents.CSVLine(i);
   verts.close();
   std::cout<< vertexFilename<< " saved\n";

   std::ofstream times;
   times.open(timeWindowFilename);
   times << fTimeWindows.CSVTitleLine();
   for (size_t i=0; i< fTimeWindows.size(); i++)
      times << fTimeWindows.CSVLine(i);
   times.close();
   std::cout<< timeWindowFilename<< " saved\n";
}
