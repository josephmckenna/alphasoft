
#include "TMath.h"
#include <vector>
#include "TVector3.h"
#include "TMatrixD.h"
#include "TMatrixDEigen.h"
using namespace std;

void sphericity(vector<double> x, vector<double> y, vector<double> z, int r, TVector3** axis, TVector3** values);
