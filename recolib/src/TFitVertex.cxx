// Vertex class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: May 2014

#include "TFitVertex.hh"

#include <iostream>
#include <iomanip>
#include <string>

#include <TMath.h>
#include <TMinuit.h>

#include "TPCconstants.hh"

#define BETA 0

static TMinuit* mindist=0;
void MinDistFunc(int&, double*, double& chi2, double* p, int)
{
  TFitVertex* fitObj = (TFitVertex*) mindist->GetObjectFit();
  TFitHelix* hel0 = fitObj->GetInit0();
  TFitHelix* hel1 = fitObj->GetInit1();
  TVector3 H0     = hel0->GetPosition(p[0]);
  //  TVector3 H0err2 = hel0->EvaluateErrors2(H0.Perp2());
  TVector3 H0err2 = hel0->GetError2(p[0]);
  TVector3 H1     = hel1->GetPosition(p[1]);
  //  TVector3 H1err2 = hel1->EvaluateErrors2(H1.Perp2());
  TVector3 H1err2 = hel1->GetError2(p[1]);
  double tx=H0.X()-H1.X(), ty=H0.Y()-H1.Y(), tz=H0.Z()-H1.Z();
  chi2 = tx*tx/(H0err2.X()+H1err2.X())
       + ty*ty/(H0err2.Y()+H1err2.Y())
       + tz*tz/(H0err2.Z()+H1err2.Z());
  return;
}

static TMinuit* hel2vtx=0;
void Hel2VtxFunc(int&, double*, double& chi2, double* p, int)
{
  TFitVertex* fitObj = (TFitVertex*) hel2vtx->GetObjectFit();
  const TObjArray* hellColl = fitObj->GetHelixStack();
  chi2=0.;
  double tx,ty,tz,s;
  int helpoints=hellColl->GetEntriesFast();
  for(int i=0; i<helpoints; ++i)
    {
      s=p[i+3];
      TVector3 h = ( (TFitHelix*) hellColl->At(i) )->GetPosition(s);
      //      TVector3 e2 = ( (TFitHelix*) hellColl->At(i) )->EvaluateErrors2(h.Perp2());
      TVector3 e2 = ( (TFitHelix*) hellColl->At(i) )->GetError2(s);
      tx=p[0]-h.X(); ty=p[1]-h.Y(); tz=p[2]-h.Z();
      chi2 += tx*tx/e2.X() + ty*ty/e2.Y() + tz*tz/e2.Z() ;
    }
  return;
}

TFitVertex::TFitVertex(int i):fID(i),fHelixArray(0),fNhelices(0),
			      fchi2(-1.),
			      fNumberOfUsedHelices(0),fHelixStack(0),
			      fChi2Cut(17.),
			      fInit0(0),fSeed0Index(-1),fSeed0Par(0.),
			      fInit1(0),fSeed1Index(-1),fSeed1Par(0.),
			      fSeedchi2(9999999.),
			      fNewChi2(-1),
			      fNewSeed0Par(0.),fNewSeed1Par(0.)
{ 
  fVertex.SetXYZ(-999.,-999.,-999.);
  fVertexError2.SetXYZ(-999.,-999.,-999.);  
  fNewVertex.SetXYZ(-999.,-999.,-999.);
  fNewVertexError2.SetXYZ(-999.,-999.,-999.);  
  fMeanVertex.SetXYZ(-999.,-999.,-999.);
  fMeanVertexError2.SetXYZ(-999.,-999.,-999.);

  //  fPoint = new TPolyMarker3D;
}

void TFitVertex::Clear(Option_t*)
{ 
  fHelixArray.Clear();
  fHelixStack.Clear();
}


TFitVertex::~TFitVertex()
{ 
  fHelixArray.Clear();
  fHelixStack.Clear();
  //  fHelixArray.Delete();
  // fHelixStack.Delete();
  //  if(fPoint) delete fPoint;
}

int TFitVertex::AddHelix(TFitHelix* anHelix)
{
  // MS error added in quadrature
  anHelix->AddMSerror();

  fHelixArray.AddLast(anHelix);
  ++fNhelices;
  return fNhelices;
}

