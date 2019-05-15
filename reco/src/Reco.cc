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

Reco::Reco(std::string json, double B):fTrace(false),fMagneticField(B),
                                       pattrec(0),
				       fPointsArray("TSpacePoint",1000),
				       fTracksArray("TTrack",50),
				       fLinesArray("TFitLine",50),
				       fHelixArray("TFitHelix",50)
{
   ana_settings=new AnaSettings(json.c_str());
   ana_settings->Print();

   fNhitsCut = ana_settings->GetInt("RecoModule","NhitsCut");
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

   fSTR = new LookUpTable(_co2frac, fMagneticField); // uniform field version (simulation)
   std::cout<<"Reco::Reco()  max time: "<<fSTR->GetMaxTime()<<" ns"<<std::endl;
}

Reco::~Reco()
{
   fHelixArray.Delete();
   fLinesArray.Delete();
   fTracksArray.Delete();
   fPointsArray.Delete();
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

         if( fTrace )
            {
               double z = ( double(sp->second.idx) + 0.5 ) * _padpitch - _halflength;
               std::cout<<"RecoRun::AddSpacePoint "<<n<<" aw: "<<sp->first.idx
                        <<" t: "<<time<<" r: "<<r
                        <<"\tcol: "<<sp->second.sec<<" row: "<<sp->second.idx<<" z: "<<z
                        <<" ~ "<<sp->second.z<<" err: "<<sp->second.errz<<std::endl;
               //<<time<<" "<<r<<" "<<correction<<" "<<err<<std::endl;
            }
         TSpacePoint* point=( (TSpacePoint*)fPointsArray.ConstructedAt(n) );
         point->Setup(sp->first.idx,
                      sp->second.sec,sp->second.idx,
                      time,
                      r,correction,zed,
                      err,erp,sp->second.errz,
                      sp->first.height);
         ++n;
      }
   //fPointsArray.Compress();
   fPointsArray.Sort();
   //if( fTrace )
   std::cout<<"RecoRun::AddSpacePoint # entries: "<<fPointsArray.GetEntriesFast()<<std::endl;
}

void Reco::AddSpacePoint( const TObjArray* p )
{
  for( int n=0; n<p->GetEntriesFast(); ++n )
    {
      new(fPointsArray[n]) TSpacePoint(*(TSpacePoint*)p->At(n));
    }
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

         TSpacePoint* point=( (TSpacePoint*)fPointsArray.ConstructedAt(j) );
         point->Setup( aw, sec, row, time,
                       rad, lor, zed,
                       err, erp, erz,
                       h->GetDepositEnergy() );
         point->SetTrackID(h->GetTrackID());
         point->SetTrackPDG(h->GetTrackPDG());
      }
}

int Reco::FindTracks(finderChoice finder)
{
   switch(finder)
      {
      case adaptive:
         pattrec = new AdaptiveFinder( &fPointsArray );
         ((AdaptiveFinder*)pattrec)->SetMaxIncreseAdapt(fMaxIncreseAdapt);
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
         break;
      }

   pattrec->SetPointsDistCut(fPointsDistCut);
   pattrec->SetNpointsCut(fNspacepointsCut);
   pattrec->SetSeedRadCut(fSeedRadCut);

   int stat = pattrec->RecTracks();
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
         TTrack* thetrack=( (TTrack*)fTracksArray.ConstructedAt(n) ) ;
         thetrack->Clear();
         thetrack->SetMagneticField(fMagneticField);
         //std::cout<<"RecoRun::AddTracks Check Track # "<<n<<" "<<std::endl;
         for( auto ip=it->begin(); ip!=it->end(); ++ip)
            {
               TSpacePoint* ap = (TSpacePoint*) fPointsArray.At(*ip);
               thetrack->AddPoint( ap );
               //std::cout<<*ip<<", ";
               //ap->Print("rphi");
               // if( diagnostics )
               //   hsprp->Fill( ap->GetPhi(), ap->GetR() );
            }
         //            std::cout<<"\n";
         ++n;
      }
   fTracksArray.Compress();
   assert(n==int(track_vector->size()));
   assert(fTracksArray.GetEntriesFast()==int(track_vector->size()));
   if( fTrace )
      std::cout<<"RecoRun::AddTracks # entries: "<<fTracksArray.GetEntriesFast()<<std::endl;
}

