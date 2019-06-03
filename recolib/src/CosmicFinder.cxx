#include "CosmicFinder.hh"

#include <iostream>
#include <TObjArray.h>
#include <TVector3.h>

#include "TSpacePoint.hh"
#include "TTrack.hh"
#include "TCosmic.hh"
#include "TStoreHelix.hh"
#include "TStoreLine.hh"

CosmicFinder::CosmicFinder(double b):fMagneticField(b),nTracks(-1),
				     fNspacepointsCut(29),
				     fLineChi2Cut(9.e9),fLineChi2Min(0.0),
                                     fIdx(-1),fRes2(9.e99),fStatus(-1)
{
   std::cout<<"CosmicFinder::CosmicFinder( B = "<<fMagneticField<<" T )"<<std::endl;
   pmap = new padmap;
   MakeOccupancyHisto();
}

CosmicFinder::CosmicFinder(double b,int pointscut,double chi2cut,double chi2min):
   fMagneticField(b),nTracks(-1),
   fNspacepointsCut(pointscut),
   fLineChi2Cut(chi2cut),fLineChi2Min(chi2min),
   fIdx(-1),fRes2(9.e99),fStatus(-1)
{
   std::cout<<"CosmicFinder::CosmicFinder( B = "<<fMagneticField<<" T )"<<std::endl;
   pmap = new padmap;
   MakeOccupancyHisto();
}

int CosmicFinder::Create(std::vector<TTrack*>* tracks)
{
   //std::cout<<"CosmicFinder::Create(TClonesArray* tracks), B = "
   //         <<fMagneticField<<" T"<<std::endl;
   nTracks = tracks->size();
   for(int i=0; i<nTracks; ++i)
      {
         TTrack* hi = tracks->at(i);
         for(int j=i+1; j<nTracks; ++j )
            {
               TTrack* hj = tracks->at(j);
               TCosmic* cos;
               if( fMagneticField > 0. )
                  cos = new TCosmic((TFitHelix*)hi,(TFitHelix*)hj,fMagneticField);
               else
                  cos = new TCosmic((TFitLine*)hi,(TFitLine*)hj);

               cos->SetChi2Cut( fLineChi2Cut );
               cos->SetChi2Min( fLineChi2Min );
               cos->SetPointsCut( fNspacepointsCut );
               //std::cout<<"CosmicFinder::Create(TClonesArray* tracks)  nPoints: "<<cos->GetNumberOfPoints()<<std::endl;
               cos->Fit();
               if( cos->IsGood() && !cos->IsWeird() )
                  {
                     //double rsq = cos->CalculateResiduals();
                     //std::cout<<"CosmicFinder::Create OK delta^2: "<<rsq<<std::endl;
                     cos->CalculateResiduals();
                     fLines.push_back( cos );
                  }
               else
                  {
                     //std::cout<<"CosmicFinder::Create(TClonesArray* tracks) NO GOOD"<<std::endl;
                     //cos->Reason();
                     delete cos;
                  }
            }
      }
   //std::cout<<"CosmicFinder::Create nTracks: "<<nTracks
   //         <<" fLines size: "<<fLines.size()<<std::endl;
   return nTracks;
}

int CosmicFinder::Create(TStoreEvent* e)
{
   //std::cout<<"CosmicFinder::Create(TStoreEvent* e), B = "<<fMagneticField<<" T"<<std::endl;
   if( fMagneticField > 0. )
      {
         const TObjArray* helices = e->GetHelixArray();
         nTracks = helices->GetEntriesFast();
         for( int i=0; i<nTracks; ++i )
            {
               TStoreHelix* hi = (TStoreHelix*) helices->At(i);
               for( int j=i+1; j<nTracks; ++j )
                  {
                     TStoreHelix* hj = (TStoreHelix*) helices->At(j);
                     TCosmic* cos = new TCosmic(hi,hj,fMagneticField);
                     cos->SetChi2Cut( fLineChi2Cut );
                     cos->SetChi2Min( fLineChi2Min );
                     cos->SetPointsCut( fNspacepointsCut );
                     //std::cout<<"CosmicFinder::Create(TStoreEvent* e) from helix nPoints: "
                     //         <<cos->GetNumberOfPoints()<<std::endl;
                     cos->Fit();
                     if( cos->IsGood() && !cos->IsWeird() )
                        {
                           cos->CalculateResiduals();
                           fLines.push_back( cos );
                        }
                     else
                        delete cos;
                  }
            }
      }
   else
      {
         const TObjArray* lines = e->GetLineArray();
         nTracks = lines->GetEntriesFast();
         for( int i=0; i<nTracks; ++i )
            {
               TStoreLine* li = (TStoreLine*) lines->At(i);
               for( int j=i+1; j<nTracks; ++j )
                  {
                     TStoreLine* lj = (TStoreLine*) lines->At(j);
                     TCosmic* cos = new TCosmic(li,lj);
                     cos->SetChi2Cut( fLineChi2Cut );
                     cos->SetChi2Min( fLineChi2Min );
                     cos->SetPointsCut( fNspacepointsCut );
                     //std::cout<<"CosmicFinder::Create(TStoreEvent* e) from line nPoints: "
                     //         <<cos->GetNumberOfPoints()<<std::endl;
                     cos->Fit();
                     if( cos->IsGood() && !cos->IsWeird() )
                        {
                           cos->CalculateResiduals();
                           fLines.push_back( cos );
                        }
                     else
                        delete cos;
                  }
            }
      }
   //std::cout<<"CosmicFinder::CosmicFinder nTracks: "<<nTracks
   //         <<" fLines size: "<<fLines.size()<<std::endl;
   return nTracks;
}

