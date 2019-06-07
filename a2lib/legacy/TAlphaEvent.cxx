#include <TBits.h>
#include <TVector.h>
#include <TVector3.h>
#include <TMatrix.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TFile.h>
#include <TMarker.h>
#include <TGraphErrors.h>
#include <TLine.h>
#include <TStyle.h>

#include <iostream>
#include <stdlib.h>

#include "TAlphaEvent.h"
#include "TAlphaEventCosmicHelix.h"
#include "THoughPeakFinder.h"


//////////////////////////////////////////////////////////////////////
//
//  TAlphaEvent is the fundamental class of Alpha Event
//
//////////////////////////////////////////////////////////////////////

ClassImp(TAlphaEvent);


//____________________________________________________________________
TAlphaEvent::TAlphaEvent(TAlphaEventMap* a)
  : TObject(),
  fVerbose(0)
{
  map=a;

  fSil.reserve(72);
  fCosmicHelices = new TObjArray();

  fVertex.Clear();
  fMCVertex.SetXYZ(0,0,0);

  fProjClusterVertex = NULL;

  fNHitsCut = 200.; //Overwritten by alphaAnalysis.exe
  nClusterSigmaCut=0.;
  pClusterSigmaCut=0.;

  // 'Projection Method' cut valuses
  fMinDistCut = 10.;
  fMinClosestCut = 5.;

  // default track cut values
  //2017_Q1 defaults:
  fHitSepCutPhi = 0.45;
  fHitSepCutZ   = 5.5;
  fCorCut       = .975;
  fChi2Cut	= 29.;
  fd0_trapCut = 6.;

  //2014-2016:
  //fHitSepCutPhi = 0.35;
  //fHitSepCutZ   = 5.;
  //fCorCut       = .95;
  //fChi2Cut	= 63.; //21.
  //fd0_trapCut = 7.;

  //Regular Reconstruction Method Vertex candidate cut

  //2017_Q1 defaults:
  fVertRadCut = 3.6;
  fVertDCACut = 0.3;
  //2014-2016:
  //fVertRadCut = 4.;
  //fVertDCACut = 0.4;

   if(nSil==60)
     {
       // 'Projection Method' cut valuses
       fMinDistCut    = 3.;
       fMinClosestCut = 1.5;
       // default cut values
       fHitSepCutZ = 6.;
       fCorCut     = .9;
       fChi2Cut	   = 15.;
       fd0_trapCut = 1.5;

       fVertRadCut = 4.;
       fVertDCACut = 0.4;
     }

  fIsCosmic=kFALSE;
  fDebug = kFALSE;

  fCosmic.Clear();
}

//____________________________________________________________________
TAlphaEvent::~TAlphaEvent() {
  //set ownership to tobjectarray to delete properly

  fCosmicHelices->SetOwner( kTRUE );
  fMCPoint.SetOwner( kTRUE );
  fxylines.SetOwner( kTRUE );
  fyzlines.SetOwner( kTRUE );
  fprojp.SetOwner( kTRUE );

  DeleteEvent();

  int n=GetNSil();
  for (int i=0; i<n; i++)
     delete fSil[i];
  fSil.clear();

  n=GetNTracks();
  for (int i=0; i<n; i++)
     delete fTrack[i];
  fTrack.clear();

  n=GetNHelices();
  for (int i=0; i<n; i++)
     delete fHelices[i];
  fHelices.clear();
  delete fCosmicHelices;

  fProjClusterVertex = NULL;
}

//____________________________________________________________________
void TAlphaEvent::Reset()
{

  //fSil should be owned by TSiliconEvent and deleted there...
  int n=GetNSil();
  for (int i=0; i<n; i++)
     delete fSil[i];
  fSil.clear();

  n=GetNTracks();
  for (int i=0; i<n; i++)
     delete fTrack[i];
  fTrack.clear();

  n=GetNHelices();
  for (int i=0; i<n; i++)
     delete fHelices[i];
  fHelices.clear();
  
  fCosmicHelices->SetOwner(kTRUE);
  fCosmicHelices->Delete();
  delete fCosmicHelices;
  fCosmicHelices = NULL;
  fCosmicHelices = new TObjArray();

  fxylines.SetOwner(kTRUE);
  fxylines.Clear();
  fxylines.SetOwner(kTRUE);
  fyzlines.Clear();
  
  fprojp.SetOwner(kTRUE);
  fprojp.Clear();

  //TObjArray* hits=GatherHits();
  //hits->SetOwner(kTRUE);
  //hits->Delete();
  fVertex.Clear();
  
  if( fProjClusterVertex )
  {
    delete fProjClusterVertex;
    fProjClusterVertex = NULL;
  }
  fVertex.Clear();
  fCosmic.Clear();
}

//____________________________________________________________________
void TAlphaEvent::DeleteEvent()
{
//  fHits.SetOwner(kTRUE);
  //fHits.Clear();
  //fHits.Delete();
  int n=GetNSil();
  for (int i=0; i<n; i++)
     delete fSil[i];
  fSil.clear();

  n=GetNTracks();
  for (int i=0; i<n; i++)
     delete fTrack[i];
  fTrack.clear();
  
  n=GetNHelices();
  for (int i=0; i<n; i++)
     delete fHelices[i];
  fHelices.clear();
  
  fCosmicHelices->SetOwner(kTRUE);
  fCosmicHelices->Delete();
  delete fCosmicHelices;
  fCosmicHelices = NULL;
  fCosmicHelices = new TObjArray();
  fVertex.Clear();
  fprojp.Clear();
  if( fProjClusterVertex )
    {
      delete fProjClusterVertex;
      fProjClusterVertex = NULL;
    }
  //fSil should be owned by TSiliconEvent and deleted there...
  //fSil.SetOwner(kTRUE);
  fprojp.SetOwner(kTRUE);
  fprojp.Delete();
  fMCPoint.SetOwner(kTRUE);
  fMCPoint.Delete();
  fMCVertex.SetXYZ(0.,0.,0.);
  fxylines.SetOwner(kTRUE);
  fxylines.Clear();
  fyzlines.SetOwner(kTRUE);
  fyzlines.Clear();
  fCosmic.Clear();
}
void TAlphaEvent::CalcGoodHelices()
{
  Int_t nh=0;
  for(Int_t i = 0; i<GetNHelices(); i++)
    {
      TAlphaEventHelix * t = GetHelix(i);
      if (!t) continue;
      if(t->GetHelixStatus()>0) nh++;
    }

  fNGoodHelices = nh;
  //printf("NGoodHelices: %d\n",fNGoodHelices);

}
//_____________________________________________________________________
void TAlphaEvent::RecEvent( Bool_t debug )
{
  RecClusters();
  RecHits();
  GatherTrackCandidates();
  RecTrackCandidates();
  PruneTracks();
  RecVertex();
  RecRPhi();
  CalcGoodHelices();

  fDebug = debug;
}
/*
//_____________________________________________________________________
TAlphaEventSil *TAlphaEvent::GetSilByName(Char_t *name)
{
  for (Int_t k=0; k<GetNSil(); k++)
    {
      TAlphaEventSil *sil = GetSil(k);
      if (strcmp(name,sil->GetName())==0) return sil;
    }
  return (TAlphaEventSil*) NULL;
}
*/
//_____________________________________________________________________
TAlphaEventSil *TAlphaEvent::GetSilByNumber(Int_t n, bool read_only)
{
 for (Int_t k=0; k<GetNSil(); k++)
    {
      TAlphaEventSil *sil = GetSil(k);
      if (sil->GetSilNum() == n) return sil;
    }
 if (read_only) return NULL;
 TAlphaEventSil * sil = new TAlphaEventSil(n,this,map);
 AddSil(sil);

 return sil;
}

//_____________________________________________________________________
void TAlphaEvent::RecHits()
{
  for (Int_t n=0;n<GetNSil();n++)
    {
      TAlphaEventSil *sil = GetSil(n);
      if(sil)
	sil->RecHit();
    }
}

//_____________________________________________________________________
void TAlphaEvent::RecClusters()
{
  for (Int_t n=0;n<GetNSil();n++)
    {
      TAlphaEventSil *sil = GetSil(n);
      if(sil)
	sil->RecCluster();
    }
}

//_____________________________________________________________________
Int_t TAlphaEvent::LayerMulti( const char * layernum )
{
  Int_t multi = 0;
  for (Int_t k=0; k<GetNSil(); ++k)
    {
      TAlphaEventSil *sil = GetSil(k);
      if (strncmp(sil->GetName(),layernum,1)==0)
        if( sil->GetOrPhi() )
          ++multi;
    }
  return multi;
}

//_____________________________________________________________________
Int_t TAlphaEvent::ModuleMulti( Int_t sinum )
{
  Int_t multi = 0;
  for (Int_t k=0; k<GetNSil(); k++)
    {
      TAlphaEventSil *sil = GetSil(k);
      if( sinum == sil->GetSilNum() )
        if( sil->GetOrPhi() )
          multi++;
    }
  return multi;
}

//____________________________________________________________________
Int_t TAlphaEvent::IsROTrig() // ALPHA-1
{
  Int_t layer0 = LayerMulti("0");
  Int_t layer1 = LayerMulti("1");
  Int_t layer2 = LayerMulti("2");
  Int_t layer3 = LayerMulti("3");
  Int_t layer4 = LayerMulti("4");
  Int_t layer5 = LayerMulti("5");

  if(((layer0>1) && (layer1>1) && (layer2>1)) ||
     ((layer3>1) && (layer4>1) && (layer5>1)) )
    {
      return kTRUE;
    }
  else
    {
      return kFALSE;
    }
}

Int_t TAlphaEvent::IsSig1Trig() // ALPHA-1
{
  Int_t innerlayerDS = LayerMulti("0"); // downstream
  Int_t innerlayerUS = LayerMulti("3"); // upstream

  if((innerlayerDS>=2) || (innerlayerUS>=2) || ((innerlayerDS>=1)&&(innerlayerUS>=1)) )
    return kTRUE;
  else
    return kFALSE;
}

//_____________________________________________________________________
Bool_t TAlphaEvent::IsTrig(Int_t inner, Int_t middle, Int_t outer) // ALPHA-2
{
  Int_t inLayAD=0, midLayAD=0, outLayAD=0,
    inLayPOS=0, midLayPOS=0, outLayPOS=0;

  for (Int_t k=0; k<GetNSil(); ++k)
    {
      TAlphaEventSil *sil = GetSil(k);
      Int_t multi=sil->GetOrPhi(); // ASICs that have p-strips
      if(multi==0) continue;
      char silname[6];
      strcpy(silname,sil->GetName());
      switch(silname[0])// collect multiplicity per layer per end
	{
	case '0':
	  inLayAD+=multi;
	  break;
	case '1':
	  midLayAD+=multi;
	  break;
	case '2':
	  outLayAD+=multi;
	  break;
	case '3':
	  inLayPOS+=multi;
	  break;
	case '4':
	  midLayPOS+=multi;
	  break;
	case '5':
	  outLayPOS+=multi;
	  break;
	}
    }

  Int_t inLay  =  inLayAD + inLayPOS; // collect multiplicity per layer
  Int_t midLay = midLayAD + midLayPOS;
  Int_t outLay = outLayAD + outLayPOS;

  Bool_t trig = (inLay>=inner) && (midLay>=middle) && (outLay>=outer); // final decision

  return trig;
}


