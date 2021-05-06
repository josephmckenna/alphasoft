#include "TA2Plot.h"
#include <fstream>


#ifdef BUILD_A2
ClassImp(TA2Plot);

TA2Plot::TA2Plot(bool zerotime): TAPlot(zerotime)
{
   ZMinCut=-99999.;
   ZMaxCut= 99999.;
}
TA2Plot::TA2Plot(double zmin, double zmax, bool zerotime): TAPlot(zerotime)
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
  trig.insert(             std::pair<int,int>(runNumber, (int)sisch->GetChannel("IO32_TRIG")));
  trig_nobusy.insert(      std::pair<int,int>(runNumber, (int)sisch->GetChannel("IO32_TRIG_NOBUSY")));
  atom_or.insert(          std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_PMT_ATOM_OR")));
  Beam_Injection.insert(   std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_AD")));
  Beam_Ejection.insert(    std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_AD_2")));
  CATStart.insert(         std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_PBAR_DUMP_START")));
  CATStop.insert(          std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_PBAR_DUMP_STOP")));
  RCTStart.insert(         std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_RECATCH_DUMP_START")));
  RCTStop.insert(          std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_RECATCH_DUMP_STOP")));
  ATMStart.insert(         std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_ATOM_DUMP_START")));
  ATMStop.insert(          std::pair<int,int>(runNumber, (int)sisch->GetChannel("SIS_ATOM_DUMP_STOP")));


  //Add all valid SIS channels to a list for later:
  SISChannels.clear();
  if (trig.find(runNumber)->second>0)           SISChannels.push_back(trig.find(runNumber)->second);
  if (trig_nobusy.find(runNumber)->second>0)    SISChannels.push_back(trig_nobusy.find(runNumber)->second);
  if (atom_or.find(runNumber)->second>0)        SISChannels.push_back(atom_or.find(runNumber)->second);
  if (Beam_Injection.find(runNumber)->second>0) SISChannels.push_back(Beam_Injection.find(runNumber)->second);
  if (Beam_Ejection.find(runNumber)->second>0)  SISChannels.push_back(Beam_Ejection.find(runNumber)->second);
  if (CATStart.find(runNumber)->second>0)       SISChannels.push_back(CATStart.find(runNumber)->second);
  if (CATStop.find(runNumber)->second>0)        SISChannels.push_back(CATStop.find(runNumber)->second);
  if (RCTStart.find(runNumber)->second>0)       SISChannels.push_back(RCTStart.find(runNumber)->second);
  if (RCTStop.find(runNumber)->second>0)        SISChannels.push_back(RCTStop.find(runNumber)->second);
  if (ATMStart.find(runNumber)->second>0)       SISChannels.push_back(ATMStart.find(runNumber)->second);
  if (ATMStop.find(runNumber)->second>0)        SISChannels.push_back(ATMStop.find(runNumber)->second);
  //cout <<"Trig:"<<trig<<endl;
  //cout <<"TrigNoBusy:"<<trig_nobusy<<endl;
  //cout <<"Beam Injection:"<<Beam_Injection<<endl;
  //cout <<"Beam Ejection:"<<Beam_Ejection<<endl;
  delete sisch;
  return;
}

void TA2Plot::AddEvent(TSVD_QOD* event, double time_offset)
{
   double tminusoff = (event->t - time_offset);
   AddVertexEvent(event->RunNumber, event->VF48NEvent, event->NPassedCuts+event->MVA*2, event->NVertices, 
      event->x, event->y, event->z, tminusoff, event->VF48Timestamp, event->t, -1, event->NTracks);
}

void TA2Plot::AddEvent(TSISEvent* event, int channel, double time_offset)
{
   SISEvents.AddEvent(event->GetRunNumber(), event->GetRunTime() - time_offset, 
      event->GetRunTime(), event->GetCountsInChannel(channel), channel, event->GetMidasEventID());
}

//TODO: Pre sort windows (we should never have overlapping windows...)
//Sort tmins... then search for first tmin < t
//check if valid tmax > t
//Perhaps replace std::vector<TimeWindows> with its own class...
// two vectors would make a better memory layout... not a struct?
void TA2Plot::AddSVDEvent(TSVD_QOD* SVDEvent)
{
   double t=SVDEvent->t;
   if (SVDEvent->z < ZMinCut) return;
   if (SVDEvent->z > ZMaxCut) return;

   int index = GetTimeWindows()->GetValidWindowNumber(t);
   
   
   //Checks to make sure GetValidWindowNumber hasn't returned -1 (in which case it will be ignored) and 
   //if not it will add the event. 
   if(index >= 0)
   {
      AddEvent(SVDEvent, GetTimeWindows()->tzero.at(index));
   }
}

void TA2Plot::AddSISEvent(TSISEvent* SISEvent)
{
   int n_sis=SISChannels.size();
   double t=SISEvent->GetRunTime();

   //Loop over all time windows
   int index = GetTimeWindows()->GetValidWindowNumber(t);
   if(index>=0)
   {
      for (int i=0; i<n_sis; i++)
      {
         int counts=SISEvent->GetCountsInChannel(SISChannels[i]);
         if (counts)
         {
            AddEvent(SISEvent, SISChannels[i], GetTimeWindows()->tzero[index]);
         }
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
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficialTime");
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

void TA2Plot::SetUpHistograms()
{
   const double XMAX(4.),YMAX(4.),RMAX(4.), ZMAX(30.);

   double TMin;
   double TMax;
   if (ZeroTimeAxis)
   {
      TMin=GetBiggestTzero();
      TMax=GetMaxDumpLength() + TMin;
   }
   else
   {
      TMin=GetFirstTmin();
      TMax=GetLastTmax();
   }

   AddHistogram("zvtx",new TH1D((GetTAPlotTitle() + "_zvtx").c_str(), "Z Vertex;z [cm];events", GetNBins(), -ZMAX, ZMAX));

   TH1D* hr = new TH1D((GetTAPlotTitle() + "_rvtx").c_str(), "R Vertex;r [cm];events", GetNBins(), 0., RMAX);
   hr->SetMinimum(0);
   AddHistogram("rvtx",hr);

   TH1D* hphi = new TH1D((GetTAPlotTitle() + "_phivtx").c_str(), "phi Vertex;phi [rad];events", GetNBins(), -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   AddHistogram("phivtx",hphi);

   TH2D* hxy = new TH2D((GetTAPlotTitle() + "_xyvtx").c_str(), "X-Y Vertex;x [cm];y [cm]", GetNBins(), -XMAX, XMAX, GetNBins(), -YMAX, YMAX);
   AddHistogram("xyvtx",hxy);

   TH2D* hzr = new TH2D((GetTAPlotTitle() + "_zrvtx").c_str(), "Z-R Vertex;z [cm];r [cm]", GetNBins(), -ZMAX, ZMAX, GetNBins(), 0., RMAX);
   AddHistogram("zrvtx",hzr);

   TH2D* hzphi = new TH2D((GetTAPlotTitle() + "_zphivtx").c_str(), "Z-Phi Vertex;z [cm];phi [rad]", GetNBins(), -ZMAX, ZMAX, GetNBins(), -TMath::Pi(), TMath::Pi());
   AddHistogram("zphivtx",hzphi);
   std::string units;
   if (GetMaxDumpLength()<SCALECUT) 
   {
      tFactor = 1000;
      units = "[ms]";
   }
   else
   {
      tFactor = 1;
      units = "[s]";
   }
   //SIS channels:
   TH1D* triggers=new TH1D((GetTAPlotTitle() + "_tIO32_nobusy").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), TMin*tFactor, TMax*tFactor);
   triggers->SetMarkerColor(kRed);
   triggers->SetLineColor(kRed);
   triggers->SetMinimum(0);
   AddHistogram("tIO32_nobusy",triggers);

   TH1D* read_triggers=new TH1D((GetTAPlotTitle() + "_tIO32").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), TMin*tFactor, TMax*tFactor);
   read_triggers->SetMarkerColor(kViolet);
   read_triggers->SetLineColor(kViolet);
   read_triggers->SetMinimum(0);
   AddHistogram("tIO32",read_triggers);

   TH1D* atom_or=new TH1D((GetTAPlotTitle() + "_tAtomOR").c_str(), (std::string("t;t ") + units + ";events").c_str(), GetNBins(), TMin*tFactor, TMax*tFactor);
   atom_or->SetMarkerColor(kGreen);
   atom_or->SetLineColor(kGreen);
   atom_or->SetMinimum(0);
   AddHistogram("tAtomOR",atom_or);

   TH1D* ht = new TH1D((GetTAPlotTitle() + "_tvtx").c_str(), (std::string("t Vertex;t ") + units + ";events").c_str(), GetNBins(), TMin*tFactor, TMax*tFactor);
   ht->SetLineColor(kMagenta);
   ht->SetMarkerColor(kMagenta);
   ht->SetMinimum(0);
   AddHistogram("tvtx",ht);

   TH2D* hzt = new TH2D((GetTAPlotTitle() + "_ztvtx").c_str(), (std::string("Z-T Vertex;z [cm];t ") + units).c_str(), GetNBins(), -ZMAX, ZMAX, GetNBins(), TMin*tFactor, TMax*tFactor);
   AddHistogram("ztvtx",hzt);

   //TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),TMin*1000., TMax*1000);
   //AddHistogram("phitvtx",hphit);

   //if (MVAMode)
   //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [ms];Counts", Nbin, TMin*1000., TMax*1000.);
  return;
}

void TA2Plot::FillHisto(bool ApplyCuts, int MVAMode)
{

   ClearHisto();
   SetUpHistograms();
   //FillfeGEMHistograms();
   const double max_dump_length=GetMaxDumpLength();
   //Fill SIS histograms
   int runno=0;
   for (int i = 0; i<SISEvents.t.size(); i++)
   {
      if (SISEvents.runNumber[i]!=runno)
      {
         runno=SISEvents.runNumber[i];
         SetSISChannels(runno);
      }
      double time;
      if (ZeroTimeAxis)
         time = SISEvents.t[i];
      else
         time = SISEvents.OfficialTime[i];
      if (max_dump_length<SCALECUT) 
         time=time*1000.;
      int Channel         = SISEvents.SIS_Channel[i];
      int CountsInChannel = SISEvents.Counts[i];
      if (Channel == trig.find(SISEvents.runNumber[i])->second)
         FillHistogram("tIO32",time,CountsInChannel);
      else if (Channel == trig_nobusy.find(SISEvents.runNumber[i])->second)
         FillHistogram("tIO32_nobusy",time,CountsInChannel);
      else if (Channel == atom_or.find(SISEvents.runNumber[i])->second)
         FillHistogram("tAtomOR",time,CountsInChannel);
      else if (Channel == Beam_Injection.find(SISEvents.runNumber[i])->second)
         AddInjection(time);
      else if (Channel == Beam_Ejection.find(SISEvents.runNumber[i])->second)
         AddEjection(time);
      else if (Channel == CATStart.find(SISEvents.runNumber[i])->second || Channel == RCTStart.find(SISEvents.runNumber[i])->second || Channel == ATMStart.find(SISEvents.runNumber[i])->second)
         AddStopDumpMarker(time);
      else if (Channel == CATStop.find(SISEvents.runNumber[i])->second || Channel == RCTStop.find(SISEvents.runNumber[i])->second || Channel == ATMStop.find(SISEvents.runNumber[i])->second)
         AddStartDumpMarker(time);
      else std::cout <<"Unconfigured SIS channel in TAlhaPlot"<<std::endl;
   }

   //Fill Vertex Histograms
   TVector3 vtx;
   const TVertexEvents* VEs = GetVertexEvents();
   //for (auto& vtxevent: GetVertexEvents())   
   for (int i=0; i<VEs->xs.size(); i++)
   {
      Double_t time;
      if (ZeroTimeAxis)
         time = VEs->ts[i];
      else
         time = VEs->RunTimes[i];
      if (max_dump_length<SCALECUT)
         time=time*1000.;
      vtx=TVector3(VEs->xs[i],VEs->ys[i],VEs->zs[i]);
      FillHistogram("tSVD",time);

      int CutsResult=VEs->CutsResults[i];
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
            if ( VEs->VertexStatuses[i] > 0) 
            {
               FillHistogram("tvtx",time);
            }
            else 
               continue;
         }
         if (VEs->VertexStatuses[i] <= 0) continue; //Don't draw invaid vertices
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

void TA2Plot::WriteEventList(std::string filename, bool append)
{
   //TODO LMG - Check if file already exists and if so, append to it. If not, delete the old .list and rewrite.
   //Change .ats to []s for speed. 

   //Initiate an ofstream to write to
   std::ofstream myfile;
   std::string file = filename + ".list";
   myfile.open (file);
   
   //Assert that the runNumbers and EventNos match up in terms of size.
   assert(VertexEvents.runNumbers.size() == VertexEvents.EventNos.size());

   int index = 0; //Initialise at index 0
   int currentEventNo = VertexEvents.EventNos.at(index); //Set the current run number to be the one at index (0)
   int currentRunNo = SISEvents.runNumber.at(index); //Same for event no.
   myfile << currentRunNo << ":" << currentEventNo; //Print an initial statement to file, will look something like "39993:2"
   
   //While index is in range lets do all the checks to decide what to write.
   while(index < VertexEvents.runNumbers.size()-1)
   {
      index++; //Increment index since we're in a while loop not a for.
      if(VertexEvents.runNumbers.at(index)!=currentRunNo)
      {
         //If runNumber has changed:
         myfile << "-" << currentEventNo << std::endl; //1. Close off current range eg: "39993:2-5"
         currentRunNo = VertexEvents.runNumbers.at(index); //Update currentrunNo and EventNo
         currentEventNo = VertexEvents.EventNos.at(index);
         myfile << currentRunNo << ":" << currentEventNo; //Print initial line of new run eg: "45000:3"
      }
      else if(VertexEvents.EventNos.at(index) == (currentEventNo+1))
      {
         //Else if runNumber is the same but the event is consecutive to the one after (ie 2-3)
         currentEventNo++; //Increment currentEventNo. This is equiv to currentEventNo = VertexEvents.EventNos.at(index) just quicker since we've already checked its consecutive.
      }
      else
      {
         //Else: Ie run number is the samer but the event number is not consecutive:
         myfile << "-" << currentEventNo << std::endl; //Close line, eg: "39993:2-5"
         currentRunNo = VertexEvents.runNumbers.at(index); //Update run and event no.
         currentEventNo = VertexEvents.EventNos.at(index);
         myfile << currentRunNo << ":" << currentEventNo; //Start new line eg: "39993:27"
      }
   }
   //Once out of the loop close what we have.
   myfile << "-" << currentEventNo << std::endl;
   //Close the file.
   myfile.close();
} 

TCanvas* TA2Plot::DrawCanvas(const char* Name, bool ApplyCuts, int MVAMode)
{
   
   SetTAPlotTitle(Name);
   std::cout<<"TAPlot Processing time : ~" << GetApproximateProcessingTime() <<"s"<<std::endl;
   TCanvas *cVTX = new TCanvas(Name, Name, 1800, 1000);
   FillHisto(ApplyCuts,MVAMode);


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
  if (HaveGEMData() && HaveLVData())
  {
   cVTX_2 = cVTX->cd(6);
   gPad->Divide(1, 2);
  }
  if (cVTX_2) 
     cVTX_2->cd(1);
  /*DrawHistogram(feGEM.at(0).GetName().c_str(),"HIST");
  for (int i=1; i<feGEM.size(); i++)
     DrawHistogram(feGEM.at(i).GetName().c_str(),"HIST SAME");*/
   std::pair<TLegend*,TMultiGraph*> gm=GetGEMGraphs();
   if (gm.first)
   {
      //Draw TMultigraph
      gm.second->Draw("AL*");
      //Draw legend
      gm.first->Draw();
   }

   if (cVTX_2) 
      cVTX_2->cd(2);
   
   std::pair<TLegend*,TMultiGraph*> lv=GetLVGraphs();
   if (lv.first)
   {
      //Draw TMultigraph
      lv.second->Draw("AL*");
      //Draw legend
      lv.first->Draw();
   }

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

TA2Plot& TA2Plot::operator=(const TA2Plot& plotA)
{
   //Inherited TAPlot members
   std::cout << "TA2Plot equals operator" << std::endl;
   SISChannels    = plotA.SISChannels;
   trig           = plotA.trig;
   trig_nobusy    = plotA.trig_nobusy;
   atom_or        = plotA.atom_or;
   CATStart       = plotA.CATStart;
   CATStop        = plotA.CATStop;
   RCTStart       = plotA.RCTStart;
   RCTStop        = plotA.RCTStop;
   ATMStart       = plotA.ATMStart;
   ATMStop        = plotA.ATMStop;
   Beam_Injection = plotA.Beam_Injection;
   Beam_Ejection  = plotA.Beam_Ejection;
   ZMinCut        = plotA.ZMinCut;
   ZMaxCut        = plotA.ZMaxCut;
   SISEvents   = plotA.SISEvents;

   return *this;
}

TA2Plot::TA2Plot(const TA2Plot& m_TA2Plot) : TAPlot(m_TA2Plot)
{
   std::cout << "This is TA2Plot copy constructor" << std::endl;
   SISChannels    = m_TA2Plot.SISChannels;
   trig           = m_TA2Plot.trig;
   trig_nobusy    = m_TA2Plot.trig_nobusy;
   atom_or        = m_TA2Plot.atom_or;
   CATStart       = m_TA2Plot.CATStart;
   CATStop        = m_TA2Plot.CATStop;
   RCTStart       = m_TA2Plot.RCTStart;
   RCTStop        = m_TA2Plot.RCTStop;
   ATMStart       = m_TA2Plot.ATMStart;
   ATMStop        = m_TA2Plot.ATMStop;
   Beam_Injection = m_TA2Plot.Beam_Injection;
   Beam_Ejection  = m_TA2Plot.Beam_Ejection;
   ZMinCut        = m_TA2Plot.ZMinCut;
   ZMaxCut        = m_TA2Plot.ZMaxCut;
   SISEvents      = m_TA2Plot.SISEvents;
}

TA2Plot::TA2Plot(const TAPlot& m_TAPlot) : TAPlot(m_TAPlot)
{
   std::cout << "This is TA2Plot constructor from TAPlot" << std::endl;
   ZMinCut=-99999.;
   ZMaxCut= 99999.;
}

TA2Plot operator+(const TA2Plot& PlotA, const TA2Plot& PlotB)
{
   //In order to call the parents addition first it is important to statically cast the Plots to their parent class,
   //add the parents together, then initialise a TA2Plot from the TAPlot and fill in the rest of the values as a constructor would.
   TAPlot PlotACast = static_cast<TAPlot>(PlotA); //2 Static casts
   TAPlot PlotBCast = static_cast<TAPlot>(PlotB);
   TAPlot ParentSum = PlotACast + PlotBCast; //Add as TAPlots
   TA2Plot BasePlot = TA2Plot(ParentSum); //Initialise a TA2Plot from the now summed TAPlots

   //Now we fill in the (empty) values of this newly initiated TA2Plot with the values we need from the 2 input arguments:
   //For all these copying A is fine.
   BasePlot.trig.insert(            PlotB.trig.begin(), PlotB.trig.end() );
   BasePlot.trig_nobusy.insert(     PlotB.trig_nobusy.begin(), PlotB.trig_nobusy.end() );
   BasePlot.atom_or.insert(         PlotB.atom_or.begin(), PlotB.atom_or.end() );
   BasePlot.CATStart.insert(        PlotB.CATStart.begin(), PlotB.CATStart.end() );
   BasePlot.CATStop.insert(         PlotB.CATStop.begin(), PlotB.CATStop.end() );
   BasePlot.RCTStart.insert(        PlotB.RCTStart.begin(), PlotB.RCTStart.end() );
   BasePlot.RCTStop.insert(         PlotB.RCTStop.begin(), PlotB.RCTStop.end() );
   BasePlot.ATMStart.insert(        PlotB.ATMStart.begin(), PlotB.ATMStart.end() );
   BasePlot.ATMStop.insert(         PlotB.ATMStop.begin(), PlotB.ATMStop.end() );
   BasePlot.Beam_Injection.insert(  PlotB.Beam_Injection.begin(), PlotB.Beam_Injection.end() );
   BasePlot.Beam_Ejection.insert(   PlotB.Beam_Ejection.begin(), PlotB.Beam_Ejection.end() );
   
   BasePlot.ZMinCut        = PlotA.ZMinCut;
   BasePlot.ZMaxCut        = PlotA.ZMaxCut;

   //Vectors need concacting.
   BasePlot.SISChannels.insert(BasePlot.SISChannels.end(), PlotB.SISChannels.begin(), PlotB.SISChannels.end() );

   BasePlot.SISEvents += PlotA.SISEvents;
   BasePlot.SISEvents += PlotB.SISEvents;
   
   return BasePlot;
}

TA2Plot& TA2Plot::operator+=(const TA2Plot& plotB)
{
   //In order to call the parents addition first it is important to statically cast the Plots to their parent class,
   //add the parents together, then initialise a TA2Plot from the TAPlot and fill in the rest of the values as a constructor would.
   //TAPlot PlotACast = static_cast<TAPlot>(this); //2 Static casts
   //TAPlot PlotBCast = static_cast<TAPlot>(plotB);
   //TAPlot ParentSum = PlotACast + PlotBCast; //Add as TAPlots
   //TA2Plot BasePlot = TA2Plot(ParentSum); //Initialise a TA2Plot from the now summed TAPlots

   TAPlot::operator+=(plotB);

   //Now we fill in the (empty) values of this newly initiated TA2Plot with the values we need from the 2 input arguments:
   //For all these copying A is fine.
   trig.insert(            plotB.trig.begin(), plotB.trig.end() );
   trig_nobusy.insert(     plotB.trig_nobusy.begin(), plotB.trig_nobusy.end() );
   atom_or.insert(         plotB.atom_or.begin(), plotB.atom_or.end() );
   CATStart.insert(        plotB.CATStart.begin(), plotB.CATStart.end() );
   CATStop.insert(         plotB.CATStop.begin(), plotB.CATStop.end() );
   RCTStart.insert(        plotB.RCTStart.begin(), plotB.RCTStart.end() );
   RCTStop.insert(         plotB.RCTStop.begin(), plotB.RCTStop.end() );
   ATMStart.insert(        plotB.ATMStart.begin(), plotB.ATMStart.end() );
   ATMStop.insert(         plotB.ATMStop.begin(), plotB.ATMStop.end() );
   Beam_Injection.insert(  plotB.Beam_Injection.begin(), plotB.Beam_Injection.end() );
   Beam_Ejection.insert(   plotB.Beam_Ejection.begin(), plotB.Beam_Ejection.end() );
   
   //this.ZMinCut        = plotB.ZMinCut;
   //this.ZMaxCut        = plotB.ZMaxCut;

   //Vectors need concacting.
   SISChannels.insert(SISChannels.end(), plotB.SISChannels.begin(), plotB.SISChannels.end() );

   SISEvents += plotB.SISEvents;
   
   return *this;
}

#endif