// main function for vertexing
int TFitVertex::Calculate()
{
  if(fNhelices<2) return 0;

  // sort the helices by |c|, so that lowest |c| (highest momentum) is first
  fHelixArray.Sort();

  // ============= FIRST PASS =============
  // find the minimum distance 
  // between a pair of helices
  fchi2 = FindSeed(); 
  // ------------- debug -----------------
  // std::cout<<"1st pass chi^2: "<<fchi2<<std::endl;

  if(fSeed0Index<0||fSeed1Index<0) return -1;
  fNumberOfUsedHelices=2;
  // the SeedVertex is the mean point
  // on the segment joining the minimum-distance-pair
  fVertex = EvaluateMeanPoint();
  fVertexError2 = EvaluateMeanPointError2();

  // ------------- debug -----------------
  // std::cout<<"Seed Vertex"<<std::endl;
  // Print();
  // std::cout<<"----------------"<<std::endl;
  
  // ============= SECOND PASS =============
  // minimize the distance of the minimum-distance-pair
  // to a point, called NewVertex
  fHelixStack.AddLast((TFitHelix*) fHelixArray.At( fSeed0Index ));
  fHelixStack.AddLast((TFitHelix*) fHelixArray.At( fSeed1Index ));

  fchi2=Recalculate();

  // // ------------- debug -----------------
  // std::cout<<"Recalc Vertex"<<std::endl;
  // Print();
  //  if(fVertex!=fNewVertex) std::cout<<"Recalc Error"<<std::endl;
  // std::cout<<"----------------"<<std::endl;

  int val=1;
  // ============= THIRD PASS =============
  // if there are more than 2 helices, improve
  // vertex by finding a new one with more helices
  if(fNhelices>2) val=Improve();

  // // ------------- debug -----------------
  // std::cout<<" Number Of Used Helices = "<<fNumberOfUsedHelices
  // 	   <<"\t Helix Stack Entries  = "<<fHelixStack.GetEntriesFast()<<std::endl;
  if(fNumberOfUsedHelices!=fHelixStack.GetEntriesFast()) 
    std::cerr<<"Improve Error"<<std::endl;

  // notify the helix in the stack whether it has been used for 
  // the seed (2)
  // the improvement (3)
  // ( all the helices outside the stack have status 1 )
  AssignHelixStatus();
  fHelixStack.Compress();

  // return code is
  //  0: only one good helix
  // -1: failed to find minimum-distance-pair
  //  1: only two good helices
  //  2: didn't improve vertex (more than 2 helices)
  //  3: vertex improved
  return val;
}

double TFitVertex::FindSeed(double trapradius2)
{
  double s0,s1, // arclength parameters
    chi2; // normalized chi^2
  for(int n=0; n<fNhelices; ++n)
    {
      fInit0=(TFitHelix*) fHelixArray.At(n);
#if BETA>0
      s0=fInit0->GetArcLengthB(trapradius2);
#else
      s0=fInit0->GetArcLength(trapradius2);
#endif
      for(int m=n+1; m<fNhelices; ++m)
	{
	  fInit1=(TFitHelix*) fHelixArray.At(m);
#if BETA>0
	  s1=fInit1->GetArcLengthB(trapradius2);
#else
	  s1=fInit1->GetArcLength(trapradius2);
#endif
	  chi2=FindMinDistance(s0,s1); 
	  if( chi2 < fSeedchi2 )
	    {
	      fSeedchi2=chi2;
	      fSeed0Index=n; fSeed0Par=s0;
	      fSeed1Index=m; fSeed1Par=s1;
	    }
	}
    }
   return fSeedchi2;
}