//_____________________________________________________________________
Int_t TAlphaEvent::IsCosmic()
{
// Use a Hough tranform to attemp to classify an event as cosmic-like or pbar-like

  // Average all the points in a module--reduces noise in
  // Hough transformed space, but reduces resolution
  TObjArray * avgHitList = new TObjArray();
   TObjArray* Hits=GatherHits();
  const Int_t NHits = Hits->GetEntries();

  for( Int_t isil = 0; isil < nSil; isil++ )
    {
      TAlphaEventHit * avg = new TAlphaEventHit(map);
      avg->SetSilNum( isil );
      avg->SetXYZMRS( 0.,0.,0. );
      for( Int_t ipoint = 0; ipoint < NHits; ipoint++ )
        {
          TAlphaEventHit * c = (TAlphaEventHit*)Hits->At(ipoint);
          if( c->GetSilNum() == isil )
            {
              if( ( avg->XMRS() == 0. && avg->YMRS() == 0. && avg->ZMRS() == 0. ))
                {
                  // First point
                  avg->SetXYZMRS( c->XMRS(), c->YMRS(), c->ZMRS() );
                }
              else
                {
                  // Average the points
                  avg->SetXYZMRS( 0.5*(c->XMRS() + avg->XMRS()),
                                  0.5*(c->YMRS() + avg->YMRS()),
                                  0.5*(c->ZMRS() + avg->ZMRS()));
                }
            }
        }
      if( fabs(avg->XMRS()) < 50. && fabs(avg->YMRS()) < 50. && fabs(avg->ZMRS()) < 100.
          && avg->XMRS() != 0. && avg->YMRS() != 0. && avg->ZMRS() != 0.)
        avgHitList->AddLast(avg);
      else
        delete avg;
    }

  //  for( Int_t i = 0; i<avgHitList->GetEntries(); i++ )
  //  ((TAlphaEventCluster *)avgHitList->At(i))->Print();


 // Create the accumulator histogram
  const Int_t numvotes = 1000;

  const Int_t xbin = 250;
  const Double_t xmin = 0;
  const Double_t xmax = 180;
  const Int_t ybin = 250;
  const Double_t ymin = -25;
  const Double_t ymax = 25;

  static TH2D * pxy = new TH2D("pspace xy","pspace xy",
			       xbin,xmin,xmax,ybin,ymin,ymax);
  static TH2D * pyz = new TH2D("pspace yz","pspace yz",
			       xbin,xmin,xmax,ybin,ymin,ymax);

  pxy->Reset();
  pyz->Reset();

  double binsperx = pxy->GetNbinsX()/
    (pxy->GetXaxis()->GetXmax()-pxy->GetXaxis()->GetXmin());
  double binspery = pxy->GetNbinsY()/
    (pxy->GetYaxis()->GetXmax()-pxy->GetYaxis()->GetXmin());
  double binmaxx = (pxy->GetXaxis()->GetXmin())*binsperx;
  double binmaxy = (pxy->GetYaxis()->GetXmin())*binspery;

  for(int ipoint = 0; ipoint< avgHitList->GetEntries(); ipoint++)
    {
      TH2D * pointxy = new TH2D("point xy","point xy",
				xbin,xmin,xmax,ybin,ymin,ymax);
      TH2D * pointyz = new TH2D("point yz","point yz",
				xbin,xmin,xmax,ybin,ymin,ymax);
      for(int i = 0; i < numvotes; i++ )
	{
	  Double_t x = ((TAlphaEventHit*) avgHitList->At(ipoint))->XMRS();
	  Double_t y = ((TAlphaEventHit*) avgHitList->At(ipoint))->YMRS();
	  Double_t z = ((TAlphaEventHit*) avgHitList->At(ipoint))->ZMRS();

	  double theta =  double(i) / double(numvotes) * (2*3.14159) -3.14159;
	  double ryz = z*cos(theta) + y*sin(theta);
	  double rxy = x*cos(theta) + y*sin(theta);
	  // printf( "rxy: %lf, theta: %lf\n", rxy, theta*180./3.14159 );
	  if(pointxy->GetBinContent((Int_t)(theta*180./3.14159*binsperx-binmaxx),(Int_t)(rxy*binspery-binmaxy))==0)
	    pointxy->SetBinContent((Int_t)(theta*180./3.14159*binsperx-binmaxx),(Int_t)(rxy*binspery-binmaxy),1);
	  if(pointyz->GetBinContent((Int_t)(theta*180./3.14159*binsperx-binmaxx),(Int_t)(ryz*binspery-binmaxy))==0)
	    pointyz->SetBinContent((Int_t)(theta*180./3.14159*binsperx-binmaxx),(Int_t)(ryz*binspery-binmaxy),1);
	}
      pxy->Add(pointxy);
      pyz->Add(pointyz);
      delete pointxy;
      delete pointyz;
    }

  // make the canvas
  /*  static TCanvas * cp = NULL;
  if(!cp)
    {
      cp = new TCanvas();
      cp->Divide(2,1);
      }*/

  //cp->cd(1);
  //pxy->Draw("colz");

  THoughPeakFinder * pf = new THoughPeakFinder( pxy, 3 );
  pf->SetXlimit( 10 );
  pf->SetYlimit( 10 );
  pf->Reduce();
  for( Int_t ipeak = 0; ipeak < pf->GetEntries(); ipeak++ )
    {
      THoughPeak * p = pf->At(ipeak);

      Double_t theta = p->GetX()/binsperx + pxy->GetXaxis()->GetXmin();
      Double_t r = p->GetY()/binspery + pxy->GetYaxis()->GetXmin();

      //TMarker * marker = new TMarker(theta,r,23);

      Double_t costheta = cos(theta * 3.14159/180.);
      Double_t sintheta = sin(theta * 3.14159/180.);

      if( sintheta < 10E-14 )
	continue;

      Double_t m = - costheta/sintheta;
      Double_t b = r/sintheta;

      TVector3 * v = new TVector3( m,b,0 );
      Addxyline( v );

      //marker->SetMarkerColor(2);
      //marker->Draw("same");
      }

  Int_t xylines = pf->GetEntries();

  delete pf;

  //cp->cd(2);
  //pyz->Draw("colz");

  pf = new THoughPeakFinder( pyz, 3 );
  pf->SetXlimit( 10 );
  pf->SetYlimit( 10 );
  pf->Reduce();
  for( Int_t ipeak = 0; ipeak < pf->GetEntries(); ipeak++ )
    {
      THoughPeak * p = pf->At(ipeak);

      Double_t theta = p->GetX()/binsperx + pxy->GetXaxis()->GetXmin();
      Double_t r = p->GetY()/binspery + pxy->GetYaxis()->GetXmin();

      //TMarker * marker = new TMarker(theta,r,23);

      Double_t costheta = cos(theta * 3.14159/180.);
      Double_t sintheta = sin(theta * 3.14159/180.);

      if( sintheta < 10E-14 )
	continue;

      Double_t m = - costheta/sintheta;
      Double_t b = r/sintheta;

      TVector3 * v = new TVector3( m,b,0 );
      Addyzline( v );

      //marker->SetMarkerColor(2);
      //marker->Draw("same");
      }
  Int_t yzlines = pf->GetEntries();

  delete pf;

  //cp->Modified();
  //cp->Update();

  delete avgHitList;

  if( xylines == 1 && yzlines == 1)
    return 1; // cosmic
  else if( xylines > 1 || yzlines > 1 )
    return 2; // pbar-like
  else
    return 0;
}

//_____________________________________________________________________
/*Int_t TAlphaEvent::Classify()
{
  //printf("---------- Classify --------------\n");

  Int_t NHits = GatherHits();

  if( NHits < 3 ) return 0; // noise
  if( NHits > fNHitsCut ) return 3; // unreconstructable

  Int_t c = IsCosmic();

  if( c == 1 ) // cosmic candidate
    return 1;
  else if( c == 2 )
    return 2; // Pbar-like

  return 4;
}*/

//_____________________________________________________________________
TObjArray* TAlphaEvent::GatherHits()
{
  fVerbose.GatherHits();
  TObjArray* hits=new TObjArray();
  TAlphaEventSil *sil; 	// The silicon module objects that have the detector hits
  // Fill the silicon module objects and count the number of hits
  for(Int_t i = 0; i < GetNSil(); i++)
    {
      sil = GetSil(i); 		// Fill the silicon module array
      for(Int_t j = 0; j < sil->GetNHits(); j++) // Loop over the points
          hits->Add(sil->GetHit(j));
    }

  fVerbose.Message("TAlphaEvent::GatherHits",
                   "NumSil: %d, NumHits: %d\n",GetNSil(),hits->GetEntries());
  //printf("TAlphaEvent::GatherHits - NumSil: %d, NumHits: %d\n",GetNSil(),GetNHits());
  return hits;
}

//_____________________________________________________________________
Int_t TAlphaEvent::GatherTrackCandidates()
{
  fVerbose.GatherTracks();

  if( GetNTracks() )
    ClearTracks();
  TObjArray* Hits=GatherHits();
  const Int_t NHits = Hits->GetEntries();
  if(NHits < 3)
    {
      fVerbose.Warning("TAlphaEvent::GatherTracks",
		       "Less than three hits, aborting\n");
      delete Hits;
      return 0;		// Can reconstruct anything
    }
  if(NHits > fNHitsCut)
    {
      fVerbose.Warning("TAlphaEvent::GatherTracks",
		       "Too many hits, aborting\n");
      delete Hits;
      return 0;
    }

  for(Int_t i = 0; i < NHits; i++)
  {
    TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
    for(Int_t j = i + 1; j < NHits; j++)
    {
      TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
      // make sure the hits are on different layers
      if (hi->GetLayer() == hj->GetLayer()) continue;
      for(Int_t k = j + 1; k < NHits; k++)
        {
          TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);

          // make sure the hits are on different layers
          if( (hi->GetLayer() == hk->GetLayer()) ||
              (hj->GetLayer() == hk->GetLayer()) ) continue;

	  // sort the hits, so the first layer is always array
	  // zero, etc
	  TAlphaEventHit *h0 = NULL;
	  if( hi->GetLayer() == 0 ) h0 = hi;
	  else if( hj->GetLayer() == 0 ) h0 = hj;
	  else if( hk->GetLayer() == 0 ) h0 = hk;

	  TAlphaEventHit *h1 = NULL;
	  if( hi->GetLayer() == 1 ) h1 = hi;
	  else if( hj->GetLayer() == 1 ) h1 = hj;
	  else if( hk->GetLayer() == 1 ) h1 = hk;

	  TAlphaEventHit *h2 = NULL;
	  if( hi->GetLayer() == 2 ) h2 = hi;
	  else if( hj->GetLayer() == 2 ) h2 = hj;
	  else if( hk->GetLayer() == 2 ) h2 = hk;

	  // find the azimuthal angle between the hits
	  // and deal with the multivaluedness of ATan2
	  Double_t phi0 = TMath::ATan2(h0->YMRS(),h0->XMRS());
	  Double_t phi1 = TMath::ATan2(h1->YMRS(),h1->XMRS());
	  Double_t phi2 = TMath::ATan2(h2->YMRS(),h2->XMRS());
	  Double_t phi0_2pi = phi0 + 2*TMath::Pi();
	  Double_t phi1_2pi = phi1 + 2*TMath::Pi();
	  Double_t phi2_2pi = phi2 + 2*TMath::Pi();

	  Double_t phi01[3] = { fabs(phi0-phi1),
				fabs(phi0-phi2_2pi),
				fabs(phi0_2pi-phi1) };
	  Double_t phi12[3] = { fabs(phi1-phi2),
				fabs(phi1-phi2_2pi),
				fabs(phi1_2pi-phi2) };

	  // take the smallest
	  Double_t mphi01 = TMath::MinElement( 3, phi01 );
	  Double_t mphi12 = TMath::MinElement( 3, phi12 );

      if( mphi01 + mphi12 > fHitSepCutPhi ) continue;
          Double_t Z01 = fabs(h0->ZMRS() - h1->ZMRS());		// first distance
          Double_t Z12 = fabs(h1->ZMRS() - h2->ZMRS());		// second distance

	  
	  if( Z01 > fHitSepCutZ || Z12 > fHitSepCutZ ) continue;

	  // passed the cuts, make a track candidate
          TAlphaEventTrack * Track = new TAlphaEventTrack();

          Track->AddHit(h0);
          Track->AddHit(h1);
          Track->AddHit(h2);

          Track->MakeLinePCA();

          fVerbose.Message("TAlphaEvent::GatherTracks",
                           "nTracks: %d cor: %lf ",
			   GetNTracks(),Track->Getcor());

          if( Track->Getcor() > fCorCut )
            {
              fVerbose.Message("","YES");
              AddTrack( Track );
            }
          else
            {
              fVerbose.Message("","NO");
              delete Track; // condidate track not being placed into container, so delete here
            }
          fVerbose.Message(""," %d %d %d %d\n",NHits,i,j,k);

        }
      }
    }

  fVerbose.Message("TAlphaEvent::GatherTracks",
		   "Total number of track candidates: %d\n",GetNTracks());
  //printf("TAlphaEvent::GatherTracks - Total number of track candidates: %d\n",GetNTracks());
  delete Hits;
  return GetNTracks();
}

//_____________________________________________________________________
Int_t TAlphaEvent::IsSameHit( TAlphaEventHit * hit1, TAlphaEventHit * hit2 )
{
  // Compare two hits
  if( ( hit1->XMRS() == hit2->XMRS() ) &&
      ( hit1->YMRS() == hit2->YMRS() ) &&
      ( hit1->ZMRS() == hit2->ZMRS() ) )
    return kTRUE;

  return kFALSE;
}

Bool_t TAlphaEvent::IsSameHelix(int& ai, int& bi, Bool_t DeleteOne)
{
  TAlphaEventHelix* a=fHelices[ai];
  TAlphaEventHelix* b=fHelices[bi];
  for( Int_t ih = 0; ih<3; ih++)
  {
    TAlphaEventHit * ihit = (TAlphaEventHit*) a->GetHit( ih );
    for( Int_t jh = 0; jh<3; jh++)
    {
      
      // for each pair, compare each point (nine combinations)
      
      TAlphaEventHit * jhit = (TAlphaEventHit*) b->GetHit( jh );

      if( IsSameHit( ihit,jhit ) )
        {
        // Flag the doubles
        fVerbose.Warning("TAlphaEvent::RemoveDuplicateHelices",
                 "Found double, removing helix\n");

        if( fabs(a->Getfc()) < fabs(b->Getfc()) && DeleteOne )
        {
          a->SetHelixStatus( 1);
          //fHelices->Remove(b);
          if (b) {
            delete b;
            fHelices[bi]=NULL;
            //hj->SetHelixStatus(-6);
          }
        }
        else
        {
          //fHelices->Remove(a);
          //hi->SetHelixStatus(-6);
          b->SetHelixStatus( 1);
          if (a){
            delete a;
            fHelices[ai]=NULL;
          }
        }
      return kTRUE;
      }
    }
  }
  return kFALSE;
}

