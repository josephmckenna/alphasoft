/*
 * Minuit2 minimization. Class derived from FCNBase. 
 * The current helix fitting minimizes multiple things instead of a single helix
 *
 * Hence there is a parent abstract Hel2VtxFCN, and a child (which overloads the
 * operator() with the appropriate function) for each thing that is currently
 * minimized.
 */

#ifndef VERTEXFCN_HH
#define VERTEXFCN_HH

#include"Minuit2/FCNBase.h"
#include"TFitVertex.hh"
//#include"TSpacePoint.hh"
#include<vector>

class Hel2VtxFCN : public ROOT::Minuit2::FCNBase {
	protected:
		TFitVertex vtx;
		const TObjArray* hellColl;
		double error_def;
	public:
		Hel2VtxFCN(TFitVertex* a_vtx);

		double Up() const;		
};

class VertFuncFCN : public Hel2VtxFCN {
	public:
		VertFuncFCN(TFitVertex* a_vtx);

		double operator()(const std::vector<double>& params) const;
};


class MinDistFunctionFCN : public ROOT::Minuit2::FCNBase {
	protected:
		TFitVertex vtx;
		TFitHelix* hel0;
        TFitHelix* hel1;
		double error_def;
	public:
		MinDistFunctionFCN(TFitVertex* a_vtx);

		double Up() const;		
};

class MinDistFCN : public  MinDistFunctionFCN{
	public:
		MinDistFCN(TFitVertex* a_vtx);

		double operator()(const std::vector<double>& params) const;
};

#endif