double TFitVertex::FindMinDistance(double& s0, double& s1)
{
  static double step = 0.01;

  mindist = new TMinuit(2);
  mindist->SetObjectFit(this);
  mindist->SetFCN(MinDistFunc);

  double arglist[10];
  int ierflg = 0;

  mindist->SetPrintLevel(-1);

  arglist[0] = 1.0;
  mindist->mnexcm("SET ERR", arglist , 1, ierflg);
  
  mindist->mnparm(0, "s0", s0, step, 0,0,ierflg);
  mindist->mnparm(1, "s1", s1, step, 0,0,ierflg);

  arglist[0] = 6.0;
  mindist->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500.0;
  arglist[1] = 0.1;
  mindist->mnexcm("MIGRAD", arglist, 2, ierflg);

  double chi2,nused0,nused1;
  int npar, stat;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  mindist->mnstat(chi2,nused0,nused1,npar,npar,stat);

  double es0,es1;
  mindist->GetParameter(0,s0,es0);
  mindist->GetParameter(1,s1,es1);

  delete mindist;	  
  // degrees of freedom is ndf=3H-2
  // for H=2, ndf=4
  return 0.25*chi2;
}

TVector3 TFitVertex::EvaluateMeanPoint()
{
  return EvaluateMeanPoint( ((TFitHelix*) fHelixArray.At(fSeed0Index) )->GetPosition(fSeed0Par),
			    ((TFitHelix*) fHelixArray.At(fSeed0Index) )->GetError2(fSeed0Par),
			    ((TFitHelix*) fHelixArray.At(fSeed1Index) )->GetPosition(fSeed1Par),
			    ((TFitHelix*) fHelixArray.At(fSeed1Index) )->GetError2(fSeed1Par)
			  );
}

TVector3 TFitVertex::EvaluateMeanPoint(TVector3 p0, TVector3 e0, 
				       TVector3 p1, TVector3 e1)
{
  TVector3 zero(0.,0.,0.);
  if(e0!=zero && e1!=zero)
    {
      double px=p0.X()/e0.X() + p1.X()/e1.X(),
	py=p0.Y()/e0.Y() + p1.Y()/e1.Y(),
	pz=p0.Z()/e0.Z() + p1.Z()/e1.Z();
      fMeanVertex.SetXYZ(px/(1./e0.X() + 1./e1.X()),
			 py/(1./e0.Y() + 1./e1.Y()),
			 pz/(1./e0.Z() + 1./e1.Z()));
    }
  else fMeanVertex = 0.5*(p0+p1);
  return fMeanVertex;
}

TVector3 TFitVertex::EvaluateMeanPointError2()
{
  TVector3 e0 = ((TFitHelix*) fHelixArray.At(fSeed0Index) )->GetError2(fSeed0Par);
  TVector3 e1 = ((TFitHelix*) fHelixArray.At(fSeed1Index) )->GetError2(fSeed1Par);
  fMeanVertexError2 = (e0+e1)*0.25;
  return fMeanVertexError2;
}

double TFitVertex::Recalculate()
{
  static double step = 0.01;

  hel2vtx = new TMinuit(5);
  hel2vtx->SetObjectFit(this);
  hel2vtx->SetFCN(Hel2VtxFunc);

  double arglist[10];
  int ierflg = 0;

  hel2vtx->SetPrintLevel(-1);

  arglist[0] = 1.0;
  hel2vtx->mnexcm("SET ERR", arglist , 1, ierflg);

  hel2vtx->mnparm(0, "vx", fVertex.X(), step, 0,0,ierflg);
  hel2vtx->mnparm(1, "vy", fVertex.Y(), step, 0,0,ierflg);
  hel2vtx->mnparm(2, "vz", fVertex.Z(), step, 0,0,ierflg);
  hel2vtx->mnparm(3, "s0", fSeed0Par, step, 0,0,ierflg);
  hel2vtx->mnparm(4, "s1", fSeed1Par, step, 0,0,ierflg);
  
  arglist[0] = 6.0;
  hel2vtx->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500.0;
  arglist[1] = 0.1;
  hel2vtx->mnexcm("MIGRAD", arglist, 2, ierflg);

  double chi2,nused0,nused1;
  int npar, stat;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  hel2vtx->mnstat(chi2,nused0,nused1,npar,npar,stat);

  double vx,vy,vz,ex,ey,ez;
  hel2vtx->GetParameter(0,vx,ex);
  hel2vtx->GetParameter(1,vy,ey);
  hel2vtx->GetParameter(2,vz,ez);
  fVertex.SetXYZ(vx,vy,vz);
  fVertexError2.SetXYZ(ex*ex,ey*ey,ez*ez);
  double es0,es1;
  hel2vtx->GetParameter(3,fNewSeed0Par,es0);
  hel2vtx->GetParameter(4,fNewSeed1Par,es1);

  delete hel2vtx;

  // store the NewVertex and the NewChi2
  fNewChi2=chi2;
  fNewVertex.SetXYZ(vx,vy,vz);
  fNewVertexError2.SetXYZ(ex*ex,ey*ey,ez*ez);

  // degrees of freedom is ndf=3H-5
  // for H=2, ndf=1
  return fNewChi2; 
}

