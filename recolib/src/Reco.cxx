#include "Reco.hh"

#include "TPCconstants.hh"

#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "TMChit.hh"

#include <TDirectory.h>

Reco::Reco(std::string json, double B):fTrace(false),fMagneticField(B),
                                       f_rfudge(1.),f_pfudge(1.),
                                       pattrec(0)
{
   std::cout<<"Reco ctor! (1)"<<std::endl;
   ana_settings = new AnaSettings(json.c_str());
   ana_settings->Print();

   fNspacepointsCut = ana_settings->GetInt("RecoModule","NpointsCut");
   fPointsDistCut = ana_settings->GetDouble("RecoModule","PointsDistCut");
   fMaxIncreseAdapt = ana_settings->GetDouble("RecoModule","MaxIncreseAdapt");
   fSeedRadCut = ana_settings->GetDouble("RecoModule","SeedRadCut");
   fSmallRadCut = ana_settings->GetDouble("RecoModule","SmallRadCut");
   fLastPointRadCut = ana_settings->GetDouble("RecoModule","LastPointRadCut");
   fLineChi2Cut = ana_settings->GetDouble("RecoModule","LineChi2Cut");
   fLineChi2Min = ana_settings->GetDouble("RecoModule","LineChi2Min");;
   fHelChi2RCut = ana_settings->GetDouble("RecoModule","HelChi2RCut");
   fHelChi2ZCut = ana_settings->GetDouble("RecoModule","HelChi2ZCut");
   fHelChi2RMin = ana_settings->GetDouble("RecoModule","HelChi2RMin");
   fHelChi2ZMin = ana_settings->GetDouble("RecoModule","HelChi2ZMin");
   fHelDcut = ana_settings->GetDouble("RecoModule","HelDcut");
   fVtxChi2Cut = ana_settings->GetDouble("RecoModule","VtxChi2Cut");

   fLambda = ana_settings->GetDouble("RecoModule","Lambda_NN");
   fAlpha = ana_settings->GetDouble("RecoModule","Alpha_NN");
   fB = ana_settings->GetDouble("RecoModule","B_NN");
   fTemp = ana_settings->GetDouble("RecoModule","T_NN");
   fC = ana_settings->GetDouble("RecoModule","C_NN");
   fMu = ana_settings->GetDouble("RecoModule","Mu_NN");
   fCosCut = ana_settings->GetDouble("RecoModule","CosCut_NN");
   fVThres = ana_settings->GetDouble("RecoModule","VThres_NN");

   fDNormXY = ana_settings->GetDouble("RecoModule","DNormXY_NN");
   fDNormZ = ana_settings->GetDouble("RecoModule","DNormZ_NN");

   fTscale = ana_settings->GetDouble("RecoModule","TScale_NN");
   fMaxIt = ana_settings->GetInt("RecoModule","MaxIt_NN");
   fItThres = ana_settings->GetDouble("RecoModule","ItThres_NN");

   if( fMagneticField < 0. )
      fSTR = new LookUpTable(_co2frac); // field map version (simulation)
   else
      fSTR = new LookUpTable(_co2frac, fMagneticField); // uniform field version (simulation)
   std::cout<<"Reco::Reco()  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
   
   // std::cout<<"Reco::Reco() Saving AnaSettings to rootfile... ";
   // TObjString sett = ana_settings->GetSettingsString();
   // int bytes_written = gDirectory->WriteTObject(&sett,"ana_settings");
   // if( bytes_written > 0 )
   //    std::cout<<" DONE ("<<bytes_written<<")"<<std::endl;
   // else
   //    std::cout<<" FAILED"<<std::endl;

   track_not_advancing = 0;
   points_cut = 0;
   rad_cut = 0;
}