CosmicFinder::~CosmicFinder()
{
   Reset();
   delete pmap;
}

void CosmicFinder::Reset()
{
   for(auto l: fLines) delete l;
   fLines.clear();  
   nTracks=-1;
   fRes2=9.e99;
   fIdx=-1;
}

int CosmicFinder::Process()
{
   fStatus = 2;
   if( nTracks < 2 ) return 2;
   fStatus = 1;
   if( fLines.size() < 1 ) return 1;
   //fLines.shrink_to_fit();
   fStatus = 3;
   return Residuals();
}

int CosmicFinder::Residuals()
{
   //std::cout<<"CosmicFinder::Residuals Size: "<<fLines.size()<<std::endl;
   int i=0;
   for( auto l: fLines )
      {
         double lres2 = l->GetResidualsSquared(),
            nPoints = (double) l->GetNumberOfPoints();
         //std::cout<<"CosmicFinder::Residuals Candidate: "<<i
         //         <<") delta^2: "<<lres2
         //         <<" nPoints: "<<nPoints<<std::endl;
         lres2/=nPoints;
         if( lres2 < fRes2 )
            {
               fRes2=lres2;
               fIdx=i;
            }
         ++i;
      }
   //std::cout<<"CosmicFinder::Residuals Cosmic delta^2: "<<fRes2<<" @ "<<fIdx<<std::endl;

   if( fIdx < 0 )
      return 3;

   hRes2min->Fill(fRes2);
   FillOccupancyHisto();
   fStatus = 0;
   return 0;
}

void CosmicFinder::MakeOccupancyHisto()
{
   hDCAeq2 = new TH1D("hDCAeq2","Distance of Closest Approach between Helices in =2-tracks Events;DCA [mm]",
                      500,0.,50.);
   hDCAgr2 = new TH1D("hDCAgr2","Distance of Closest Approach between Helices in >2-tracks Events;DCA [mm]",
                      500,0.,50.);
	 
   hAngeq2 = new TH1D("hAngeq2","Cosine of the Angle formed by Two Helices in =2-tracks Events;cos(angle)",
                      1000,-1.,1.);
   hAnggr2 = new TH1D("hAnggr2","Cosine of the Angle formed by Two Helices in >2-tracks Events;cos(angle)",
                      1000,-1.,1.);

   hAngDCAeq2 = new TH2D("hAngDCAeq2","DCA and Cosine of Angle between Helices in =2-tracks Events;cos(angle);DCA [mm]",
                         100,-1.,1.,100,0.,50.);
   hAngDCAgr2 = new TH2D("hAngDCAgr2","DCA and Cosine of Angle between Helices in >2-tracks Events;cos(angle);DCA [mm]",
                         100,-1.,1.,100,0.,50.);

   hcosaw = new TH1D("hcosaw","Occupancy per AW due to cosmics",256,-0.5,255.5);
   hcosaw->SetMinimum(0.);
   hcospad = new TH2D("hcospad","Occupancy per PAD due to cosmics;Pads Row;Pads Sector",
                      576,-0.5,575.5,32,-0.5,31.5);
   hRes2min = new TH1D("hRes2min","Minimum Residuals Squared Divide by Number of Spacepoints from 2 Helices;#delta [mm^{2}]",1000,0.,1000.);

   // // cosmic time distribution
   // hpois = new TH1D("hpois","Delta t between cosmics;#Delta t [ms]",300,0.,300.);
   // temp = 0.;            

   hcosphi = new TH1D("hcosphi","Direction #phi;#phi [deg]",200,-180.,180.);
   hcosphi->SetMinimum(0.);
   hcostheta = new TH1D("hcostheta","Direction #theta;#theta [deg]",200,0.,180.);
   hcostheta->SetMinimum(0.);

   hcosthetaphi = new TH2D("hcosthetaphi","Direction #theta Vs #phi;#theta [deg];#phi [deg]",
                           200,0.,180.,200,-180.,180.);

  // z axis intersection
  hlr = new TH1D("hlr","Minimum Radius;r [mm]",200,0.,190.);
  hlz = new TH1D("hlz","Z intersection with min rad;z [mm]",300,-1200.,1200.);
  hlp = new TH1D("hlp","#phi intersection with min rad;#phi [deg]",100,0.,360.);
  hlp->SetMinimum(0.);
  hlzp = new TH2D("hlzp","Z-#phi intersection with min rad;z [mm];#phi [deg]",
		  100,-1200.,1200.,90,0.,360.);
  hlzp->SetStats(kFALSE);
  hlzr = new TH2D("hlzr","Z-R intersection with min rad;z [mm];r [mm]",
		  100,-1200.,1200.,100,0.,190.);
  hlrp = new TH2D("hlrp","R-#phi intersection with min rad;r [mm];#phi [deg]",
		  100,0.,190.,90,0.,360.);
  hlxy = new TH2D("hlxy","X-Y intersection with min rad;x [mm];y [mm]",
		  100,-190.,190.,100,-190.,190.);
}

