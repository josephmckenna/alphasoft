/*
 * Minuit2 minimization. Class derived from FCNBase. Operator() calculates the
 * value to be minimized.
 */

#ifndef LINEFCN_HH
#define LINEFCN_HH

#include"Minuit2/FCNBase.h"
#include"TFitLine.hh"
#include"TSpacePoint.hh"
#include<vector>

class LineFCN : public ROOT::Minuit2::FCNBase {
	private:
		TFitLine* track;
		const std::vector<TSpacePoint>* points; 
		double error_def;
	public:
		LineFCN(TFitLine* a_track);

		double Up() const;
		double operator()(const std::vector<double>& params) const;
};

#endif