Reco::Reco(AnaSettings* ana_set, double B):fTrace(false),fMagneticField(B),
                                           ana_settings(ana_set),
                                           f_rfudge(1.),f_pfudge(1.),
                                           pattrec(0)
{
   std::cout<<"Reco ctor! (2)"<<std::endl;
   ana_settings->Print();

   //   fNhitsCut = ana_settings->GetInt("RecoModule","NhitsCut");
   fNspacepointsCut = ana_settings->GetInt("RecoModule","NpointsCut");
   fPointsDistCut = ana_settings->GetDouble("RecoModule","PointsDistCut");
   fMaxIncreseAdapt = ana_settings->GetDouble("RecoModule","MaxIncreseAdapt");
   fSeedRadCut = ana_settings->GetDouble("RecoModule","SeedRadCut");
   fSmallRadCut = ana_settings->GetDouble("RecoModule","SmallRadCut");
   fLastPointRadCut = ana_settings->GetDouble("RecoModule","LastPointRadCut");
   fLineChi2Cut = ana_settings->GetDouble("RecoModule","LineChi2Cut");
   fLineChi2Min = ana_settings->GetDouble("RecoModule","LineChi2Min");;
   fHelChi2RCut = ana_settings->GetDouble("RecoModule","HelChi2RCut");
   fHelChi2ZCut = ana_settings->GetDouble("RecoModule","HelChi2ZCut");
   fHelChi2RMin = ana_settings->GetDouble("RecoModule","HelChi2RMin");
   fHelChi2ZMin = ana_settings->GetDouble("RecoModule","HelChi2ZMin");
   fHelDcut = ana_settings->GetDouble("RecoModule","HelDcut");
   fVtxChi2Cut = ana_settings->GetDouble("RecoModule","VtxChi2Cut");

   fLambda = ana_settings->GetDouble("RecoModule","Lambda_NN");
   fAlpha = ana_settings->GetDouble("RecoModule","Alpha_NN");
   fB = ana_settings->GetDouble("RecoModule","B_NN");
   fTemp = ana_settings->GetDouble("RecoModule","T_NN");
   fC = ana_settings->GetDouble("RecoModule","C_NN");
   fMu = ana_settings->GetDouble("RecoModule","Mu_NN");
   fCosCut = ana_settings->GetDouble("RecoModule","CosCut_NN");
   fVThres = ana_settings->GetDouble("RecoModule","VThres_NN");

   fDNormXY = ana_settings->GetDouble("RecoModule","DNormXY_NN");
   fDNormZ = ana_settings->GetDouble("RecoModule","DNormZ_NN");

   fTscale = ana_settings->GetDouble("RecoModule","TScale_NN");
   fMaxIt = ana_settings->GetInt("RecoModule","MaxIt_NN");
   fItThres = ana_settings->GetDouble("RecoModule","ItThres_NN");

   if( fMagneticField < 0. ) // garfield++ sim with field map
      {
         fSTR = new LookUpTable(_co2frac); // field map version (simulation)
         fMagneticField = 1.;
      }
   else
      fSTR = new LookUpTable(_co2frac, fMagneticField); // uniform field version (simulation)
   std::cout<<"Reco::Reco()  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
   
   track_not_advancing = 0;
   points_cut = 0;
   rad_cut = 0;
}

void Reco::Setup(TFile* OutputFile)
{
   OutputFile->cd(); // select correct ROOT directory
   gDirectory->mkdir("reco")->cd();
   hsprp = new TH2D("hsprp","Spacepoints #phi-R in Tracks;#phi [deg];r [mm]",
                    180,0.,360.,200,109.,175.);
   hspxy = new TH2D("hspxy","Spacepoint X-Y for Tracks;x [mm];y [mm]",100,-190.,190.,100,-190.,190.);
   hspzp = new TH2D("hspzp","Spacepoint Axial-Azimuth for Tracks;z [mm];#phi [deg]",500,-1152.,1152.,180,0.,360.);
   hspaw = new TH1D("hOccAw","Aw Occupancy in Tracks;aw",256,-0.5,255.5);
   hchi2 = new TH1D("hchi2","#chi^{2} of Straight Lines",100,0.,200.);
   hchi2sp = new TH2D("hchi2sp","#chi^{2} of Straight Lines Vs Number of Spacepoints",
                               100,0.,200.,100,0.,100.);
}

void Reco::UseSTRfromData(int runNumber)
{
   delete fSTR;
   std::cout<<"Reco:::UseSTRfromData( "<<runNumber<<" )"<<std::endl;
   fSTR = new LookUpTable(runNumber);
}

Reco::~Reco()
{
   delete ana_settings;
   //if(pattrec) delete pattrec;
   delete fSTR;
}

