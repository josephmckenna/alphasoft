#include "TA2Plot.h"
#include <fstream>


#ifdef BUILD_A2
ClassImp(TA2Plot)

//Default members, operators, and prints.
TA2Plot::TA2Plot(bool zeroTime): TAPlot(zeroTime)
{
   fZMinCut=-99999.;
   fZMaxCut= 99999.;
}

TA2Plot::TA2Plot(double zMin, double zMax, bool zeroTime): TAPlot(zeroTime)
{
   fZMinCut = zMin;
   fZMaxCut = zMax;
}

//Construct TA2Plot from TAPlot.
TA2Plot::TA2Plot(const TAPlot& object) : TAPlot(object)
{
   std::cout << "This is TA2Plot constructor from TAPlot" << std::endl;
   fZMinCut=-99999.;
   fZMaxCut= 99999.;
}

//Copy ctor.
TA2Plot::TA2Plot(const TA2Plot& object) : TAPlot(object)
{
   //std::cout << "This is TA2Plot copy constructor" << std::endl;
   fSISChannels    = object.fSISChannels;
   fTrig           = object.fTrig;
   fTrigNobusy    = object.fTrigNobusy;
   fAtomOr        = object.fAtomOr;
   fCATStart       = object.fCATStart;
   fCATStop        = object.fCATStop;
   fRCTStart       = object.fRCTStart;
   fRCTStop        = object.fRCTStop;
   fATMStart       = object.fATMStart;
   fATMStop        = object.fATMStop;
   fBeamInjection = object.fBeamInjection;
   fBeamEjection  = object.fBeamEjection;
   fZMinCut        = object.fZMinCut;
   fZMaxCut        = object.fZMaxCut;
   SISEvents      = object.SISEvents;
}

TA2Plot::~TA2Plot()
{
}

//Assignment operator
TA2Plot& TA2Plot::operator=(const TA2Plot& rhs)
{
   //Inherited TAPlot members
   //std::cout << "TA2Plot equals operator" << std::endl;
   fSISChannels      = rhs.fSISChannels;
   fTrig             = rhs.fTrig;
   fTrigNobusy       = rhs.fTrigNobusy;
   fAtomOr           = rhs.fAtomOr;
   fCATStart         = rhs.fCATStart;
   fCATStop          = rhs.fCATStop;
   fRCTStart         = rhs.fRCTStart;
   fRCTStop          = rhs.fRCTStop;
   fATMStart         = rhs.fATMStart;
   fATMStop          = rhs.fATMStop;
   fBeamInjection    = rhs.fBeamInjection;
   fBeamEjection     = rhs.fBeamEjection;
   fZMinCut          = rhs.fZMinCut;
   fZMaxCut          = rhs.fZMaxCut;
   SISEvents         = rhs.SISEvents;

   return *this;
}