//_____________________________________________________________________
void TAlphaEvent::RemoveDuplicateHelices()
{
  //  fHelices->Compress();

  const Int_t NHelices = GetNHelices();
  Int_t rh=NHelices;
  fVerbose.Message("RemoveDuplicateHelices",
		   "Number of helices before removing doubles: %d\n",NHelices);

  // compare all helices
  for( Int_t i = 0; i < NHelices; i++ )
  {
    TAlphaEventHelix * hi = (TAlphaEventHelix*) fHelices.at( i );
    if (!hi) continue;
    for( Int_t j = i+1; j <NHelices; j++ )
      {
        TAlphaEventHelix * hj = (TAlphaEventHelix*) fHelices.at( j );
        if( !hj ) continue;
        if( hi->GetHelixStatus() <0 || hj->GetHelixStatus()<0 ) continue;
        if (IsSameHelix(i,j)) --rh; //Delete duplicate in here
        if (!fHelices[i]) break;
      }
  }
  //fHelices->Compress();

  fVerbose.Message("TAlphaEvent::RemoveDuplicateHelices",
                   "After removing doubles: %d\n",rh);
  //printf("TAlphaEvent::RemoveDuplicateHelices - After removing doubles: %d\n",rh);
}

//_____________________________________________________________________
Int_t TAlphaEvent::RecTrackCandidates()
{
  // Calculate the helix parameters for all the track candidates

  // find the total number of candidate tracks
  const Int_t NTracks = GetNTracks();
  fHelices.reserve(NTracks);
  // Compute the helix parameters for each candidate track
  for( Int_t iTrack = 0; iTrack < NTracks; iTrack++ )
    {
      TAlphaEventTrack * track = GetTrack( iTrack );
      TAlphaEventHelix * helix = new TAlphaEventHelix( track );
      AddHelix( helix );
    }

  return 0;
}

//_____________________________________________________________________
Int_t TAlphaEvent::PruneTracks()
{
  // Cut
  int NUnpruned = GetNHelices();
  for( Int_t iHelix = 0; iHelix<NUnpruned; iHelix++ )
    {
      TAlphaEventHelix *h = GetHelix(iHelix);
      if(h->GetChi2()>fChi2Cut ||(fabs(h->Getfd0_trap())>fd0_trapCut) )
      {
        //fHelices->SetOwner(kTRUE);
        //fHelices->RemoveAt(iHelix); //cut on chisq or cut on track dca
        delete h;
        fHelices[iHelix]=NULL;
      }
    }
  //fHelices->Compress();
  RemoveDuplicateHelices();
  IsGhostTrack();

  return 1;
}

//_____________________________________________________________________
Int_t TAlphaEvent::RecVertex()
{
  // start by including all the helices
  TAlphaEventVertex * vertex = new TAlphaEventVertex();
  int nhel=GetNHelices();
  for(Int_t ihelix=0; ihelix<nhel; ihelix++)
    {
      TAlphaEventHelix *h = GetHelix(ihelix);
      if (!h) continue;
      if(h->GetHelixStatus()<0) {delete h; continue;}
	//printf("Helix %d Stat %d\n",ihelix,h->GetHelixStatus());
      vertex->AddHelix(h);

    }
  vertex->RecVertex();

  /* printf("Initial DCA = %lf (%lf,%lf,%lf)\n",
	 vertex->GetDCA(),
	 vertex->X(),
	 vertex->Y(),
	 vertex->Z());
  */
  Int_t totalNHelices = vertex->GetNHelices();
  // printf("totalNHelices: %d\n",totalNHelices);

  while(totalNHelices>2)
    {
      static TObjArray *vertices = NULL;
      if(!vertices)
	{
	  vertices = new TObjArray();
	}

      Double_t dcas[totalNHelices];

      for(Int_t excluded_helix=0; excluded_helix<totalNHelices; excluded_helix++ )
	{
	  TAlphaEventVertex * tvertex = new TAlphaEventVertex();

	  for(Int_t ihelix=0; ihelix<vertex->GetNHelices(); ihelix++)
	    {
	      if( ihelix == excluded_helix ) continue;
	      TAlphaEventHelix *h = vertex->GetHelix(ihelix);

	      tvertex->AddHelix(h);
	    }
	  tvertex->RecVertex();

	  // If the reconstructed vertex is well far from the origin it
	  // probably means the routine is getting confused by a electron-positron
	  // pair producing near the detector. These can form secondary vertices
	  // with small dcas. I'll artifically increase this dca to bias away
	  // from these vertices, because we're only interested in the
	  // primary vertex
	  TVector3 * v = new TVector3(tvertex->X(),tvertex->Y(),tvertex->Z());
	  if(v->XYvector().Mod() > fVertRadCut) // vertex radius
	    {
	      tvertex->SetDCA(9999);
	    }
	  delete v;

	  /*
	  printf("DCA = %lf (%1.2lf,%1.2lf,%1.2lf)\n",
		 tvertex->GetDCA(),
		 tvertex->X(),
		 tvertex->Y(),
		 tvertex->Z()); */
	  dcas[excluded_helix] = tvertex->GetDCA();

	  vertices->Add(tvertex);
	}

      Int_t vidx[totalNHelices];
      TMath::Sort(totalNHelices,dcas,vidx,kFALSE);
      /*
      printf(
	     "Min: %lf idx: %d, improve: %lf\n",
	     dcas[vidx[0]],
	     vidx[0],
	     (vertex->GetDCA()-dcas[vidx[0]])/vertex->GetDCA()
	     );
      */
      // check if removing any of the helices made a significant improvement
      if( (vertex->GetDCA()-dcas[vidx[0]])/vertex->GetDCA() > fVertDCACut )
	{
	  delete vertex;
	  vertex = (TAlphaEventVertex*) vertices->At(vidx[0]);
	  totalNHelices--;

	  vertices->Remove(vertex);

	  // delete the helices that aren't used
	  vertices->Delete();
	}
      else
	{
	  vertices->Delete();
	  break;
	}
    }

  // Keep the best vertex
  fVertex.SetXYZ( vertex->X(),
		  vertex->Y(),
		  vertex->Z());
  fVertex.SetDCA( vertex->GetDCA() );
  for(Int_t ihelix=0; ihelix<vertex->GetNHelices(); ihelix++)
    {
      fVertex.AddHelix( vertex->GetHelix(ihelix) );
    }
  for(Int_t iha=0; iha<vertex->GetNDCAa(); iha++)
    {
      fVertex.AddDCAa( vertex->GetDCAa(iha) );
      fVertex.AddDCAb( vertex->GetDCAb(iha) );
    }
  fVertex.SetIsGood( vertex->IsGood() );

  // delete the vertex when done
  delete vertex;

  //fVerbose.PrintVertex();
  fVerbose.PrintMem("ReconstructTracks::End");
  return fVertex.IsGood();
}

//_____________________________________________________________________
Double_t TAlphaEvent::RecRPhi( Bool_t PlotProj )
{
  // This reconstruction method projects the tracks onto the trap surface.
  // The intersection points are then clustered in the z-rphi space and
  // the resulting cluster taken as the event's primary vertex.
  // A measure of the quality of fit is how closely the intersection points
  // cluster together.
  if (GetNHelices() < 2) return 0.;
  Double_t initial_seed = 999999.;
  TProjClusterAna * projana = new TProjClusterAna(this);

  // determine the intersection points to the trap surface
  int n=GetNHelices();
  for( Int_t i = 0; i<n; i++ )
    {
      TAlphaEventHelix *h = GetHelix(i);
      if (!h) continue;
      if(h->GetHelixStatus()<0) continue;

      // use the Helix methods to determine the where along the
      // track it crosses the trap radius
      Int_t iflag=0;
      Double_t s1 = h->GetsFromR( TrapRadius, iflag );
      // the outgoing point has the opposite arclength parameter
      Double_t s2 = -s1;

      if( s1 != 0.0 ) // proper intersection (should be two points)
	{
	  TVector3* v1 = new TVector3(h->GetPoint3D_C( s1 ));
	  TVector3* v2 = new TVector3(h->GetPoint3D_C( s2 ));
	  //  AddMCPoint( v1 );
	  //  AddMCPoint( v2 );

	  // create the first cluster objects
	  TProjClusterBase *p1 = new TProjClusterBase(TrapRadius*v1->Phi(),v1->Z());
	  TProjClusterBase *p2 = new TProjClusterBase(TrapRadius*v2->Phi(),v2->Z());

	  p1->SetTrack(i);
	  p2->SetTrack(i);

	  TVector3 *vv1 = new TVector3(h->GetPoint3D_C(s1+0.5));
	  TVector3 *vv2 = new TVector3(h->GetPoint3D_C(s2-0.5));

	  // find the angle between the track and the trap surface
	  p1->SetAngle( vv1, v1 );
	  p2->SetAngle( vv2, v2 );

	  delete vv1;
	  delete vv2;

	  p1->SetLambda( h->Getflambda() );
	  p2->SetLambda( h->Getflambda() );

	  // determine the resolutions of each cluster point
	  p1->SetSigmas(kTRUE);
	  p2->SetSigmas(kTRUE);

	  // when doing MC simulations, determine which point of the two is
	  // closer to the true vertex
	  if(GetMCVertex())
	    {
	      Double_t ss1  = TrapRadius*(fMCVertex.Phi() - v1->Phi());
	      Double_t ss11 = TrapRadius*(fMCVertex.Phi() - (v1->Phi()-2*TMath::Pi()));
	      Double_t ss12 = TrapRadius*(fMCVertex.Phi() - (v1->Phi()+2*TMath::Pi()));
	      Double_t ss2  = TrapRadius*(fMCVertex.Phi() - v2->Phi());
	      Double_t ss21 = TrapRadius*(fMCVertex.Phi() - (v2->Phi()-2*TMath::Pi()));
	      Double_t ss22 = TrapRadius*(fMCVertex.Phi() - (v2->Phi()+2*TMath::Pi()));

	      Double_t min1 = TMath::Min( fabs(ss1), TMath::Min( fabs(ss11), fabs(ss12) ) );
	      Double_t min2 = TMath::Min( fabs(ss2), TMath::Min( fabs(ss21), fabs(ss22) ) );

	      //if( fabs(TMath::Min(ss1,ss11)) < fabs(TMath::Min(ss2,ss22)) )
	      if( min1 < min2 )
		{
		  p1->SetMCClosest(kTRUE);
		  p2->SetMCClosest(kFALSE);
		}
	      else
		{
		  p1->SetMCClosest(kFALSE);
		  p2->SetMCClosest(kTRUE);
		}
	    }

	  delete v1;
	  delete v2;
	  TProjCluster *c1 = new TProjCluster(p1);
	  TProjCluster *c2 = new TProjCluster(p2);

	  // add the cluster points to the analysis
	  projana->AddLast(c1);
	  projana->AddLast(c2);

	  // duplicate the outer points to take care of the phi wraparound
	  if( p1->RPhi() >= TrapRadius*0.75*TMath::Pi() &&
	      p1->RPhi() <= TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p11 = new TProjClusterBase(p1);
	      p11->SetRPhi( p11->RPhi() - 2.*TrapRadius*TMath::Pi() );
	      TProjCluster *c11 = new TProjCluster(p11);
	      delete p11;
	      projana->AddLast(c11);
	    }
	  if( p2->RPhi() >= TrapRadius*0.75*TMath::Pi() &&
	      p2->RPhi() <= TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p22 = new TProjClusterBase(p2);
	      p22->SetRPhi( p22->RPhi() - 2.*TrapRadius*TMath::Pi() );
	      TProjCluster *c22 = new TProjCluster(p22);
	      delete p22;
	      projana->AddLast(c22);
	    }
	  if( p1->RPhi() <= -TrapRadius*0.75*TMath::Pi() &&
	      p1->RPhi() >= -TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p11 = new TProjClusterBase(p1);
	      p11->SetRPhi( p11->RPhi() + 2.*TrapRadius*TMath::Pi() );
	      TProjCluster *c11 = new TProjCluster(p11);
	      delete p11;
	      projana->AddLast(c11);
	    }
	  if( p2->RPhi() <= -TrapRadius*0.75*TMath::Pi() &&
	      p2->RPhi() >= -TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p22 = new TProjClusterBase(p2);
	      p22->SetRPhi( p22->RPhi() + 2.*TrapRadius*TMath::Pi() );
	      TProjCluster *c22 = new TProjCluster(p22);
	      delete p22;
	      projana->AddLast(c22);
	    }
      delete p1;
      delete p2;
	}

      else // the helices that don't intersect the trap wall
	{
	  TVector3* v = new TVector3(h->GetPoint3D_C( s1 ));
	  // if( h->Getfd0_trap() > 1.5 ) continue;

	  Double_t vx = v->X();
	  Double_t vy = v->y();
	  Double_t vd = TMath::Sqrt( vx*vx + vy*vy );
	  vx = vx/vd*TrapRadius;
	  vy = vy/vd*TrapRadius;
	  v->SetX( vx );
	  v->SetY( vy );

	  // AddMCPoint( v );

	  TProjClusterBase *p = new TProjClusterBase(TrapRadius*v->Phi(),v->Z());
	  delete v;

	  p->SetTrack(i);
	  p->SetAngleFromNormal( -999. );
	  p->Setd0( h->Getfd0() );
	  p->SetSigmas(kTRUE);
	  TProjCluster *c = new TProjCluster(p);

	  projana->AddLast(c);

	  if( p->RPhi() >= TrapRadius*0.75*TMath::Pi() &&
	      p->RPhi() <= TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p_m = new TProjClusterBase(p);
	      p_m->SetRPhi( p->RPhi() - 2*TrapRadius*TMath::Pi() );
	      TProjCluster *c_m = new TProjCluster(p_m);
	      delete p_m;
	      projana->AddLast(c_m);
	    }
	  if( p->RPhi() <= -TrapRadius*0.75*TMath::Pi() &&
	      p->RPhi() >= -TrapRadius*1.00*TMath::Pi() )
	    {
	      TProjClusterBase *p_m = new TProjClusterBase(p);
	      p_m->SetRPhi( p->RPhi() + 2.*TrapRadius*TMath::Pi() );
	      TProjCluster *c_m = new TProjCluster(p_m);
	      delete p_m;
	      projana->AddLast(c_m);
	    }
      delete p;
      
	}
  }

  initial_seed = projana->FindMinPair();
  // do the clustering
  // This is a hierarachal clustering method, ala http://en.wikipedia.org/wiki/Cluster_analysis
  if( initial_seed < fMinDistCut )
  {
      projana->CombineMinPair();

      for( Int_t i = 0; i<projana->GetEntriesFast()-1; i++ )
      {
	  Double_t min = projana->FindClosestPoint();
	  //printf("min: %lf\n",min);
	  if( min > fMinClosestCut ) break;
	  projana->CombineMinPair(); // combine to form the clusters
      }
  }


  // Get ready to do the vertexing
//   TAlphaEventVertex * vertex = GetVertex();
//
//   for( Int_t i = 0; i<GetNHelices(); i++)
//     {
//       TAlphaEventHelix * h = GetHelix(i);
//       //h->SetIsIncluded( kFALSE );
//       vertex->AddHelix( h );
//     }

  if( projana->GetSeed() >= 0. )
    {
      TProjCluster *pcbv = (TProjCluster*) projana->GetProjCluster( projana->GetSeed() );
      fProjClusterVertex = (TProjClusterBase*) pcbv->GetMean();

      //fProjClusterVertex->Print();
      //fVerbose.PrintProjClusterVertex();

//       for( Int_t j = 0; j < pcbv->GetEntries(); j++ )
// 	{
// 	  TProjClusterBase *pc = pcbv->At(j);
//
// 	  TAlphaEventHelix * h = vertex->GetHelix(pc->GetTrack());
// 	  //h->SetIsIncluded( kTRUE );
// 	  vertex->AddHelix( h );
// 	}
    }

  if( PlotProj ) projana->Draw();
  delete projana;
  return initial_seed;
}


