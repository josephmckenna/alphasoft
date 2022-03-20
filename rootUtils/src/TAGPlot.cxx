#ifdef BUILD_AG

#include "TAGPlot.h"
#define SCALECUT 0.6

ClassImp(TAGPlot)

//Default Constructor
TAGPlot::TAGPlot(bool zeroTime): TAPlot(zeroTime)
{
   fZMinCut=-99999.;
   fZMaxCut= 99999.;
   BarMultiplicityCut = 3;
}

TAGPlot::TAGPlot(double zMin, double zMax, bool zeroTime): TAPlot(zeroTime)
{
   fZMinCut = zMin;
   fZMaxCut = zMax;
   BarMultiplicityCut = 3;
}

TAGPlot::TAGPlot(double zMin, double zMax, int barCut, bool zeroTime): TAPlot(zeroTime)
{
   fZMinCut = zMin;
   fZMaxCut = zMax;
   BarMultiplicityCut = barCut;
}

//Copy ctor.
TAGPlot::TAGPlot(const TAGPlot& object):
   TAPlot(object),
   top(object.top),
   bottom(object.bottom),
   sipmad(object.sipmad),
   sipmcf(object.sipmcf),
   TPC_TRIG(object.TPC_TRIG),
   fCATStart(object.fCATStart),
   fCATStop(object.fCATStop),
   fRCTStart(object.fRCTStart),
   fRCTStop(object.fRCTStop),
   fATMStart(object.fATMStart),
   fATMStop(object.fATMStop),
   fBeamInjection(object.fBeamInjection),
   fBeamEjection(object.fBeamEjection)
{
   BarMultiplicityCut = object.BarMultiplicityCut;
}

//Default Destructor
TAGPlot::~TAGPlot()
{
  Reset();
}

void TAGPlot::Reset() //Destroy all histograms
{
  fEjections.clear();
  fInjections.clear();
  fDumpStarts.clear();
  fDumpStops.clear();
  fChronoChannels.clear();
  fRuns.clear();
  fVertexEvents.Clear();
  fChronoEvents.Clear();
}

TAGPlot& TAGPlot::operator=(const TAGPlot& rhs)
{
   TAPlot::operator=(rhs);
   top = rhs.top;
   bottom = rhs.bottom;
   sipmad = rhs.sipmad;
   sipmcf = rhs.sipmcf;
   TPC_TRIG = rhs.TPC_TRIG;
   fCATStart = rhs.fCATStart;
   fCATStop = rhs.fCATStop;
   fRCTStart = rhs.fRCTStart;
   fRCTStop = rhs.fRCTStop;
   fATMStart = rhs.fATMStart;
   fATMStop = rhs.fATMStop;
   fBeamInjection = rhs.fBeamInjection;
   fBeamEjection = rhs.fBeamEjection;
   return *this;
}



TAGPlot& TAGPlot::operator+=(const TAGPlot& rhs)
{
   //This calls the parent += operator first.
   TAPlot::operator+=(rhs);
   top.insert( rhs.top.begin(), rhs.top.end());
   bottom.insert( rhs.bottom.begin(), rhs.bottom.end());
   sipmad.insert( rhs.sipmad.begin(), rhs.sipmad.end());
   sipmcf.insert( rhs.sipmcf.begin(), rhs.sipmcf.end());
   TPC_TRIG.insert( rhs.TPC_TRIG.begin(), rhs.TPC_TRIG.end());
   fCATStart.insert( rhs.fCATStart.begin(), rhs.fCATStart.end());
   fCATStop.insert( rhs.fCATStop.begin(), rhs.fCATStop.end());
   fRCTStart.insert( rhs.fRCTStart.begin(), rhs.fRCTStart.end());
   fRCTStop.insert( rhs.fRCTStop.begin(), rhs.fRCTStop.end());
   fATMStart.insert( rhs.fATMStart.begin(), rhs.fATMStart.end());
   fATMStop.insert( rhs.fATMStop.begin(), rhs.fATMStop.end());
   fBeamInjection.insert( rhs.fBeamInjection.begin(), rhs.fBeamInjection.end());
   fBeamEjection.insert( rhs.fBeamEjection.begin(), rhs.fBeamEjection.end());
   return *this;
}



