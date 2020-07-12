#include "TA2Plot.h"

ClassImp(TA2Plot);

TA2Plot::TA2Plot(): TAPlot()
{

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

void TA2Plot::LoadRun(int runNumber)
{
   double last_time=0;
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
      double t=SVDEvent->t;
      if (t>last_time) break;
      
      //Loop over all time windows
      for (auto& window: GetTimeWindows())
      {
         //If inside the time window
         if ( (t>window.tmin && t< window.tmax) ||
         //Or if after tmin and tmax is invalid (-1)
              (t>window.tmin && window.tmax<0) )
         {
            AddEvent(&(*SVDEvent),window.tmin);
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   SetSISChannels(runNumber);
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   int n_sis=SISChannels.size();
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SISReader->Next())
   {
      double t=SISEvent->GetRunTime();
      if (t>last_time) break;

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
                  AddEvent(&(*SISEvent),SISChannels[i],window.tmin);
               }
            }
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }
}

void TA2Plot::AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition )
{
   std::vector<TA2Spill*> spills=Get_A2_Spills(runNumber,description,repetition);
   std::vector<double> tmin;
   std::vector<double> tmax;
   
   for (auto & spill: spills)
   {
      if (spill->ScalerData)
      {
         tmin.push_back(spill->ScalerData->StartTime);
         tmax.push_back(spill->ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return AddTimeGates(runNumber,tmin,tmax);

}

void TA2Plot::SetUpHistograms(bool zeroTime)
{

   const double XMAX(40),YMAX(40),RMAX(40), ZMAX(300.);

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
   TH1D* SVD=new TH1D("SVD_TRIG", "t;t [s];events", GetNBins(), TMin, TMax);
   SVD->SetMarkerColor(kRed);
   SVD->SetLineColor(kRed);
   SVD->SetMinimum(0);
   AddHistogram("tSVD",SVD);
   

   
   AddHistogram("zvtx",new TH1D("zvtx", "Z Vertex;z [mm];events", GetNBins(), -ZMAX, ZMAX));

   TH1D* hr = new TH1D("rvtx", "R Vertex;r [mm];events", GetNBins(), 0., RMAX);
   hr->SetMinimum(0);
   AddHistogram("rvtx",hr);

   TH1D* hphi = new TH1D("phivtx", "phi Vertex;phi [rad];events", GetNBins(), -TMath::Pi(), TMath::Pi());
   hphi->SetMinimum(0);
   AddHistogram("phivtx",hphi);

   TH2D* hxy = new TH2D("xyvtx", "X-Y Vertex;x [mm];y [mm]", GetNBins(), -XMAX, XMAX, GetNBins(), -YMAX, YMAX);
   AddHistogram("xyvtx",hxy);

   TH2D* hzr = new TH2D("zrvtx", "Z-R Vertex;z [mm];r [mm]", GetNBins(), -ZMAX, ZMAX, GetNBins(), 0., RMAX);
   AddHistogram("zrvtx",hzr);

   TH2D* hzphi = new TH2D("zphivtx", "Z-Phi Vertex;z [mm];phi [rad]", GetNBins(), -ZMAX, ZMAX, GetNBins(), -TMath::Pi(), TMath::Pi());
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

      TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),TMin*1000., TMax*1000);
      AddHistogram("phitvtx",hphit);

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

      TH2D* hzt = new TH2D("ztvtx", "Z-T Vertex;z [mm];t [s]", GetNBins(), -ZMAX, ZMAX, GetNBins(), TMin, TMax);
      AddHistogram("ztvtx",hzt);

      TH2D* hphit = new TH2D("phitvtx", "Phi-T Vertex;phi [rad];t [s]", GetNBins(),-TMath::Pi(), TMath::Pi() ,  GetNBins(),TMin, TMax);
      AddHistogram("phitvtx",hphit);

      //if (MVAMode)
      //   ht_MVA = new TH1D("htMVA", "Vertex, Passcut and MVA;t [s];Counts", GetNBins(), TMin, TMax);
  }
  return;
}

void TA2Plot::FillHisto(bool ApplyCuts, int MVAMode)
{

   ClearHisto();
   SetUpHistograms();
   
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
      FillHistogram("phitvtx",vtx.Phi(),time);
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