//_____________________________________________________________________
TVector3 *TAlphaEvent::GetCosmicVector()
{
  Int_t nHelix = GetNHelices();

  TAlphaEventTrack * Best_Cosmic = new TAlphaEventTrack();
  Double_t  Best_Cor = 0.;

  printf("NTracks: %d\n",nHelix);
  if( nHelix < 1 ) return (TVector3*)NULL;

  // Gather 6 hits from every pair of helices and find the set with the highest correlation coefficient
  for(Int_t i = 0;i<nHelix;i++)
    {
        TAlphaEventHelix * trackone = GetHelix(i);

        TAlphaEventHit * ha = trackone->GetHit(0);
        TAlphaEventHit * hb = trackone->GetHit(1);
        TAlphaEventHit * hc = trackone->GetHit(2);

        TAlphaEventTrack * Cosmic = new TAlphaEventTrack();

        Cosmic->AddHit(ha);
        Cosmic->AddHit(hb);
        Cosmic->AddHit(hc);

        Cosmic->MakeLinePCA();

        Double_t cor = Cosmic->Getcor();

        if( cor > Best_Cor )
          {
            if( Best_Cosmic )
              {
                delete Best_Cosmic;
                Best_Cosmic = NULL;
              }
            Best_Cosmic = Cosmic;
            Best_Cor = cor;
          }
        else
          {
            delete Cosmic;
          }
      }
  return new TVector3(Best_Cosmic->Getunitvector());
}

//_____________________________________________________________________
Double_t TAlphaEvent::CosmicHelixTest()
{
  Int_t nHelix = GetNHelices();

  //printf("NTracks: %d\n",nHelix);
  if( nHelix < 2 ) return -1;

  Double_t best_chi2 = 99999999;


  // Gather 6 hits from every pair of helices and find the set with the highest correlation coefficient
  for(Int_t i = 0;i<nHelix;i++)
    for(Int_t j=i+1;j<nHelix;j++)
      {
        TAlphaEventHelix * trackone = GetHelix(i);
        TAlphaEventHelix * tracktwo = GetHelix(j);

        TAlphaEventHit * ha = trackone->GetHit(0);
        TAlphaEventHit * hb = trackone->GetHit(1);
        TAlphaEventHit * hc = trackone->GetHit(2);
        TAlphaEventHit * hd = tracktwo->GetHit(0);
        TAlphaEventHit * he = tracktwo->GetHit(1);
        TAlphaEventHit * hf = tracktwo->GetHit(2);

        TAlphaEventTrack * Cosmic = new TAlphaEventTrack();

        Cosmic->AddHit(ha);
        Cosmic->AddHit(hb);
        Cosmic->AddHit(hc);
        Cosmic->AddHit(hd);
        Cosmic->AddHit(he);
        Cosmic->AddHit(hf);

	TAlphaEventCosmicHelix * Helix = new TAlphaEventCosmicHelix( Cosmic );
	if(!Helix->IsGood())
	  {
	    continue;
	  }

	Double_t chi2 = Helix->GetChi2();
	if( chi2 < best_chi2 )
	  {
	    best_chi2 = chi2;
	  }

	delete Helix;
      }

  // printf("best_chi2: %lf\n",best_chi2);
  return best_chi2;
}


//_____________________________________________________________________
Double_t TAlphaEvent::CosmicTest()
{
  // Used for rejection of cosmic background.
  // For events with 2+ helices this will take every pair of helices and fit a straight line to those 6 hits. After its found the set of 6 hits with the highest correlation coefficient,
  // it will then find the sum of the residuals squared,  where here the residual is defined as the perpendicular distance from the point to the line (rather than the distance on the
  // plane of each module). The routine will return the best set of 6 hits and the associated residual (mislabled chi here).

  Int_t nHelix = GetNHelices();

  Double_t res = -1.;
  TAlphaEventTrack * Best_Cosmic = NULL;// new TAlphaEventTrack();
  Double_t  Best_Cor = 0.;

  if( nHelix < 2 ) return -1;

  Int_t nh=0;
  for(Int_t i=0;i<nHelix;i++)
    {
      TAlphaEventHelix * t = GetHelix(i);
      if(t->GetHelixStatus()>0) nh++;
    }

  //printf("NTracks: %d nh: %d\n",nHelix,nh);
  if( nh < 2 ) return -1;

  // Gather 6 hits from every pair of helices and find the set with the highest correlation coefficient
  for(Int_t i=0;i<nHelix;i++)
    for(Int_t j=i+1;j<nHelix;j++)
      {
        TAlphaEventHelix * trackone = GetHelix(i);
        TAlphaEventHelix * tracktwo = GetHelix(j);

	if(trackone->GetHelixStatus()<0) continue;
	if(tracktwo->GetHelixStatus()<0) continue;

        TAlphaEventHit * ha = trackone->GetHit(0);
        TAlphaEventHit * hb = trackone->GetHit(1);
        TAlphaEventHit * hc = trackone->GetHit(2);
        TAlphaEventHit * hd = tracktwo->GetHit(0);
        TAlphaEventHit * he = tracktwo->GetHit(1);
        TAlphaEventHit * hf = tracktwo->GetHit(2);

        TAlphaEventTrack * Cosmic = new TAlphaEventTrack();

        Cosmic->AddHit(ha);
        Cosmic->AddHit(hb);
        Cosmic->AddHit(hc);
        Cosmic->AddHit(hd);
        Cosmic->AddHit(he);
        Cosmic->AddHit(hf);

        Cosmic->MakeLinePCA();

        Double_t cor = Cosmic->Getcor();

        if( cor > Best_Cor && cor != 1.0 )
          {
            if( Best_Cosmic )
              {
                delete Best_Cosmic;
                Best_Cosmic = NULL;
              }
            Best_Cosmic = Cosmic;
            Best_Cor = cor;
          }
        else
          {
            delete Cosmic;
          }
      }
      //printf("BestCor = %lf\n",Best_Cor);
  if( Best_Cor == 0. ) return -1;

  //Double_t dca=Best_Cosmic->DetermineDCA();
  Best_Cosmic->DetermineDCA();
  res=Best_Cosmic->CalculateTheResidual();
  //printf("cosmic dir %lf\n",Best_Cosmic->Getunitvector().Mag());
  SetCosmicTrack(Best_Cosmic);
  delete Best_Cosmic;
//  if(Best_Cosmic)
//   {
//   	dca=Best_Cosmic->DetermineDCA();
//   	res=Best_Cosmic->CalculateTheResidual();
//   	fCosmic.Setcor(Best_Cosmic->Getcor());
// 	fCosmic.Setunitvector(Best_Cosmic->Getunitvector());
// 	fCosmic.Setr0(Best_Cosmic->Getr0());
// 	fCosmic.SetDCAandRES(dca,res);
//
//   	delete Best_Cosmic;
//   }
  return res;
}
//_____________________________________________________________________
void TAlphaEvent::SetCosmicTrack(TAlphaEventTrack* Track )
{
	fCosmic.Setcor(Track->Getcor());
	fCosmic.Setunitvector(Track->Getunitvector());
	fCosmic.Setr0(Track->Getr0());
	fCosmic.SetDCA(Track->GetDCA());
	fCosmic.SetRES(Track->GetRES());
    for(Int_t i=0;i<Track->GetNHits();++i)
      fCosmic.AddHit(Track->GetHit(i));
}