TAGPlot& operator+(const TAGPlot& lhs, const TAGPlot& rhs)
{
   TAGPlot* basePlot = new TAGPlot;
   *basePlot += lhs;
   *basePlot += rhs;
   return *basePlot;
}


void TAGPlot::SetChronoChannels(Int_t runNumber)
{
   top.insert(           std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "SiPM_B")));
   bottom.insert(        std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "SiPM_E")));
   sipmad.insert(        std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "SiPM_A_OR_D")));
   sipmcf.insert(        std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "SiPM_C_OR_F")));
   TPC_TRIG.insert(      std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "ADC_TRG")));
   fBeamInjection.insert(std::pair<int,TChronoChannel>(runNumber, Get_Chrono_Channel( runNumber, "AD_TRIG")));

  //Add all valid SIS channels to a list for later:
  if (top.find(runNumber)->second.IsValidChannel())             fChronoChannels.push_back(top.find(runNumber)->second);
  if (bottom.find(runNumber)->second.IsValidChannel())          fChronoChannels.push_back(bottom.find(runNumber)->second);
  if (sipmad.find(runNumber)->second.IsValidChannel())          fChronoChannels.push_back(sipmad.find(runNumber)->second);
  if (sipmcf.find(runNumber)->second.IsValidChannel())          fChronoChannels.push_back(sipmcf.find(runNumber)->second);
  if (TPC_TRIG.find(runNumber)->second.IsValidChannel())        fChronoChannels.push_back(TPC_TRIG.find(runNumber)->second);
  if (fBeamInjection.find(runNumber)->second.IsValidChannel())  fChronoChannels.push_back(fBeamInjection.find(runNumber)->second);

   //std::cout <<"Top:"<<top<<std::endl;
   //std::cout <<"Bottom:"<<bottom<<std::endl;
   //std::cout <<"TPC_TRIG:"<<TPC_TRIG<<std::endl;
   return;
}

void TAGPlot::AddAGDetectorEvent(const TAGDetectorEvent& event)
{
  double time = event.fRunTime;
  const double z = event.fVertex.Z();
  if ( z < fZMinCut ) return;
  if ( z > fZMaxCut ) return;

  int index = GetTimeWindows()->GetValidWindowNumber(time);

  if (index >= 0)
  {
    AddEvent(event, GetTimeWindows()->fZeroTime.at(index));
  }
}


void TAGPlot::AddChronoEvent(const TCbFIFOEvent& event, const std::string& board)
{
  const size_t numChronoChannels = fChronoChannels.size();
  const double time = event.GetRunTime();
  
  //Loop over all time windows
  const int index = GetTimeWindows()->GetValidWindowNumber(time);
  if(index >= 0)
  {
     for (const TChronoChannel& c: fChronoChannels)
     {
        if (event == c)
        {
          AddEvent(event, c, GetTimeWindows()->fZeroTime[index]);
        }
     }
  }
}

void TAGPlot::AddEvent(const TAGDetectorEvent& event, const double timeOffset)
{
  const double tMinusOffset = (event.fRunTime - timeOffset);
   AddVertexEvent(
     event.fRunNumber,
     event.fEventNo,
     event.fCutsResult,
     event.fVertexStatus,
     event.fVertex.X(),
     event.fVertex.Y(),
     event.fVertex.Z(),
     tMinusOffset,
     event.fRunTime,
     event.fTPCTime,
     event.fNumHelices,
     event.fNumTracks
   );
  return;

}

