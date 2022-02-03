#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"

#include "fitBV.hh"
#include <TError.h>

double SkewGaussFcn::operator()(const std::vector<double>& par) const
{
   SkewGaussFunction skewGauss(par);

   double chi2 = 0.;
   for(unsigned int n = 0; n < theMeasurements.size(); n++) {
      chi2 += ((skewGauss(n) - theMeasurements[n]) *
      (skewGauss(n) - theMeasurements[n]) *
      theWeights[n]);
   }
   return chi2;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