//_____________________________________________________________________
TAlphaEventCosmicHelix *TAlphaEvent::FindHelix()// Int_t hlimit = 3 )
{
	
  TObjArray* Hits=GatherHits();
  const Int_t nHits = Hits->GetEntriesFast();
 if(nHits < 5)
    {
      fVerbose.Warning("TAlphaEvent::CosmicSearch",
                       "Less than four hits, aborting\n");
      //      printf("less than 3 \n");
      return 0;		// Can reconstruct anything
    }
    
 TAlphaEventCosmicHelix* best_track = NULL;
 Double_t best_cor = 0.;

  if(nHits > 5)// && hlimit <= 6)
  {
    fVerbose.Message("TAlphaEvent::CosmicSearch","Trying to find a good set of 6 hits\n");
    //printf("Trying to find a good set of 6 hits\n");
    for(Int_t i = 0; i < nHits; i++)
    {
      TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
      for(Int_t j = i + 1; j < nHits; j++)
      {
        TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
        for(Int_t k = 0; k < nHits && k != i && k != j ; k++)
        {
          TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);
          for(Int_t l = 0; l < nHits && l != i && l != j && l != k ; l++)
          {
            TAlphaEventHit * hl = (TAlphaEventHit*)Hits->At(l);
            for(Int_t m = 0; m < nHits && m != i && m != j && m != k  && m != l ; m++)
            {
              TAlphaEventHit * hm = (TAlphaEventHit*)Hits->At(m);
              for(Int_t n = 0; n < nHits  && n != i && n != j && n != k  && n != l && n !=m; n++)
              {
                TAlphaEventHit * hn = (TAlphaEventHit*)Hits->At(n);
                Int_t layer_tot[3] ={0,0,0};
                // Filter out the case where more than two of the hits are on the same layer
                for(Int_t i = 0; i < 3; i++){
                  if (hk->GetLayer()==i) layer_tot[i] ++;
                  if (hl->GetLayer()==i) layer_tot[i] ++;
                  if (hm->GetLayer()==i) layer_tot[i] ++;
                  if (hn->GetLayer()==i) layer_tot[i] ++;
                  if (hi->GetLayer()==i) layer_tot[i] ++;
                  if (hj->GetLayer()==i) layer_tot[i] ++;
                }
                //if (layer_tot[0] > 2 || layer_tot[1] > 2 || layer_tot[2] > 2 || layer_tot[0] < 1 ) continue;
                if (layer_tot[0] < 1) continue;
                // Don't use hits from the same module
                if((hk->GetSilNum() == hl->GetSilNum()) ||
                  (hk->GetSilNum() == hm->GetSilNum()) ||
                  (hk->GetSilNum() == hn->GetSilNum()) ||
                  (hk->GetSilNum() == hi->GetSilNum()) ||
                  (hk->GetSilNum() == hj->GetSilNum()) ||
                  (hl->GetSilNum() == hm->GetSilNum()) ||
                  (hl->GetSilNum() == hn->GetSilNum()) ||
                  (hl->GetSilNum() == hi->GetSilNum()) ||
                  (hl->GetSilNum() == hj->GetSilNum()) ||
                  (hm->GetSilNum() == hn->GetSilNum()) ||
                  (hm->GetSilNum() == hi->GetSilNum()) ||
                  (hm->GetSilNum() == hj->GetSilNum()) ||
                  (hn->GetSilNum() == hi->GetSilNum()) ||
                  (hn->GetSilNum() == hj->GetSilNum()) ||
                  (hi->GetSilNum() == hj->GetSilNum()))
                  continue;

                TAlphaEventTrack * Track = new TAlphaEventTrack();

                Track->AddHit(hi);
                Track->AddHit(hj);
                Track->AddHit(hk);
                Track->AddHit(hl);
                Track->AddHit(hm);
                Track->AddHit(hn);
                // Track->Print();
                TAlphaEventCosmicHelix* Helix=new TAlphaEventCosmicHelix(Track);
                
                Helix->Print();
                if (Helix->GetResiduals()>best_cor) best_track=Helix;
                //if (Helix->GetResiduals()>500) return Helix;
                
 //TAlphaEventCosmicHelix* best_track = NULL;
 //Double_t best_cor = 0.;
              }
            }
          }
        }
      }
    }
  }
  return best_track;
  //return 0;
}
//_____________________________________________________________________
TAlphaEventTrack *TAlphaEvent::FindCosmic( Int_t hlimit = 3 )
{
  // Search each event for a straight cosmic ray track. First searches for tracks of 6 hits, failing that it searches for tracks of 5 hits, then 4, then 3.
  // There is probably a faster way of doing this search, at the moment it searches through every possible set of hits.

  //  printf("Enter Search");

  // if( GetNTracks() )
  //   ClearTracks();

//  Int_t nHits = GetNHits(); // Use this method if calling from a macro

  TObjArray* Hits=GatherHits();
  const Int_t nHits = Hits->GetEntries();

//    Int_t nHits = GatherHits(); // Use this method if calling from within alphaAnalysis


  if(nHits < 3)
    {
      fVerbose.Warning("TAlphaEvent::CosmicSearch",
                       "Less than four hits, aborting\n");
      //      printf("less than 3 \n");
      return 0;		// Can reconstruct anything
    }
  if(nHits > 40)
    {
      fVerbose.Warning("TAlphaEvent::CosmicSearch",
                       "Too many hits, aborting\n");
      return 0;
    }

  // this function will be used for analysing cosmic runs ONLY
  // best the function AddTrack at the end of this function may
  // cause confusion with GatherTrackCandidates
  if(!IsACosmic()) { printf("For use in cosmic runs ONLY\n"); return 0;}
  //else printf("cosmic run\n");

  TAlphaEventTrack * best_track = NULL;
  Double_t best_cor = 0.;

  if(nHits > 5 && hlimit <= 6)
    {
      fVerbose.Message("TAlphaEvent::CosmicSearch","Trying to find a good set of 6 hits\n");
      //printf("Trying to find a good set of 6 hits\n");
      for(Int_t i = 0; i < nHits; i++)
        {
          TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
          for(Int_t j = i + 1; j < nHits; j++)
            {
              TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
              for(Int_t k = 0; k < nHits && k != i && k != j ; k++)
                for(Int_t l = 0; l < nHits && l != i && l != j && l != k ; l++)
                  for(Int_t m = 0; m < nHits && m != i && m != j && m != k  && m != l ; m++)
                    for(Int_t n = 0; n < nHits  && n != i && n != j && n != k  && n != l && n !=m; n++)
                      {
                        TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);
                        TAlphaEventHit * hl = (TAlphaEventHit*)Hits->At(l);
                        TAlphaEventHit * hm = (TAlphaEventHit*)Hits->At(m);
                        TAlphaEventHit * hn = (TAlphaEventHit*)Hits->At(n);

                        Int_t layer_tot[3] ={0,0,0};

                        // Filter out the case where more than two of the hits are on the same layer
                        for(Int_t i = 0; i < 3; i++){
                          if (hk->GetLayer()==i) layer_tot[i] ++;
                          if (hl->GetLayer()==i) layer_tot[i] ++;
                          if (hm->GetLayer()==i) layer_tot[i] ++;
                          if (hn->GetLayer()==i) layer_tot[i] ++;
                          if (hi->GetLayer()==i) layer_tot[i] ++;
                          if (hj->GetLayer()==i) layer_tot[i] ++;
                        }
                        if (layer_tot[0] > 2 || layer_tot[1] > 2 || layer_tot[2] > 2 || layer_tot[0] < 1 ) continue;

                        // Don't use hits from the same module
                        if((hk->GetSilNum() == hl->GetSilNum()) ||
                           (hk->GetSilNum() == hm->GetSilNum()) ||
                           (hk->GetSilNum() == hn->GetSilNum()) ||
                           (hk->GetSilNum() == hi->GetSilNum()) ||
                           (hk->GetSilNum() == hj->GetSilNum()) ||
                           (hl->GetSilNum() == hm->GetSilNum()) ||
                           (hl->GetSilNum() == hn->GetSilNum()) ||
                           (hl->GetSilNum() == hi->GetSilNum()) ||
                           (hl->GetSilNum() == hj->GetSilNum()) ||
                           (hm->GetSilNum() == hn->GetSilNum()) ||
                           (hm->GetSilNum() == hi->GetSilNum()) ||
                           (hm->GetSilNum() == hj->GetSilNum()) ||
                           (hn->GetSilNum() == hi->GetSilNum()) ||
                           (hn->GetSilNum() == hj->GetSilNum()) ||
                           (hi->GetSilNum() == hj->GetSilNum()))
                          continue;

                        TAlphaEventTrack * Track = new TAlphaEventTrack();

                        Track->AddHit(hi);
                        Track->AddHit(hj);
                        Track->AddHit(hk);
                        Track->AddHit(hl);
                        Track->AddHit(hm);
                        Track->AddHit(hn);

                        Track->MakeLinePCA();

                        fVerbose.Message("TAlphaEvent::GatherTrackCandidates",
                                         "cor: %lf \n",Track->Getcor());

                        Double_t cor = Track->Getcor();

                        if( cor > best_cor && cor > .99 )
                          {
                            if( best_track )
                              {
                                delete best_track;
                                best_track = NULL;
                              }

                            best_track = Track;
                            best_cor = cor;
                          }
                        else
                          {
                            delete Track;
                          }

                      }
            }
        }

      if(best_track) SetCosmicTrack(best_track);
    }

  if(!best_track && nHits > 4 && hlimit <= 5)
    {
      fVerbose.Message("TAlphaEvent::CosmicSearch","Trying to find a good set of 5 hits\n");
      //printf("Trying to find good set of 5 hits\n");
      for(Int_t i = 0; i < nHits; i++)
        {
          for(Int_t j = i + 1; j < nHits; j++)
            {
              for(Int_t k = 0; k < nHits && k != i && k != j ; k++)
                for(Int_t l = 0; l < nHits && l != i && l != j && l != k ; l++)
                  for(Int_t m = 0; m < nHits && m != i && m != j && m != k  && m != l ; m++)
                    {
                      TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
                      TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
                      TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);
                      TAlphaEventHit * hl = (TAlphaEventHit*)Hits->At(l);
                      TAlphaEventHit * hm = (TAlphaEventHit*)Hits->At(m);


                      Int_t layer_tot[3] ={0,0,0};

                      for(Int_t i = 0; i < 3; i++){
                        if (hk->GetLayer()==i) layer_tot[i] ++;
                        if (hl->GetLayer()==i) layer_tot[i] ++;
                        if (hm->GetLayer()==i) layer_tot[i] ++;
                        if (hi->GetLayer()==i) layer_tot[i] ++;
                        if (hj->GetLayer()==i) layer_tot[i] ++;
                      }
                      if (layer_tot[0] > 2 || layer_tot[1] > 2 || layer_tot[2] > 2 || layer_tot[0] < 1) continue;

                      if((hk->GetSilNum() == hl->GetSilNum()) ||
                         (hk->GetSilNum() == hm->GetSilNum()) ||
                         (hk->GetSilNum() == hi->GetSilNum()) ||
                         (hk->GetSilNum() == hj->GetSilNum()) ||
                         (hl->GetSilNum() == hm->GetSilNum()) ||
                         (hl->GetSilNum() == hi->GetSilNum()) ||
                         (hl->GetSilNum() == hj->GetSilNum()) ||
                         (hm->GetSilNum() == hi->GetSilNum()) ||
                         (hm->GetSilNum() == hj->GetSilNum()) ||
                         (hi->GetSilNum() == hj->GetSilNum()))
                        continue;



                      TAlphaEventTrack * Track = new TAlphaEventTrack();

                      Track->AddHit(hi);
                      Track->AddHit(hj);
                      Track->AddHit(hk);
                      Track->AddHit(hl);
                      Track->AddHit(hm);

                      Track->MakeLinePCA();

                      fVerbose.Message("TAlphaEvent::GatherTrackCandidates",
                                       "cor: %lf \n",Track->Getcor());

                      Double_t cor = Track->Getcor();

                      if( cor > best_cor && cor > 0.99 )
                        {
                          if( best_track )
                            {
                              delete best_track;
                              best_track = NULL;
                            }

                          best_track = Track;
                          best_cor = cor;
                        }
                      else
                        {
                          delete Track;
                        }

                    }
            }
        }
    }

  if(!best_track && hlimit <= 4 )
    {
      fVerbose.Message("TAlphaEvent::CosmicSearch","Trying to find a good set of 4 hits\n");
      //printf("Trying to find good set of 4 hits\n");
      for(Int_t i = 0; i < nHits; i++)
        {
          for(Int_t j = i + 1; j < nHits; j++)
            {
              for(Int_t k = 0; k < nHits && k != i && k != j ; k++)
                for(Int_t l = 0; l < nHits && l != i && l != j && l != k ; l++)
                  {
                    TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
                    TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
                    TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);
                    TAlphaEventHit * hl = (TAlphaEventHit*)Hits->At(l);

                    Int_t layer_tot[3] ={0,0,0};

                    for(Int_t i = 0; i < 3; i++){
                      if (hk->GetLayer()==i) layer_tot[i] ++;
                      if (hl->GetLayer()==i) layer_tot[i] ++;
                      if (hi->GetLayer()==i) layer_tot[i] ++;
                      if (hj->GetLayer()==i) layer_tot[i] ++;
                    }
                    if (layer_tot[0] > 2 || layer_tot[1] > 2 || layer_tot[2] > 2 || layer_tot[0] < 1) continue;

                    if((hk->GetSilNum() == hl->GetSilNum()) ||
                       (hk->GetSilNum() == hi->GetSilNum()) ||
                       (hk->GetSilNum() == hj->GetSilNum()) ||
                       (hl->GetSilNum() == hi->GetSilNum()) ||
                       (hl->GetSilNum() == hj->GetSilNum()) ||
                       (hi->GetSilNum() == hj->GetSilNum()))
                      continue;

                    TAlphaEventTrack * Track = new TAlphaEventTrack();

                    Track->AddHit(hi);
                    Track->AddHit(hj);
                    Track->AddHit(hk);
                    Track->AddHit(hl);

                    Track->MakeLinePCA();

                    fVerbose.Message("TAlphaEvent::GatherTrackCandidates",
                                     "cor: %lf \n",Track->Getcor());

                    Double_t cor = Track->Getcor();

                    if( cor > best_cor && cor > 0.99 )
                      {
                        if( best_track )
                          {
                            delete best_track;
                            best_track = NULL;
                          }

                        best_track = Track;
                        best_cor = cor;
                      }
                    else
                      {
                        delete Track;
                      }

                  }
            }
        }
    }

  if(!best_track && hlimit <=3 )
    {
      fVerbose.Message("TAlphaEvent::CosmicSearch","Trying to find a good set of 3 hits\n");
      //printf("Trying to find good set of 3 hits\n");
      for(Int_t i = 0; i < nHits; i++)
        {
          for(Int_t j = i + 1; j < nHits; j++)
            {
              for(Int_t k = 0; k < nHits && k != i && k != j ; k++)
                {
                  TAlphaEventHit * hj = (TAlphaEventHit*)Hits->At(j);
                  TAlphaEventHit * hi = (TAlphaEventHit*)Hits->At(i);
                  TAlphaEventHit * hk = (TAlphaEventHit*)Hits->At(k);


                  Int_t layer_tot[3] ={0,0,0};

                  for(Int_t i = 0; i < 3; i++){
                    if (hk->GetLayer()==i) layer_tot[i] ++;
                    if (hi->GetLayer()==i) layer_tot[i] ++;
                    if (hj->GetLayer()==i) layer_tot[i] ++;
                  }

                  if (layer_tot[0] > 2 || layer_tot[1] > 2 || layer_tot[2] > 2 || layer_tot[0] < 1) continue;

                  if((hk->GetSilNum() == hi->GetSilNum()) ||
                     (hk->GetSilNum() == hj->GetSilNum()) ||
                     (hi->GetSilNum() == hj->GetSilNum()))
                    continue;

                  if( (hi->GetLayer() == hj->GetLayer()) ||
                      (hi->GetLayer() == hk->GetLayer()) ||
                      (hj->GetLayer() == hk->GetLayer()) ) continue;

                  // If the hits are too far apart from each other (on the other side of the detector), then discard
                  TVector3 v1( hi->XMRS(), hi->YMRS(), hi->ZMRS() );
                  TVector3 v2( hj->XMRS(), hj->YMRS(), hj->ZMRS() );
                  TVector3 v3( hk->XMRS(), hk->YMRS(), hk->ZMRS() );

                  TVector3 d01(v1 - v2);		// first distance
                  TVector3 d12(v2 - v3);		// second distance

                  //printf("d01: %lf, d12: %lf  tot: %lf\n",d01.Mag(),d12.Mag(),d01.Mag()+d12.Mag());

                  if( d01.Mag() > 5. || d12.Mag() > 5. ) continue; // Make the cut on hit separation

                  TAlphaEventTrack * Track = new TAlphaEventTrack();

                  Track->AddHit(hi);
                  Track->AddHit(hj);
                  Track->AddHit(hk);

                  Track->MakeLinePCA();

                  fVerbose.Message("TAlphaEvent::GatherTrackCandidates",
                                   "cor: %lf \n",Track->Getcor());

                  Double_t cor = Track->Getcor();

                  if( cor > best_cor && cor > 0.99 )
                    {
                      if( best_track )
                        {
                          delete best_track;
                          best_track = NULL;
                        }

                      best_track = Track;
                      best_cor = cor;
                    }
                  else
                    {
                      delete Track;
                    }
                }
            }
        }
    }

  //printf(" -- COSMIC TRACK --\n");
  // if(best_track)
  // {
  //     //printf(" cor = %lf \n",best_cor);
  //     //printf(" X = %lf \t Y = %lf \t Z = %lf\n",
  //     //(best_track->Getr0()).X(),(best_track->Getr0()).Y(),(best_track->Getr0()).Z());
  //     //AddTrack(best_track);
  //     //fCosmic=*best_track;
  //     SetCosmicTrack(best_track);
  // }
  //else printf("NO\n");
  return best_track;
}

