#if BUILD_AG
#include "TAGPlotTracks.h"


ClassImp(TAGPlotTracks)

TAGPlotTracks::TAGPlotTracks(bool zeroTime): TAGPlot(zeroTime)
{

}

TAGPlotTracks::TAGPlotTracks(double zMin, double zMax, bool zeroTime): TAGPlot(zMin, zMax, zeroTime)
{

}

TAGPlotTracks::TAGPlotTracks(double zMin, double zMax, int barCut, bool zeroTime): TAGPlot(zMin, zMax, barCut, zeroTime)
{

}

TAGPlotTracks::TAGPlotTracks(const TAGPlotTracks& object):
   TAGPlot(object),
   fHelixEvents(object.fHelixEvents),
   fUsedHelixEvents(object.fUsedHelixEvents),
   fSpacePointHelixEvents(object.fSpacePointHelixEvents),
   fSpacePointUsedHelixEvents(object.fSpacePointUsedHelixEvents)
{

}

TAGPlotTracks::~TAGPlotTracks()
{
   Reset();
}

void TAGPlotTracks::Reset()
{
   TAGPlot::Reset();
   fHelixEvents.clear();
   fUsedHelixEvents.clear();
   fSpacePointHelixEvents.clear();
   fSpacePointUsedHelixEvents.clear();
}



TAGPlotTracks& TAGPlotTracks::operator=(const TAGPlotTracks& rhs)
{
   TAGPlot::operator=(rhs);
   fHelixEvents = rhs.fHelixEvents;
   fUsedHelixEvents = rhs.fUsedHelixEvents;
   fSpacePointHelixEvents = rhs.fSpacePointHelixEvents;
   fSpacePointUsedHelixEvents = rhs.fSpacePointUsedHelixEvents;
   return *this;
}

TAGPlotTracks& TAGPlotTracks::operator+=(const TAGPlotTracks& rhs)
{
   //This calls the parent += operator first.
   TAGPlot::operator+=(rhs);
   fHelixEvents += rhs.fHelixEvents;
   fUsedHelixEvents += fUsedHelixEvents;
   fSpacePointHelixEvents += fSpacePointHelixEvents;
   fSpacePointUsedHelixEvents += fSpacePointUsedHelixEvents;
   return *this;
}

TAGPlotTracks& operator+(const TAGPlotTracks& lhs, const TAGPlotTracks& rhs)
{
   TAGPlotTracks* basePlot = new TAGPlotTracks;
   *basePlot += lhs;
   *basePlot += rhs;
   return *basePlot;
}
void TAGPlotTracks::AddEvent(const TStoreEvent& event, const double timeOffset)
{
   // Use the same method as used online to build a TAGDetectorEvent... recycles 
   // code nicely so should avoid any discrepancies from future development
   const TAGDetectorEvent detectorEvent(&event);
   TAGPlot::AddEvent(detectorEvent,timeOffset);

   ProcessUsedHelices(event.GetRunNumber(),event.GetTimeOfEvent() - timeOffset,event.GetTimeOfEvent(), event.GetUsedHelices());
   ProcessHelices(event.GetRunNumber(),event.GetTimeOfEvent() - timeOffset,event.GetTimeOfEvent(), event.GetHelixArray());
  return;
}


void TAGPlotTracks::LoadRun(const int runNumber, const double firstTime, const double lastTime)
{
   TTreeReader* TPCTreeReader = Get_StoreEvent_TreeReader(runNumber);
   TTreeReaderValue<TStoreEvent> TPCEvent(*TPCTreeReader, "StoredEvent");
   while (TPCTreeReader->Next())
   {
      const double t = TPCEvent->GetTimeOfEvent();
      if (t < firstTime)
         continue;
      if (t > lastTime)
         break;
      AddStoreEvent(*TPCEvent);
   }
   LoadChronoEvents(runNumber,firstTime,lastTime);
}


void TAGPlotTracks::AddStoreEvent(const TStoreEvent& event)
{
  double time = event.GetTimeOfEvent();
  const double z = event.GetVertex().Z();
  if ( z < fZMinCut ) return;
  if ( z > fZMaxCut ) return;

  int index = GetTimeWindows()->GetValidWindowNumber(time);

  if (index >= 0)
  {
    AddEvent(event, GetTimeWindows()->fZeroTime.at(index));
  }
}


