#include "TA2Plot.h"


#ifdef BUILD_A2
ClassImp(TA2Plot);

TA2Plot::TA2Plot(): TAPlot()
{
   ZMinCut=-99999.;
   ZMaxCut= 99999.;
}
TA2Plot::TA2Plot(double zmin, double zmax): TAPlot()
{
   ZMinCut = zmin;
   ZMaxCut = zmax;
}
TA2Plot::~TA2Plot()
{
}
void TA2Plot::SetSISChannels(int runNumber)
{

  TSISChannels *sisch = new TSISChannels(runNumber);
  trig =           sisch->GetChannel("IO32_TRIG");
  trig_nobusy =    sisch->GetChannel("IO32_TRIG_NOBUSY");
  atom_or =        sisch->GetChannel("SIS_PMT_ATOM_OR");
  Beam_Injection = sisch->GetChannel("SIS_AD");
  Beam_Ejection =  sisch->GetChannel("SIS_AD_2");
  CATStart =       sisch->GetChannel("SIS_PBAR_DUMP_START");
  CATStop =        sisch->GetChannel("SIS_PBAR_DUMP_STOP");
  RCTStart =       sisch->GetChannel("SIS_RECATCH_DUMP_START");
  RCTStop =        sisch->GetChannel("SIS_RECATCH_DUMP_STOP");
  ATMStart =       sisch->GetChannel("SIS_ATOM_DUMP_START");
  ATMStop =        sisch->GetChannel("SIS_ATOM_DUMP_STOP");


  //Add all valid SIS channels to a list for later:
  SISChannels.clear();
  if (trig>0)           SISChannels.push_back(trig);
  if (trig_nobusy>0)    SISChannels.push_back(trig_nobusy);
  if (atom_or>0)        SISChannels.push_back(atom_or);
  if (Beam_Injection>0) SISChannels.push_back(Beam_Injection);
  if (Beam_Ejection>0)  SISChannels.push_back(Beam_Ejection);
  if (CATStart>0)       SISChannels.push_back(CATStart);
  if (CATStop>0)        SISChannels.push_back(CATStop);
  if (RCTStart>0)       SISChannels.push_back(RCTStart);
  if (RCTStop>0)        SISChannels.push_back(RCTStop);
  if (ATMStart>0)       SISChannels.push_back(ATMStart);
  if (ATMStop>0)        SISChannels.push_back(ATMStop);
  //cout <<"Trig:"<<trig<<endl;
  //cout <<"TrigNoBusy:"<<trig_nobusy<<endl;
  //cout <<"Beam Injection:"<<Beam_Injection<<endl;
  //cout <<"Beam Ejection:"<<Beam_Ejection<<endl;
  delete sisch;
  return;
}

void TA2Plot::AddEvent(TSVD_QOD* event, double time_offset)
{
   VertexEvent Event;
   Event.runNumber    =event->RunNumber;
   Event.EventNo      =event->VF48NEvent;
   // Encode Passed cuts and online MVA (CutsResult&1 is passed cuts, 
   // CutsResult&2 is online MVA)
   Event.CutsResult   =event->NPassedCuts;
   Event.CutsResult  +=event->MVA*2;
   Event.VertexStatus =event->NVertices;
   Event.x            =event->x;
   Event.y            =event->y;
   Event.z            =event->z;
   Event.t            =event->t-time_offset; //Plot time (based off offical time)
   Event.EventTime    =event->VF48Timestamp; //TPC time stamp
   Event.RunTime      =event->t; //Official Time
   Event.nHelices     =-1; // helices used for vertexing
   Event.nTracks      =event->NTracks; // reconstructed (good) helices
   AddVertexEvent(Event);

}

void TA2Plot::AddEvent(TSISEvent* event, int channel, double time_offset)
{
   SISPlotEvent Event;
   Event.runNumber    =event->GetRunNumber(); // I don't get set yet...
   //int clock
   Event.t            =event->GetRunTime()-time_offset; //Plot time (based off offical time)
   Event.OfficialTime =event->GetRunTime();
   Event.Counts       =event->GetCountsInChannel(channel);
   Event.SIS_Channel  =channel;

   SISEvents.push_back(Event);
}

