#include <iostream>
#include <assert.h>

#include <TMath.h>

#include "TAlphaEventSil.h"

ClassImp(TAlphaEventSil);

//////////////////////////////////////////////////////////////////////
//
//  TAlphaEventSil  
//
//////////////////////////////////////////////////////////////////////
//____________________________________________________________________
TAlphaEventSil::TAlphaEventSil(Char_t *silname, TAlphaEvent* e,TAlphaEventMap* m) 
  : TAlphaEventObject(m,silname,1) 
{
  memset(fADCp,0,sizeof(fADCp));
  memset(fADCn,0,sizeof(fADCn));
  memset(fASIC1,0,sizeof(fASIC1));
  memset(fASIC2,0,sizeof(fASIC2));
  memset(fASIC3,0,sizeof(fASIC3));
  memset(fASIC4,0,sizeof(fASIC4));
  
  memset(fRMSn,0,sizeof(fRMSn));
  memset(fRMSp,0,sizeof(fRMSp));
  memset(fRMS1,0,sizeof(fRMS1));
  memset(fRMS2,0,sizeof(fRMS2));
  memset(fRMS3,0,sizeof(fRMS3));
  memset(fRMS4,0,sizeof(fRMS4));
  
  fHits.clear();
  fNClusters.clear();
  fPClusters.clear();
  
  Event=e;
}

//______________________________________________________________________________
TAlphaEventSil::TAlphaEventSil(const int num,TAlphaEvent* e,TAlphaEventMap* m) 
  : TAlphaEventObject(m,num,1)
{
  memset(fADCp,0,sizeof(fADCp));
  memset(fADCn,0,sizeof(fADCn));
  memset(fASIC1,0,sizeof(fASIC1));
  memset(fASIC2,0,sizeof(fASIC2));
  memset(fASIC3,0,sizeof(fASIC3));
  memset(fASIC4,0,sizeof(fASIC4));	

  memset(fRMSn,0,sizeof(fRMSn));
  memset(fRMSp,0,sizeof(fRMSp));
  memset(fRMS1,0,sizeof(fRMS1));
  memset(fRMS2,0,sizeof(fRMS2));
  memset(fRMS3,0,sizeof(fRMS3));
  memset(fRMS4,0,sizeof(fRMS4));
  fHits.clear();
  fNClusters.clear();
  fPClusters.clear();

  Event=e;
}


//____________________________________________________________________
TAlphaEventSil::~TAlphaEventSil() 
{
  //int h=fHits.size();
  //for (int i=0; i<h; i++)
  //  delete fHits[i];
  fHits.clear();
  
  int n=fNClusters.size();
  for (int i=0; i<n; i++)
    delete fNClusters[i];
  fNClusters.clear();

  int p=fPClusters.size();
  for (int i=0; i<p; i++)
    delete fPClusters[i];
  fPClusters.clear();

}

//____________________________________________________________________
void TAlphaEventSil::GetStrippStartEnd (Int_t n, TVector3 &a, TVector3 &b) 
{
  // Provide positions for both ends of the pside strips for plotting 
  assert( n >= 0 );
  assert( n < 256);
	
  Double_t x = +SilX()/2.;
  Double_t y = GetpPos(n);
  Double_t z = SilZ()/2.;

  Double_t cos = GetCos();
  Double_t sin = GetSin();

  a.SetX(x*cos - y*sin + GetXCenter());
  a.SetY(x*sin + y*cos + GetYCenter());
  a.SetZ(z + GetZCenter()); 

  x = +SilX()/2.;
  y = GetpPos(n);
  z = -SilZ()/2.;
  
  b.SetX(x*cos - y*sin + GetXCenter());
  b.SetY(x*sin + y*cos + GetYCenter());
  b.SetZ(z + GetZCenter()); 

}

//____________________________________________________________________
void TAlphaEventSil::GetStripnStartEnd(Int_t n, TVector3 &a, TVector3 &b) 
{
  // Provide positions for both of the nside strips for plotting
  assert( n >= 0 );
  assert( n < 256);
	
  Double_t x = -SilX()/2.;
  Double_t y =  SilY()/2.;
  Double_t z = GetnPos(n);
  
  Double_t cos = GetCos();
  Double_t sin = GetSin();

  a.SetX(x*cos - y*sin + GetXCenter());
  a.SetY(x*sin + y*cos + GetYCenter());
  a.SetZ(z); 
  
  x = -SilX()/2.;
  y = -SilY()/2.;
  z = GetnPos(n);
  
  b.SetX(x*cos - y*sin + GetXCenter());
  b.SetY(x*sin + y*cos + GetYCenter());
  b.SetZ(z);
}
//____________________________________________________________________
Bool_t TAlphaEventSil::boundary_flag(Int_t f)
{
  for(Int_t i=-1; i>=-8; --i)
    if(f==i) return kTRUE;
  return kFALSE;
}

