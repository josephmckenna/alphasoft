//
// Process Events
// (with one-click)
// 
// Author: A. Capra
// Date: June 2020
//

#include "ProcessEvents.hh"
#include <iostream>

#include "TFitVertex.hh"

ProcessEvents::ProcessEvents( AnaSettings* a, double B, 
                              std::string f, bool issim ):d(a),leaw(),lepad(),
                                                          m(a),r(a,B),rMC(a,B),
                                                          u(f,B),kFinder(adaptive),
                                                          EventNo(-1),kDraw(false),
                                                          kVerb(0)
{
   if( issim )
      {
         d.Setup();
         d.SetPWBdelay(50.);
         d.SetPedestalLength(0);
      }
   //d.SetTrace(true);
   std::cout<<"--------------------------------------------------"<<std::endl;
   std::cout<<"[proc]# Deconv Settings"<<std::endl;
   d.PrintADCsettings();
   d.PrintPWBsettings();
   std::cout<<"--------------------------------------------------"<<std::endl;


   m.SetDiagnostic(true);
   if( issim ) m.Setup(0);

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
   else
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
   int nsig = d.FindAnodeTimes( awsignals );
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes: "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( d.GetAnodeSignal() );

   // pad deconv
   nsig = d.FindPadTimes( padsignals );
   std::cout<<"[proc]# "<<EventNo<<"\tFindPadTimes: "<<nsig<<std::endl;
   m.Init();
   if( nsig == 0 ) return;
   if( nsig > 70000 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( d.GetPadSignal() );
         
   // combine pads
 
   if(kVerb>=2) m.SetTrace(true);
   m.CombinePads( d.GetPadSignal() );
   m.SetTrace(false);
   uint npads = m.GetCombinedPads()->size();
   std::cout<<"[proc]# "<<EventNo<<"\tCombinePads: "<<npads<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
               
   if( kVerb>=2 ) u.PrintSignals( m.GetCombinedPads() );
               
   if( kDraw ) u.Draw(d.GetAnodeSignal(),d.GetPadSignal(),m.GetCombinedPads(),false);
               
   if( npads == 0 ) return;

   // match electrodes
   m.MatchElectrodes( d.GetAnodeSignal() );

   ProcessPoints();
}

void ProcessEvents::ProcessWaveform_2D(TClonesArray* awsignals)
{
   // anode deconv
   int nsig = d.FindAnodeTimes( awsignals );
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes: "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if( kVerb>=2 ) u.PrintSignals( d.GetAnodeSignal() );

   m.Init();
   m.FakePads( d.GetAnodeSignal() );

   ProcessPoints();
}

void ProcessEvents::ProcessPoints()
{
   uint nmatch = m.GetSpacePoints()->size();
   std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<nmatch<<std::endl;
   if( nmatch == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   // combine points
   m.CombPoints();
   uint nsp = m.GetSpacePoints()->size();
   std::cout<<"[proc]# "<<EventNo<<"\tCombinePoints: "<<nsp<<std::endl;
   if( nsp == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
}

void ProcessEvents::ProcessWaveform_led(TClonesArray* awsignals, TClonesArray* padsignals)
{
   int nsig = leaw.FindAnodeTimes(awsignals);
   std::cout<<"[proc]# "<<EventNo<<"\tFindAnodeTimes (led): "<<nsig<<std::endl;
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   nsig = lepad.FindPadTimes(padsignals);
   std::cout<<"[proc]# "<<EventNo<<"\tFindPadTimes (led): "<<nsig<<std::endl;        
   if( kDraw ) u.Draw(leaw.GetSignal(), lepad.GetSignal());
   m.Init();
   if( nsig == 0 ) return;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //m.SetTrace(true);
   m.MatchElectrodes( leaw.GetSignal(), lepad.GetSignal() );
   if( m.GetSpacePoints() )
      std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: "<<m.GetSpacePoints()->size()<<std::endl;
   else
      std::cout<<"[proc]# "<<EventNo<<"\tMatchElectrodes: No Spacepoints..."<<std::endl;
   //m.SetTrace(false);
}


void ProcessEvents::ProcessTracks()
{
   // reco points
   //   if( kVerb>=2 ) 
   //r.SetTrace(true);
   if( m.GetSpacePoints() ) 
      r.AddSpacePoint( m.GetSpacePoints() );
   //  else return;
   std::cout<<"[proc]# "<<EventNo<<"\tspacepoints: "<<r.GetNumberOfPoints()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   // find tracks
   //r.SetTrace(true);
   int ntracks = r.FindTracks(kFinder);
   std::cout<<"[proc]# "<<EventNo<<"\tpattrec: "<<ntracks<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   if(kFinder == neural) 
      u.DebugNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
         
   r.PrintPattRec();
   std::cout<<"[proc]# "<<EventNo<<"\ttracks: "<<r.GetNumberOfTracks()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   //r.SetTrace( true );
   int nlin = r.FitLines();
   std::cout<<"[proc]# "<<EventNo<<"\tline: "<<nlin<<std::endl;
   //r.SetTrace(true);
   int nhel = r.FitHelix();
   r.SetTrace(false);
   std::cout<<"[proc]# "<<EventNo<<"\thelix: "<<nhel<<std::endl;
   u.HelixPlots( r.GetHelices() );
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   //r.SetTrace( false );
}

void ProcessEvents::ProcessVertex(TVector3* mcvtx)
{ 
   TFitVertex Vertex(EventNo);
   int sv = r.RecVertex(&Vertex);
   std::cout<<"[proc]# "<<EventNo<<"\t";
   if( sv > 0 ) Vertex.Print();
   else 
      {
         std::cout<<"No Vertex"<<std::endl;
         return;
      }

   std::cout<<"[proc]# "<<EventNo<<"\tMCvertex: "; 
   mcvtx->Print();

   double res = kUnknown;
   if( sv > 0 ) 
      { 
         res = u.VertexResolution(Vertex.GetVertex(),mcvtx);
         u.VertexPlots(&Vertex);
      }
   else res = u.PointResolution(r.GetHelices(),mcvtx);

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
                     
   rMC.AddMChits( aw_hits );
   std::cout<<"[proc]# "<<EventNo<<"\tMC spacepoints: "<<rMC.GetNumberOfPoints()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
   // find tracks
   int ntracksMC = rMC.FindTracks(kFinder);
   std::cout<<"[proc]# "<<EventNo<<"\tMCpattrec: "<<ntracksMC<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
   if(kFinder == neural) 
      u.DebugNeuralNet( (NeuralFinder*) rMC.GetTracksFinder() );
                     
   std::cout<<"[proc]# "<<EventNo<<"\tMC tracks: "<<rMC.GetNumberOfTracks()<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
   rMC.SetTrace( true );
   int nlin = rMC.FitLines();
   std::cout<<"[proc]# "<<EventNo<<"\tline: "<<nlin<<std::endl;
   int nhel = rMC.FitHelix();
   std::cout<<"[proc]# "<<EventNo<<"\tMC helix: "<<nhel<<std::endl;
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   rMC.SetTrace( false );
                     
   TFitVertex MCVertex(EventNo);
   int svMC = r.RecVertex(&MCVertex);
   if( svMC > 0 ) MCVertex.Print();
                     
   double res = u.PointResolution(rMC.GetHelices(),mcvtx);
   std::cout<<"[proc]# "<<EventNo<<"\tMC Resolution: ";
   auto prec = std::cout.precision();
   std::cout.precision(2);
   std::cout<<res<<" mm"<<std::endl;
   std::cout.precision(prec);
   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                     
   rMC.Reset();
}


void ProcessEvents::Finish()
{
   std::cout<<"[proc]# "<<EventNo<<"\tProcessEvents::Finish()"<<std::endl;
   if( kDraw )
      {
         u.Display(r.GetPoints(), r.GetTracks(), r.GetHelices());
         if(kFinder == neural) 
            u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
      }
   r.Reset();
}

void ProcessEvents::Finish(TClonesArray* garfpp_hits, TClonesArray* aw_hits)
{
   if( kDraw )
      {
         u.Display(garfpp_hits, aw_hits, r.GetPoints(), r.GetTracks(), r.GetHelices());
         if(kFinder == neural) 
            u.DisplayNeuralNet( (NeuralFinder*) r.GetTracksFinder() );
      }

   r.Reset();
}



/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