void TA2Plot::AddSVDEvent(TSVD_QOD* SVDEvent)
{
   double t=SVDEvent->t;
   if (SVDEvent->z < ZMinCut) return;
   if (SVDEvent->z > ZMaxCut) return;

   //Loop over all time windows
   for (auto& window: GetTimeWindows())
   {
      //If inside the time window
      if ( (t>window.tmin && t< window.tmax) ||
      //Or if after tmin and tmax is invalid (-1)
           (t>window.tmin && window.tmax<0) )
      {
         AddEvent(SVDEvent,window.tmin);
         //This event has been written to the array... so I dont need
         //to check the other winodws... break! Move to next SISEvent
         break;
      }
   }
}

void TA2Plot::AddSISEvent(TSISEvent* SISEvent)
{
   int n_sis=SISChannels.size();
   double t=SISEvent->GetRunTime();
   //Loop over all time windows
   for (auto& window: GetTimeWindows())
   {
      //If inside the time window
      if ( (t>window.tmin && t< window.tmax) ||
      //Or if after tmin and tmax is invalid (-1)
           (t>window.tmin && window.tmax<0) )
      {
         for (int i=0; i<n_sis; i++)
         {
            int counts=SISEvent->GetCountsInChannel(SISChannels[i]);
            if (counts)
            {
               AddEvent(SISEvent,SISChannels[i],window.tmin);
            }
         }
         //This event has been written to the array... so I dont need
         //to check the other winodws... break! Move to next SISEvent
         break;
      }
   }
}

void TA2Plot::LoadRun(int runNumber, double first_time, double last_time)
{
   //Something smarter for the future?
   //TSVDQODIntegrator SVDCounts(TA2RunQOD* q,tmin[0], tmax[0]);
   
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SVDReader=Get_A2_SVD_Tree(runNumber);
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficalTime");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SVDReader->Next())
   {
      double t = SVDEvent->t;
      if (t < first_time)
         continue;
      if (t > last_time)
         break;
      AddSVDEvent(&(*SVDEvent));
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   SetSISChannels(runNumber);
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SISReader->Next())
   {
      double t = SISEvent->GetRunTime();
      if (t < first_time)
         continue;
      if (t > last_time)
         break;
      AddSISEvent(&(*SISEvent));
   }
}

void TA2Plot::AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition )
{
   std::vector<TA2Spill> spills=Get_A2_Spills(runNumber,description,repetition);
   return AddDumpGates(runNumber, spills );
}

void TA2Plot::AddDumpGates(int runNumber, std::vector<TA2Spill> spills )
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         tmin.push_back(spill.ScalerData->StartTime);
         tmax.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return AddTimeGates(runNumber,tmin,tmax);
}