//_____________________________________________________________________
Int_t TAlphaEvent::GetResidual(TAlphaEventTrack *best_track,Int_t hitnumber,Double_t &yres,Double_t &zres)
{
  // Calculates the residual (in the plane of the module) for each hit.

  // Get cluster and fit information
  Double_t x = best_track->GetHit(hitnumber)->XMRS();
  Double_t y = best_track->GetHit(hitnumber)->YMRS();
  Double_t z = best_track->GetHit(hitnumber)->ZMRS();

  Double_t cos = best_track->GetHit(hitnumber)->GetCos();
  Double_t sin = best_track->GetHit(hitnumber)->GetSin();

  TVector3 h( best_track->Getunitvector() );
  TVector3 r0( best_track->Getr0() ); // First point

  if( r0.X() == 0. && r0.Y() == 0. && r0.Z() == 0. ) return 0; // fit failed
  TVector3 r1( r0 + 1.*h );		   // Second points
  TVector3 n( cos, sin, 0.);

  // Find the parameter that leaves us in the plane of the module
  Double_t d = -n.X()*x - n.Y()*y - n.Z()*z;
  Double_t t = ( d + n * r0 ) / ( n * (r0 -r1) );

  // Impact point of the fitted line on plane of the module
  TVector3 impact( r0 + t*(r1 - r0) );

  // Transform into local coordinates
  TVector3 impactlocal( impact.X()*cos + impact.Y()*sin,
                        -impact.X()*sin + impact.Y()*cos,
                        impact.Z());

  TVector3 coorlocal( x*cos + y*sin,-x*sin + y*cos, z);

  // Calculate Residuals
  yres = coorlocal.Y() - impactlocal.Y();
  zres = coorlocal.Z() - impactlocal.Z();

  //  printf("Sil Num: %d   yres: %lf  \t zres: %lf \n", best_track->GetHit(hitnumber)->GetSilNum(),yres,zres);
  return 1;
}

//_____________________________________________________________________
void TAlphaEvent::CosmicHitEfficiency(TH1D *phits, TH1D *pexpected,
				      TH1D *nhits, TH1D *nexpected)
{
  // Finds Cosmic tracks and fills two histograms, one for the registered hits produced by the track and another for the hits that should have been registered.
  // One can divide these two histograms to find a module by module hit efficiency (due to cosmics)

  TAlphaEventTrack * track = FindCosmic( 5 );

  Double_t good = 1.;

  if(track){

    // Make sure the track has no large residuals
    for(Int_t i = 0; i < track->GetNHits(); i++)
      {
        Double_t yres = 0.;
        Double_t zres = 0.;

        GetResidual(track,i,yres,zres);

        if(fabs(yres) > .05 || fabs(zres) > .1) good = 0.;
      }

    if(good)
      {
	TVector3 u(track->Getunitvector() );
        TVector3 r0(track->Getr0());     // First point
        TVector3 r1( r0 + 1.*u );		   // Second point

        // Fill hitogram with modules that should have been hit.
        for(Int_t m = 0; m<nSil;m++)
          {
            TAlphaEventSil * sil = GetSilByNumber(m);
            if(!sil) continue;

            Double_t cos = sil->GetCos();
            double_t sin = sil->GetSin();
            TVector3 n( cos, sin, 0.);

            if(( n * (r0 -r1) )==0.) continue;
            Double_t x = sil->GetXCenter();
            Double_t y = sil->GetYCenter();
            Double_t z = sil->GetZCenter();

            // Find the intersection of the track with a plane defined by each module
            Double_t d = -n.X()*x - n.Y()*y - n.Z()*z;
            Double_t t = ( d + n * r0 ) / ( n * (r0 -r1) );

            // Impact point of the fitted line on plane of the module
            TVector3 impact( r0 + t*(r1 - r0) );

            // Transform into local module coordinates
            TVector3 impactlocal( impact.X()*cos + impact.Y()*sin,
                                  -impact.X()*sin + impact.Y()*cos,
                                  impact.Z());

            // Check if the track crosses the plane within the actual area of the module.
            // If it didn't hit then ReturnPStrip/ReturnNStrip will return -1.
            Int_t phit = sil->ReturnPStrip(impactlocal[1]);
            Int_t nhit = sil->ReturnNStrip(impactlocal[2]);

            if(phit >=0 && nhit >=0)
	      {
		pexpected->Fill(m);
		nexpected->Fill(m);
		// there should be a hit on this module, so we'll go looking for it

		// at this point we can't find a hit on the module, so we'll
		// check at the cluster level (didn't reconstruct a hit)
		//printf("ineff!!\n");
		//printf("expected p: %lf %d\n",impactlocal[1],phit);
		for(Int_t ip=0; ip<sil->GetNPClusters(); ip++)
		  {
		    TAlphaEventPCluster * p = sil->GetPCluster(ip);
		    //printf("p: %lf\n",p->Y());

		    if( fabs(p->Y()-impactlocal[1])<0.2)  // found it
		      {
			phits->Fill(m);
			break;
		      }
		  }
		for(Int_t in=0; in<sil->GetNNClusters(); in++)
		  {
		    TAlphaEventNCluster * n = sil->GetNCluster(in);
		    //		    printf("n: %lf\n",n->Z());
		    if( fabs(n->Z()-impactlocal[2])<0.2)  // found it
		      {
			nhits->Fill(m);
			break;
		      }
		  }
	      }
          }
      }
  }
}
/*
//_____________________________________________________________________
void TAlphaEvent::ShiftHits( Double_t z)
{
  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );
      if( h->GetLayer() != 2 ) continue;
      h->SetZMRS( h->ZMRS() + z );
      h->SetZ( h->Z() + z );
    }
}
*/
//_____________________________________________________________________
Int_t TAlphaEvent::RecSTEvent()
{
  Int_t zz = 0;
  for (Int_t rr = 0; rr < GetNHelices(); rr++)
    {
      TAlphaEventHelix * H2 = (TAlphaEventHelix*) GetHelix(rr);
      if (H2->GetHelixStatus()>0) zz++;
    }
      TObjArray* Hits=GatherHits();
  const Int_t NHits = Hits->GetEntries();
  if (zz > 1 ||NHits < 5) return 0;
  if (zz == 1 && NHits > 4)
    {
      for (Int_t p = 0; p < GetNHelices(); p++)
	{
	  TAlphaEventHelix *helix = (TAlphaEventHelix*) GetHelix(p);
	  if (helix->GetHelixStatus()< 1) continue;

	  //printf("Helix Status = %d\n", helix->GetHelixStatus());
	  TAlphaEventHit *hit0 = new TAlphaEventHit(map);
	  TAlphaEventHit *hit1 = new TAlphaEventHit(map);


	  // ........................... ..Locating the intersection points v1 & v2

	  Int_t iflag = 0;
	  Double_t s1 = helix->GetsFromR ( TrapRadius, iflag );
	  Double_t s2 = -s1;
	  TVector3* v2 = new TVector3(helix->GetPoint3D_C( s2));
	  TVector3* v1 = new TVector3(helix->GetPoint3D_C( s1));
	  Double_t fLambda = helix->Getlambda();
	  Double_t fZSigma = 0.21 + 0.30 * fabs(fLambda);
	  Double_t zRes = fZSigma*fZSigma;

	  if( iflag < 0 )
	    {
	      Double_t vx = v1->X();
	      Double_t vy = v1->y();
	      Double_t vd = TMath::Sqrt( vx*vx + vy*vy );
	      vx = vx/vd*TrapRadius;
	      vy = vy/vd*TrapRadius;
	      v1->SetX( vx );
	      v1->SetY( vy );

	      hit0->SetXMRS(v1->X());
	      hit0->SetYMRS(v1->Y());
	      hit0->SetZMRS(v1->Z());
	      hit0->SetNSigma2(0.57*0.57);


	    }
	  else
	    {
	      hit0->SetXMRS(v1->X());
	      hit0->SetYMRS(v1->Y());
	      hit0->SetZMRS(v1->Z());
	      hit0->SetNSigma2(zRes);

	      hit1->SetXMRS(v2->X());
	      hit1->SetYMRS(v2->Y());
	      hit1->SetZMRS(v2->Z());
	      hit1->SetNSigma2(zRes);


	    }
  TObjArray* Hits=GatherHits();
  const Int_t NHits = Hits->GetEntries();
	  // ............................Extracting non-track Hits from Event  hitArray
	  TObjArray* hitArray = new TObjArray();
	  for(Int_t iHit = 0; iHit < NHits; iHit++)
	    {
	      TAlphaEventHit * hit =  (TAlphaEventHit*) (TAlphaEventHit*)Hits->At(iHit);
	      TAlphaEventHit * ahit=  (TAlphaEventHit*) helix->GetHit(0);
	      TAlphaEventHit * bhit=  (TAlphaEventHit*) helix->GetHit(1);
	      TAlphaEventHit * chit=  (TAlphaEventHit*) helix->GetHit(2);

	      if (  !  (ahit->XMRS() == hit->XMRS() && ahit->YMRS() == hit->YMRS() && ahit->ZMRS() == hit->ZMRS())  )
		{ if (  !  (bhit->XMRS() == hit->XMRS() && bhit->YMRS() == hit->YMRS() && bhit->ZMRS() == hit->ZMRS())  )
		    { if (  !  (chit->XMRS() == hit->XMRS() && chit->YMRS() == hit->YMRS() && chit->ZMRS() == hit->ZMRS())  )
			{
			  hitArray->AddLast(hit);
			}
		    }
		}
	    }
	  //.............. Creating Tracks of all combinations of hitArray t1 and t2....

	  for(Int_t i=0; i<hitArray->GetEntries(); i++)
	    {
	      for(Int_t j=i+1; j<hitArray->GetEntries(); j++)
		{
		  TAlphaEventHit * hi = (TAlphaEventHit*) hitArray->At(i);
		  TAlphaEventHit * hj = (TAlphaEventHit*) hitArray->At(j);


		  //................... Cut .... No hits from same layer...............................
		  if( hi->GetLayer() == hj->GetLayer() ) continue;

		  Double_t ax = hi->XMRS();
		  Double_t ay = hi->YMRS();
		  Double_t az = hi->ZMRS();
		  Double_t bx = hj->XMRS();
		  Double_t by = hj->YMRS();
		  Double_t bz = hj->ZMRS();

		  if (ax == bx && ay == by) continue;
		  if (TMath::Abs(az - bz) > 10) continue;


		  Double_t xy = TMath::Sqrt((ax - bx)*(ax - bx) + (ay - by)*(ay -by));
		  // Double_t phis = (TMath::ATan(by/ay) - TMath::ATan(bx-ax));

		  // if (TMath::Abs(phis) > 0.5) continue;
		  // printf("The phi angle between hits is : %f\n", TMath::Abs(phis));
		  if (xy > 7)continue;

		  //.................Creating the Tracks.............
		  TAlphaEventTrack * t0 = new TAlphaEventTrack();
		  TAlphaEventTrack * t1 = new TAlphaEventTrack();

		  //................ If DCA Track....................

		  if (( hit1->XMRS()== 0 && hit1->YMRS()==0 && hit1->ZMRS()== 0))
		    {
		      AddMCPoint(v1);
		      t0->AddHit(hit0);
		      t0->AddHit(hi);
		      t0->AddHit(hj);


		      TAlphaEventHelix * helix0 = new TAlphaEventHelix(t0);

		      if(helix0->GetChi2()> 5){helix0->SetHelixStatus(-5);}
		      if (helix0->GetR()< 20){helix0->SetHelixStatus(-5);}
		      if(helix0->GetHelixStatus()>0)
			{
			  if (MissHitStatus(helix0) > 0)
			    {
			      AddHelix(helix0);
			      helix0->SetHelixStatus(2);
			    }
			  else {helix0->SetHelixStatus(-6);}
			}
		    }
		  else
		    {
		      AddMCPoint(v1);
		      AddMCPoint(v2);

		      t0->AddHit(hit0);
		      t0->AddHit(hi);
		      t0->AddHit(hj);

		      t1->AddHit(hit1);
		      t1->AddHit(hi);
		      t1->AddHit(hj);


		      TAlphaEventHelix * helix0 = new TAlphaEventHelix(t0);
		      TAlphaEventHelix * helix1 = new TAlphaEventHelix(t1);


		      //............................ Cut ................ Chi Square ................................

		      if(helix0->GetChi2() > 5){helix0->SetHelixStatus(-5);}
		      if(helix1->GetChi2() > 5){helix1->SetHelixStatus(-5);}
		      //............................cut...................... Pick helices with Radius of curvature larger than 19 .....


		      if (helix0->GetR()< 20){helix0->SetHelixStatus(-5);}
		      if (helix1->GetR()< 20){helix1->SetHelixStatus(-5);}
		      //.......................... Cut ...................... Validation......   AddHelix(helix0);

		      if(helix0->GetHelixStatus()>0)
			{
			  if (MissHitStatus(helix0) < 1)
			    {
			      helix0->SetHelixStatus(-6);

			    }
			}


		      if(helix1->GetHelixStatus()>0)
			{
			  if (MissHitStatus(helix1) < 1)
			    {
			      helix1->SetHelixStatus(-6);
			    }
			}

		      //.......................... Cut ...................... Pick the helix with smaller Chi2......
		      if(helix0->GetHelixStatus() > 0 && helix1->GetHelixStatus() > 0)
			{
			  if(helix0->GetChi2() > helix1->GetChi2())
			    {
			      helix0->SetHelixStatus(-4);
			    }
			  if(helix0->GetChi2() < helix1->GetChi2())
			    {
			      helix1->SetHelixStatus(-4);
			    }
			}

		      if(helix0->GetHelixStatus()>0)
			{
			  AddHelix(helix0);
			  helix1->SetHelixStatus(2);
			}

		      if(helix1->GetHelixStatus()>0)
			{
			  AddHelix(helix1);
			  helix1->SetHelixStatus(2);
			}

		    }
		}
	    }


	  Int_t xx = 0;
	  for (Int_t tt = 0; tt < GetNHelices(); tt++)
	    {
	      TAlphaEventHelix * H0 = (TAlphaEventHelix*) GetHelix(tt);
	      if (H0->GetHelixStatus()> 0) xx++;
	    }

	  // ......................... cut ........... excluding helices with shared hits using the one with least chi2
	  if (xx > 1)
	    {
	      for (Int_t n1 = 0; n1 < GetNHelices(); n1++)
		for (Int_t n2 = n1+1; n2 < GetNHelices(); n2++)
		  {
		    TAlphaEventHelix* helix2 = (TAlphaEventHelix*) GetHelix(n1);
		    TAlphaEventHelix* helix3 = (TAlphaEventHelix*) GetHelix(n2);
		    if (IsSameHit(helix2->GetHit(1),helix3->GetHit(1))||IsSameHit(helix2->GetHit(2),helix3->GetHit(2))||IsSameHit(helix2->GetHit(1),helix3->GetHit(2))|| IsSameHit(helix2->GetHit(2),helix3->GetHit(1)) )
		      {
			if(helix2->GetChi2() > helix3->GetChi2())
			  {
			    helix2->SetHelixStatus(-7);
			  }
			if(helix2->GetChi2() < helix3->GetChi2())
			  {
			    helix3->SetHelixStatus(-7);
			  }
		      }
		    Int_t xxx = 0;
		    for (Int_t ttt = 0; ttt < GetNHelices(); ttt++)
		      {
			TAlphaEventHelix * H00 = (TAlphaEventHelix*) GetHelix(ttt);
			if (H00->GetHelixStatus()>0) xxx++;
		      }
		    //    printf("Added %d Helices \n", (xxx-1));
		    fVertex.SetIsGood(true);
		    return 1;
		  }
	    }
	  /*
	  if (xx == 2)
	    {
	      printf("Single Helix Added  \n");
	      Int_t xxx = 0;
	      for (Int_t ttt = 0; ttt < GetNHelices(); ttt++)
		{
		  TAlphaEventHelix * H00 = (TAlphaEventHelix*) GetHelix(ttt);
		  if (H00->GetHelixStatus()>0) xxx++;
		}
	      printf("Added %d Helix \n", (xxx-1));
	      fVertex.SetIsGood(true);
	      return 1;
	    }
	  */
	}
    }
  return 0;
}