//____________________________________________________________________
Int_t TAlphaEventSil::GetOrPhi() {
	
  // Can have to p-side triggers from each module
  Int_t ta1 = 0;
  Int_t ta2 = 0;
  
  for(Int_t i=0;i<256;i++) 
    {
      if(fADCp[i] > 0.) 
        {
          if( i < 128 ) ta1 = 1;
          else ta2 = 1;
        }
    }

  //printf("ORPHI: %d\n",ta1+ta2);
  return ta1 + ta2;
}

//____________________________________________________________________
Int_t TAlphaEventSil::GetOrN() {
	
  // Can have to n-side triggers from each module
  Bool_t ta1 = false;
  Bool_t ta2 = false;
  
  for(Int_t i=0;i<256;i++) 
    if(fADCn[i]) 
      {
	if( i < 128 ) ta1 = true;
	else ta2 = true;
      }
  //printf("ORPHI: %d\n",ta1+ta2);
  return ta1 + ta2;
}

//______________________________________________________________________________
void TAlphaEventSil::MapASICtoStrips()
{
  // Take the 4 ASIC 128 arrays and make two 256 arrays
  
  for( Int_t i = 0; i < 128; i++)
    {
      fADCn[i]     = fASIC1[i];
      fADCn[i+128] = fASIC2[i];
      fADCp[i]     = fASIC3[i];
      fADCp[i+128] = fASIC4[i];
      
      
      fRMSn[i]     = fRMS1[i];
      fRMSn[i+128] = fRMS2[i];
      fRMSp[i]     = fRMS3[i];
      fRMSp[i+128] = fRMS4[i];
      
      
    }
}


//______________________________________________________________________________
void TAlphaEventSil::RecCluster()
{
   /*Event->GetVerbose()->Message("RecCluster",
                                "---Clustering %s (%d)---\n",
                                GetName(),GetSilNum());*/
  MapASICtoStrips();
  
  // pside
  Bool_t A = kFALSE;                                 // run-length encoding
  Int_t pBeg[256];
  Int_t pRun[256]; 
  Int_t N=0;
  for (Int_t k=0; k< 256; k++) 
    { 
      Bool_t B = fADCp[k];
      if (!A &&  B) { pBeg[N]=k; pRun[N]=1; }  // 01 = begin
      if ( A &&  B)   pRun[N]++;               // 11
      if ( A && !B)   N++;                     // 10 = end
      A = B;
    }
  if (A) N++; 
  Int_t Npside=N; 
  
  // nside
  A = kFALSE;                                 // run-length encoding
  Int_t nBeg[256];
  Int_t nRun[256]; 
  N=0;
  for (Int_t k=0; k< 256; k++) 
    { 
      Bool_t B = fADCn[k];
      if (!A &&  B) { nBeg[N]=k; nRun[N]=1; }  // 01 = begin
      if ( A &&  B)   nRun[N]++;               // 11
      if ( A && !B)   N++;                     // 10 = end
      A = B;
    }
  if (A) N++; 
  Int_t Nnside=N; 
  
   /*Event->GetVerbose()->Message("RecCluster",
                                "Nside: %d Pside: %d Total: %d\n",
                                Nnside,
                                Npside,
                                Nnside*Npside);*/
  
  for( Int_t inside = 0; inside < Nnside; inside++)
    {
      TAlphaEventNCluster * c = new TAlphaEventNCluster(GetSilNum(),map);
      c->Reserve(nRun[inside]);
      for (Int_t h=0; h<nRun[inside]; h++)
        {
          c->AddStrip( nBeg[inside]+h,fabs(fADCn[nBeg[inside]+h]),fRMSn[nBeg[inside]+h] );
        }
      c->Calculate();
      if (c->GetSigma() > Event->GetNClusterSigma() /*NGetNClusterSigma()*/) fNClusters.push_back( c );
      else { delete c;}
    }
  
  for( Int_t ipside = 0; ipside < Npside; ipside++)
    {
      TAlphaEventPCluster * c = new TAlphaEventPCluster(GetSilNum(),map);
      c->Reserve(nRun[ipside]);
      for (Int_t h=0; h<pRun[ipside]; h++)
        {
          c->AddStrip( pBeg[ipside]+h,fabs(fADCp[pBeg[ipside]+h]),fRMSp[pBeg[ipside]+h]);
        }
      c->Calculate();
      //c->Print();
      if (c->GetSigma() >  Event->GetPClusterSigma())
        fPClusters.push_back( c );
      else { delete c; }
    }
}