//If spills are from one run, it is faster to call the function above
void TA2Plot::AddDumpGates(std::vector<TA2Spill> spills )
{
   for (TA2Spill& spill: spills)
   {
      if (spill.ScalerData)
      {
         AddTimeGate(spill.RunNumber,spill.GetStartTime(),spill.GetStopTime());
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return;
}

void TA2Plot::SetUpHistograms(bool zeroTime)
{
   const double XMAX(4.),YMAX(4.),RMAX(4.), ZMAX(30.);

   double TMin;
   double TMax;
   if (zeroTime)
   {
      TMin=0;
      TMax=GetMaxDumpLength();
   }
   else
   {
      TMin=GetFirstTmin();
      TMax=GetLastTmax();
   }

   //SIS channels:
   TH1D* triggers=new TH1D("tIO32_nobusy", "t;t [s];events", GetNBins(), TMin, TMax);
   triggers->SetMarkerColor(kRed);
   triggers->SetLineColor(kRed);
   triggers->SetMinimum(0);
   AddHistogram("tIO32_nobusy",triggers);
   
   TH1D* read_triggers=new TH1D("tIO32", "t;t [s];events", GetNBins(), TMin, TMax);
   read_triggers->SetMarkerColor(kViolet);
   read_triggers->SetLineColor(kViolet);
   read_triggers->SetMinimum(0);
   AddHistogram("tIO32",read_triggers);
   
   
   TH1D* atom_or=new TH1D("tAtomOR", "t;t [s];events", GetNBins(), TMin, TMax);
   atom_or->SetMarkerColor(kGreen);
   atom_or->SetLineColor(kGreen);
   atom_or->SetMinimum(0);
   AddHistogram("tAtomOR",atom_or);

   //feGEM plots
   for (int i=0; i<feGEM.size(); i++)
   {
      std::cout<<"Adding feGEM data:"<<feGEM[i].GetTitle().c_str() << " ("<<feGEM[i].GetName().c_str()<<")"<<std::endl;
      //TH1D* GEM_Plot = new TH1D(feGEM[i].GetTitle().c_str(),"t;t [s]; unknown units", GetNBins(), TMin, TMax);
      //std::pair<double,double> minmax = feGEM[i].GetMinMax();
      //std::cout<<"Range: {"<<minmax.first<<","<<minmax.second<<"}"<<std::endl;
      //GEM_Plot->SetMinimum(minmax.first);
      //GEM_Plot->SetMaximum(minmax.second);
      //AddHistogram(feGEM[i].GetName().c_str(),GEM_Plot);
   }

   AddHistogram("zvtx",new TH1D("zvtx", "Z Vertex;z [cm];events", GetNBins(), -ZMAX, ZMAX));

   TH1D* hr = new TH1D("rvtx", "R Vertex;r [cm];events", GetNBins(), 0., RMAX);
   hr->SetMinimum(0);
   AddHistogram("rvtx",hr);

   TH1D* hphi = new TH1D("phivtx", "phi Vertex;phi [rad];events", GetNBins(), -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   AddHistogram("phivtx",hphi);

   TH2D* hxy = new TH2D("xyvtx", "X-Y Vertex;x [cm];y [cm]", GetNBins(), -XMAX, XMAX, GetNBins(), -YMAX, YMAX);
   AddHistogram("xyvtx",hxy);

   TH2D* hzr = new TH2D("zrvtx", "Z-R Vertex;z [cm];r [cm]", GetNBins(), -ZMAX, ZMAX, GetNBins(), 0., RMAX);
   AddHistogram("zrvtx",hzr);

   TH2D* hzphi = new TH2D("zphivtx", "Z-Phi Vertex;z [cm];phi [rad]", GetNBins(), -ZMAX, ZMAX, GetNBins(), -TMath::Pi(), TMath::Pi());
   AddHistogram("zphivtx",hzphi);

   if (GetMaxDumpLength()<SCALECUT) 
   {
      TH1D* ht = new TH1D("tvtx", "t Vertex;t [ms];events", GetNBins(), TMin*1000., TMax*1000.);
      ht->SetLineColor(kMagenta);
      ht->SetMarkerColor(kMagenta);
      ht->SetMinimum(0);
      AddHistogram("tvtx",ht);

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [cm];t [ms]", GetNBins(), -ZMAX, ZMAX, GetNBins(), TMin*1000., TMax*1000.);
      AddHistogram("ztvtx",hzt);

      //TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),TMin*1000., TMax*1000);
      //AddHistogram("phitvtx",hphit);

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [ms];Counts", Nbin, TMin*1000., TMax*1000.);
   }
   else
   {
      TH1D* ht = new TH1D("tvtx", "t Vertex;t [s];events", GetNBins(), TMin, TMax); 
      ht->SetLineColor(kMagenta);
      ht->SetMarkerColor(kMagenta);
      ht->SetMinimum(0);
      AddHistogram("tvtx",ht);

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [cm];t [s]", GetNBins(), -ZMAX, ZMAX, GetNBins(), TMin, TMax);
      AddHistogram("ztvtx",hzt);

      //TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),TMin, TMax);
      //AddHistogram("phitvtx",hphit);

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [s];Counts", GetNBins(), TMin, TMax);
  }
  return;
}

void TA2Plot::FillHisto(bool ApplyCuts, int MVAMode)
{

   ClearHisto();
   SetUpHistograms();
   //FillfeGEMHistograms();
   const double max_dump_length=GetMaxDumpLength();
   //Fill SIS histograms
   //for (UInt_t i=0; i<ChronoPlotEvents.size(); i++)
   int runno=0;
   for (auto& sisevent: SISEvents)
   {
      //This is a new run number... SIS channels could have changed! update!
      if (sisevent.runNumber!=runno)
      {
         runno=sisevent.runNumber;
         SetSISChannels(runno);
      }
      double time = sisevent.t;
      if (max_dump_length<SCALECUT) 
         time=time*1000.;
      int Channel         = sisevent.SIS_Channel;
      int CountsInChannel = sisevent.Counts;
 
      if (Channel == trig)
         FillHistogram("tIO32",time,CountsInChannel);
      else if (Channel == trig_nobusy)
         FillHistogram("tIO32_nobusy",time,CountsInChannel);
      else if (Channel == atom_or)
         FillHistogram("tAtomOR",time,CountsInChannel);
      else if (Channel == Beam_Injection)
         AddInjection(time);
      else if (Channel == Beam_Ejection)
         AddEjection(time);
      else if (Channel == CATStart || Channel == RCTStart || Channel == ATMStart)
         AddStopDumpMarker(time);
      else if (Channel == CATStop || Channel == RCTStop || Channel == ATMStop)
         AddStartDumpMarker(time);
      else std::cout <<"Unconfigured SIS channel in TAlhaPlot"<<std::endl;
   }

   //Fill Vertex Histograms

   TVector3 vtx;
   for (auto& vtxevent: GetVertexEvents())
   {
      Double_t time = vtxevent.t;
      if (max_dump_length<SCALECUT)
         time=time*1000.;
      vtx=TVector3(vtxevent.x,vtxevent.y,vtxevent.z);
      FillHistogram("tSVD",time);

      int CutsResult=vtxevent.CutsResult;
      if (MVAMode>0)
      {
         if (CutsResult & 1)//Passed cut result!
         {
            FillHistogram("tvtx",time);
         }
         if (CutsResult & 2)
         {
            FillHistogram("tvtx",time);
         }
         else
            continue; //Don't draw vertex if it tails MVA cut
      }
      else
      {
         if (ApplyCuts)
         {
            if (CutsResult & 1)//Passed cut result!
            {
               FillHistogram("tvtx",time);
            }
            else
               continue;
         }
         else
         {
            if ( vtxevent.VertexStatus > 0) 
            {
               FillHistogram("tvtx",time);
            }
            else 
               continue;
         }
         if (vtxevent.VertexStatus <= 0) continue; //Don't draw invaid vertices
      }
      FillHistogram("phivtx",vtx.Phi());
      FillHistogram("zphivtx",vtx.Z(), vtx.Phi());
      //FillHistogram("phitvtx",vtx.Phi(),time);
      FillHistogram("xyvtx",vtx.X(), vtx.Y());
      FillHistogram("zvtx",vtx.Z());
      FillHistogram("rvtx",vtx.Perp());
      FillHistogram("zrvtx",vtx.Z(), vtx.Perp());
      FillHistogram("ztvtx",vtx.Z(), time);
   }
   TH1D* hr=GetTH1D("rvtx");
   if (hr)
   {
      TH1D *hrdens = (TH1D *)hr->Clone("radial density");
      hrdens->Sumw2();
      TF1 *fr = new TF1("fr", "x", -100, 100);
      hrdens->Divide(fr);
      hrdens->Scale(hr->GetBinContent(hr->GetMaximumBin()) / hrdens->GetBinContent(hrdens->GetMaximumBin()));
      hrdens->SetMarkerColor(kRed);
      hrdens->SetLineColor(kRed);
      delete fr;
      AddHistogram("rdens",hrdens);
   }

}

TCanvas* TA2Plot::DrawCanvas(const char* Name, bool ApplyCuts, int MVAMode)
{

   FillHisto(ApplyCuts,MVAMode);

   TCanvas *cVTX = new TCanvas(Name, Name, 1800, 1000);
   //Scale factor to scale down to ms:
   if (GetMaxDumpLength()<SCALECUT) SetTimeFactor(1000.);
   cVTX->Divide(4, 2);
/*
   if (gLegendDetail >= 1)
   {
      gStyle->SetOptStat(11111);
   }
   else
   {
      gStyle->SetOptStat("ni");	//just like the knights of the same name
   }*/

   cVTX->cd(1);
   DrawHistogram("zvtx","HIST E1");

   cVTX->cd(2); // Z-counts (with electrodes?)4
   TVirtualPad *cVTX_1 = cVTX->cd(2);
   gPad->Divide(1, 2);
   cVTX_1->cd(1);
   //cVTX->cd(2)->SetFillStyle(4000 );
      //cVTX->cd(1)->SetFillStyle(4000 );
   // R-counts
   DrawHistogram("rvtx","HIST E1");

   DrawHistogram("rdens","HIST E1 SAME");

   TPaveText *rdens_label = new TPaveText(0.6, 0.8, 0.90, 0.85, "NDC NB");
   rdens_label->AddText("radial density [arbs]");
   rdens_label->SetTextColor(kRed);
   rdens_label->SetFillStyle(0);
   rdens_label->SetLineStyle(0);
   rdens_label->Draw();
   cVTX_1->cd(2);
   
   DrawHistogram("phivtx","HIST E1");

   cVTX->cd(3); // T-counts
   //cVTX->cd(3)->SetFillStyle(4000 );
   TLegend* legend = NULL;
   DrawHistogram("tIO32_nobusy","HIST");
   legend=AddLegendIntegral(legend,"Triggers: %5.0lf","tIO32_nobusy");
   DrawHistogram("tIO32","HIST SAME");
   legend=AddLegendIntegral(legend,"Reads: %5.0lf","tIO32");
   DrawHistogram("tAtomOR","HIST SAME");
   //((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->Draw("HIST"); // io32-notbusy = readouts
   //((TH1D *)hh[VERTEX_HISTO_IO32])->Draw("HIST SAME");    // io32
   //((TH1D *)hh[VERTEX_HISTO_ATOM_OR])->Draw("HIST SAME");    // ATOM OR PMTs

    
   //DrawHistogram("SVD_TRIG","HIST");

   DrawHistogram("tvtx","HIST SAME");

   
   //((TH1D *)hh[VERTEX_HISTO_VF48])->Draw("HIST SAME");    //io32 sistime
   if (MVAMode)
      DrawHistogram("tmva","HIST SAME");


   //auto legend = new TLegend(0.1,0.7,0.48,0.9);(0.75, 0.8, 1.0, 0.95
   //auto legend = new TLegend(1., 0.7, 0.45, 1.);//, "NDC NB");
   /*auto legend = new TLegend(1, 0.7, 0.55, .95); //, "NDC NB");
   char line[201];
   TH1D* TPC=((TH1D*)HISTOS.At(HISTO_POSITION.at("TPC_TRIG")));
   snprintf(line, 200, "TPC_TRIG: %5.0lf", TPC->Integral());
   //   snprintf(line, 200, "TPC_TRIG: %5.0lf", TPC->Integral("width"));
   legend->AddEntry(TPC, line, "f");*/
   
   //snprintf(line, 200, "TPC Events: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
   //legend->AddEntry(hh[VERTEX_HISTO_VF48], line, "f");
   if (MVAMode)
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
      //      snprintf(line, 200, "Vertices: %5.0lf", ((TH1D *)hh[VERTEX_HISTO_T])->Integral());
    //std::cout <<"Drawing lines"<<std::endl;
    legend=DrawLines(legend,"tIO32_nobusy");
   /*

    }*/
  }

  // legend->AddEntry("f1","Function abs(#frac{sin(x)}{x})","l");
  // legend->AddEntry("gr","Graph with error bars","lep");
  // legend->Draw();
  cVTX->cd(4);
  // X-Y-counts
  //cVTX->cd(4)->SetFillStyle(4000 );
  DrawHistogram("xyvtx","colz");

  cVTX->cd(5);
  // Z-R-counts
  //cVTX->cd(5)->SetFillStyle(4000 );
  //DrawHistogram("phitvtx","colz");
  // Z-T-counts
  //cVTX->cd(6)->SetFillStyle(4000 );
  DrawHistogram("ztvtx","colz");

  cVTX->cd(6);
  TVirtualPad *cVTX_2 = NULL;
  if (feGEM.size() && feLV.size())
  {
   cVTX_2 = cVTX->cd(6);
   gPad->Divide(1, 2);
  }
  if (cVTX_2) 
     cVTX_2->cd(1);
  /*DrawHistogram(feGEM.at(0).GetName().c_str(),"HIST");
  for (int i=1; i<feGEM.size(); i++)
     DrawHistogram(feGEM.at(i).GetName().c_str(),"HIST SAME");*/
   AlphaColourWheel colours;
   for (auto& f: feGEM)
   {
      for (auto& plot: f.plots)
      {
         TGraph* graph = plot->GetGraph();
         graph->SetMarkerColor(colours.GetNewColour());
         feGEMmg->Add(graph);
      }
   }
   feGEMmg->Draw("A*");
  if (cVTX_2) 
     cVTX_2->cd(2);
   for (auto& f: feLV)
   {
      for (auto& plot: f.plots)
      {
         TGraph* graph = plot->GetGraph();
         graph->SetMarkerColor(colours.GetNewColour());
         feLVmg->Add(graph);
      }
   }
   feLVmg->Draw("A*");

  cVTX->cd(7);
  // phi counts
  //cVTX->cd(7)->SetFillStyle(4000 );
  /*((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->SetStats(0);
  ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetCumulative()->Draw("HIST");
  ((TH1D *)hh[VERTEX_HISTO_IO32])->GetCumulative()->Draw("HIST SAME");*/
  DrawHistogram("tvtx","HIST SAME");
  
  //((TH1D *)hh[VERTEX_HISTO_VF48])->GetCumulative()->Draw("HIST SAME");
#if 0
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
  //IO32_NOTBUSY Halfway point
  /*
  TH1 *h2 = ((TH1D *)hh[VERTEX_HISTO_IO32_NOTBUSY])->GetCumulative();
  //Draw line at halfway point
  Double_t Max = h2->GetBinContent(h2->GetMaximumBin());
  Double_t Tmax = h2->GetXaxis()->GetXmax();
  for (Int_t i = 0; i < h2->GetMaximumBin(); i++)
  {
    if (h2->GetBinContent(i) > Max / 2.)
    {
      TLine *half = new TLine(TMax *tFactor* (Double_t)i / (Double_t)Nbin, 0., Tmax * (Double_t)i / (Double_t)Nbin, Max / 2.);
      half->SetLineColor(kRed);
      half->Draw();
      break;
    }
  }
*/
  cVTX->cd(8);
  // Z-PHI-counts
  //cVTX->cd(8)->SetFillStyle(4000 );
  DrawHistogram("zphivtx","colz");
  if (ApplyCuts)
  {
    cVTX->cd(1);
    TPaveText *applycuts_label = new TPaveText(0., 0.95, 0.20, 1.0, "NDC NB");
    if (MVAMode>0)
      applycuts_label->AddText("RF cut applied");
    else
      applycuts_label->AddText("Cuts applied");
    applycuts_label->SetTextColor(kRed);
    applycuts_label->SetFillColor(kWhite);
    applycuts_label->Draw();
  };
  //TLatex* runs_label = new  TLatex(-32.5,-.0625, 32.5, 5.6, "NDC NB");
  //
  cVTX->cd(0);
  TString run_txt = "Run(s): ";
  run_txt += GetListOfRuns();
  run_txt += " ";
  run_txt += GetNBins();
  run_txt += " bins";
  /*if (gZcutMin > -998.)
    {
      run_txt += " gZcutMin=";
      run_txt += gZcutMin;
    }
    if (gZcutMax < 998.)
    {
      run_txt += " gZcutMax=";
      run_txt += gZcutMax;
    }*/
  //runs_label->AddText(run_txt);
  TLatex *runs_label = new TLatex(0., 0., run_txt);
  runs_label->SetTextSize(0.016);
  //runs_label->SetTextColor(kRed);
  //runs_label->SetFillColor(kWhite);
  //runs_label->SetFillStyle(1001);
  runs_label->Draw();

  std::cout<<run_txt<<std::endl;
  return cVTX;
}

#endif