//_____________________________________________________________________
Int_t TAlphaEvent::RecMTEvent()
{
  Int_t yy = 0;
    TObjArray* Hits=GatherHits();
  const Int_t NHits = Hits->GetEntries();
  for (Int_t ss = 0; ss < GetNHelices(); ss++)
    {
	TAlphaEventHelix * H1 = (TAlphaEventHelix*) GetHelix(ss);
	if (H1->GetHelixStatus()>0) yy++;
      }
    if (yy == 2 && NHits > (3 + 3 + 1))
      {
	if (CosmicTest() > 2) return 0;

	TAlphaEventHelix *helix1 = (TAlphaEventHelix*) GetHelix(0);
	TAlphaEventHelix *helix2 = (TAlphaEventHelix*) GetHelix(1);
	TAlphaEventHit *newhit = new TAlphaEventHit(map);

	TProjClusterBase* ProVert = (TProjClusterBase*)GetProjClusterVertex();

	if (ProVert)
	  {
	    newhit->SetXMRS(TrapRadius*(TMath::Cos(ProVert->RPhi()/TrapRadius)));
	    newhit->SetYMRS(TrapRadius*(TMath::Sin(ProVert->RPhi()/TrapRadius)));
	    newhit->SetZMRS(ProVert->Z());
	    newhit->SetNSigma2(0.271*0.271);

	  }

	else return 0;
	// ............................Extracting non-track Hits from Event  hitArray
	TObjArray* hitArray = new TObjArray();

	for(Int_t iHit = 0; iHit < NHits; iHit++)
	  {
	    TAlphaEventHit * hit =  (TAlphaEventHit*) (TAlphaEventHit*)Hits->At(iHit);
	    TAlphaEventHit * a1hit=  (TAlphaEventHit*) helix1->GetHit(0);
	    TAlphaEventHit * b1hit=  (TAlphaEventHit*) helix1->GetHit(1);
	    TAlphaEventHit * c1hit=  (TAlphaEventHit*) helix1->GetHit(2);
	    TAlphaEventHit * a2hit=  (TAlphaEventHit*) helix2->GetHit(0);
	    TAlphaEventHit * b2hit=  (TAlphaEventHit*) helix2->GetHit(1);
	    TAlphaEventHit * c2hit=  (TAlphaEventHit*) helix2->GetHit(2);

	    if (  !  (a1hit->XMRS() == hit->XMRS() && a1hit->YMRS() == hit->YMRS() && a1hit->ZMRS() == hit->ZMRS())  )
	      { if (  !  (b1hit->XMRS() == hit->XMRS() && b1hit->YMRS() == hit->YMRS() && b1hit->ZMRS() == hit->ZMRS())  )
		  { if (  !  (c1hit->XMRS() == hit->XMRS() && c1hit->YMRS() == hit->YMRS() && c1hit->ZMRS() == hit->ZMRS())  )
		      { if (  !  (a2hit->XMRS() == hit->XMRS() && a2hit->YMRS() == hit->YMRS() && a2hit->ZMRS() == hit->ZMRS())  )
			  { if (  !  (b2hit->XMRS() == hit->XMRS() && b2hit->YMRS() == hit->YMRS() && b2hit->ZMRS() == hit->ZMRS())  )
			      { if (  !  (c2hit->XMRS() == hit->XMRS() && c2hit->YMRS() == hit->YMRS() && c2hit->ZMRS() == hit->ZMRS())  )
				  {
				    hitArray->AddLast(hit);
				  }
			      }
			  }
		      }
		  }
	      }

	  }


	//.............. Creating Tracks of all combinations of hitArray t1 and t2....

	for(Int_t i=0; i<hitArray->GetEntries(); i++)
	  {
	    for(Int_t j=i+1; j<hitArray->GetEntries(); j++)
	      {
		TAlphaEventHit * hi = (TAlphaEventHit*) hitArray->At(i);
		TAlphaEventHit * hj = (TAlphaEventHit*) hitArray->At(j);


		//................... Cut .... No hits from same layer...............................
		if( hi->GetLayer() == hj->GetLayer() ) continue;

		Double_t ax = hi->XMRS();
		Double_t ay = hi->YMRS();

		Double_t bx = hj->XMRS();
		Double_t by = hj->YMRS();

		Double_t xy = TMath::Sqrt((ax - bx)*(ax - bx) + (ay - by)*(ay -by));


		if (ax == bx && ay == by) continue;

		if (xy > 6)continue;

		//.................Creating the Tracks.............
		TAlphaEventTrack * newtrack = new TAlphaEventTrack();

		if (!( newhit->XMRS()== 0 && newhit->YMRS()==0 && newhit->ZMRS()== 0))
		  {
		    newtrack->AddHit(newhit);
		    newtrack->AddHit(hi);
		    newtrack->AddHit(hj);
		    TAlphaEventHelix * newhelix = new TAlphaEventHelix(newtrack);
		    if(newhelix->GetChi2()> 5.){newhelix->SetHelixStatus(-4);}
		    if (newhelix->GetR()< 20.){newhelix->SetHelixStatus(-4);}
		    if(newhelix->GetHelixStatus()>0)
		      {

			if (MissHitStatus(newhelix)>0)
			  {
			    AddHelix(newhelix);
			    newhelix->SetHelixStatus(2);
			  }
		      }
		  }
	      }
	  }
	hitArray->SetOwner(kTRUE);
	hitArray->Delete();
	delete hitArray;


	Int_t xx = 0;
	for (Int_t tt = 0; tt < GetNHelices(); tt++)
	  {
	    TAlphaEventHelix * H0 = (TAlphaEventHelix*) GetHelix(tt);
	    if (H0->GetHelixStatus()>0) xx++;
	  }

	// ......................... cut ........... excluding helices with shared hits using the one with least chi2
	if (xx > 2)
	  {
	    for (Int_t n1 = 2; n1 < GetNHelices(); n1++)
	      for (Int_t n2 = n1+2; n2 < GetNHelices(); n2++)
		{
		  TAlphaEventHelix* helix2 = (TAlphaEventHelix*) GetHelix(n1);
		  TAlphaEventHelix* helix3 = (TAlphaEventHelix*) GetHelix(n2);
		  if (IsSameHit(helix2->GetHit(1),helix3->GetHit(1))||IsSameHit(helix2->GetHit(2),helix3->GetHit(2))||IsSameHit(helix2->GetHit(1),helix3->GetHit(2))|| IsSameHit(helix2->GetHit(2),helix3->GetHit(1)) )
		    {
		      if(helix2->GetChi2() > helix3->GetChi2())
			{
			  helix2->SetHelixStatus(-4);
			}
		      if(helix2->GetChi2() < helix3->GetChi2())
			{
			  helix3->SetHelixStatus(-4);
			}
		    }
		  // printf("Best Helices Added # %d \n", xx-1);
		  Int_t xxx = 0;
		  for (Int_t ttt = 0; ttt < GetNHelices(); ttt++)
		    {
		      TAlphaEventHelix * H00 = (TAlphaEventHelix*) GetHelix(ttt);
		      if (H00->GetHelixStatus()>0) xxx++;
		    }
		  //printf("Added %d Helices \n", (xxx-2));
		  fVertex.SetIsGood(true);
		  return 1;
		}
	  }
	/*
	if (xx == 3)
	  {
	    printf("Single Helix Added  \n");
	    Int_t xxx = 0;
	    for (Int_t ttt = 0; ttt < GetNHelices(); ttt++)
	      {
		TAlphaEventHelix * H00 = (TAlphaEventHelix*) GetHelix(ttt);
		if (H00->GetHelixStatus()>0) xxx++;
	      }
	    printf("Added %d Helix \n", (xxx-2));
	    fVertex.SetIsGood(true);
	    return 1;
	  }
	*/
      }
    return 0;
  }

