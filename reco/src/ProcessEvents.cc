//
// Process Events
// (with one-click)
// 
// Author: A. Capra
// Date: June 2020
//

#include "ProcessEvents.hh"
#include <iostream>
#include <mutex>          // std::mutex

#include "TFitVertex.hh"

ProcessEvents::ProcessEvents( AnaSettings* a, double B, 
                              std::string f, bool issim ):dAW(a),dPad(a),leaw(),lepad(),m(a),
                                                          r(a,B,"CERN", false),rMC(a,B,"CERN", false),
                                                          u(f,B),kFinder(adaptive),
                                                          EventNo(-1),kDraw(false),
                                                          kVerb(0)
{
   if( issim )
      {
         dAW.Setup();
         dPad.Setup();
         dPad.SetPWBdelay(50.);
         dAW.SetPedestalLength(0);
         dPad.SetPedestalLength(0);
      }
   //d.SetTrace(true);
   std::cout<<"--------------------------------------------------"<<std::endl;
   std::cout<<"[proc]# Deconv Settings"<<std::endl;
   dAW.PrintADCsettings();
   dPad.PrintPWBsettings();
   std::cout<<"--------------------------------------------------"<<std::endl;

   std::mutex* Lock = new std::mutex();
   m.SetDiagnostic(true);
   if( issim )
      {
         m.Setup(0);
         m.SetMultiThread(true);
         m.SetGlobalLockVariable(Lock);
      }

   //leaw.SetDebug();
   leaw.SetRMSBaselineCut( a->GetDouble("LEModule","ADCrms") );
   leaw.SetPulseHeightThreshold( a->GetDouble("LEModule","ADCthr") );
   leaw.SetCFDfraction( a->GetDouble("LEModule","CFDfrac") );
   //leaw.SetTimeOffset( a->GetDouble("LEModule","ADCtime") );
   leaw.SetGain( a->GetDouble("LEModule","ADCgain") );

   //lepad.SetDebug();
   lepad.SetRMSBaselineCut( a->GetDouble("LEModule","PWBrms") );
   lepad.SetPulseHeightThreshold( a->GetDouble("LEModule","PWBthr") );
   lepad.SetCFDfraction( a->GetDouble("LEModule","CFDfrac") );
   //lepad.SetTimeOffset( a->GetDouble("LEModule","PWBtime") );
   lepad.SetGain( a->GetDouble("LEModule","PWBgain") );

   if( issim )
      {
         leaw.SetPedestalLength(0);
         lepad.SetPedestalLength(0);
         u.BookG4Histos();
         u.BookAGG4Histos();
      }
      u.BookRecoHistos();

   TObjString sett = a->GetSettingsString();
   u.WriteSettings(&sett);
}

void ProcessEvents::SetDraw()
{
   kDraw=true;
   u.MakeCanvases();
}


