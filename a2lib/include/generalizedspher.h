
#include "TMath.h"
#include <vector>
#include "TVector3.h"
#include "TMatrixD.h"
#include "TMatrixDEigen.h"

void sphericity(std::vector<double> x, std::vector<double> y, std::vector<double> z, int r, TVector3** axis, TVector3** values);