//Addition
TA2Plot operator+(const TA2Plot& lhs, const TA2Plot& rhs)
{
   //In order to call the parents addition first it is important to statically cast the Plots to their parent class,
   //add the parents together, then initialise a TA2Plot from the TAPlot and fill in the rest of the values as a constructor would.
   TAPlot lhsCast = static_cast<TAPlot>(lhs); //2 Static casts
   TAPlot rhsCast = static_cast<TAPlot>(rhs);
   TAPlot parentSum = lhsCast + rhsCast; //Add as TAPlots
   TA2Plot basePlot = TA2Plot(parentSum); //Initialise a TA2Plot from the now summed TAPlots

   //Now we fill in the (empty) values of this newly initiated TA2Plot with the values we need from the 2 input arguments:
   //For all these copying A is fine.
   basePlot.fTrig.insert(              rhs.fTrig.begin(), rhs.fTrig.end() );
   basePlot.fTrigNobusy.insert(        rhs.fTrigNobusy.begin(), rhs.fTrigNobusy.end() );
   basePlot.fAtomOr.insert(            rhs.fAtomOr.begin(), rhs.fAtomOr.end() );
   basePlot.fCATStart.insert(          rhs.fCATStart.begin(), rhs.fCATStart.end() );
   basePlot.fCATStop.insert(           rhs.fCATStop.begin(), rhs.fCATStop.end() );
   basePlot.fRCTStart.insert(          rhs.fRCTStart.begin(), rhs.fRCTStart.end() );
   basePlot.fRCTStop.insert(           rhs.fRCTStop.begin(), rhs.fRCTStop.end() );
   basePlot.fATMStart.insert(          rhs.fATMStart.begin(), rhs.fATMStart.end() );
   basePlot.fATMStop.insert(           rhs.fATMStop.begin(), rhs.fATMStop.end() );
   basePlot.fBeamInjection.insert(     rhs.fBeamInjection.begin(), rhs.fBeamInjection.end() );
   basePlot.fBeamEjection.insert(      rhs.fBeamEjection.begin(), rhs.fBeamEjection.end() );
   
   basePlot.fZMinCut        = lhs.fZMinCut;
   basePlot.fZMaxCut        = lhs.fZMaxCut;

   //Vectors need concacting.
   basePlot.fSISChannels.insert(basePlot.fSISChannels.end(), rhs.fSISChannels.begin(), rhs.fSISChannels.end() );

   basePlot.SISEvents += lhs.SISEvents;
   basePlot.SISEvents += rhs.SISEvents;
   
   return basePlot;
}

TA2Plot& TA2Plot::operator+=(const TA2Plot& rhs)
{
   //This calls the parent += operator first.
   TAPlot::operator+=(rhs);

   //For all these copying A is fine.
   fTrig.insert(            rhs.fTrig.begin(), rhs.fTrig.end() );
   fTrigNobusy.insert(     rhs.fTrigNobusy.begin(), rhs.fTrigNobusy.end() );
   fAtomOr.insert(         rhs.fAtomOr.begin(), rhs.fAtomOr.end() );
   fCATStart.insert(        rhs.fCATStart.begin(), rhs.fCATStart.end() );
   fCATStop.insert(         rhs.fCATStop.begin(), rhs.fCATStop.end() );
   fRCTStart.insert(        rhs.fRCTStart.begin(), rhs.fRCTStart.end() );
   fRCTStop.insert(         rhs.fRCTStop.begin(), rhs.fRCTStop.end() );
   fATMStart.insert(        rhs.fATMStart.begin(), rhs.fATMStart.end() );
   fATMStop.insert(         rhs.fATMStop.begin(), rhs.fATMStop.end() );
   fBeamInjection.insert(  rhs.fBeamInjection.begin(), rhs.fBeamInjection.end() );
   fBeamEjection.insert(   rhs.fBeamEjection.begin(), rhs.fBeamEjection.end() );

   //Vectors need concacting.
   fSISChannels.insert(fSISChannels.end(), rhs.fSISChannels.begin(), rhs.fSISChannels.end() );

   //Object addition.
   SISEvents += rhs.SISEvents;
   
   return *this;
}