void CosmicFinder::FillOccupancyHisto()
{
   TCosmic* cosmic = fLines.at( fIdx );

   double dca = cosmic->GetDCA(), cosangle = cosmic->GetCosAngle();
   if( nTracks == 2 )
      {
         hDCAeq2->Fill( dca );
         hAngeq2->Fill( cosangle );
         hAngDCAeq2->Fill( cosangle, dca );
      }
   else if( nTracks > 2 )
      {
         hDCAgr2->Fill( dca );
         hAnggr2->Fill( cosangle );
         hAngDCAgr2->Fill( cosangle, dca );
      }
   else return;
         
   for( uint i=0; i<cosmic->GetPointsArray()->size(); ++i )
      {
         TSpacePoint* p = (TSpacePoint*) cosmic->GetPointsArray()->at( i );
         int aw = p->GetWire(), sec,row;
         pmap->get( p->GetPad(), sec,row );
         if( 0 )
            {
               double time = p->GetTime(),
                  height = p->GetHeight();
               std::cout<<aw<<"\t\t"<<sec<<"\t"<<row<<"\t\t"<<time<<"\t\t"<<height<<std::endl;
            }
         hcosaw->Fill( double(aw) );
         hcospad->Fill( double(row), double(sec) );
      }
      
   TVector3 u = cosmic->GetU();
   hcosphi->Fill(u.Phi()*TMath::RadToDeg());
   hcostheta->Fill(u.Theta()*TMath::RadToDeg());
   hcosthetaphi->Fill(u.Theta()*TMath::RadToDeg(),u.Phi()*TMath::RadToDeg());

   TVector3 zint = cosmic->Zintersection();
   double zint_phi = zint.Phi();
   if( zint_phi < 0. ) zint_phi+=TMath::TwoPi();
   zint_phi*=TMath::RadToDeg();
   hlr->Fill( zint.Perp() );
   hlz->Fill( zint.Z() );
   hlp->Fill( zint_phi );
   hlzp->Fill( zint.Z(), zint_phi );
   hlzr->Fill( zint.Z(), zint.Perp() );
   hlrp->Fill( zint.Perp(), zint_phi);
   hlxy->Fill( zint.X(), zint.Y() );
}

void CosmicFinder::Status()
{
   switch(fStatus)
      {
      case -1:
         std::cout<<"CosmicFinder::Status: Not Processed"<<std::endl;
         break;
      case 0:
         std::cout<<"CosmicFinder::Status: OK"<<std::endl;
         break;
      case 1:
         std::cout<<"CosmicFinder::Status: Not Enough Cosmic Candidates"<<std::endl;
         break;
      case 2:
         std::cout<<"CosmicFinder::Status: Not Enough Reconstructed Tracks"<<std::endl;
         break;
      case 3:
         std::cout<<"CosmicFinder::Status: Failed to find minimal residual"<<std::endl;
         break;
      default:
         std::cout<<"CosmicFinder::Status: What happened?"<<std::endl;
         break;
      }
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
