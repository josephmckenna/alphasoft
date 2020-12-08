#define GAUSNORM 0

Double_t BiGaus(Double_t *x, Double_t *k)
{
  Double_t t1 = (x[0]-k[1])/k[2], t2 = (x[0]-k[4])/k[5];
  Double_t f1,f2;
  
#if GAUSNORM
  f1 = k[0]*pow(TMath::TwoPi()*pow(k[2],2.),-0.5)*exp(-0.5*pow(t1,2.));
  f2 = k[3]*pow(TMath::TwoPi()*pow(k[5],2.),-0.5)*exp(-0.5*pow(t2,2.));
#else
  f1 = k[0]*exp(-0.5*pow( t1, 2.) );
  f2 = k[3]*exp(-0.5*pow( t2, 2.) );
#endif

  return f1+f2;
}

double FWHM(TF1* f)
{
  double half_maximum = 0.5*f->GetMaximum(),
    xAtMax = f->GetMaximumX();
  double xmin,xmax;
  f->GetRange(xmin,xmax);
  if(xmin>=xAtMax)
    {
      cerr<<"*** FWHM error in "<<f->GetName()
	  <<": xmin = "<<xmin
	  <<"> x@Max = "<<xAtMax<<endl;
      return -1.;
    }
  else if(xmax<=xAtMax)
    {
      cerr<<"*** FWHM error in "<<f->GetName()
	  <<": xmax = "<<xmax
	  <<"< x@Max = "<<xAtMax<<endl;
      return -1.;
    }
  double x1 = f->GetX(half_maximum,xmin,xAtMax),
    x2 = f->GetX(half_maximum,xAtMax,xmax);
  assert(x1<x2);
  return x2-x1;
}

double Resolution(TF1* f, double& res, double& err)
{
  res = TMath::Abs(f->GetParameter(2)); err = f->GetParError(2);
  double s2 = TMath::Abs(f->GetParameter(5)), s2err = f->GetParError(5);
  if( s2 < res )
    {
      res = s2;
      err = s2err;
    }
  return f->GetChisquare()/(double)f->GetNDF();
}


double WeightedResolution(TF1* f, double& res, double& err)
{
  double A1 = f->GetParameter(0), A1err = f->GetParError(0),
    A2 = f->GetParameter(3), A2err = f->GetParError(3),
    s1 = TMath::Abs(f->GetParameter(2)), s1err = f->GetParError(2), 
    s2 = TMath::Abs(f->GetParameter(5)), s2err = f->GetParError(5);

  double totw = A1+A2;
  res = (A1*s1+A2*s2) / totw;
  err = TMath::Sqrt(A1err*A1err*(s1-res)*(s1-res)/totw/totw + 
		    A2err*A2err*(s2-res)*(s2-res)/totw/totw + 
		    A1*A1*s1err*s1err + A2*A2*s2err*s2err);
  
  return f->GetChisquare()/(double)f->GetNDF();
}