int TFitVertex::Improve()
{
  double chi2;// normalized chi^2
  int last, npar=3+fNhelices;
  double* ipar = new double[npar];
  ipar[0]=fVertex.X(); ipar[1]=fVertex.Y(); ipar[2]=fVertex.Z();
  ipar[3]=fNewSeed0Par; ipar[4]=fNewSeed1Par;
  for(int i=5; i<npar; ++i) ipar[i]=0.0;
  double* iparerr = new double[npar];
  double* spar = new double[npar];
  double* sparerr = new double[npar];
  for(int i=0; i<npar; ++i) 
    {
      iparerr[i]=sparerr[i]=0.0;
      spar[i]=ipar[i];
    }
  int hels=fHelixArray.GetEntriesFast();
  for(int n=0; n<hels; ++n)
    {
      if(n==fSeed0Index || n==fSeed1Index) continue;
      fHelixStack.AddLast((TFitHelix*) fHelixArray.At(n));
      last=fHelixStack.GetEntriesFast()-1;

#if BETA>0
      ipar[last+3]=((TFitHelix*) fHelixStack.At(last))->GetArcLengthB(GetRadius()*GetRadius());
#else
      ipar[last+3]=((TFitHelix*) fHelixStack.At(last))->GetArcLength(GetRadius()*GetRadius());
#endif

      chi2=FindNewVertex(ipar,iparerr);
      // alternate cut based on delta chi2
      //if(((chi2 - fNewChi2) <= 0.4) &&
      //   ((chi2 - fNewChi2) >= 0.)){
      if(chi2 < fChi2Cut){

	fchi2=chi2;
	for(int i=0; i<npar; ++i){
	  spar[i]=ipar[i];
	  sparerr[i]=iparerr[i];
	}
	++fNumberOfUsedHelices;
      } else {
	fHelixStack.RemoveAt(last);
	for(int i=0; i<npar; ++i){
	  ipar[i]=spar[i];
	  iparerr[i]=sparerr[i];
	}
	fHelixStack.Compress();
      }
    }
  
  fVertex.SetXYZ(spar[0],spar[1],spar[2]);
  fVertexError2.SetXYZ(sparerr[0]*sparerr[0],sparerr[1]*sparerr[1],sparerr[2]*sparerr[2]);

  delete[] ipar;  delete[] iparerr;  
  delete[] spar;  delete[] sparerr;

  if(fNumberOfUsedHelices>2) return 3;
  else return 2;
}

double TFitVertex::FindNewVertex(double* ipar, double* iparerr)
{
  static double step = 0.01;

  int mpar=3+fHelixStack.GetEntriesFast();
  hel2vtx = new TMinuit(mpar);
  hel2vtx->SetObjectFit(this);
  hel2vtx->SetFCN(Hel2VtxFunc);

  double arglist[10];
  int ierflg = 0;

  hel2vtx->SetPrintLevel(-1);

  arglist[0] = 1.0;
  hel2vtx->mnexcm("SET ERR", arglist , 1, ierflg);

  char parname[12];
  for(int i=0; i<mpar; ++i)
    {
      sprintf(parname,"p%d",i);
      hel2vtx->mnparm(i, parname, ipar[i], step, 0,0,ierflg);
    }

  arglist[0] = 6.0;
  hel2vtx->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500.0;
  arglist[1] = 0.1;
  hel2vtx->mnexcm("MIGRAD", arglist, 2, ierflg);

  double chi2,nused0,nused1;
  int npar, stat;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  hel2vtx->mnstat(chi2,nused0,nused1,npar,npar,stat);

  for(int i=0; i<mpar; ++i)
    hel2vtx->GetParameter(i,ipar[i],iparerr[i]);

  delete hel2vtx;

  double ndf = 3.*(double) fHelixStack.GetEntriesFast() - (double) npar;
  return chi2/ndf; 
}