void TAGPlotTracks::SetupTrackHistos()
{
  // reco helices
  TH1D* hNhel = new TH1D("hNhel","Reconstructed Helices",10,0.,10.);
  AddHistogram(hNhel->GetName(),hNhel);

  TH1D* hhD = new TH1D("hhD","Hel D;[mm]",200,-100.,100.);
  AddHistogram(hhD->GetName(),hhD);
  // TH1D* hhc = new TH1D("hhc","Hel c;[mm^{-1}]",200,-1.e-2,1.e-2);
  // HISTOS.Add(hhc);
  // fHistoPositions[hhc->GetName()]=HISTOS.GetEntries()-1;
  TH1D* hhRc = new TH1D("hhRc","Hel Rc;[mm]",200,-2000.,2000.);
  AddHistogram(hhRc->GetName(),hhRc);

  TH1D* hpt = new TH1D("hpt","Helix Transverse Momentum;p_{T} [MeV/c]",200,0.,1000.);
  AddHistogram(hpt->GetName(),hpt);

  TH1D* hpz = new TH1D("hpz","Helix Longitudinal Momentum;p_{Z} [MeV/c]",500,-1000.,1000.);
  AddHistogram(hpz->GetName(),hpz);

  TH1D* hpp = new TH1D("hpp","Helix Total Momentum;p_{tot} [MeV/c]",200,0.,1000.);
  AddHistogram(hpp->GetName(),hpp);

  TH2D* hptz = new TH2D("hptz","Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		  100,0.,1000.,200,-1000.,1000.);
  AddHistogram(hptz->GetName(),hptz);

  // reco helices spacepoints
  TH2D* hhspxy = new TH2D("hhspxy","Spacepoints in Helices;x [mm];y [mm]",
		    100,-190.,190.,100,-190.,190.);
  hhspxy->SetStats(kFALSE);
  AddHistogram(hhspxy->GetName(),hhspxy);

  TH2D* hhspzr = new TH2D("hhspzr","Spacepoints in Helices;z [mm];r [mm]",
		    600,-1200.,1200.,60,109.,174.);
  hhspzr->SetStats(kFALSE);
  AddHistogram(hhspzr->GetName(),hhspzr);

  TH2D* hhspzp = new TH2D("hhspzp","Spacepoints in Helices;z [mm];#phi [deg]",
		    600,-1200.,1200.,180,0.,360.);
  hhspzp->SetStats(kFALSE);
  AddHistogram(hhspzp->GetName(),hhspzp);

  // TH2D* hhsprp = new TH2D("hhsprp","Spacepoints in Helices;#phi [deg];r [mm]",
  // 		    180,0.,TMath::TwoPi(),200,108.,175.);
  // hhsprp->SetStats(kFALSE);
  // fHistos.Add(hhsprp);
  // fHistoPositions[hhsprp->GetName()]=fHistos.GetEntries()-1;

  // used helices
  TH1D* hNusedhel = new TH1D("hNusedhel","Used Helices",10,0.,10.);
  AddHistogram(hNusedhel->GetName(),hNusedhel);

  TH1D* huhD = new TH1D("huhD","Used Hel D;[mm]",200,-100.,100.);
  AddHistogram(huhD->GetName(),huhD);
  // TH1D* huhc = new TH1D("huhc","Used Hel c;[mm^{-1}]",200,-1.e-2,1.e-2);
  // fHistos.Add(huhc);
  // fHistoPositions[huhc->GetName()]=fHistos.GetEntries()-1;
  TH1D* huhRc = new TH1D("huhRc","Used Hel Rc;[mm]",200,-2000.,2000.);
  AddHistogram(huhRc->GetName(),huhRc);
  
  TH1D* huhpt = new TH1D("huhpt","Used Helix Transverse Momentum;p_{T} [MeV/c]",200,0.,1000.);
  AddHistogram(huhpt->GetName(),huhpt);

  TH1D* huhpz = new TH1D("huhpz","Used Helix Longitudinal Momentum;p_{Z} [MeV/c]",500,-1000.,1000.);
  AddHistogram(huhpz->GetName(),huhpz);

  TH1D* huhpp = new TH1D("huhpp","Used Helix Total Momentum;p_{tot} [MeV/c]",200,0.,1000.);
  AddHistogram(huhpp->GetName(),huhpp);

  TH2D* huhptz = new TH2D("huhptz","Used Helix Momentum;p_{T} [MeV/c];p_{Z} [MeV/c]",
		    100,0.,1000.,200,-1000.,1000.);
  AddHistogram(huhptz->GetName(),huhptz);

  // used helices spacepoints
  TH2D* huhspxy = new TH2D("huhspxy","Spacepoints in Used Helices;x [mm];y [mm]",
		     100,-190.,190.,100,-190.,190.);
  huhspxy->SetStats(kFALSE);
  AddHistogram(huhspxy->GetName(),huhspxy);

  TH2D* huhspzr = new TH2D("huhspzr","Spacepoints in Used Helices;z [mm];r [mm]",
		     600,-1200.,1200.,60,109.,174.);
  huhspzr->SetStats(kFALSE);
  AddHistogram(huhspzr->GetName(),huhspzr);

  TH2D* huhspzp = new TH2D("huhspzp","Spacepoints in Used Helices;z [mm];#phi [deg]",
		     600,-1200.,1200.,180,0.,360.);
  huhspzp->SetStats(kFALSE);
  AddHistogram(huhspzp->GetName(),huhspzp);
  // TH2D* huhsprp = new TH2D("huhsprp","Spacepoints in Used Helices;#phi [deg];r [mm]",
  // 		     180,0.,TMath::TwoPi(),200,108.,175.);
  // huhsprp->SetStats(kFALSE);
  // fHistos.Add(huhsprp);
  // fHistoPositions[huhsprp->GetName()]=fHistos.GetEntries()-1;
}


void TAGPlotTracks::FillHisto(bool applyCuts, int mode)
{
  
   ClearHisto();
   SetUpHistograms();
   
   FillChronoHistograms();
   //Fill Vertex Histograms
   FillVertexHistograms(applyCuts, mode);
   
   FillTrackHisto();
}



void TAGPlotTracks::FillTrackHisto()
{ 
   std::cout<<"TAGPlotTracks::FillTrackHisto() Number of histos: "<<fHistos.GetEntries()<<std::endl;

   for (int i = 0; i < fHelixEvents.size(); i++ )
   {
      FillHistogram("hhD",fHelixEvents.parD[i]);
      FillHistogram("hhRc",fHelixEvents.Curvature[i]);
      FillHistogram("hpt",fHelixEvents.pT[i]);
      FillHistogram("hpz",fHelixEvents.pZ[i]);
      FillHistogram("hpp",fHelixEvents.pTot[i]);
      FillHistogram("hptz",fHelixEvents.pT[i],fHelixEvents.pZ[i]);
   }

   for (int i = 0; i < fSpacePointHelixEvents.size(); i++)
   {
      FillHistogram("hhspxy",fSpacePointHelixEvents.fX[i],fSpacePointHelixEvents.fY[i]);
      FillHistogram("hhspzr",fSpacePointHelixEvents.fZ[i],fSpacePointHelixEvents.fR[i]);
      FillHistogram("hhspzp",fSpacePointHelixEvents.fZ[i],fSpacePointHelixEvents.fP[i]);
   }

   for (int i = 0; i < fUsedHelixEvents.size(); i++)
   {
      FillHistogram("huhD",fUsedHelixEvents.parD[i]);
      FillHistogram("huhRc",fUsedHelixEvents.Curvature[i]);
      FillHistogram("huhpt",fUsedHelixEvents.pT[i]);
      FillHistogram("huhpz",fUsedHelixEvents.pZ[i]);
      FillHistogram("huhpp",fUsedHelixEvents.pTot[i]);
      FillHistogram("huhptz",fUsedHelixEvents.pT[i],fUsedHelixEvents.pZ[i]);
   }

   for (int i = 0; i < fVertexEvents.size(); i++)
   {
      FillHistogram("hNhel", double(fVertexEvents.fNumTracks[i]) );
      FillHistogram("hNusedhel",double(fVertexEvents.fNumHelices[i]));
   }

   for (int i = 0; i < fSpacePointHelixEvents.size(); i++)
   {
      FillHistogram("huhspxy",fSpacePointHelixEvents.fX[i],fSpacePointHelixEvents.fY[i]);
      FillHistogram("huhspzr",fSpacePointHelixEvents.fZ[i],fSpacePointHelixEvents.fR[i]);
      FillHistogram("huhspzp",fSpacePointHelixEvents.fZ[i],fSpacePointHelixEvents.fP[i]);
   }
}


TCanvas* TAGPlotTracks::DrawTrackCanvas(TString Name)
{
   SetupTrackHistos();
   FillTrackHisto();

  TCanvas* ct = new TCanvas(Name,Name,2000,1800);
  ct->Divide(5,4);
  
  ct->cd(1);
  ((TH1D*)fHistos.At(fHistoPositions.at("hNhel")))->Draw("hist");

  ct->cd(2);
  ((TH1D*)fHistos.At(fHistoPositions.at("hhD")))->Draw("hist");
  ct->cd(3);
  //  ((TH1D*)HISTOS.At(fHistoPositions.at("hhc")))->Draw("hist");
  ((TH1D*)fHistos.At(fHistoPositions.at("hhRc")))->Draw("hist");
  ct->cd(4);
  ((TH1D*)fHistos.At(fHistoPositions.at("hpt")))->Draw("hist");
  ct->cd(5);
  ((TH1D*)fHistos.At(fHistoPositions.at("hpz")))->Draw("hist");
  ct->cd(6);
  ((TH1D*)fHistos.At(fHistoPositions.at("hpp")))->Draw("hist");
  ct->cd(7);
  ((TH2D*)fHistos.At(fHistoPositions.at("hptz")))->Draw("colz");

  ct->cd(8);
  ((TH2D*)fHistos.At(fHistoPositions.at("hhspxy")))->Draw("colz");
  ct->cd(9);
  ((TH2D*)fHistos.At(fHistoPositions.at("hhspzr")))->Draw("colz");
  ct->cd(10);
  ((TH2D*)fHistos.At(fHistoPositions.at("hhspzp")))->Draw("colz");
  //  ((TH2D*)fHistos.At(fHistoPositions.at("hhsprp")))->Draw("colz");
  
  ct->cd(11);
  ((TH1D*)fHistos.At(fHistoPositions.at("hNusedhel")))->Draw("hist");

  ct->cd(12);
  ((TH1D*)fHistos.At(fHistoPositions.at("huhD")))->Draw("hist");
  ct->cd(13);
  //((TH1D*)fHistos.At(fHistoPositions.at("huhc")))->Draw("hist");
  ((TH1D*)fHistos.At(fHistoPositions.at("huhRc")))->Draw("hist");
  ct->cd(14);
  ((TH1D*)fHistos.At(fHistoPositions.at("huhpt")))->Draw("hist");
  ct->cd(15);
  ((TH1D*)fHistos.At(fHistoPositions.at("huhpz")))->Draw("hist");
  ct->cd(16);
  ((TH1D*)fHistos.At(fHistoPositions.at("huhpp")))->Draw("hist");
  ct->cd(17);
  ((TH2D*)fHistos.At(fHistoPositions.at("huhptz")))->Draw("colz");

  ct->cd(18);
  ((TH2D*)fHistos.At(fHistoPositions.at("huhspxy")))->Draw("colz");
  ct->cd(19);
  ((TH2D*)fHistos.At(fHistoPositions.at("huhspzr")))->Draw("colz");
  ct->cd(20);
  ((TH2D*)fHistos.At(fHistoPositions.at("huhspzp")))->Draw("colz");
  //  ((TH2D*)fHistos.At(fHistoPositions.at("huhsprp")))->Draw("colz");
  
  return ct;
}


void TAGPlotTracks::ProcessHelices(const double runNumber, const double time, const double officialtime, const TObjArray* tracks)
{
  const int Nhelices = tracks->GetEntries();
  for(int i=0; i<Nhelices; ++i)
  {
      const TStoreHelix* aHelix = (TStoreHelix*) tracks->At(i);
      fHelixEvents.AddEvent(
        runNumber,
        time,
        officialtime,
        aHelix->GetMomentumV().Perp(),
        aHelix->GetMomentumV().Z(),
        aHelix->GetMomentumV().Mag(),
        aHelix->GetD(),
        aHelix->GetRc(),
        aHelix->GetNumberOfPoints()
      );

      const TObjArray* points = aHelix->GetSpacePoints();
      for(int ip = 0; ip<points->GetEntries(); ++ip  )
	    {
         const TSpacePoint* ap = (TSpacePoint*) points->At(ip);
         fSpacePointHelixEvents.AddEvent(
           runNumber,
           time,
           officialtime,
           ap->GetX(),
           ap->GetY(),
           ap->GetZ(),
           ap->GetR(),
           ap->GetPhi()*TMath::RadToDeg()
         );
      }
   }
}

void TAGPlotTracks::ProcessUsedHelices(const double runNumber, const double time, const double officialtime, const TObjArray* tracks)
{
  const int Nhelices = tracks->GetEntries();
  for(int i=0; i<Nhelices; ++i)
  {
      const TStoreHelix* aHelix = (TStoreHelix*) tracks->At(i);
      fUsedHelixEvents.AddEvent(
        runNumber,
        time,
        officialtime,
        aHelix->GetMomentumV().Perp(),
        aHelix->GetMomentumV().Z(),
        aHelix->GetMomentumV().Mag(),
        aHelix->GetD(),
        aHelix->GetRc(),
        aHelix->GetNumberOfPoints()
      );

      const TObjArray* points = aHelix->GetSpacePoints();
      for(int ip = 0; ip<points->GetEntries(); ++ip  )
	    {
         const TSpacePoint* ap = (TSpacePoint*) points->At(ip);
         fSpacePointUsedHelixEvents.AddEvent(
           runNumber,
           time,
           officialtime,
           ap->GetX(),
           ap->GetY(),
           ap->GetZ(),
           ap->GetR(),
           ap->GetPhi()*TMath::RadToDeg()
         );
      }
   }
}



#endif