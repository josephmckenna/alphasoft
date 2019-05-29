// TProjCluster
#include <TMath.h>
#include "TProjCluster.h"

ClassImp( TProjCluster )

//_____________________________________________________________________
TProjCluster::TProjCluster()
{
  //
}

//_____________________________________________________________________
TProjCluster::TProjCluster( TProjCluster *pc )
{
  //ctor
  fProjClusters = new TObjArray();
  for(Int_t i=0; i<pc->GetEntriesFast(); i++)
    {
      TProjClusterBase* p = pc->At(i);
      TProjClusterBase *pcb = new TProjClusterBase( p );
      fProjClusters->Add(pcb);
    }
  fIsVertex = pc->IsVertex();
}

//_____________________________________________________________________
TProjCluster::TProjCluster( TProjClusterBase *base )
{
  //ctor
  fProjClusters = new TObjArray();
  TProjClusterBase* b=new TProjClusterBase(base);
  fProjClusters->Add( b );
  fIsVertex = kFALSE;
}

//_____________________________________________________________________
TProjCluster::~TProjCluster()
{
  //dtor
  fProjClusters->SetOwner(kTRUE);
  fProjClusters->Delete();
  
  if (fProjClusters)
  {
    delete fProjClusters;
  }
}

/*
 * Returns a "mean" of the cluster, allocating a new TProjClusterBase.
 * Callers are responsible for deleting the newly allocated object.
 */
//_____________________________________________________________________
TProjClusterBase *TProjCluster::GetMean()
{
  Int_t n = GetEntriesFast();

  if( n==1 ) // only one
  {
       TProjClusterBase *only = At(0);
       return new TProjClusterBase(only);
  }

  // weighted mean
  TProjClusterBase * base = new TProjClusterBase();

  Double_t zw = 0.;
  Double_t zm = 0.;
  Double_t rphiw = 0.;
  Double_t rphim = 0.;
  Double_t z = 0.;
  Double_t rphi = 0.;
  Double_t zsig = 0.;
  Double_t rphisig = 0.;
  for(Int_t i=0; i<n; i++)
  {
      TProjClusterBase *pcb = (TProjClusterBase*) At(i);
      
      z = pcb->Z();
      rphi = pcb->RPhi();
      // fprintf(stderr,"Z is : %f and RPhi is: %f \n", z, rphi);
      
      zsig = pcb->GetZSigma();
      rphisig = pcb->GetRPhiSigma();
      
      zm += z;
      zw += 1./(zsig*zsig);

      rphim += rphi;
      rphiw += 1./(rphisig*rphisig);
  }
    
  Double_t wzmean = zm/n;
  Double_t wrphimean = rphim/n;
  Double_t wzstd = 0.;
  Double_t wrphistd = 0.;
  
  for(Int_t i=0; i<n; i++)
  {
      TProjClusterBase *pcb = (TProjClusterBase*) At(i);
      wzstd += TMath::Power( (pcb->Z()-wzmean)/pcb->GetZSigma(),2.);
      wrphistd += TMath::Power( (pcb->RPhi()-wrphimean)/pcb->GetRPhiSigma(),2.);
  }
  
  base->SetZ( wzmean );
  base->SetZSigma( TMath::Sqrt( 1./zw ) );

  base->SetRPhi( wrphimean );
  base->SetRPhiSigma( TMath::Sqrt( 1./rphiw ) );

  base->SetAngleFromNormal(-999.);
  base->SetN(n);
  base->Setd0(-1.);
  base->SetLambda( -999 );

  return base;
}

//_____________________________________________________________________
void TProjCluster::Print(Option_t *) const
{
    for( Int_t i = 0; i<fProjClusters->GetEntriesFast(); i++ )
    {
      printf("Cluster=%d Track=%d\n", i,((TProjClusterBase*)fProjClusters->At(i))->GetTrack());
    }
}
