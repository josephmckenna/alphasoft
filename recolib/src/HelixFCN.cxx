#include"HelixFCN.hh"
#include<cassert>

HelixFCN::HelixFCN(TFitHelix* a_track) : track(*a_track), error_def(1) {
	points = *(track.GetPointsArray());
}

double HelixFCN::Up() const {
	return error_def;
}

RadFuncFCN::RadFuncFCN(TFitHelix* a_track) : HelixFCN(a_track) {
}

double RadFuncFCN::operator()(const std::vector<double>& p) const {
	assert(p.size() == 3);

	double chi2 = 0;

	double u0 = TMath::Cos(p[1]);
	double v0 = TMath::Sin(p[1]);

	for(const auto& apnt : points) {
		double r2 = apnt->GetR() * apnt->GetR();
		Vector2 f = track.Evaluate(r2, p[0], u0, v0, p[2]);

		double tx = (apnt->GetX() - f.X) / apnt->GetErrX();
		double ty = (apnt->GetY() - f.Y) / apnt->GetErrY();
		double d2 = tx * tx + ty * ty;

		chi2 += d2;
	}
	return chi2;
}

RadFunc_FCN::RadFunc_FCN(TFitHelix* a_track) : HelixFCN(a_track) {
}

double RadFunc_FCN::operator()(const std::vector<double>& p) const {
	assert(p.size() == 3);

	double chi2 = 0;

	double u0 = TMath::Cos(p[1]);
	double v0 = TMath::Sin(p[1]);

	for(const auto& apnt : points) {
		double r2 = apnt->GetR() * apnt->GetR();
		Vector2 f = track.Evaluate_(r2, p[0], u0, v0, p[2]);

		double tx = (apnt->GetX() - f.X) / apnt->GetErrX();
		double ty = (apnt->GetY() - f.Y) / apnt->GetErrY();
		double d2 = tx * tx + ty * ty;

		chi2 += d2;
	}
	return chi2;
}

ZedFuncFCN::ZedFuncFCN(TFitHelix* a_track) : HelixFCN(a_track) {
}

double ZedFuncFCN::operator()(const std::vector<double>& p) const {
	assert(p.size() == 2);

	double chi2 = 0;

	for(const auto& apnt : points) {
		double r2 = apnt->GetR() * apnt->GetR();
		double s = track.GetArcLength(r2);
		double f = track.Evaluate(s, p[0], p[1]);

		double tz = (apnt->GetZ() - f) / apnt->GetErrZ();
		double d2 = tz * tz;

		chi2 += d2;
	}
	return chi2;
}