void ProcessEvents::ProcessWaveform_deconv(TClonesArray* awsignals, TClonesArray* padsignals)
{
   // anode deconv
   std::vector<ALPHAg::TWireSignal> awtimes;
   int nsig = dAW.FindAnodeTimes( awsignals, awtimes );
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes: "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( &awtimes );

   // pad deconv
   std::vector<ALPHAg::TPadSignal> padtimes;
   nsig = dPad.FindPadTimes( padsignals, padtimes );
   std::cout<<"[proc]# "<<EventNo<<"\tFindPadTimes: "<<nsig<<std::endl;
   m.Init();
   if( nsig == 0 ) return;
   if( nsig > 70000 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( &padtimes );
         
   // combine pads
   if(kVerb>=2) m.SetTrace(true);
   std::vector<ALPHAg::TPadSignal> CombinedPads = m.CombinePads( padtimes );
   m.SetTrace(false);
   uint npads = CombinedPads.size();
   std::cout<<"[proc]# "<<EventNo<<"\tCombinePads: "<<npads<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               
   if( kVerb>=2 ) u.PrintSignals( &CombinedPads );
               
   if( kDraw ) u.Draw(&awtimes, &padtimes, &CombinedPads,false);
               
   if( npads == 0 ) return;

   // match electrodes
   std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > spacepoints = m.MatchElectrodes( awtimes, CombinedPads );
   uint nmatch = spacepoints.size();
   std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<nmatch<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //ProcessPoints(spacepoints);
   ProcessTracks(&spacepoints);
}

void ProcessEvents::ProcessWaveform_2D(TClonesArray* awsignals)
{
   // anode deconv
   std::vector<ALPHAg::TWireSignal> awtimes;
   int nsig = dAW.FindAnodeTimes( awsignals, awtimes );
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes: "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( &awtimes );

   m.Init();
   std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > spacepoints = m.FakePads( awtimes );
   uint nmatch = spacepoints.size();
   std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<nmatch<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //   ProcessPoints(spacepoints);
   ProcessTracks(&spacepoints);
}

void ProcessEvents::ProcessWaveform_led(TClonesArray* awsignals, TClonesArray* padsignals)
{
   int nsig = leaw.FindAnodeTimes(awsignals);
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes (led): "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   nsig = lepad.FindPadTimes(padsignals);
   std::cout<<"[proc]# "<<EventNo<<"\tFindPadTimes (led): "<<nsig<<std::endl;
   std::vector<ALPHAg::TWireSignal> awtimes = leaw.GetWireSignal();
   std::vector<ALPHAg::TPadSignal> padtimes = lepad.GetPadSignal();
   if( kDraw ) u.Draw(&awtimes, &padtimes);
   m.Init();
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //m.SetTrace(true);
   std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> > spacepoints =m.MatchElectrodes( awtimes, padtimes );
   if( spacepoints.size() )
      std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<spacepoints.size()<<std::endl;
   else
      std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: No Spacepoints..."<<std::endl;
   //m.SetTrace(false);

   uint nmatch = spacepoints.size();
   std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<nmatch<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   if( nmatch == 0 ) return;

   ProcessTracks(&spacepoints);
}

void ProcessEvents::ProcessPoints(std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> >* spacepoints )
{
   uint nmatch = spacepoints->size();
   std::cout<<"[proc]# "<<EventNo<<"\tSpacepoints to Process: "<<nmatch<<std::endl;
   if( nmatch == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   // combine points
   *spacepoints = m.CombPoints( *spacepoints );
   uint nsp = spacepoints->size();
   std::cout<<"[proc]# "<<EventNo<<"\tCombinePoints: "<<nsp<<std::endl;
   if( nsp == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
}

void ProcessEvents::ProcessTracks(std::vector< std::pair<ALPHAg::TWireSignal,ALPHAg::TPadSignal> >* spacepoints)
{
   // if( kVerb>=2 ) 
   //    r.SetTrace(true);
   if( spacepoints )  // reco points
      r.BuildSpacePointArray( *spacepoints, PointsArray);
   else return;
   std::cout<<"[proc]# "<<EventNo<<"\tspacepoints: "<<PointsArray.size()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   u.FillRecoPointsHistos( &PointsArray );

   // find tracks
   //r.SetTrace(true);
   std::vector<track_t> TrackVector;
   int ntracks = r.FindTracks(PointsArray, TrackVector, kFinder);
   std::cout<<"[proc]# "<<EventNo<<"\tpattrec: "<<ntracks<<std::endl;
   r.BuildTracks(TrackVector, PointsArray, TracksArray);
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   u.FillRecoTracksHisto( &TracksArray );

   // FIXME: TracksFinder is private right now
   // if(kFinder == neural) 
   //    u.DebugNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
         
   r.PrintPattRec();
   std::cout<<"[proc]# "<<EventNo<<"\ttracks: "<<TracksArray.size()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //r.SetTrace( true );
   int nlin = r.FitLine(TracksArray, LineArray);   // no MT for now
   std::cout<<"[proc]# "<<EventNo<<"\tline: "<<nlin<<std::endl;
   //r.SetTrace(true);
   int nhel = r.FitHelix(TracksArray, HelixArray); // no MT for now
   // r.SetTrace(false);
   std::cout<<"[proc]# "<<EventNo<<"\thelix: "<<nhel<<std::endl;
   u.HelixPlots( &HelixArray );
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( nhel > 0 ) 
      u.FillFitTracksHisto(&HelixArray);
   else if( nlin > 0 )
      u.FillFitTracksHisto(&LineArray);
}

void ProcessEvents::ProcessVertex(TVector3* mcvtx)
{ 
   TFitVertex Vertex(EventNo);
   int sv = r.RecVertex(HelixArray, &Vertex);
   std::cout<<"[proc]# "<<EventNo<<"\t";
   double res = ALPHAg::kUnknown;
   if( sv > 0 )
      {
         Vertex.Print();
         u.FillRecoVertex(&Vertex);
         res = u.VertexResolution(Vertex.GetVertex(),mcvtx);
         u.VertexPlots(&Vertex);
      }
   else 
      {
         std::cout<<"No Vertex"<<std::endl;
         res = u.PointResolution(&HelixArray,mcvtx);
         //  return;
      }

   // std::cout<<"[proc]# "<<EventNo<<"\tMCvertex: "; 
   // mcvtx->Print();

   std::cout<<"[proc]# "<<EventNo<<"\tResolution: ";        
   auto prec = std::cout.precision();
   std::cout.precision(2);
   std::cout<<res<<" mm"<<std::endl;
   std::cout.precision(prec);
   // std::cout<<"[proc]# "<<i<<"\tUsedHelixPlots: "
   //     <<Vertex.GetHelixStack()->GetEntriesFast()<<std::endl;
   u.UsedHelixPlots( Vertex.GetHelixStack() );
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
}

void ProcessEvents::ProcessMonteCarlo(TClonesArray* aw_hits,TVector3* mcvtx)
{
   //================================================================
   // MC hits reco
   std::cout<<"[proc]# "<<EventNo<<"\tMC reco"<<std::endl;

   std::vector<TSpacePoint> MCPointsArray;
   rMC.BuildMCSpacePointArray(aw_hits, MCPointsArray);
   std::cout<<"[proc]# "<<EventNo<<"\tMC spacepoints: "<<MCPointsArray.size()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
   // find tracks
   std::vector<track_t> TrackVector;
   int ntracksMC = rMC.FindTracks(MCPointsArray, TrackVector, kFinder);
   std::cout<<"[proc]# "<<EventNo<<"\tMCpattrec: "<<ntracksMC<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   // FIXME: TracksFinder is private                     
   // if(kFinder == neural) 
   //    u.DebugNeuralNet( (NeuralFinder*) rMC.GetTracksFinder() );
                     
   std::cout<<"[proc]# "<<EventNo<<"\tMC tracks: "<<TrackVector.size()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   std::vector<TTrack> MCTracksArray;
   rMC.BuildTracks(TrackVector, MCPointsArray, MCTracksArray);

   std::vector<TFitLine> MCLineArray;
   std::vector<TFitHelix> MCHelixArray;
 
   // rMC.SetTrace( true );
   int nlin = rMC.FitLine(MCTracksArray, MCLineArray);
   std::cout<<"[proc]# "<<EventNo<<"\tline: "<<nlin<<std::endl;
   int nhel = rMC.FitHelix(MCTracksArray, MCHelixArray);
   std::cout<<"[proc]# "<<EventNo<<"\tMC helix: "<<nhel<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   // rMC.SetTrace( false );
                     
   TFitVertex MCVertex(EventNo);
   int svMC = r.RecVertex(MCHelixArray, &MCVertex);
   if( svMC > 0 ) MCVertex.Print();
                     
   double res = u.PointResolution(&MCHelixArray,mcvtx);
   std::cout<<"[proc]# "<<EventNo<<"\tMC Resolution: ";
   auto prec = std::cout.precision();
   std::cout.precision(2);
   std::cout<<res<<" mm"<<std::endl;
   std::cout.precision(prec);
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
}


void ProcessEvents::Finish()
{
   //   std::cout<<"[proc]# "<<EventNo<<"\tProcessEvents::Finish()"<<std::endl;
   if( kDraw )
      {
         u.Display(&PointsArray, &TracksArray, &HelixArray);
         // FIXME: TracksFinder is private
         // if(kFinder == neural) 
         //    u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
      }
   PointsArray.clear();
   TracksArray.clear();
   HelixArray.clear();
   LineArray.clear();
}

void ProcessEvents::Finish(TClonesArray* garfpp_hits, TClonesArray* aw_hits)
{
   if( kDraw )
      {
         u.Display(garfpp_hits, aw_hits, &PointsArray, &TracksArray, &HelixArray);
         // FIXME: TracksFinder is private
         // if(kFinder == neural) 
         //    u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
      }
   PointsArray.clear();
   TracksArray.clear();
   HelixArray.clear();
   LineArray.clear();
}

TStoreEvent ProcessEvents::GetStoreEvent()
{
   TStoreEvent sevt = u.CreateStoreEvent(&PointsArray, &HelixArray, &LineArray);
   return sevt;
}
void ProcessEvents::End()
{
   u.WriteHisto();
}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