//_____________________________________________________________________
  Int_t TAlphaEvent:: MissHitStatus(TAlphaEventHelix *h)
  {
  Double_t hix = h->GetHit(1)->XMRS();
  Double_t hiy = h->GetHit(1)->YMRS();
  Double_t hiz = h->GetHit(1)->ZMRS();

  Double_t hjx = h->GetHit(2)->XMRS();
  Double_t hjy = h->GetHit(2)->YMRS();
  Double_t hjz = h->GetHit(2)->ZMRS();

  TVector3 u(hjx - hix,hjy - hiy, hjz - hiz);
  TVector3 r0(hix, hiy, hiz);
  TVector3 r1( r0 + 1*u);
  Double_t Slope = u.Y()/u.X();
  Double_t Intercept = r0.Y() - Slope*r0.X();
  TVector3 * l = new TVector3(Slope, Intercept,0);
  Double_t A = h->Geta();
  Double_t B = h->Getb();
  Double_t R = h->GetR();


  Int_t Modules [3];
  Double_t r[3];

  Int_t Counter2 = 0;
  Int_t Counter = 0;
  Int_t C = 0;
  Int_t iflag = 0;

  for(Int_t m = 0; m<nSil;m++)
    {
      TAlphaEventSil * sil = (TAlphaEventSil*)GetSilByNumber(m);
      if(!sil) continue;
      // finding possible intersection points between the module line and the helix circle
      Double_t cos = sil->GetCos();
      Double_t sin = sil->GetSin();
      Double_t x = sil->GetXCenter();
      Double_t y = sil->GetYCenter();
      Double_t sm = 0;
      if (sin > 0 ) {sm = TMath::Tan(TMath::ACos(cos) + TMath::Pi()/2); }
      if (sin < 0 ) {sm = -TMath::Tan(TMath::ACos(cos) + TMath::Pi()/2); }
      Double_t sb = y - sm*x ;
      Double_t a = (1 + sm*sm);
      Double_t b = (2*sm*sb - 2*A - 2*B*sm);
      Double_t c = (A*A - R*R + B*B + sb*sb - 2*B*sb);
      Double_t D = (b*b - 4*a*c);
      Double_t XPlus = 0 ;
      Double_t XMinus = 0;
      Double_t YPlus = 0 ;
      Double_t YMinus = 0;
      Double_t RPlus = 0;
      Double_t RMinus = 0;
      Int_t plusPhit = 0;
      Int_t plusNhit = 0;
      Int_t minusPhit = 0;
      Int_t minusNhit = 0;

      if (D >= 0)
	{
	  XPlus = (-b + TMath::Sqrt(D))/(2*a);
	  XMinus = (-b - TMath::Sqrt(D))/(2*a);
	}
      // two possible intersection points - XPlus and Xminus
      YPlus = sm*XPlus + sb;
      RPlus = TMath:: Sqrt(XPlus*XPlus + YPlus*YPlus);

      Double_t s1 = h->GetsFromR( RPlus,iflag);
      TVector3* v1 = new TVector3(h->GetPoint3D_C( s1));

      TVector3 PlusLocal( XPlus*cos + YPlus*sin, -XPlus*sin + YPlus*cos, v1->Z());
      plusPhit = sil->ReturnPStrip(PlusLocal[1]);
      plusNhit = sil->ReturnNStrip(PlusLocal[2]);
      // considering only intersection Modules in the positive helix direction
      if (TMath::Abs(v1->Y()- YPlus) < 0.01 && TMath::Abs(v1->X()- XPlus) < 0.01)
	{
	  if (plusPhit >=0 && plusNhit >=0 )
	    {
	      //   printf( "..++  YAxis : %f, XAxis : %f, on Module : %d \n", YPlus, XPlus, m);
	      Modules[C] = m;
	      r[C] = RPlus;
	      C++;
	      TVector3 Vec1 (XPlus, YPlus, v1->Z());
	      TVector3* Point1 = new TVector3(Vec1);
	      AddMCPoint(Point1);
	      Counter2++;
	    }
	}

      YMinus = sm*XMinus + sb;
      RMinus = TMath::Sqrt(XMinus*XMinus + YMinus*YMinus);

      Double_t s2 = h->GetsFromR ( RMinus,iflag);
      TVector3* v2 = new TVector3(h->GetPoint3D_C( s2));

      TVector3 MinusLocal( XMinus*cos + YMinus*sin, -XMinus*sin + YMinus*cos, v2->Z());
      minusPhit = sil->ReturnPStrip(MinusLocal[1]);
      minusNhit = sil->ReturnNStrip(MinusLocal[2]);

      if (TMath::Abs(v2->Y()- YMinus) < 0.01 && TMath::Abs(v2->X()- XMinus) < 0.01)
	{
	  if (minusPhit >=0 &&  minusNhit >=0)
	    {
	      //    printf( "..--  YAxis : %f, XAxis : %f, on Module :%d \n", YMinus, XMinus, m);
	      Modules[C] = m;
	      r[C] = RMinus;
	      C++;
	      TVector3 Vec2 (XMinus, YMinus, v2->Z());
	      TVector3* Point2 = new TVector3(Vec2);
	      AddMCPoint(Point2);
	      Counter2++;
	    }
	}
    }
  // Helices passing through only two Modules
  if (Counter2 == 2)
    {
      //   printf("Modules: %d , %d \n", Modules[0], Modules[1]);
      return 2;
    }
  // Helices passing through 3 modules but with only two hits
  if (Counter2 == 3)
    {
      for (Int_t j = 0; j < C; j++)
	{
	  TAlphaEventSil * sil = (TAlphaEventSil*)GetSilByNumber(Modules[j]);
	  if(!sil) continue;

	  Double_t x_C = sil->GetXCenter();
	  Double_t y_C = sil->GetYCenter();
	  Double_t cos_C = sil->GetCos();
	  Double_t sin_C = sil->GetSin();
	  Double_t sm_C = 0;
	  if (sin_C > 0 ) {sm_C = TMath::Tan(TMath::ACos(cos_C) + TMath::Pi()/2); }
	  if (sin_C < 0 ) {sm_C = -TMath::Tan(TMath::ACos(cos_C) + TMath::Pi()/2); }
	  Double_t sb_C = y_C - sm_C*x_C ;
	  TVector3 * l_C = new TVector3(sm_C,sb_C,0);

	  Double_t s = h->GetsFromR(r[j],iflag);
	  TVector3* v = new TVector3(h->GetPoint3D_C( s));
	  TVector3 Local (v->X()*cos_C + v->Y()*sin_C, -v->X()*sin_C + v->Y()*cos_C, v->Z());

	  Int_t phit = sil->ReturnPStrip(Local[1]);
	  Int_t nhit = sil->ReturnNStrip(Local[2]);

	  // checking for a missing signal in the module with missing hit
	  if(phit >=0 && nhit >=0)
	    {
	      for(Int_t ip=0; ip<sil->GetNPClusters(); ip++)
		{
		  TAlphaEventPCluster * pp = (TAlphaEventPCluster*)  sil->GetPCluster(ip);

		  if( fabs(pp->Y()-Local[1])<0.2) // found it
		    {
		      Counter++;
		      break;
		    }
		}
	      for(Int_t in=0; in<sil->GetNNClusters(); in++)
		{
		  TAlphaEventNCluster * nn = (TAlphaEventNCluster*) sil->GetNCluster(in);
		  if( fabs(nn->Z()-Local[2])<0.2)  // found it
		    {
		      Counter--;
		      break;
		    }
		}
	    }

	  if ( Counter == 1)
	    {
	      Addxyline(l);
	      Addxyline(l_C);
	      return 1;
	    }
	  if ( Counter == -1)
	    {
	      Addxyline(l_C);
	      Addxyline(l);
	      return 1;
	    }
	}
    }
  return 0;
}

//_____________________________________________________________________
Double_t TAlphaEvent::STCosmicTest()
{
  Int_t nHelix = GetNHelices();
  printf("nHelices = %d\n", nHelix);
  Double_t cosmicchi = -1;
  TAlphaEventTrack * Best_Cosmic = new TAlphaEventTrack();
  Double_t  Best_Cor = 0;

  if( nHelix < 2 ) return -1;

  Int_t nh=0;
  for(Int_t i=0;i<nHelix;i++)
    {
      TAlphaEventHelix * t = GetHelix(i);
      if(t->GetHelixStatus()>0) nh++;
    }

  // printf("NTracks: %d nh: %d\n",nHelix,nh);
  if( nh < 2 ) return -1;

  // Gather 6 hits from every pair of helices and find the set with the highest correlation coefficient
  for(Int_t i=1;i<nHelix;i++)
    {
      TAlphaEventHelix * trackone = GetHelix(0);
      TAlphaEventHelix * tracktwo = GetHelix(i);

      if(trackone->GetHelixStatus()<0) continue;
      if(tracktwo->GetHelixStatus()<0) continue;

      TAlphaEventHit * ha = trackone->GetHit(0);
      TAlphaEventHit * hb = trackone->GetHit(1);
      TAlphaEventHit * hc = trackone->GetHit(2);
      TAlphaEventHit * hd = tracktwo->GetHit(1);
      TAlphaEventHit * he = tracktwo->GetHit(2);

      TAlphaEventTrack * Cosmic1 = new TAlphaEventTrack();

      Cosmic1->AddHit(ha);
      Cosmic1->AddHit(hb);
      Cosmic1->AddHit(hc);
      Cosmic1->AddHit(hd);
      Cosmic1->AddHit(he);

      Cosmic1->MakeLinePCA();

      Double_t cor1 = Cosmic1->Getcor();

      if( cor1 > Best_Cor && cor1 != 1.0 )
	{
	  if( Best_Cosmic )
	    {
	      delete Best_Cosmic;
	      Best_Cosmic = NULL;
	    }
	  Best_Cosmic = Cosmic1;
	  Best_Cor = cor1;	}
      else
	{
	  delete Cosmic1;
	}
    }

  if( Best_Cor == 0 ) return -1;

  Double_t PR = 0;

  TVector3 u( Best_Cosmic->Getunitvector() );
  TVector3 r0( Best_Cosmic->Getr0() );

  Int_t numhit = Best_Cosmic->GetNHits();

  for(Int_t n = 0; n < numhit; n++)
    {
      Double_t x = Best_Cosmic->GetHit(n)->XMRS();
      Double_t y = Best_Cosmic->GetHit(n)->YMRS();
      Double_t z = Best_Cosmic->GetHit(n)->ZMRS();

      // Calculate the sum of the perpendicular distances to the fitted line (the residual)
      TVector3 QP(r0.X()-x,r0.Y()-y,r0.Z()-z);
      Double_t QR = TMath::Abs(QP.Dot(u));
      PR +=  QP.Dot(QP)-QR*QR ;
      //printf("PR: %f \t Total: %f \n",QP.Dot(QP)-QR*QR,PR);
    }
  Double_t PR2 = ((PR*6)/numhit);
  cosmicchi = PR2;
  // printf("Res^2: %f \n",cosmicchi);
  //  printf("Number of hits: %i \n",numhit);
  return cosmicchi;
}

//_____________________________________________________________________
TVector3 *TAlphaEvent::GetSTVertex()
{
  TVector3 *Best_Vertex = new TVector3();
  Double_t Best_Chi2 = 9999;
  if (GetNHelices() < 2) return 0;
  if (GetNHelices() == 2)
    {
      TAlphaEventHelix * helix1 = (TAlphaEventHelix*) GetHelix(1);
      TAlphaEventHit * hit = (TAlphaEventHit*) helix1->GetHit(0);
      Best_Vertex->SetX(hit->XMRS());
      Best_Vertex->SetY(hit->YMRS());
      Best_Vertex->SetZ(hit->ZMRS());

      return Best_Vertex;
    }
  if (GetNHelices()> 2)
    { for (Int_t n = 1; n < GetNHelices(); n++)
	{
	  TAlphaEventHelix *h = GetHelix(n);
	  Double_t Chi2 = h->GetChi2();
	  TVector3 *Vertex = new TVector3();
	  TAlphaEventHit * hit1 = (TAlphaEventHit*) h->GetHit(0);
	  Vertex->SetX(hit1->XMRS());
	  Vertex->SetY(hit1->YMRS());
	  Vertex->SetZ(hit1->ZMRS());

	  if(Chi2 < Best_Chi2)
	    {
	      if( Best_Vertex )
		{
		  delete Best_Vertex;
		  Best_Vertex = NULL;
		}
	      Best_Vertex = Vertex;
	      Best_Chi2 = Chi2;
	    }
	  else
	    {
	      delete Vertex;
	    }
	}
    }

    return Best_Vertex;

}

//_____________________________________________________________________
TVector3 *TAlphaEvent::GetMTVertex()
{

  TVector3 *Best_Vertex = new TVector3();
  //TProjClusterBase * vertex = (TProjClusterBase*) GetProjClusterVertex();
  TProjClusterBase * vertex = GetProjClusterVertex();
  if (vertex)
    {
      Double_t px = (TrapRadius*(TMath::Cos(vertex->RPhi()/TrapRadius)));
      Double_t py = (TrapRadius*(TMath::Sin(vertex->RPhi()/TrapRadius)));
      Double_t pz = (vertex->Z());
      Best_Vertex->SetXYZ(px, py, pz);

      return Best_Vertex;
    }
  else return 0;
}

//_____________________________________________________________________
Int_t TAlphaEvent::IsGhostTrack()
{
  Int_t nh=0;
  Int_t nHelix = GetNHelices();
  Int_t NGhost=0;
  for(Int_t i=0;i<nHelix;i++)
    {
      TAlphaEventHelix * t = GetHelix(i);
      if (!t) continue;
      if(t->GetHelixStatus()>0) ++nh;
    }
  if( nh < 2 ) return -1;
  // printf("Total good helices = %d \n",nh);
  for(Int_t j = 0; j<nHelix; j++)
  {
    TAlphaEventHelix* helix0 = (TAlphaEventHelix*) GetHelix(j);
    if (!helix0) continue;
    if( helix0->GetHelixStatus()<0 ) continue;
    for(Int_t k = j+1; k < nHelix; k++)
    {
      TAlphaEventHelix* helix1 = (TAlphaEventHelix*) GetHelix(k);
      if( !helix1 ) continue;
      if( helix1->GetHelixStatus()<0) continue;
      Int_t nHit=0;
      for(Int_t l = 0; l < 3; l++)
      {
        TAlphaEventHit* hit0 = (TAlphaEventHit*) helix0->GetHit(l);
        TAlphaEventHit* hit1 = (TAlphaEventHit*) helix1->GetHit(l);
        if(hit0->GetSilNum() == hit1->GetSilNum()) ++nHit;
      }
      if( (nHit == 3) && TMath::AreEqualRel(helix0->GetR(),helix1->GetR(),0.001) )
      {
        //printf("The %d and %d tracks share %d panels with radius %f and %f \n",j,k,nHit,helix0->GetR(),helix1->GetR() );
        fVerbose.Warning("TAlphaEvent::IsGhostTrack","The %d and %d tracks share %d panels with radius %f and %f \n",
                        j,k,nHit,helix0->GetR(),helix1->GetR());
        ++NGhost;
        if(helix0->GetChi2() < helix1->GetChi2())
        {
          helix0->SetHelixStatus(1);
          fHelices[ k ]=NULL;
          delete helix1;
          // 	helix1->SetHelixStatus(-3);
        }
        // //if(helix0->GetChi2() > helix1->GetChi2())
        else
        {
          //helix0->SetHelixStatus(-3);
          helix1->SetHelixStatus(1);
          fHelices[ j ]=NULL;
          delete helix0;
          break; //Get next helix0
        }
      }

    } //For k (helix1)
  }// For j (helix0)
  //fHelices->Compress();
  if(NGhost > 0) //printf("  # of ghost tracks removed: %d\n",NGhost);
    fVerbose.Message("TAlphaEvent::IsGhostTrack","Number of ghost tracks removed: %d\n",NGhost);

  return NGhost;
}