//Setters and getters
void TA2Plot::SetSISChannels(int runNumber)
{
  TSISChannels *sisChannels = new TSISChannels(runNumber);
  fTrig.insert(             std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("IO32_TRIG")));
  fTrigNobusy.insert(       std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("IO32_TRIG_NOBUSY")));
  fAtomOr.insert(           std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_PMT_ATOM_OR")));
  fBeamInjection.insert(    std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_AD")));
  fBeamEjection.insert(     std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_AD_2")));
  fCATStart.insert(         std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_PBAR_DUMP_START")));
  fCATStop.insert(          std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_PBAR_DUMP_STOP")));
  fRCTStart.insert(         std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_RECATCH_DUMP_START")));
  fRCTStop.insert(          std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_RECATCH_DUMP_STOP")));
  fATMStart.insert(         std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_ATOM_DUMP_START")));
  fATMStop.insert(          std::pair<int,int>(runNumber, (int)sisChannels->GetChannel("SIS_ATOM_DUMP_STOP")));

  //Add all valid SIS channels to a list for later:
  fSISChannels.clear();
  if (fTrig.find(runNumber)->second>0)           fSISChannels.push_back(fTrig.find(runNumber)->second);
  if (fTrigNobusy.find(runNumber)->second>0)     fSISChannels.push_back(fTrigNobusy.find(runNumber)->second);
  if (fAtomOr.find(runNumber)->second>0)         fSISChannels.push_back(fAtomOr.find(runNumber)->second);
  if (fBeamInjection.find(runNumber)->second>0)  fSISChannels.push_back(fBeamInjection.find(runNumber)->second);
  if (fBeamEjection.find(runNumber)->second>0)   fSISChannels.push_back(fBeamEjection.find(runNumber)->second);
  if (fCATStart.find(runNumber)->second>0)       fSISChannels.push_back(fCATStart.find(runNumber)->second);
  if (fCATStop.find(runNumber)->second>0)        fSISChannels.push_back(fCATStop.find(runNumber)->second);
  if (fRCTStart.find(runNumber)->second>0)       fSISChannels.push_back(fRCTStart.find(runNumber)->second);
  if (fRCTStop.find(runNumber)->second>0)        fSISChannels.push_back(fRCTStop.find(runNumber)->second);
  if (fATMStart.find(runNumber)->second>0)       fSISChannels.push_back(fATMStart.find(runNumber)->second);
  if (fATMStop.find(runNumber)->second>0)        fSISChannels.push_back(fATMStop.find(runNumber)->second);
  delete sisChannels;
  return;
}

void TA2Plot::AddSVDEvent(TSVD_QOD* SVDEvent)
{
   double time=SVDEvent->t;
   if (SVDEvent->z < fZMinCut) return;
   if (SVDEvent->z > fZMaxCut) return;

   int index = GetTimeWindows()->GetValidWindowNumber(time);
   
   
   //Checks to make sure GetValidWindowNumber hasn't returned -1 (in which case it will be ignored) and 
   //if not it will add the event. 
   if(index >= 0)
   {
      AddEvent(SVDEvent, GetTimeWindows()->fZeroTime.at(index));
   }
}

void TA2Plot::AddSISEvent(TSISEvent* SISEvent)
{
   size_t numSISChannels=fSISChannels.size();
   double time=SISEvent->GetRunTime();

   //Loop over all time windows
   int index = GetTimeWindows()->GetValidWindowNumber(time);
   if(index>=0)
   {
      for (size_t i=0; i<numSISChannels; i++)
      {
         int counts = SISEvent->GetCountsInChannel(fSISChannels[i]);
         if (counts)
         {
            AddEvent(SISEvent, fSISChannels[i], GetTimeWindows()->fZeroTime[index]);
         }
      }
   }
}

void TA2Plot::AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition )
{
   std::vector<TA2Spill> spills=Get_A2_Spills(runNumber, description, repetition);
   return AddDumpGates(runNumber, spills);
}

void TA2Plot::AddDumpGates(int runNumber, std::vector<TA2Spill> spills)
{
   std::vector<double> minTime;
   std::vector<double> maxTime;
   
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         minTime.push_back(spill.ScalerData->StartTime);
         maxTime.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return TA2Plot::AddTimeGates(runNumber, minTime, maxTime);
}

//If spills are from one run, it is faster to call the function above
void TA2Plot::AddDumpGates(std::vector<TA2Spill> spills)
{
   for (TA2Spill& spill: spills)
   {
      if (spill.ScalerData)
      {
         AddTimeGate(spill.RunNumber, spill.GetStartTime(), spill.GetStopTime());
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return;
}


//Load, fill, draw, or save the object  
void TA2Plot::LoadRun(int runNumber, double firstTime, double lastTime)
{
   //Something smarter for the future?
   //TSVDQODIntegrator SVDCounts(TA2RunQOD* q,tmin[0], tmax[0]);
   
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SVDReader = Get_A2_SVD_Tree(runNumber);
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficialTime");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SVDReader->Next())
   {
      double t = SVDEvent->t;
      if (t < firstTime)
         continue;
      if (t > lastTime)
         break;
      AddSVDEvent(&(*SVDEvent));
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   SetSISChannels(runNumber);

   for (int sis_module_no = 0; sis_module_no < NUM_SIS_MODULES; sis_module_no++)
   {
      TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber, sis_module_no);
      TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
      // I assume that file IO is the slowest part of this function... 
      // so get multiple channels and multiple time windows in one pass
      while (SISReader->Next())
      {
         double t = SISEvent->GetRunTime();
         if (t < firstTime)
            continue;
         if (t > lastTime)
            break;
         AddSISEvent(&(*SISEvent));
      }
   }
}

void TA2Plot::SetUpHistograms()
{
   const double XMAX(4.),YMAX(4.),RMAX(4.), ZMAX(30.);

   double minTime;
   double maxTime;
   if (kZeroTimeAxis)
   {
      minTime=GetBiggestTzero();
      maxTime=GetMaxDumpLength() + minTime;
   }
   else
   {
      minTime=GetFirstTmin();
      maxTime=GetLastTmax();
   }

   AddHistogram("zvtx",new TH1D((GetTAPlotTitle() + "_zvtx").c_str(), "Z Vertex;z [cm];events", GetNBins(), -ZMAX, ZMAX));

   TH1D* rHisto = new TH1D((GetTAPlotTitle() + "_rvtx").c_str(), "R Vertex;r [cm];events", GetNBins(), 0., RMAX);
   rHisto->SetMinimum(0);
   AddHistogram("rvtx",rHisto);

   TH1D* phiHisto = new TH1D((GetTAPlotTitle() + "_phivtx").c_str(), "phi Vertex;phi [rad];events", GetNBins(), -TMath::Pi(), TMath::Pi());
   phiHisto->SetMinimum(0);
   AddHistogram("phivtx",phiHisto);

   TH2D* xyHisto = new TH2D((GetTAPlotTitle() + "_xyvtx").c_str(), "X-Y Vertex;x [cm];y [cm]", GetNBins(), -XMAX, XMAX, GetNBins(), -YMAX, YMAX);
   AddHistogram("xyvtx",xyHisto);

   TH2D* zrHisto = new TH2D((GetTAPlotTitle() + "_zrvtx").c_str(), "Z-R Vertex;z [cm];r [cm]", GetNBins(), -ZMAX, ZMAX, GetNBins(), 0., RMAX);
   AddHistogram("zrvtx",zrHisto);

   TH2D* zphiHisto = new TH2D((GetTAPlotTitle() + "_zphivtx").c_str(), "Z-Phi Vertex;z [cm];phi [rad]", GetNBins(), -ZMAX, ZMAX, GetNBins(), -TMath::Pi(), TMath::Pi());
   AddHistogram("zphivtx",phiHisto);
   std::string units;
   if (GetMaxDumpLength()<SCALECUT) 
   {
      fTimeFactor = 1000;
      units = "[ms]";
   }
   else
   {
      fTimeFactor = 1;
      units = "[s]";
   }

   //SIS channels:
   TH1D* triggers = new TH1D((GetTAPlotTitle() + "_tIO32_nobusy").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
   triggers->SetMarkerColor(kRed);
   triggers->SetLineColor(kRed);
   triggers->SetMinimum(0);
   AddHistogram("tIO32_nobusy",triggers);

   TH1D* readTriggers = new TH1D((GetTAPlotTitle() + "_tIO32").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
   readTriggers->SetMarkerColor(kViolet);
   readTriggers->SetLineColor(kViolet);
   readTriggers->SetMinimum(0);
   AddHistogram("tIO32",readTriggers);

   TH1D* atomOr = new TH1D((GetTAPlotTitle() + "_tAtomOR").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
   atomOr->SetMarkerColor(kGreen);
   atomOr->SetLineColor(kGreen);
   atomOr->SetMinimum(0);
   AddHistogram("tAtomOR",atomOr);

   TH1D* tHisto = new TH1D((GetTAPlotTitle() + "_tvtx").c_str(), (std::string("t Vertex;t ") + units + ";events").c_str(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
   tHisto->SetLineColor(kMagenta);
   tHisto->SetMarkerColor(kMagenta);
   tHisto->SetMinimum(0);
   AddHistogram("tvtx",tHisto);

   TH2D* ztHisto = new TH2D((GetTAPlotTitle() + "_ztvtx").c_str(), (std::string("Z-T Vertex;z [cm];t ") + units).c_str(), GetNBins(), -ZMAX, ZMAX, GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
   AddHistogram("ztvtx",ztHisto);
  return;
}

void TA2Plot::FillHisto(bool applyCuts, int mode)
{

   ClearHisto();
   SetUpHistograms();
   const double kMaxDumpLength = GetMaxDumpLength();

   //Fill SIS histograms
   int runNum = 0;
   for (size_t i = 0; i<SISEvents.fTime.size(); i++)
   {
      if (SISEvents.fRunNumber[i] != runNum)
      {
         runNum = SISEvents.fRunNumber[i];
         SetSISChannels(runNum);
      }
      double time;
      if (kZeroTimeAxis)
         time = SISEvents.fTime[i];
      else
         time = SISEvents.fOfficialTime[i];
      if (kMaxDumpLength<SCALECUT) 
         time=time*1000.;
      int channel         = SISEvents.fSISChannel[i];
      int countsInChannel = SISEvents.fCounts[i];
      if (channel == fTrig.find(SISEvents.fRunNumber[i])->second)
         FillHistogram("tIO32",time,countsInChannel);
      else if (channel == fTrigNobusy.find(SISEvents.fRunNumber[i])->second)
         FillHistogram("tIO32_nobusy",time,countsInChannel);
      else if (channel == fAtomOr.find(SISEvents.fRunNumber[i])->second)
         FillHistogram("tAtomOR",time,countsInChannel);
      else if (channel == fBeamInjection.find(SISEvents.fRunNumber[i])->second)
         AddInjection(time);
      else if (channel == fBeamEjection.find(SISEvents.fRunNumber[i])->second)
         AddEjection(time);
      else if (channel == fCATStart.find(SISEvents.fRunNumber[i])->second || channel == fRCTStart.find(SISEvents.fRunNumber[i])->second || channel == fATMStart.find(SISEvents.fRunNumber[i])->second)
         AddStopDumpMarker(time);
      else if (channel == fCATStop.find(SISEvents.fRunNumber[i])->second || channel == fRCTStop.find(SISEvents.fRunNumber[i])->second || channel == fATMStop.find(SISEvents.fRunNumber[i])->second)
         AddStartDumpMarker(time);
      else std::cout <<"Unconfigured SIS channel in TAlhaPlot"<<std::endl;
   }

   //Fill Vertex Histograms
   TVector3 vertex;
   const TVertexEvents* kVertexEvents = GetVertexEvents();
   //for (auto& vtxevent: GetVertexEvents())   
   for (size_t i=0; i<kVertexEvents->fXVertex.size(); i++)
   {
      Double_t time;
      if (kZeroTimeAxis)
         time = kVertexEvents->fTimes[i];
      else
         time = kVertexEvents->fRunTimes[i];
      if (kMaxDumpLength<SCALECUT)
         time=time*1000.;
      vertex = TVector3(kVertexEvents->fXVertex[i], kVertexEvents->fYVertex[i], kVertexEvents->fZVertex[i]);
      FillHistogram("tSVD",time);

      int cutsResult = kVertexEvents->fCutsResults[i];
      if (mode>0)
      {
         if (cutsResult & 1)//Passed cut result!
         {
            FillHistogram("tvtx",time);
         }
         if (cutsResult & 2)
         {
            FillHistogram("tvtx",time);
         }
         else
            continue; //Don't draw vertex if it tails MVA cut
      }
      else
      {
         if (applyCuts)
         {
            if (cutsResult & 1)//Passed cut result!
            {
               FillHistogram("tvtx", time);
            }
            else
               continue;
         }
         else
         {
            if ( kVertexEvents->fVertexStatuses[i] > 0) 
            {
               FillHistogram("tvtx", time);
            }
            else 
               continue;
         }
         if(kVertexEvents->fVertexStatuses[i] <= 0) continue; //Don't draw invaid vertices
      }
      FillHistogram("phivtx",vertex.Phi());
      FillHistogram("zphivtx",vertex.Z(), vertex.Phi());
      //FillHistogram("phitvtx",vtx.Phi(),time);
      FillHistogram("xyvtx",vertex.X(), vertex.Y());
      FillHistogram("zvtx",vertex.Z());
      FillHistogram("rvtx",vertex.Perp());
      FillHistogram("zrvtx",vertex.Z(), vertex.Perp());
      FillHistogram("ztvtx",vertex.Z(), time);
   }
   TH1D* rHisto = GetTH1D("rvtx");
   if (rHisto)
   {
      TH1D *rDensityHisto = (TH1D *)rHisto->Clone("radial density");
      rDensityHisto->Sumw2();
      TF1 *function = new TF1("fr", "x", -100, 100);
      rDensityHisto->Divide(function);
      rDensityHisto->Scale(rHisto->GetBinContent(rHisto->GetMaximumBin()) / rDensityHisto->GetBinContent(rDensityHisto->GetMaximumBin()));
      rDensityHisto->SetMarkerColor(kRed);
      rDensityHisto->SetLineColor(kRed);
      delete function;
      AddHistogram("rdens",rDensityHisto);
   }
}

TCanvas* TA2Plot::DrawCanvas(const char* name, bool applyCuts, int mode)
{
   
   SetTAPlotTitle(name);
   std::cout<<"TAPlot Processing time : ~" << GetApproximateProcessingTime() <<"s"<<std::endl;
   TCanvas *canvas = new TCanvas(name, name, 1800, 1000);
   FillHisto(applyCuts, mode);

   //Scale factor to scale down to ms:
   if (GetMaxDumpLength()<SCALECUT) SetTimeFactor(1000.);
   canvas->Divide(4, 2);

   //Canvas 1
   canvas->cd(1);
   DrawHistogram("zvtx","HIST E1");

   //Canvas 2
   canvas->cd(2); // Z-counts (with electrodes?)4
      TVirtualPad *subPadCD2 = canvas->cd(2);
      gPad->Divide(1, 2);
      //Canvas 2 - Pad 1
      subPadCD2->cd(1);
         DrawHistogram("rvtx","HIST E1");
         DrawHistogram("rdens","HIST E1 SAME");
         TPaveText *radialDensityLabel = new TPaveText(0.6, 0.8, 0.90, 0.85, "NDC NB");
         radialDensityLabel->AddText("radial density [arbs]");
         radialDensityLabel->SetTextColor(kRed);
         radialDensityLabel->SetFillStyle(0);
         radialDensityLabel->SetLineStyle(0);
         radialDensityLabel->Draw();
      //Canvas 2 - Pad 2
      subPadCD2->cd(2);
         DrawHistogram("phivtx","HIST E1");

   //Canvas 3
   canvas->cd(3); // T-counts
   TLegend* legend = NULL;
   DrawHistogram("tIO32_nobusy","HIST");
   legend=AddLegendIntegral(legend,"Triggers: %5.0lf","tIO32_nobusy");
   DrawHistogram("tIO32","HIST SAME");
   legend=AddLegendIntegral(legend,"Reads: %5.0lf","tIO32");
   DrawHistogram("tAtomOR","HIST SAME");
   DrawHistogram("tvtx","HIST SAME");
   if (mode)
      DrawHistogram("tmva","HIST SAME");
   if (mode)
   {
      DrawHistogram("tvtx","HIST SAME");
#if 0
      snprintf(line, 200, "Pass Cuts: %5.0lf", ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")), line, "f");
      snprintf(line, 200, "Pass MVA (rfcut %0.1f): %5.0lf", grfcut, ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->Integral());
      legend->AddEntry((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")), line, "f");
      #endif
   }
   else
   {
      if (GetCutsSettings())
         legend=AddLegendIntegral(legend,"Pass Cuts: %5.0lf","tvtx");
      else
         legend=AddLegendIntegral(legend,"Vertices: %5.0lf","tvtx");
    legend=DrawLines(legend,"tIO32_nobusy");
  }

   //Canvas 4
   canvas->cd(4);
   DrawHistogram("xyvtx","colz");

   //Canvas 5
   canvas->cd(5);
   DrawHistogram("ztvtx","colz");

   //Canvas 6
   canvas->cd(6);
   TVirtualPad *subPadCD6 = NULL;
   if (IsGEMData() && IsLVData())
   {
      subPadCD6 = canvas->cd(6);
      gPad->Divide(1, 2);
   }
   if (subPadCD6) 
      subPadCD6->cd(1);
      std::pair<TLegend*,TMultiGraph*> gemDataMG = GetGEMGraphs();
      if (gemDataMG.first)
      {
         //Draw TMultigraph
         gemDataMG.second->Draw("AL*");
         //Draw legend
         gemDataMG.first->Draw();
      }

      if (subPadCD6) 
         subPadCD6->cd(2);
      std::pair<TLegend*,TMultiGraph*> labviewDataMG=GetLVGraphs();
      if (labviewDataMG.first)
      {
         //Draw TMultigraph
         labviewDataMG.second->Draw("AL*");
         //Draw legend
         labviewDataMG.first->Draw();
      }

   //Canvas 7
   canvas->cd(7);
   DrawHistogram("tvtx","HIST SAME");
#if 0 //Why are these blocks here? They also haven't been regestered as active code when using the F2 rename variable so they are still with old names...
  if (MVAMode) && HISTO_POSITION.count("tvtx"))
  {
    ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative()->Draw("HIST SAME");
    TH1 *h = ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative();
    {
      //Draw line at halfway point
      Double_t Max = h->GetBinContent(h->GetMaximumBin());
      Double_t Tmax = h->GetXaxis()->GetXmax();
      for (Int_t i = 0; i < h->GetMaximumBin(); i++)
      {
        if (h->GetBinContent(i) > Max / 2.)
        {
          TLine *half = new TLine(TMax*tFactor * (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
          half->SetLineColor(kViolet);
          half->Draw();
          break;
        }
      }
    }
  }
  else
  {
     //Draw line at halfway point
     if (HISTO_POSITION.count("tvtx"))     //verticies
     {
        TH1 *h = ((TH1D*)HISTOS.At(HISTO_POSITION.at("tvtx")))->GetCumulative();
        Double_t Max = h->GetBinContent(h->GetMaximumBin());
        Double_t Tmax = h->GetXaxis()->GetXmax();
        for (Int_t i = 0; i < h->GetMaximumBin(); i++)
        {
           if (h->GetBinContent(i) > Max / 2.)
           {
              TLine *half = new TLine(TMax*tFactor * (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
              half->SetLineColor(kBlue);
              half->Draw();
              break;
           }
        }
     }
  }
  #endif

   //Canvas 8
   canvas->cd(8);
   DrawHistogram("zphivtx","colz");
   if (applyCuts)
   {
     canvas->cd(1);
     TPaveText *applyCutsLabel = new TPaveText(0., 0.95, 0.20, 1.0, "NDC NB");
     if (mode>0)
       applyCutsLabel->AddText("RF cut applied");
     else
       applyCutsLabel->AddText("Cuts applied");
     applyCutsLabel->SetTextColor(kRed);
     applyCutsLabel->SetFillColor(kWhite);
     applyCutsLabel->Draw();
   };

   //Canvas 0 - Global
   canvas->cd(0);
   TString runText = "Run(s): ";
   runText += GetListOfRuns();
   runText += " ";
   runText += GetNBins();
   runText += " bins";
   TLatex *runsLabel = new TLatex(0., 0., runText);
   runsLabel->SetTextSize(0.016);
   runsLabel->Draw();
 
   std::cout<<runText<<std::endl;
   return canvas;
}

void TA2Plot::WriteEventList(std::string fileName, bool append)
{
   //TODO LMG - Check if file already exists and if so, append to it. If not, delete the old .list and rewrite.
   //Change .ats to []s for speed. 

   //Initiate an ofstream to write to
   std::ofstream myFile;
   std::string file = fileName + ".list";
   myFile.open(file);
   
   //Assert that the runNumbers and EventNos match up in terms of size.
   assert(fVertexEvents.fRunNumbers.size() == fVertexEvents.fEventNos.size());

   size_t index = 0; //Initialise at index 0
   int currentEventNo = fVertexEvents.fEventNos.at(index); //Set the current run number to be the one at index (0)
   int currentRunNo = SISEvents.fRunNumber.at(index); //Same for event no.
   myFile << currentRunNo << ":" << currentEventNo; //Print an initial statement to file, will look something like "39993:2"
   
   //While index is in range lets do all the checks to decide what to write.
   while(index < fVertexEvents.fRunNumbers.size()-1)
   {
      index++; //Increment index since we're in a while loop not a for.
      if(fVertexEvents.fRunNumbers.at(index)!=currentRunNo)
      {
         //If runNumber has changed:
         myFile << "-" << currentEventNo << std::endl; //1. Close off current range eg: "39993:2-5"
         currentRunNo = fVertexEvents.fRunNumbers.at(index); //Update currentrunNo and EventNo
         currentEventNo = fVertexEvents.fEventNos.at(index);
         myFile << currentRunNo << ":" << currentEventNo; //Print initial line of new run eg: "45000:3"
      }
      else if(fVertexEvents.fEventNos.at(index) == (currentEventNo+1))
      {
         //Else if runNumber is the same but the event is consecutive to the one after (ie 2-3)
         currentEventNo++; //Increment currentEventNo. This is equiv to currentEventNo = VertexEvents.EventNos.at(index) just quicker since we've already checked its consecutive.
      }
      else
      {
         //Else: Ie run number is the samer but the event number is not consecutive:
         myFile << "-" << currentEventNo << std::endl; //Close line, eg: "39993:2-5"
         currentRunNo = fVertexEvents.fRunNumbers.at(index); //Update run and event no.
         currentEventNo = fVertexEvents.fEventNos.at(index);
         myFile << currentRunNo << ":" << currentEventNo; //Start new line eg: "39993:27"
      }
   }
   //Once out of the loop close what we have.
   myFile << "-" << currentEventNo << std::endl;
   //Close the file.
   myFile.close();
} 

void TA2Plot::ExportCSV(std::string filename, bool PassedCutOnly)
{
   //Save Time windows and vertex data
   TAPlot::ExportCSV(filename, PassedCutOnly);
   
   std::string scalerFilename = filename + ".scaler.csv";
   
   std::ofstream sis;
   sis.open(scalerFilename);
   sis << SISEvents.CSVTitleLine();
   for (size_t i=0; i< SISEvents.size(); i++)
      sis << SISEvents.CSVLine(i);
   sis.close();
   std::cout<< scalerFilename << " saved\n";
   
   
}

//Private members
void TA2Plot::AddEvent(TSVD_QOD* event, double timeOffset)
{
   double tMinusOffset = (event->t - timeOffset);
   AddVertexEvent(event->RunNumber, event->VF48NEvent, event->NPassedCuts+event->MVA*2, event->NVertices, 
      event->x, event->y, event->z, tMinusOffset, event->VF48Timestamp, event->t, -1, event->NTracks);
}

void TA2Plot::AddEvent(TSISEvent* event, int channel, double timeOffset)
{
   SISEvents.AddEvent(event->GetRunNumber(), event->GetRunTime() - timeOffset, 
      event->GetRunTime(), event->GetCountsInChannel(channel), channel);
}

#endif
