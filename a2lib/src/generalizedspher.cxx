#include "generalizedspher.h"

void sphericity(std::vector<double> x, std::vector<double> y, std::vector<double> z, int r, TVector3** axis, TVector3** values){

  // only integer r implemented so far; most common values are 1 (linearized sphericity) and 2 (default, not infrared-safe);

  int ntrks = x.size();

  double momx, momy, momz, mag2, mag;

  double Sxx=0, Syy=0, Szz=0, Sxy=0, Sxz=0, Syz=0, norm=0;

  for (int i=0; i<ntrks; i++){

    // Sab = sum_i(p_i(a) . p_i(b) * |p_i|^{r-2}) / sum_i |p_i|^r

    // for r=0

    momx=x.at(i);
    momy=y.at(i);
    momz=z.at(i);
    mag2=momx*momx+momy*momy+momz*momz;
    mag=TMath::Sqrt(mag2);

    Double_t weight = 1;
    Double_t inorm = 1;

    for (int kk = 0; kk<r; kk++){
      weight*=mag;
    }

    Sxx += weight*momx*momx/mag2;
    Syy += weight*momy*momy/mag2;
    Szz += weight*momz*momz/mag2;
    Sxy += weight*momx*momy/mag2;
    Sxz += weight*momx*momz/mag2;
    Syz += weight*momy*momz/mag2;
    norm += weight;

  }

  //  cout << endl << endl << "S, eigenvalues, eigenvectors" << endl;

  //  cout << x.size() << " " << Sxx << " " << norm << endl;

  //  cout << " ------------------------------------- " << endl;

  if (TMath::IsNaN(norm)) exit(123);

  Sxx /= norm;
  Syy /= norm;
  Szz /= norm;
  Sxy /= norm;
  Sxz /= norm;
  Syz /= norm;

  TMatrixD S(3,3);

  S(0,0) = Sxx;
  S(0,1) = Sxy;
  S(1,0) = Sxy;

  S(1,1) = Syy;
  S(1,2) = Syz;
  S(2,1) = Syz;

  S(2,2) = Szz;
  S(0,2) = Sxz;
  S(2,0) = Sxz;


  TMatrixDEigen eS(S);


  eS.GetEigenVectors();

  //  S.Print();

  //  (eS.GetEigenValues()).Print();

  //  (eS.GetEigenVectors()).Print();



  if (TMath::IsNaN(Sxx)) exit(123);
  if (TMath::IsNaN(Syy)) exit(123);
  if (TMath::IsNaN(Szz)) exit(123);
  if (TMath::IsNaN(Sxy)) exit(123);
  if (TMath::IsNaN(Sxz)) exit(123);
  if (TMath::IsNaN(Syz)) exit(123);



  Double_t l[3];
  TVector3* v[3];

  TMatrixD eval(eS.GetEigenValues());
  TMatrixD evec(eS.GetEigenVectors());

  for (int i=0; i<3; i++){
    l[i] = eval(i,i);
    v[i]=new TVector3(evec(0,i),
                      evec(1,i),
                      evec(2,i));
  }


  int ind[3];

  TMath::Sort(3,l,ind,kTRUE);

  Double_t S_gen = 2.*(l[ind[1]]+l[ind[2]])/3.;

  Double_t A_gen = 2.*l[ind[2]]/3.;

  // well defined for linearized sphericity (r=1)

  Double_t C_gen = 3.*( l[ind[1]]*l[ind[2]] + l[ind[0]]*l[ind[2]] + l[ind[0]]*l[ind[1]] );

  Double_t D_gen = 27.*(l[ind[0]]*l[ind[1]]*l[ind[2]]);



  //  cout << " (l1 = " << l[ind[0]] << ") > (l2 =" << l[ind[1]] << ") > (l3 =" << l[ind[2]] << ") ; sum=" << l[ind[0]] + l[ind[1]] + l[ind[2]] << " ; S=" << sphericity <<  endl;


  //  cout << A_gen << " " << S_gen << " " << C_gen << " " << D_gen << endl;

  *values = new TVector3(l[ind[0]],l[ind[1]],l[ind[2]]);

  *axis = new TVector3(*v[ind[0]]);

  //  (*axis)->Print();

  delete v[0];
  delete v[1];
  delete v[2];
  //  delete[] v;

  return;

}




    /*
  cout << "l1 = " << eS.GetEigenValues()(0,0) << endl;
  cout << "l2 = " << eS.GetEigenValues()(1,1) << endl;
  cout << "l3 = " << eS.GetEigenValues()(2,2) << endl << endl;


  cout <<"     " << eS.GetEigenVectors()(0,0) << endl;
  cout <<"v1 = " << eS.GetEigenVectors()(1,0) << endl;
  cout <<"     " << eS.GetEigenVectors()(2,0) << endl << endl;


  cout <<"     " << eS.GetEigenVectors()(0,1) << endl;
  cout <<"v2 = " << eS.GetEigenVectors()(1,1) << endl;
  cout <<"     " << eS.GetEigenVectors()(2,1) << endl << endl;


  cout <<"     " << eS.GetEigenVectors()(0,2) << endl;
  cout <<"v3 = " << eS.GetEigenVectors()(1,2) << endl;
  cout <<"     " << eS.GetEigenVectors()(2,2) << endl << endl;

    */