void TAGPlot::AddEvent(const TCbFIFOEvent& event, const TChronoChannel& channel, const double StartOffset)
{
   if (!event.IsLeadingEdge())
      return;
   fChronoEvents.AddEvent(
     event.GetRunNumber(),
     event.GetRunTime(),
     event.GetRunTime(),
     event.fCounts,
     channel);
}

//Maybe dont have this function... lets see...
void TAGPlot::AddDumpGates(const int runNumber, const std::vector<std::string> description, const std::vector<int> dumpIndex)
{
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber,{description},{dumpIndex});
   return AddDumpGates(runNumber, spills);
}

void TAGPlot::AddDumpGates(const int runNumber, const std::vector<TAGSpill> spills)
{
   std::vector<double> minTime;
   std::vector<double> maxTime;
   
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         minTime.push_back(spill.ScalerData->fStartTime);
         maxTime.push_back(spill.ScalerData->fStopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return AddTimeGates(runNumber, minTime, maxTime);
}

//If spills are from one run, it is faster to call the function above
void TAGPlot::AddDumpGates(const std::vector<TAGSpill> spills)
{
   for (const TAGSpill& spill: spills)
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

void TAGPlot::LoadRun(const int runNumber, const double firstTime, const double lastTime)
{
   TTreeReader* TPCTreeReader = Get_AGDetectorEvent_TreeReader(runNumber);
   TTreeReaderValue<TAGDetectorEvent> VertexEvent(*TPCTreeReader, "TADetectorEvent");
   while (TPCTreeReader->Next())
   {
      const double t =VertexEvent->fRunTime;
      if (t < firstTime)
         continue;
      if (t > lastTime)
         break;
      AddAGDetectorEvent(*VertexEvent);
   }
   LoadChronoEvents(runNumber,firstTime,lastTime);
   return;
}

void TAGPlot::LoadChronoEvents(const int runNumber, const double firstTime, const double lastTime)
{
   SetChronoChannels(runNumber);
   for (UInt_t j=0; j<fChronoChannels.size(); j++)
   {
      std::cout << "j:"<< j<<std::endl;
      TTreeReader* ChronoReader = Get_Chrono_TreeReader(runNumber, fChronoChannels[j].GetBranchName());
      TTreeReaderValue<TCbFIFOEvent> ChronoEvent(*ChronoReader, "FIFOData");
      while (ChronoReader->Next())
      {
        const double t = ChronoEvent->GetRunTime();
        if (t < firstTime)
            continue;
         if (t > lastTime)
            break;
         AddChronoEvent(*ChronoEvent,fChronoChannels[j].GetBoard());
      }
   }
}

void TAGPlot::SetUpHistograms()
{
   const double XMAX(100.),YMAX(100.),RMAX(100.), ZMAX(1300.);

   double minTime;
   double maxTime;
   if (kZeroTimeAxis)
   {
      minTime = GetBiggestTzero();
      maxTime = GetMaxDumpLength() + minTime;
   }
   else
   {
      minTime = GetFirstTmin();
      maxTime = GetLastTmax();
   }
   std::string units;
   if (GetMaxDumpLength() < SCALECUT) 
   {
      fTimeFactor = 1000;
      units = "[ms]";
   }
   else
   {
      fTimeFactor = 1;
      units = "[s]";
   }
   TH1D* top=new TH1D((GetTAPlotTitle() + "top_pm").c_str(), (std::string("t;t ")+ units + ";events").c_str(), GetNBins(), minTime, maxTime);
   top->SetLineColor(kGreen);
   top->SetMarkerColor(kGreen);
   top->SetMinimum(0);
   AddHistogram("top_pm", top); 

  
   TH1D* bot=new TH1D((GetTAPlotTitle() +"bot_pm").c_str(), (std::string("t;t ")+ units + ";events").c_str(), GetNBins(), minTime, maxTime);
   bot->SetLineColor(kAzure - 8);
   bot->SetMarkerColor(kAzure - 8);
   bot->SetMinimum(0);
   AddHistogram("bot_pm",bot);

   TH1D* aandd=new TH1D((GetTAPlotTitle() +"aandd_pm").c_str(), (std::string("t;t ")+ units + ";events").c_str(), GetNBins(), minTime, maxTime);
   aandd->SetLineColor(kGreen);
   aandd->SetMarkerColor(kGreen);
   aandd->SetMinimum(0);
   AddHistogram("aandd_pm", aandd);
  
   TH1D* candf=new TH1D((GetTAPlotTitle() +"candf_pm").c_str(), (std::string("t;t ")+ units + ";events").c_str(), GetNBins(), minTime, maxTime);
   candf->SetLineColor(kAzure - 8);
   candf->SetMarkerColor(kAzure - 8);
   candf->SetMinimum(0);
   AddHistogram("candf_pm",candf);

   TH1D* TPC=new TH1D((GetTAPlotTitle() +"TPC_TRIG").c_str(), (std::string("t;t ")+ units + ";events").c_str(), GetNBins(), minTime, maxTime);
   TPC->SetMarkerColor(kRed);
   TPC->SetLineColor(kRed);
   TPC->SetMinimum(0);
   AddHistogram("TPC_TRIG",TPC);

   AddHistogram("zvtx",new TH1D((GetTAPlotTitle() +"zvtx").c_str(), "Z Vertex;z [mm];events", GetNBins(), -ZMAX, ZMAX));

   TH1D* hr = new TH1D((GetTAPlotTitle() +"rvtx").c_str(), "R Vertex;r [mm];events", GetNBins(), 0., RMAX);
   hr->SetMinimum(0);
   AddHistogram("rvtx",hr);

   TH1D* hphi = new TH1D((GetTAPlotTitle() +"phivtx").c_str(), "phi Vertex;phi [rad];events", GetNBins(), -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   AddHistogram("phivtx",hphi);

   TH2D* hxy = new TH2D((GetTAPlotTitle() +"xyvtx").c_str(), "X-Y Vertex;x [mm];y [mm]", GetNBins(), -XMAX, XMAX, GetNBins(), -YMAX, YMAX);
   AddHistogram("xyvtx",hxy);

   TH2D* hzr = new TH2D((GetTAPlotTitle() +"zrvtx").c_str(), "Z-R Vertex;z [mm];r [mm]", GetNBins(), -ZMAX, ZMAX, GetNBins(), 0., RMAX);
   AddHistogram("zrvtx",hzr);

   TH2D* hzphi = new TH2D((GetTAPlotTitle() +"zphivtx").c_str(), "Z-Phi Vertex;z [mm];phi [rad]", GetNBins(), -ZMAX, ZMAX, GetNBins(), -TMath::Pi(), TMath::Pi());
   AddHistogram("zphivtx",hzphi);



  TH1D* ht = new TH1D((GetTAPlotTitle() +"tvtx").c_str(), (std::string("t Vertex;t ") + units + ";events").c_str(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
      ht->SetLineColor(kMagenta);
      ht->SetMarkerColor(kMagenta);
      ht->SetMinimum(0);
      fHistos.Add(ht);
      fHistoPositions["tvtx"]=fHistos.GetEntries()-1;

      TString BarTitle="t bars>";
      BarTitle+=BarMultiplicityCut;
      BarTitle+=";t ";
      BarTitle+=units;
      BarTitle+=";events";
      TH1D* htbar = new TH1D((GetTAPlotTitle() +"tbar").c_str(), BarTitle.Data(), GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
      htbar->SetMinimum(0);
      AddHistogram("tbar",htbar);

      TH2D* hzt = new TH2D((GetTAPlotTitle() +"ztvtx").c_str(), (std::string("Z-T Vertex;z [mm];t ") + units).c_str() , GetNBins(), -ZMAX, ZMAX, GetNBins(), minTime*fTimeFactor, maxTime*fTimeFactor);
       AddHistogram("ztvtx",hzt);

      TH2D* hphit = new TH2D((GetTAPlotTitle() +"phitvtx").c_str(), (std::string("Phi-T Vertex;phi [rad];t ") + units).c_str() , GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),minTime*fTimeFactor, maxTime*fTimeFactor);
      AddHistogram("phitvtx",hphit);

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [ms];Counts", GetNBins(), TMin*1000., TMax*1000.);
  return;
}

void SetUpBarHistos()
{
   //Get some cool looking bar plots going!
}


void TAGPlot::FillHisto(bool applyCuts, int mode)
{
  
   ClearHisto();
   SetUpHistograms();
   
   FillChronoHistograms();
   //Fill Vertex Histograms
   FillVertexHistograms(applyCuts, mode);
   
   //FillTrackHisto();
}

void TAGPlot::FillChronoHistograms()
{
   const double kMaxDumpLength = GetMaxDumpLength();
   int runNum = 0;
   // Fill Chronobox events
   for (size_t i = 0; i<fChronoEvents.fTime.size(); i++)
   {
      if (fChronoEvents.fRunNumber[i] != runNum)
      {
         runNum = fChronoEvents.fRunNumber[i];
         SetChronoChannels(runNum);
      }
      double time;
      if (kZeroTimeAxis)
         time = fChronoEvents.fTime[i];
      else
         time = fChronoEvents.fOfficialTime[i];
      if (kMaxDumpLength<SCALECUT) 
         time=time*1000.;

      const TChronoChannel& channel = fChronoEvents.fChronoChannel[i];
      const int CountsInChannel = fChronoEvents.fCounts[i];
 
      if (channel == top.find(fChronoEvents.fRunNumber[i])->second)
         FillHistogram("top_pm",time,CountsInChannel);
      else if (channel == bottom.find(fChronoEvents.fRunNumber[i])->second)
         FillHistogram("bot_pm",time,CountsInChannel);
      else if (channel == sipmad.find(fChronoEvents.fRunNumber[i])->second)
         FillHistogram("aandd_pm",time,CountsInChannel);
      else if (channel == sipmcf.find(fChronoEvents.fRunNumber[i])->second)
         FillHistogram("candf_pm",time,CountsInChannel);
      else if (channel == TPC_TRIG.find(fChronoEvents.fRunNumber[i])->second)
         FillHistogram("TPC_TRIG",time,CountsInChannel);
      else std::cout <<"Unconfigured Chrono channel in TAGPlot"<<std::endl;
   }
}

void TAGPlot::FillVertexHistograms(bool applyCuts,int mode)
{
   TVector3 vertex;
   const size_t NVertexEvents = fVertexEvents.size();
   for (size_t i = 0; i < NVertexEvents; i++)
   {
      Double_t time;
      if (kZeroTimeAxis)
         time = fVertexEvents.fTimes[i];
      else
         time = fVertexEvents.fRunTimes[i];
      if (fMaxDumpLength < SCALECUT)
         time = time*1000.;

      vertex = TVector3(fVertexEvents.fXVertex[i], fVertexEvents.fYVertex[i], fVertexEvents.fZVertex[i]);

      FillHistogram("tTPC",time);
      
      //if (fVertexEvents.NBars[i] > BarMultiplicityCut)
      //   FillHistogram("tbar",time);
      const int cutsResult = fVertexEvents.fCutsResults[i];
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
            if ( fVertexEvents.fVertexStatuses[i] > 0) 
            {
               FillHistogram("tvtx", time);
            }
            else 
               continue;
         }
         if(fVertexEvents.fVertexStatuses[i] <= 0) continue; //Don't draw invaid vertices
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


TCanvas *TAGPlot::DrawVertexCanvas(const char* name, bool applyCuts, int mode)
{
   SetTAPlotTitle(name);
   std::cout<<"TAGPlot Processing time : ~" << GetApproximateProcessingTime() <<"s"<<std::endl;
   TCanvas *canvas = new TCanvas(name, name, 1800, 1000);
   std::cout <<"Filling histograms"<<std::endl;
   FillHisto(applyCuts,mode);
   std::cout <<"Filling histograms done"<<std::endl;
   //Scale factor to scale down to ms:
   if (GetMaxDumpLength()<SCALECUT) SetTimeFactor(1000.);
   canvas->Divide(4, 2);

   if (fLegendDetail >= 1)
   {
      gStyle->SetOptStat(11111);
   }
   else
   {
      gStyle->SetOptStat("ni");	//just like the knights of the same name
   }

   //Canvas 1
   canvas->cd(1);
   DrawHistogram("zvtx","HIST E1");

   //Canvas 2
   canvas->cd(2); // Z-counts (with electrodes?)4
   TVirtualPad *cVTX_1 = canvas->cd(2);
   gPad->Divide(1, 2);
   //Canvas 2 - Pad 1
      cVTX_1->cd(1);
         DrawHistogram("rvtx","HIST E1");
         DrawHistogram("rdens","HIST E1 SAME");
         TPaveText *radialDensityLabel = new TPaveText(0.6, 0.8, 0.90, 0.85, "NDC NB");
         radialDensityLabel->AddText("radial density [arbs]");
         radialDensityLabel->SetTextColor(kRed);
         radialDensityLabel->SetFillStyle(0);
         radialDensityLabel->SetLineStyle(0);
         radialDensityLabel->Draw();
      //Canvas 2 - Pad 2
      cVTX_1->cd(2);
         DrawHistogram("phivtx","HIST E1");

   //Canvas 3
   canvas->cd(3); // T-counts

   TLegend* legend = NULL;
   DrawHistogram("TPC_TRIG","HIST");
   legend=AddLegendIntegral(legend,"Triggers: %5.0lf","tTPC_TRIG");
   DrawHistogram("tvtx","HIST SAME");
   legend=AddLegendIntegral(legend,"Verts: %5.0lf","ttvtx");
   DrawHistogram("tbar","HIST SAME");
   legend=AddLegendIntegral(legend,"Reads: %5.0lf","ttbar");
   DrawHistogram("tTPC","HIST SAME");
   legend=AddLegendIntegral(legend,"Reads: %5.0lf","tTPC");
   DrawHistogram("top_pm","HIST SAME");
   legend=AddLegendIntegral(legend,"Trigs: %5.0lf","ttop_pm");
   DrawHistogram("bot_pm","HIST SAME");
   legend=AddLegendIntegral(legend,"Trigs: %5.0lf","tbot_pm");
   if (mode)
   {
      DrawHistogram("tmva","HIST SAME");
      legend=AddLegendIntegral(legend,"Pass MVA: %5.0lf","tPassMVA");
   }
   if (mode)
   {
      DrawHistogram("tvtx","HIST SAME");
      if (applyCuts)
         legend=AddLegendIntegral(legend,"Pass Cuts: %5.0lf","tvtx");
      else
         legend=AddLegendIntegral(legend,"Vertices: %5.0lf","tvtx");
      legend=DrawLines(legend,"tTPC");
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
      std::pair<TLegend*,TMultiGraph*> labviewDataMG = GetLVGraphs();
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
   }

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


void TAGPlot::ExportCSV(const std::string filename, Bool_t PassedCutOnly)
{
   //Save Time windows and vertex data
   TAPlot::ExportCSV(filename, PassedCutOnly);
 
   std::string scalerFilename = filename + ".scaler.csv";
   std::ofstream chrono;
   chrono.open(scalerFilename);
   chrono << fChronoEvents.CSVTitleLine();
   for (size_t i=0; i< fChronoEvents.size(); i++)
      chrono << fChronoEvents.CSVLine(i);
   chrono.close();
   std::cout<< scalerFilename << " saved\n";
}

#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