void Reco::AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints )
{
   int n = 0;
   for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
      {
         // STR: (t,z)->(r,phi)
         const double time = sp->first.t, zed = sp->second.z;
         double r = fSTR->GetRadius( time , zed ),
            correction = fSTR->GetAzimuth( time , zed ),
            err = fSTR->GetdRdt( time , zed ),
            erp = fSTR->GetdPhidt( time , zed );
         
         r*=f_rfudge;
         correction*=f_pfudge;

         if( fTrace )
            {
               double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
               std::cout<<"Reco::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                        <<" t: "<<time<<" r: "<<r
                        <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                        <<" ~ "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
               //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
            }
         TSpacePoint* point=new TSpacePoint();
         point->Setup(sp->first.idx,
                      sp->second.sec,sp->second.idx,
                      time,
                      r,correction,zed,
                      err,erp,sp->second.errz,
                      sp->first.height);
         fPointsArray.push_back(point);
         ++n;
      }
   //fPointsArray.Compress();
   //fPointsArray.Sort();
   TSeqCollection::QSort((TObject**)fPointsArray.data(),0,fPointsArray.size());
   if( fTrace )
      std::cout<<"Reco::AddSpacePoint # entries: "<<fPointsArray.size()<<std::endl;
}

void Reco::AddSpacePoint( std::vector< std::pair<signal,signal> > *spacepoints, double z_fid )
{
   int n = 0;
   for( auto sp=spacepoints->begin(); sp!=spacepoints->end(); ++sp )
      {
         // STR: (t,z)->(r,phi)
         const double time = sp->first.t, zed = sp->second.z;
         // skip over points outside fiducial region
         if( fabs(zed) > z_fid ) continue;

         double r = fSTR->GetRadius( time , zed ),
            correction = fSTR->GetAzimuth( time , zed ),
            err = fSTR->GetdRdt( time , zed ),
            erp = fSTR->GetdPhidt( time , zed );
         
         r*=f_rfudge;
         correction*=f_pfudge;

         if( fTrace )
            {
               double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
               std::cout<<"Reco::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                        <<" t: "<<time<<" r: "<<r
                        <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                        <<" ~ "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
               //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
            }
         TSpacePoint* point=new TSpacePoint();
         point->Setup(sp->first.idx,
                      sp->second.sec,sp->second.idx,
                      time,
                      r,correction,zed,
                      err,erp,sp->second.errz,
                      sp->first.height);
         fPointsArray.push_back(point);
         ++n;
      }
   //fPointsArray.Compress();
   //fPointsArray.Sort();
   TSeqCollection::QSort((TObject**)fPointsArray.data(),0,fPointsArray.size());
   if( fTrace )
      std::cout<<"Reco::AddSpacePoint # entries: "<<fPointsArray.size()<<std::endl;
}

void Reco::AddSpacePoint( const TObjArray* p )
{
   for( int n=0; n<p->GetEntriesFast(); ++n )
      {
         //new(fPointsArray[n]) TSpacePoint(*(TSpacePoint*)p->At(n));
         //fPointsArray.push_back( (TSpacePoint*)p->At(n) );
         TSpacePoint* point=new TSpacePoint(*(TSpacePoint*)p->At(n));
         //point->Print("rphi");
         fPointsArray.push_back( point );
      }
   if( fTrace )
      std::cout<<"Reco::AddSpacePoint # entries: "<<fPointsArray.size()<<std::endl;
}

void Reco::AddMChits( const TClonesArray* points )
{
   int Npoints = points->GetEntries();
   for( int j=0; j<Npoints; ++j )
      {
         TMChit* h = (TMChit*) points->At(j);
         double time = h->GetTime(),
            zed = h->GetZ();

         double rad = fSTR->GetRadius( time , zed ), lor = fSTR->GetAzimuth( time , zed ),
            err = fSTR->GetdRdt( time , zed ), erp = fSTR->GetdPhidt( time , zed );

         double phi = h->GetPhi() - lor;
         if( phi < 0. ) phi += TMath::TwoPi();
         if( phi >= TMath::TwoPi() )
            phi = fmod(phi,TMath::TwoPi());
         int aw = phi/_anodepitch - 0.5;
         int sec = int( phi/(2.*M_PI)*_padcol ),
            row = int( zed/_halflength*0.5*_padrow );

         //      double y = rad*TMath::Sin( phi ), x = rad*TMath::Cos( phi );

         double erz = 1.5;
         TSpacePoint* point=new TSpacePoint();
         point->Setup( aw, sec, row, time,
                       rad, lor, zed,
                       err, erp, erz,
                       h->GetDepositEnergy() );
         point->SetTrackID(h->GetTrackID());
         point->SetTrackPDG(h->GetTrackPDG());
         fPointsArray.push_back(point);
      }
}