int Reco::FitLines()
{
   int n=0;
   int ntracks=fTracksArray.GetEntriesFast();
   for(int it=0; it<ntracks; ++it )
      {
         TTrack* at = (TTrack*) fTracksArray.At(it);
         //at->Print();
         new(fLinesArray[n]) TFitLine(*at);
         TFitLine* line=(TFitLine*)fLinesArray.ConstructedAt(n);
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
            }
         else
            {
               if( fTrace )
                  line-> Reason();
               fLinesArray.RemoveAt(n);
            }
      }
   fLinesArray.Compress();
   if( n != fLinesArray.GetEntriesFast() )
      std::cerr<<"Reco::FitLines() ERROR number of lines "<<n
               <<" differs from array size "<<fLinesArray.GetEntriesFast()<<std::endl;
   return n;
}

int Reco::FitHelix()
{
   int n=0;
   int ntracks=fTracksArray.GetEntriesFast();
   for(int it=0; it<ntracks; ++it )
      {
         TTrack* at = (TTrack*) fTracksArray.At(it);
         //at->Print();
         new(fHelixArray[n]) TFitHelix(*at);
         TFitHelix* helix = (TFitHelix*)fHelixArray.ConstructedAt(n);
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
                     std::cout<<"RecoRun::FitHelix()  hel # "<<n
                              <<" p_T = "<<pt
                              <<" MeV/c in B = "<<helix->GetMagneticField()
                              <<" T"<<std::endl;
                  }
               ++n;
            }
         else
            {
               if( fTrace )
                  helix->Reason();
               helix->Clear();
               fHelixArray.RemoveAt(n);

            }
      }
   fHelixArray.Compress();
   if( n != fHelixArray.GetEntriesFast() )
      std::cerr<<"Reco::FitHelix() ERROR number of lines "<<n
               <<" differs from array size "<<fHelixArray.GetEntriesFast()<<std::endl;
   return n;
}

int Reco::RecVertex(TFitVertex* Vertex)
{
   int Nhelices = 0;
   Vertex->SetChi2Cut( fVtxChi2Cut );
   int nhel=fHelixArray.GetEntriesFast();
   for( int n = 0; n<nhel; ++n )
      {
         TFitHelix* hel = (TFitHelix*)fHelixArray.ConstructedAt(n);
         if( hel->IsGood() )
            {
               Vertex->AddHelix(hel);
               ++Nhelices;
            }
      }
   if( fTrace )
      std::cout<<"RecoRun::RecVertex(  )   # helices: "<<nhel<<"   # good helices: "<<Nhelices<<std::endl;
   // reconstruct the vertex
   int sv = -2;
   if( Nhelices )// find the vertex!
      {
         sv = Vertex->Calculate();
         if( fTrace )
            std::cout<<"RecoRun::RecVertex(  )   # used helices: "<<Vertex->GetNumberOfHelices()<<std::endl;
      }
   return sv;
}

void Reco::Reset()
{
   if( pattrec ) 
      { 
         //std::cout<<"RecoRun::Reset() deleting pattrec"<<std::endl;
         delete pattrec;
      }
   fHelixArray.Delete(); //I can't get Clear to work... I will keep trying Joe
   //fHelixArray.Clear("C");
   fLinesArray.Delete();
   //fLinesArray.Clear("C");
   fTracksArray.Clear("C"); // Ok, I need a delete here to cure leaks... further work needed
   fPointsArray.Clear(); //Simple objects here, do not need "C" (recursive clear)
   fTrace=false;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
