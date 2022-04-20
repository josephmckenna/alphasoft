/*
 * Minuit2 minimization. Class derived from FCNBase. 
 * The current helix fitting minimizes multiple things instead of a single helix
 *
 * Hence there is a parent abstract HelixFCN, and a child (which overloads the
 * operator() with the appropriate function) for each thing that is currently
 * minimized.
 */

#ifndef HELIXFCN_HH
#define HELIXFCN_HH

#include"Minuit2/FCNBase.h"
#include"TFitHelix.hh"
#include"TSpacePoint.hh"
#include<vector>

class HelixFCN : public ROOT::Minuit2::FCNBase {
	protected:
		TFitHelix* track;
		const std::vector<TSpacePoint>* points;
		double error_def;
	public:
		HelixFCN(TFitHelix* a_track);

		double Up() const;		
};

class RadFuncFCN : public HelixFCN {
	public:
		RadFuncFCN(TFitHelix* a_track);

		double operator()(const std::vector<double>& params) const;
};

class RadFunc_FCN : public HelixFCN {
	public:
		RadFunc_FCN(TFitHelix* a_track);

		double operator()(const std::vector<double>& params) const;
};

class ZedFuncFCN : public HelixFCN {
	public:
		ZedFuncFCN(TFitHelix* a_track);

		double operator()(const std::vector<double>& params) const;
};
#endif