int Reco::FindTracks(finderChoice finder)
{
   switch(finder)
      {
      case adaptive:
         pattrec = new AdaptiveFinder( &fPointsArray );
         ((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(fMaxIncreseAdapt);
         ((AdaptiveFinder*)pattrec)->SetLastPointRadCut(fLastPointRadCut);
         break;
      case neural:
         pattrec = new NeuralFinder( &fPointsArray );
         ((NeuralFinder*)pattrec)->SetLambda(fLambda);
         ((NeuralFinder*)pattrec)->SetAlpha(fAlpha);
         ((NeuralFinder*)pattrec)->SetB(fB);
         ((NeuralFinder*)pattrec)->SetTemp(fTemp);
         ((NeuralFinder*)pattrec)->SetC(fC);
         ((NeuralFinder*)pattrec)->SetMu(fMu);
         ((NeuralFinder*)pattrec)->SetCosCut(fCosCut);
         ((NeuralFinder*)pattrec)->SetVThres(fVThres);
         ((NeuralFinder*)pattrec)->SetDNormXY(fDNormXY);
         ((NeuralFinder*)pattrec)->SetDNormZ(fDNormZ);
         ((NeuralFinder*)pattrec)->SetTscale(fTscale);
         ((NeuralFinder*)pattrec)->SetMaxIt(fMaxIt);
         ((NeuralFinder*)pattrec)->SetItThres(fItThres);
         break;
      case base:
         pattrec = new TracksFinder( &fPointsArray ); 
         break;
      default:
         pattrec = new AdaptiveFinder( &fPointsArray );
         ((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(fMaxIncreseAdapt);
         ((AdaptiveFinder*)pattrec)->SetLastPointRadCut(fLastPointRadCut);
         break;
      }

   pattrec->SetPointsDistCut(fPointsDistCut);
   pattrec->SetNpointsCut(fNspacepointsCut);
   pattrec->SetSeedRadCut(fSeedRadCut);

   int stat = pattrec->RecTracks();
   if( fTrace ) 
      std::cout<<"Reco::FindTracks status: "<<stat<<std::endl;
   int tk,npc,rc;
   pattrec->GetReasons(tk,npc,rc);
   track_not_advancing += tk;
   points_cut += npc;
   rad_cut += rc;

   AddTracks( pattrec->GetTrackVector() );

   return stat;
}

void Reco::AddTracks( const std::vector<track_t>* track_vector )
{
   int n=0;
   for( auto it=track_vector->begin(); it!=track_vector->end(); ++it)
      {
         TTrack* thetrack=new TTrack();
         thetrack->Clear();
         thetrack->SetMagneticField(fMagneticField);
         //std::cout<<"Reco::AddTracks Check Track # "<<n<<" "<<std::endl;
         for( auto ip=it->begin(); ip!=it->end(); ++ip)
            {
               TSpacePoint* ap = (TSpacePoint*) fPointsArray.at(*ip);
               thetrack->AddPoint( ap );
               //std::cout<<*ip<<", ";
               //ap->Print("rphi");
               // if( diagnostics )
               //   hsprp->Fill( ap->GetPhi(), ap->GetR() );
            }
         fTracksArray.push_back(thetrack);
         //            std::cout<<"\n";
         ++n;
      }
   //fTracksArray.Compress();
   //std::cout<<"Reco::AddTracks "<<n<<"\t"<<track_vector->size()<<"\t"<<fTracksArray.size()<<std::endl;
   assert(n==int(track_vector->size()));
   assert(fTracksArray.size()==track_vector->size());
   if( fTrace )
      std::cout<<"Reco::AddTracks # entries: "<<fTracksArray.size()<<std::endl;
}

int Reco::FitLines()
{
   int n=0;
   int ntracks=fTracksArray.size();
   for(int it=0; it<ntracks; ++it )
      {
         TTrack* at = fTracksArray.at(it);
         TFitLine* line=new TFitLine(*at); //Copy constructor
         line->SetChi2Cut( fLineChi2Cut );
         line->SetChi2Min( fLineChi2Min );
         line->SetPointsCut( fNspacepointsCut );
         line->Fit();
         if( line->GetStat() > 0 )
            {
               line->CalculateResiduals();
            }
         if( line->IsGood() )
            {
               if( fTrace )
                  line->Print();
               ++n;
               fLinesArray.push_back(line);
            }
         else
            {
               if( fTrace )
                  line-> Reason();
               delete line;
            }
      }
   //fLinesArray.Compress();
   if( n != (int)fLinesArray.size() )
      std::cerr<<"Reco::FitLines() ERROR number of lines "<<n
               <<" differs from array size "<<fLinesArray.size()<<std::endl;
   return n;
}

int Reco::FitHelix()
{
   int n=0;
   int ntracks=fTracksArray.size();
   for(int it=0; it<ntracks; ++it )
      {
         TTrack* at = (TTrack*) fTracksArray.at(it);
         //at->Print();
         TFitHelix* helix = new TFitHelix(*at); //Copy constructor
         helix->SetChi2ZCut( fHelChi2ZCut );
         helix->SetChi2RCut( fHelChi2RCut );
         helix->SetChi2RMin( fHelChi2RMin );
         helix->SetChi2ZMin( fHelChi2ZMin );
         helix->SetDCut( fHelDcut );
         helix->Fit();

         if( helix-> GetStatR() > 0 &&
             helix-> GetStatZ() > 0 )
            helix->CalculateResiduals();

         if( helix->IsGood() )
            {
               // calculate momumentum
               double pt = helix->Momentum();
               if( fTrace )
                  {
                     helix->Print();
                     std::cout<<"Reco::FitHelix()  hel # "<<n
                              <<" p_T = "<<pt
                              <<" MeV/c in B = "<<helix->GetMagneticField()
                              <<" T"<<std::endl;
                  }
               fHelixArray.push_back(helix);
               ++n;
            }
         else
            {
               if( fTrace )
                  helix->Reason();
               helix->Clear();
               delete helix;
            }
      }
   //fHelixArray.Compress();
   if( n != (int)fHelixArray.size() )
      std::cerr<<"Reco::FitHelix() ERROR number of lines "<<n
               <<" differs from array size "<<fHelixArray.size()<<std::endl;
   return n;
}

int Reco::RecVertex(TFitVertex* Vertex)
{
   int Nhelices = 0;
   Vertex->SetChi2Cut( fVtxChi2Cut );
   int nhel=fHelixArray.size();
   for( int n = 0; n<nhel; ++n )
      {
         TFitHelix* hel = (TFitHelix*)fHelixArray.at(n);
         if( hel->IsGood() )
            {
               Vertex->AddHelix(hel);
               ++Nhelices;
            }
      }
   if( fTrace )
      std::cout<<"Reco::RecVertex(  )   # helices: "<<nhel<<"   # good helices: "<<Nhelices<<std::endl;
   // reconstruct the vertex
   int sv = -2;
   if( Nhelices )// find the vertex!
      {
         sv = Vertex->Calculate();
         if( fTrace )
            std::cout<<"Reco::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
      }
   return sv;
}

void Reco::Reset()
{
   if( pattrec ) 
      { 
         //std::cout<<"Reco::Reset() deleting pattrec"<<std::endl;
         delete pattrec;
      }
   //   fTrace=false;
   // fPointsArray.clear();
   // fTracksArray.clear();
   // fLinesArray.clear();
   // fHelixArray.clear();
   
   for (size_t i=0; i<fHelixArray.size(); i++)
      delete fHelixArray.at(i);
   fHelixArray.clear();
   
   for (size_t i=0; i<fLinesArray.size(); i++)
      delete fLinesArray.at(i);
   fLinesArray.clear();
   for (size_t i=0; i<fTracksArray.size(); i++)
      delete fTracksArray.at(i);
   fTracksArray.clear(); 
   for (size_t i=0; i<fPointsArray.size(); i++)
      delete fPointsArray.at(i);
   fPointsArray.clear(); 
}

void Reco::PrintPattRec()
{
   std::cout<<"Reco:: pattrec failed\ttrack not advanving: "<<track_not_advancing
            <<"\tpoints cut: "<<points_cut
            <<"\tradius cut: "<<rad_cut<<std::endl;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