void TAlphaEventSil::RemoveHit(TAlphaEventHit* remove)
{
	Int_t hits=0;
	if (fNClusters.empty() ) return;
	if (fPClusters.empty() ) return;
  int nc=fNClusters.size();
  int np=fPClusters.size();
  for( Int_t in = 0; in < nc; in++ )
  {
    TAlphaEventNCluster * n = GetNCluster( in );
    if (!n) continue;
    //n->Print();
    for( Int_t ip = 0; ip < np; ip++ )
    {
      TAlphaEventPCluster * p = GetPCluster( ip );
      if (!p) continue;
      //p->Print();
      TAlphaEventHit * h = new TAlphaEventHit(map, GetSilNum(), p,n );
      if ( h->Y() == remove->Y() && h->Z() == remove->Z())
      {
        delete n;
        delete p;
        fNClusters.at(in)=NULL;
        fPClusters.at(ip)=NULL;
        //std::cout <<"Removing hit"<<std::endl;
      }
      hits++;
    }
  }
  //std::cout<<"Pre hits:"<<hits<<std::endl;
  if (hits)
  {
    //fNClusters.Compress();
    //fPClusters.Compress();
    fHits.clear();
    //fHits.Compress();
    RecHit();
  }
}
//______________________________________________________________________________
void TAlphaEventSil::RecHit()
{
  //printf("%d NClusters: %d, PClusters: %d\n",GetSilNum(),fNClusters.GetEntriesFast(),fPClusters.GetEntriesFast());
  Int_t Hits=0;
 // Double_t Sig[10000];
  //Double_t Sig[fNClusters.GetEntriesFast()*fPClusters.GetEntriesFast()];
 /*  for( Int_t in = 0; in < fNClusters.GetEntriesFast(); in++ )
    {
      for( Int_t ip = 0; ip < fPClusters.GetEntriesFast(); ip++ )
        {
          TAlphaEventNCluster * n = GetNCluster( in );
          TAlphaEventPCluster * p = GetPCluster( ip );
          TAlphaEventHit * h = new TAlphaEventHit( GetSilNum(), p,n );
          //Sig[Hits]=h->GetHitSignifance();
          
          delete h;
        }
    }
   // std::cout<<"Hits:"<<Hits<<std::endl;
 Double_t SigCut=-1.; //Off by default
  if (Hits>gEvent->GetHitThreshold())
  {
    std::sort(Sig,Sig + Hits);
   
   // for (Int_t i = Hits; i> 0; i-- )
   // {
   //    std::cout <<Sig[i]<<std::endl;
   // }
    if (Hits>gEvent->GetHitThreshold() && Int_t(Hits-gEvent->GetHitThreshold())>0)
    {
      SigCut=Sig[(Int_t(Hits-gEvent->GetHitSignificance()))];
      std::cout <<"SigCut: "<<SigCut<<" at "<<
      (Int_t(Hits-gEvent->GetHitSignificance()))
      <<std::endl;
    }
  }*/
  int nc=fNClusters.size();
  int np=fPClusters.size();
  for( Int_t in = 0; in < nc; in++ )
    {
      TAlphaEventNCluster * n = GetNCluster( in );
      if (!n) continue;
      for( Int_t ip = 0; ip < np; ip++ )
        {      
          TAlphaEventPCluster * p = GetPCluster( ip );
          if (!p) continue;
          TAlphaEventHit * h = new TAlphaEventHit(map, GetSilNum(), p,n );
          //h->Print();
          //if (h->GetHitSignifance() < SigCut) delete h;
          //else AddHit( h );
          Hits++;
          AddHit( h );
        }
    }
}

void TAlphaEventSil::Print(Option_t*) const
{
  //  std::cout<<"TAlphaEventSil::Silicon #:"<<GetSilNum()<<" name: "<<ReturnSilName(GetSilNum())<<" layer: "<<GetLayer()<<std::endl;
  std::cout<<"TAlphaEventSil::Silicon #:"<<GetSilNum()<<" layer: "<<map->GetLayer(GetSilNum())<<std::endl;
  for( int s=0; s<128; ++s)
    {
      std::cout<<s<<"\t"<<fASIC1[s]<<"\t"<<fASIC2[s]<<"\t"<<fASIC3[s]<<"\t"<<fASIC4[s]<<std::endl;
    }
}