void TFitVertex::AssignHelixStatus()
{
  int nhelstack=fHelixStack.GetEntriesFast();
  for(int h=0; h<nhelstack; ++h)
    {
      if(h==0 || h==1) 
	( (TFitHelix*) fHelixStack.At(h) )->SetStatus(2);
      else
	( (TFitHelix*) fHelixStack.At(h) )->SetStatus(3);
    }
}

bool TFitVertex::InRadiusRange(double r)
{
  return r<(2.*_trapradius)?true:false;
}

int TFitVertex::FindDCA()
{
  if(fNhelices<2) return 0;

  FindSeed(_trapradius*_trapradius); 
  // ------------- debug -----------------
  std::cout<<"TFitVertex::FindDCA() "<<fchi2<<std::endl;

  if(fSeed0Index<0||fSeed1Index<0) return -1;
  fNumberOfUsedHelices=2;

  fHelixStack.AddLast((TFitHelix*) fHelixArray.At( fSeed0Index ));
  fHelixStack.AddLast((TFitHelix*) fHelixArray.At( fSeed1Index ));

  // the DCA is calculated using the existing variables since I'm re-using this 
  // vertex class
  fMeanVertex = ( ((TFitHelix*) fHelixArray.At(fSeed0Index) )->GetPosition(fSeed0Par) ) -
    ( ((TFitHelix*) fHelixArray.At(fSeed1Index) )->GetPosition(fSeed1Par) );
  // the DCA is assigned to a chi^2 variable
  fNewChi2 = fMeanVertex.Mag();

  return 1;
}

void TFitVertex::Print(Option_t* opt) const
{
  std::cout<<"TFitVertex:: # of Used Helices: "<<fNumberOfUsedHelices<<", ";
  if( !strcmp(opt,"rphi") )
    {
      std::cout<<"(r,phi,z) = ("
	       <<std::setw(5)<<std::left<<fVertex.Perp()<<", "
	       <<std::setw(5)<<std::left<<fVertex.Phi()<<", "
	       <<std::setw(5)<<std::left<<fVertex.Z()<<"), ";
    }
  else if( !strcmp(opt,"xy") )
  {
      std::cout<<"(x,y,z) = ("
	       <<std::setw(5)<<std::left<<fVertex.X()<<", "
	       <<std::setw(5)<<std::left<<fVertex.Y()<<", "
	       <<std::setw(5)<<std::left<<fVertex.Z()<<"), ";
    }
  else std::cout<<"Unknown coordinate system, "<<opt<<std::endl;
  std::cout<<"Normalized chi^2 = "<<fchi2<<std::endl;
}

void TFitVertex::Reset()
{
  fVertex.SetXYZ(-999.,-999.,-999.);
  fVertexError2.SetXYZ(-999.,-999.,-999.);
  //  fHelixArray.Delete();
  fHelixArray.Clear();
  fID=-1;
  fNhelices=-1;
  fchi2=-999.;
  //  fPoint = 0;
  fInit0=0;
  fInit1=0;
  fSeed0Index=-1;
  fSeed1Index=-1;
  fMeanVertex.SetXYZ(-999.,-999.,-999.);
  fMeanVertexError2.SetXYZ(-999.,-999.,-999.);
  fNumberOfUsedHelices=-1;
  //  fHelixStack.Delete();
  fHelixStack.Clear();
  fNewChi2=-999.;
  fNewVertex.SetXYZ(-999.,-999.,-999.);
  fNewVertexError2.SetXYZ(-999.,-999.,-999.);
  fNewSeed0Par=-999.;
  fNewSeed1Par=-999.;
  fChi2Cut=0.;
}

ClassImp(TFitVertex)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
