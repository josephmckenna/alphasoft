#include"LineFCN.hh"
#include<cassert>

LineFCN::LineFCN(TFitLine* a_track) : track(*a_track), error_def(1) {
	points = *(track.GetPointsArray());
}

double LineFCN::Up() const {
	return error_def;
}

double LineFCN::operator()(const std::vector<double>& p) const {
	assert(p.size() == 6);	
	//Just do exactly what Andrea's Minuit1 does for chi2
	//This is a copy-paste of FitFunc(...) in TFitLine.cxx
	double chi2 = 0;

	for(const auto& apnt : points) {
		double r2 = apnt->GetR() * apnt->GetR();
		TVector3 f = track.Evaluate(r2, p[0], p[1], p[2], p[3], p[4], p[5]);
		double tx = (apnt->GetX() - f.X()) / apnt->GetErrX(); 
		double ty = (apnt->GetY() - f.Y()) / apnt->GetErrY();
		double tz = (apnt->GetZ() - f.Z()) / apnt->GetErrZ();
		double d2 = tx*tx + ty*ty + tz*tz;
		chi2 += d2;
	}
	return chi2;
}
