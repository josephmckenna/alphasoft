//TAlphaEventVertex
#include <TMinuit.h>

#include "TAlphaEvent.h"
#include "TAlphaEventVertex.h"

ClassImp(TAlphaEventVertex)

void fcnDCA(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
void fcnMeanDCA(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
void fcnDCAToVertex(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);


//_____________________________________________________________________
TAlphaEventVertex::TAlphaEventVertex()
{
//ctor
  Clear();
}

//_____________________________________________________________________
TAlphaEventVertex::~TAlphaEventVertex()
{
//dtor
  Clear();
}

//_____________________________________________________________________
void TAlphaEventVertex::Clear(Option_t * /*option*/)
{
  fX = 0.0;
  fY = 0.0;
  fZ = 0.0;
  fDCA = 0.0;
  fIsGood = kFALSE;
  
  fDCAs.SetOwner(kTRUE);
  fDCAa.SetOwner(kTRUE);
  fDCAb.SetOwner(kTRUE);
  //fHelices.SetOwner(kTRUE);
  fHelices.Clear();
  
  fDCAs.Delete();
  fDCAa.Delete();
  fDCAb.Delete();
}

//_____________________________________________________________________
void TAlphaEventVertex::RecVertex()
{
  const Int_t NHelices = fHelices.GetEntries();
  if(NHelices < 2)
    {
      return;
    }

  for(Int_t hi=0; hi<NHelices; hi++)
    for(Int_t hj=hi+1; hj<NHelices; hj++)
      {
	TAlphaEventHelix * ha = GetHelix(hi);
	TAlphaEventHelix * hb = GetHelix(hj);

	fhi = hi;
	fhj = hj;

	TVector3 *dca = FindDCA(ha,hb);
	AddDCA( dca );
      }

  const Int_t NDCAs = fDCAs.GetEntries();
  if( NDCAs == 0 ) return;

  for(Int_t idca=0; idca<NDCAs; idca++)
    {
      TVector3 *dca = GetDCA(idca);
      fX += dca->X();
      fY += dca->Y();
      fZ += dca->Z();
    }
  fX /= NDCAs;
  fY /= NDCAs;
  fZ /= NDCAs;

  if(NHelices>2)
    fDCA = MinimizeVertexMeanDCA();
  else
    fDCA = CalculateVertexMeanDCA( fX,fY,fZ );

  if( fDCA>0 )
    fIsGood = kTRUE;
  else
    fIsGood = kFALSE;
}

static TMinuit *minimdca = 0;

//_____________________________________________________________________
Double_t TAlphaEventVertex::MinimizeVertexMeanDCA()
{

  minimdca = new TMinuit(3);
  minimdca->SetPrintLevel(-1);

  minimdca->SetFCN(fcnMeanDCA);
  minimdca->SetObjectFit(this);

  minimdca->DefineParameter(0, "fX", fX, 0.1, -10, 10  );
  minimdca->DefineParameter(1, "fY", fY, 0.1, -10, 10  );
  minimdca->DefineParameter(2, "fZ", fZ, 0.1, -50, 50 );


  // This function shouldn't take that many iterations to converge,
  // but whateves.
  //AO  minimdca->SetMaxIterations(400);
  minimdca->SetMaxIterations(10);
  // Perform the function minimization
  minimdca->Migrad();

  Double_t minx; Double_t errx;
  Double_t miny; Double_t erry;
  Double_t minz; Double_t errz;

  minimdca->GetParameter(0,minx,errx);
  minimdca->GetParameter(1,miny,erry);
  minimdca->GetParameter(2,minz,errz);

  fX = minx;
  fY = miny;
  fZ = minz;

  delete minimdca;

  return CalculateVertexMeanDCA(fX,fY,fZ);
}

//_____________________________________________________________________
Double_t TAlphaEventVertex::CalculateVertexMeanDCA( Double_t vx,
						    Double_t vy,
						    Double_t vz )
{
  Double_t MeanDCA = 0;
  for(Int_t hi=0; hi<GetNHelices(); hi++)
    {
      TAlphaEventHelix *helix = GetHelix(hi);

      fhi = hi;

      TVector3 *dca = FindDCAToVertex( helix );

      Double_t d = TMath::Sqrt( (dca->X()-vx)*(dca->X()-vx) +
   				(dca->Y()-vy)*(dca->Y()-vy) +
   				(dca->Z()-vz)*(dca->Z()-vz));
      MeanDCA+=d;
      delete dca;
    }
  MeanDCA /= GetNHelices();
  return MeanDCA;
}

//_____________________________________________________________________
void fcnMeanDCA(Int_t &/*npar*/, Double_t * /*gin*/ , Double_t &f, Double_t *par, Int_t /*iflag*/ )
{
  // par[0] = x
  // par[1] = y
  // par[1] = z
  TAlphaEventVertex * vertex = (TAlphaEventVertex*)minimdca->GetObjectFit();
  f = vertex->CalculateVertexMeanDCA(par[0],par[1],par[2]);
  //printf("x: %lf y: %lf z: %lf, f: %lf\n",par[0],par[1],par[2],f);
}

static TMinuit * mini = NULL;

//_____________________________________________________________________
TVector3 *TAlphaEventVertex::FindDCAToVertex( TAlphaEventHelix *helix )
{
  // here
  TVector3 * dca = new TVector3();

  mini = new TMinuit(1);
  mini->SetPrintLevel(-1);

  mini->SetFCN(fcnDCAToVertex);
  mini->SetObjectFit(this);

  mini->DefineParameter(0, "s", 0, 0.1, -100, 100 );


  // This function shouldn't take that many iterations to converge,
  // but whateves.
  //AO  mini->SetMaxIterations(400);
  mini->SetMaxIterations(10);
  // Perform the function minimization
  mini->Migrad();

  Double_t s;
  Double_t errs;

  // Grab the results
  mini->GetParameter(0,s,errs);

  TVector3 h = helix->GetPoint3D_C(s);
  dca->SetXYZ( h.X(), h.Y(), h.Z() );

  delete mini;
  return dca;
}

static TMinuit * minidca = 0;

//_____________________________________________________________________
TVector3 *TAlphaEventVertex::FindDCA( TAlphaEventHelix * ha, TAlphaEventHelix * hb)
{
  TVector3 * dca = new TVector3();

  minidca = new TMinuit(2);
   minidca->SetPrintLevel(-1);
  minidca->SetFCN(fcnDCA);
  minidca->SetObjectFit(this);

  minidca->DefineParameter(0, "s_a", 0, 0.1, -100, 100 );
  minidca->DefineParameter(1, "s_b", 0, 0.1, -100, 100 );


  // This function shouldn't take that many iterations to converge,
  // but whatever.
  minidca->SetMaxIterations(400);

  // Perform the function minimization
  minidca->Migrad();

  Double_t s_a;
  Double_t s_b;
  Double_t errs_a;
  Double_t errs_b;

  // Grab the results
  minidca->GetParameter(0,s_a,errs_a);
  minidca->GetParameter(1,s_b,errs_b);

  TVector3 vha = ha->GetPoint3D_C(s_a);
  TVector3 vhb = hb->GetPoint3D_C(s_b);

  TVector3 *va = new TVector3(vha.X(),vha.Y(),vha.Z());
  TVector3 *vb = new TVector3(vhb.X(),vhb.Y(),vhb.Z());

  // add the distances of closest approaches between helices
  // for plotting
  // AddDCAa(va);
  // AddDCAb(vb);

  // calculate the midpoint between the helices
  dca->SetXYZ( 0.5*(vha.X()+vhb.X()),
	       0.5*(vha.Y()+vhb.Y()),
	       0.5*(vha.Z()+vhb.Z()));
  delete va;
  delete vb;
  delete minidca;

  return dca;
}

//_____________________________________________________________________
void fcnDCA(Int_t &/*npar*/, Double_t * /*gin*/ , Double_t &f, Double_t *par, Int_t /*iflag*/ )
{
  // par[0] = s_a
  // par[1] = s_b
  Double_t d2 = 999999;

  TAlphaEventVertex * vertex = (TAlphaEventVertex*)minidca->GetObjectFit();

  TAlphaEventHelix *hi = vertex->GetHelix(vertex->Getfhi());
  TAlphaEventHelix *hj = vertex->GetHelix(vertex->Getfhj());

  TVector3 vhi = hi->GetPoint3D_C(par[0]);
  TVector3 vhj = hj->GetPoint3D_C(par[1]);

  d2 = (vhi.X()-vhj.X())*(vhi.X()-vhj.X())
    + (vhi.Y()-vhj.Y())*(vhi.Y()-vhj.Y())
    + (vhi.Z()-vhj.Z())*(vhi.Z()-vhj.Z());

  f = d2;
}

//_____________________________________________________________________
void fcnDCAToVertex(Int_t &/*npar*/, Double_t * /*gin*/ , Double_t &f, Double_t *par, Int_t /*iflag*/ )
{
  // par[0] = s
  Double_t d2 = 999999;

  // here
  TAlphaEventVertex * vertex = (TAlphaEventVertex*)mini->GetObjectFit();

  TAlphaEventHelix *hi = vertex->GetHelix(vertex->Getfhi());

  TVector3 vhi = hi->GetPoint3D_C(par[0]);

  d2 = (vhi.X()-vertex->X())*(vhi.X()-vertex->X()) +
    (vhi.Y()-vertex->Y())*(vhi.Y()-vertex->Y()) +
    (vhi.Z()-vertex->Z())*(vhi.Z()-vertex->Z());

  f = d2;
}


//_____________________________________________________________________
void TAlphaEventVertex::Print(const Option_t* /* option */) const
{
  printf("\n-------- TAlphaEventVertex --------\n");
  printf("Number of helices: %d\n",fHelices.GetEntries());

  Int_t iUsed = 0;
  for( Int_t i = 0; i < fHelices.GetEntries(); i++ )
   {
     //     if( ((TAlphaEventHelix*)fHelices.At( i ))->IsIncludable() )
        iUsed++;
   }
  printf("Numbed of helices used: %d\n",iUsed);

  printf("X: %lf Y: %lf Z: %lf\n",fX,fY,fZ);
  if( fIsGood == kTRUE )
   printf(" IS good\n");
  else
   printf("NOT good\n");
  printf("-------------------------------------\n\n");
}



//end
