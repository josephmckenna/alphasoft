#include "TSplineGetters.h"



// convert the voltage ramp file to
// a usable function
TSpline5* InterpolateVoltageRamp(const char* filename)
{
  Double_t* t = new Double_t[100000];
  Double_t* V = new Double_t[100000];
  Int_t nPoints = LoadRampFile(filename,t,V);
  if(nPoints<=0) return 0;
  TString lname = "Voltage Ramp : ";
  lname += filename;
  lname +=";dt [a.u.];Voltage [V]";
  //for (int i = 0; i < nPoints; i++)
  //{
  //   std::cout<<"t: "<<t[i]<<"\tv:"<<V[i]<<std::endl;
  //}
  std::cout<<"nPoints: "<<nPoints<<std::endl;
  TSpline5* spline = new TSpline5(lname.Data(), t, V, nPoints);
  spline->SetLineColor(kBlue);
  delete[] t;
  delete[] V;
  return spline;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
